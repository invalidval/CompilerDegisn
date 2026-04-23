#!/usr/bin/env python3
"""Optimized Terminal UI wrapper for pascc compilation progress.

This UI is opt-in and does not affect normal pascc CLI usage.
"""

from __future__ import annotations

import argparse
import time
import curses
import json
import os
import shlex
import subprocess
import sys
import threading
from collections import deque
from dataclasses import dataclass
from queue import Empty, Queue
from typing import Deque, Dict, List, Optional

EVENT_PREFIX = "[PASCC_EVT]"
LOG_PREFIX = "[PASCC_LOG]"
STAGE_ORDER = ["init", "lexer", "parser", "semantic", "codegen", "output", "gcc_verify", "compiler"]
STAGE_TITLES = {
    "init": "初始化",
    "lexer": "词法分析",
    "parser": "语法分析",
    "semantic": "语义分析",
    "codegen": "代码生成",
    "output": "写入输出",
    "gcc_verify": "GCC 验证",
    "compiler": "总体流程",
}


# -- Color pair IDs --
CP_TITLE = 1
CP_OK = 2
CP_RUNNING = 3
CP_FAILED = 4
CP_PENDING = 5
CP_LOG_ERROR = 6
CP_LOG_WARN = 7
CP_LOG_INFO = 8
CP_LOG_DEBUG = 9
CP_FOOTER_OK = 10
CP_FOOTER_FAIL = 11
CP_FOOTER_RUN = 12
CP_SCROLL_HINT = 13
CP_SECTION = 14
CP_DETAIL = 15


def init_colors() -> None:
    """Initialize curses color pairs. Safe to call even if terminal has limited colors."""
    curses.start_color()
    curses.use_default_colors()
    # -1 = default terminal background
    curses.init_pair(CP_TITLE, curses.COLOR_CYAN, -1)
    curses.init_pair(CP_OK, curses.COLOR_GREEN, -1)
    curses.init_pair(CP_RUNNING, curses.COLOR_YELLOW, -1)
    curses.init_pair(CP_FAILED, curses.COLOR_RED, -1)
    curses.init_pair(CP_PENDING, curses.COLOR_WHITE, -1)
    curses.init_pair(CP_LOG_ERROR, curses.COLOR_RED, -1)
    curses.init_pair(CP_LOG_WARN, curses.COLOR_YELLOW, -1)
    curses.init_pair(CP_LOG_INFO, curses.COLOR_WHITE, -1)
    curses.init_pair(CP_LOG_DEBUG, curses.COLOR_CYAN, -1)
    curses.init_pair(CP_FOOTER_OK, curses.COLOR_BLACK, curses.COLOR_GREEN)
    curses.init_pair(CP_FOOTER_FAIL, curses.COLOR_WHITE, curses.COLOR_RED)
    curses.init_pair(CP_FOOTER_RUN, curses.COLOR_BLACK, curses.COLOR_YELLOW)
    curses.init_pair(CP_SCROLL_HINT, curses.COLOR_MAGENTA, -1)
    curses.init_pair(CP_SECTION, curses.COLOR_BLUE, -1)
    curses.init_pair(CP_DETAIL, curses.COLOR_WHITE, -1)


def color_for_status(status: str) -> int:
    """Return curses color pair attr for a stage status."""
    if status == "done":
        return curses.color_pair(CP_OK)
    if status == "running":
        return curses.color_pair(CP_RUNNING) | curses.A_BOLD
    if status == "failed":
        return curses.color_pair(CP_FAILED) | curses.A_BOLD
    return curses.color_pair(CP_PENDING) | curses.A_DIM


def color_for_log_line(line: str) -> int:
    """Return curses color pair attr based on log level prefix."""
    if "[ERROR]" in line or "[!!]" in line:
        return curses.color_pair(CP_LOG_ERROR) | curses.A_BOLD
    if "[WARN]" in line:
        return curses.color_pair(CP_LOG_WARN)
    if "[DEBUG]" in line:
        return curses.color_pair(CP_LOG_DEBUG) | curses.A_DIM
    if "[INFO]" in line:
        return curses.color_pair(CP_LOG_INFO)
    if "[EVT]" in line:
        return curses.color_pair(CP_OK)
    if "[UI]" in line:
        return curses.color_pair(CP_SCROLL_HINT)
    return curses.color_pair(CP_LOG_INFO)


@dataclass
class StageState:
    status: str = "pending"
    message: str = "等待中"


def locate_pascc(project_root: str, override: Optional[str]) -> str:
    if override:
        return override

    candidates = [
        os.path.join(project_root, "build", "pascc"),
        os.path.join(project_root, "pascc"),
    ]
    for path in candidates:
        if os.path.isfile(path) and os.access(path, os.X_OK):
            return path

    return "pascc"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="pascc 可视化终端界面（仅此模式显示流程）",
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument("-i", "--input", required=True, help="Pascal-S 输入文件")
    parser.add_argument("-o", "--output", help="输出 C 文件")
    parser.add_argument("--pascc-bin", help="pascc 可执行文件路径，默认自动查找")
    parser.add_argument(
        "--pascc-args",
        default="",
        help="透传给 pascc 的附加参数，示例: \"--dump-annotated-ast\"",
    )
    parser.add_argument(
        "--log-level",
        default="info",
        choices=["off", "error", "warn", "info", "debug"],
        help="过程日志级别（仅影响TUI模式）",
    )
    return parser.parse_args()


def build_command(args: argparse.Namespace, project_root: str) -> List[str]:
    pascc_bin = locate_pascc(project_root, args.pascc_bin)
    cmd = [pascc_bin, "-i", args.input]
    if args.output:
        cmd.extend(["-o", args.output])

    extra = shlex.split(args.pascc_args)
    cmd.extend(extra)
    return cmd


def symbol_for_status(status: str) -> str:
    if status == "done":
        return "[OK]"
    if status == "running":
        return "[..]"
    if status == "failed":
        return "[!!]"
    return "[  ]"


def read_stream(proc: subprocess.Popen, queue: Queue) -> None:
    assert proc.stdout is not None
    for raw_line in iter(proc.stdout.readline, ""):
        queue.put(raw_line.rstrip("\n"))
    queue.put(None)


def run_ui(stdscr: curses.window, cmd: List[str], project_root: str, log_level: str, output_file: Optional[str]) -> int:
    stdscr.keypad(True)

    curses.curs_set(0)
    curses.noecho()
    curses.cbreak()
    init_colors()
    stdscr.nodelay(True)
    stdscr.timeout(120)

    stage_states: Dict[str, StageState] = {name: StageState() for name in STAGE_ORDER}
    stage_details: Dict[str, Deque[str]] = {name: deque(maxlen=40) for name in STAGE_ORDER}
    logs: Deque[str] = deque(maxlen=2000)
    current_stage = "init"

    log_offset = 0
    is_scrolling = False
    gcc_verified = False

    env = os.environ.copy()
    env["PASCC_EVENT_STREAM"] = "1"
    env["PASCC_LOG_LEVEL"] = log_level

    proc = subprocess.Popen(
        cmd,
        cwd=project_root,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
        universal_newlines=True,
    )

    queue: Queue = Queue()
    reader = threading.Thread(target=read_stream, args=(proc, queue), daemon=True)
    reader.start()

    done_reading = False
    spinner = ["|", "/", "-", "\\"]
    spinner_idx = 0
    need_refresh = True
    last_logs_len = 0
    last_draw_time = time.time()

    while True:
        # 处理输入
        try:
            while True:
                item = queue.get_nowait()
                if item is None:
                    done_reading = True
                    break

                if item.startswith(EVENT_PREFIX):
                    payload = item[len(EVENT_PREFIX):]
                    try:
                        evt = json.loads(payload)
                        stage = evt.get("stage", "")
                        if stage in stage_states:
                            stage_states[stage].status = evt.get("status", "pending")
                            stage_states[stage].message = evt.get("message", "")
                            if stage_states[stage].status == "running":
                                current_stage = stage
                            stage_details[stage].append(f"[EVT] {stage_states[stage].message}")
                    except json.JSONDecodeError:
                        logs.append(f"[WARN] 事件解析失败: {item}")
                elif item.startswith(LOG_PREFIX):
                    payload = item[len(LOG_PREFIX):]
                    try:
                        entry = json.loads(payload)
                        stage = entry.get("stage", "compiler")
                        level = entry.get("level", "INFO")
                        message = entry.get("message", "")
                        if stage not in stage_details:
                            stage = "compiler"
                        formatted = f"[{level}][{stage}] {message}"
                        logs.append(formatted)
                        stage_details[stage].append(f"[{level}] {message}")
                        current_stage = stage
                    except json.JSONDecodeError:
                        logs.append(f"[WARN] 日志解析失败: {item}")
                else:
                    logs.append(item)
        except Empty:
            pass

        # 处理按键
        key = stdscr.getch()
        if key != -1:
            if key == ord('q') or key == ord('Q'):
                if proc.poll() is None:
                    proc.terminate()
                    logs.append("[UI] 已发送终止信号")
                else:
                    break
            elif key == curses.KEY_UP:
                max_offset = max(0, len(logs) - 1)
                if log_offset < max_offset:
                    log_offset += 1
                    is_scrolling = True
                    need_refresh = True
            elif key == curses.KEY_DOWN:
                if log_offset > 0:
                    log_offset -= 1
                    if log_offset == 0:
                        is_scrolling = False
                    need_refresh = True
            elif key == curses.KEY_PPAGE:  # Page Up
                h, _ = stdscr.getmaxyx()
                page_size = max(1, (h - 6) // 2)
                max_offset = max(0, len(logs) - 1)
                log_offset = min(max_offset, log_offset + page_size)
                is_scrolling = log_offset > 0
                need_refresh = True
            elif key == curses.KEY_NPAGE:  # Page Down
                h, _ = stdscr.getmaxyx()
                page_size = max(1, (h - 6) // 2)
                log_offset = max(0, log_offset - page_size)
                is_scrolling = log_offset > 0
                need_refresh = True
            elif key == curses.KEY_HOME:
                max_offset = max(0, len(logs) - 1)
                log_offset = max_offset
                is_scrolling = log_offset > 0
                need_refresh = True
            elif key == curses.KEY_END:
                log_offset = 0
                is_scrolling = False
                need_refresh = True
            elif key in (curses.KEY_ENTER, 10, 13):
                if proc.poll() is not None:
                    break

        # 检查进程状态
        if proc.poll() is not None and done_reading and not gcc_verified:
            # pascc 编译完成，如果成功则运行 gcc 验证
            if proc.returncode == 0 and output_file:
                gcc_verified = True
                current_stage = "gcc_verify"
                stage_states["gcc_verify"].status = "running"
                stage_states["gcc_verify"].message = "正在验证生成的 C 代码"
                logs.append("[INFO][gcc_verify] 开始 GCC 验证")
                stage_details["gcc_verify"].append("[INFO] 开始 GCC 验证")
                need_refresh = True

                # 运行 gcc 编译
                gcc_output_bin = output_file.rsplit(".", 1)[0] if "." in output_file else output_file + ".out"
                gcc_cmd = ["gcc", "-o", gcc_output_bin, output_file, "-lm"]

                try:
                    logs.append(f"[INFO][gcc_verify] 执行: {' '.join(gcc_cmd)}")
                    stage_details["gcc_verify"].append(f"[INFO] 执行: {' '.join(gcc_cmd)}")

                    gcc_proc = subprocess.run(
                        gcc_cmd,
                        cwd=project_root,
                        capture_output=True,
                        text=True,
                        timeout=30
                    )

                    if gcc_proc.returncode == 0:
                        stage_states["gcc_verify"].status = "done"
                        stage_states["gcc_verify"].message = "C 代码验证成功"
                        logs.append(f"[INFO][gcc_verify] ✓ GCC 编译成功，生成可执行文件: {gcc_output_bin}")
                        stage_details["gcc_verify"].append(f"[INFO] ✓ GCC 编译成功")
                        stage_details["gcc_verify"].append(f"[INFO] 可执行文件: {gcc_output_bin}")
                    else:
                        stage_states["gcc_verify"].status = "failed"
                        stage_states["gcc_verify"].message = f"C 代码编译失败 (退出码 {gcc_proc.returncode})"
                        logs.append(f"[ERROR][gcc_verify] GCC 编译失败，退出码: {gcc_proc.returncode}")
                        stage_details["gcc_verify"].append(f"[ERROR] GCC 编译失败")

                        if gcc_proc.stdout:
                            for line in gcc_proc.stdout.strip().split("\n"):
                                if line:
                                    logs.append(f"[ERROR][gcc_verify] {line}")
                                    stage_details["gcc_verify"].append(f"[ERROR] {line}")

                        if gcc_proc.stderr:
                            for line in gcc_proc.stderr.strip().split("\n"):
                                if line:
                                    logs.append(f"[ERROR][gcc_verify] {line}")
                                    stage_details["gcc_verify"].append(f"[ERROR] {line}")

                except subprocess.TimeoutExpired:
                    stage_states["gcc_verify"].status = "failed"
                    stage_states["gcc_verify"].message = "GCC 编译超时"
                    logs.append("[ERROR][gcc_verify] GCC 编译超时")
                    stage_details["gcc_verify"].append("[ERROR] GCC 编译超时")
                except FileNotFoundError:
                    stage_states["gcc_verify"].status = "failed"
                    stage_states["gcc_verify"].message = "未找到 gcc 命令"
                    logs.append("[ERROR][gcc_verify] 未找到 gcc 命令，请确保已安装 GCC")
                    stage_details["gcc_verify"].append("[ERROR] 未找到 gcc 命令")
                except Exception as e:
                    stage_states["gcc_verify"].status = "failed"
                    stage_states["gcc_verify"].message = f"GCC 验证异常: {str(e)}"
                    logs.append(f"[ERROR][gcc_verify] 异常: {str(e)}")
                    stage_details["gcc_verify"].append(f"[ERROR] 异常: {str(e)}")

                need_refresh = True
            elif proc.returncode != 0:
                # pascc 编译失败，跳过 gcc 验证
                gcc_verified = True
                stage_states["gcc_verify"].status = "pending"
                stage_states["gcc_verify"].message = "跳过（pascc 编译失败）"
                logs.append("[INFO][gcc_verify] 跳过 GCC 验证（pascc 编译失败）")
            elif not output_file:
                # 没有输出文件，跳过 gcc 验证
                gcc_verified = True
                stage_states["gcc_verify"].status = "pending"
                stage_states["gcc_verify"].message = "跳过（无输出文件）"
                logs.append("[INFO][gcc_verify] 跳过 GCC 验证（无输出文件）")
            else:
                gcc_verified = True

            need_refresh = True

        spinner_idx = (spinner_idx + 1) % len(spinner)
        
        now = time.time()
        if now - last_draw_time > 0.25:
            need_refresh = True
            last_draw_time = now

        if need_refresh or len(logs) != last_logs_len:
            draw_ui(
                stdscr, cmd, stage_states, stage_details, current_stage,
                logs, proc.poll(), spinner[spinner_idx], log_level,
                is_scrolling, log_offset
            )
            need_refresh = False
            last_logs_len = len(logs)

    # 最终刷新 — 保留滚动能力
    draw_ui(stdscr, cmd, stage_states, stage_details, current_stage,
            logs, proc.returncode, " ", log_level, is_scrolling, log_offset)

    stdscr.timeout(120)
    while True:
        key = stdscr.getch()
        if key in (ord("q"), ord("Q"), 10, 13):
            break
        elif key == curses.KEY_UP:
            max_offset = max(0, len(logs) - 1)
            if log_offset < max_offset:
                log_offset += 1
                is_scrolling = True
                draw_ui(stdscr, cmd, stage_states, stage_details, current_stage,
                        logs, proc.returncode, " ", log_level, is_scrolling, log_offset)
        elif key == curses.KEY_DOWN:
            if log_offset > 0:
                log_offset -= 1
                is_scrolling = log_offset > 0
                draw_ui(stdscr, cmd, stage_states, stage_details, current_stage,
                        logs, proc.returncode, " ", log_level, is_scrolling, log_offset)
        elif key == curses.KEY_PPAGE:
            height, _ = stdscr.getmaxyx()
            page_size = max(1, (height - 6) // 2)
            max_offset = max(0, len(logs) - 1)
            log_offset = min(max_offset, log_offset + page_size)
            is_scrolling = log_offset > 0
            draw_ui(stdscr, cmd, stage_states, stage_details, current_stage,
                    logs, proc.returncode, " ", log_level, is_scrolling, log_offset)
        elif key == curses.KEY_NPAGE:
            height, _ = stdscr.getmaxyx()
            page_size = max(1, (height - 6) // 2)
            log_offset = max(0, log_offset - page_size)
            is_scrolling = log_offset > 0
            draw_ui(stdscr, cmd, stage_states, stage_details, current_stage,
                    logs, proc.returncode, " ", log_level, is_scrolling, log_offset)
        elif key == curses.KEY_HOME:
            max_offset = max(0, len(logs) - 1)
            log_offset = max_offset
            is_scrolling = log_offset > 0
            draw_ui(stdscr, cmd, stage_states, stage_details, current_stage,
                    logs, proc.returncode, " ", log_level, is_scrolling, log_offset)
        elif key == curses.KEY_END:
            log_offset = 0
            is_scrolling = False
            draw_ui(stdscr, cmd, stage_states, stage_details, current_stage,
                    logs, proc.returncode, " ", log_level, is_scrolling, log_offset)

    return int(proc.returncode or 0)


def draw_ui(
    stdscr: curses.window,
    cmd: List[str],
    stage_states: Dict[str, StageState],
    stage_details: Dict[str, Deque[str]],
    current_stage: str,
    logs: Deque[str],
    return_code: Optional[int],
    spinner: str,
    log_level: str,
    is_scrolling: bool,
    log_offset: int
) -> None:
    stdscr.erase()
    height, width = stdscr.getmaxyx()
    
    # 确保最小尺寸
    if height < 20 or width < 80:
        stdscr.addstr(0, 0, "终端窗口太小，请调整到至少 80x20")
        stdscr.refresh()
        return

    title = "pascc 编译过程可视化 (q 终止, ↑↓/PgUp/PgDn 滚动, Home/End 跳转, Enter/q 退出)"
    stdscr.addnstr(0, 0, title, width - 1, curses.A_BOLD | curses.color_pair(CP_TITLE))

    command_text = "命令: " + " ".join(shlex.quote(p) for p in cmd)
    stdscr.addnstr(1, 0, command_text[:width-1], width - 1)
    stdscr.addnstr(2, 0, f"日志级别: {log_level.upper()}", width - 1)

    split_col = max(38, width // 3)
    split_col = min(split_col, width - 25)

    stdscr.addnstr(3, 0, f"阶段状态 {spinner}", split_col - 1,
                   curses.A_UNDERLINE | curses.color_pair(CP_SECTION) | curses.A_BOLD)
    stdscr.addnstr(3, split_col + 1, "实时日志", width - split_col - 2,
                   curses.A_UNDERLINE | curses.color_pair(CP_SECTION) | curses.A_BOLD)

    # 显示阶段状态
    row = 4
    for stage in STAGE_ORDER:
        if row >= height - 8:  # 预留空间
            break
        state = stage_states[stage]
        line = f"{symbol_for_status(state.status)} {STAGE_TITLES[stage]}: {state.message}"
        attr = color_for_status(state.status)
        stdscr.addnstr(row, 0, line[:split_col-1], split_col - 1, attr)
        row += 1

    # 显示阶段细节
    details_stage = current_stage if current_stage in stage_details else "compiler"
    row += 1
    if row < height - 6:
        title_line = f"阶段细节: {STAGE_TITLES.get(details_stage, details_stage)}"
        stdscr.addnstr(row, 0, title_line[:split_col-1], split_col - 1,
                       curses.A_UNDERLINE | curses.color_pair(CP_SECTION) | curses.A_BOLD)
        row += 1

    detail_rows = max(0, height - row - 4)
    detail_lines = list(stage_details.get(details_stage, deque()))[-detail_rows:] if detail_rows > 0 else []
    for detail in detail_lines:
        if row >= height - 4:
            break
        attr = color_for_log_line(detail)
        stdscr.addnstr(row, 0, detail[:split_col-1], split_col - 1, attr)
        row += 1

    log_start_y = 4
    log_height = height - 6
    log_width = width - split_col - 2

    total_logs = len(logs)
    if total_logs > 0:
        log_list = list(logs)
        # log_offset 表示从底部向上偏移的行数
        # 默认 (offset=0) 显示最新的 log_height 条日志
        end_index = max(0, total_logs - log_offset)
        start_index = max(0, end_index - log_height)
        visible_logs = log_list[start_index:end_index]

        for i, line in enumerate(visible_logs):
            y = log_start_y + i
            if y >= height - 2:
                break
            if len(line) > log_width - 1:
                line = line[:log_width - 4] + "..."
            attr = color_for_log_line(line)
            try:
                stdscr.addnstr(y, split_col + 1, line, log_width - 1, attr)
            except curses.error:
                pass
    else:
        try:
            stdscr.addstr(log_start_y, split_col + 1, "等待日志输出...",
                          curses.color_pair(CP_PENDING) | curses.A_DIM)
        except curses.error:
            pass

    # 显示滚动状态信息
    scroll_status = "滚动模式 ↑↓" if is_scrolling else "实时模式 (自动跟随)"
    scroll_info = f"{scroll_status} | 偏移: {log_offset} | 总计: {len(logs)} 条日志"
    scroll_attr = curses.color_pair(CP_SCROLL_HINT) if is_scrolling else curses.A_DIM
    try:
        stdscr.addnstr(height - 2, split_col + 1, scroll_info[:width - split_col - 2],
                      width - split_col - 2, scroll_attr)
    except curses.error:
        pass

    # 底部状态栏
    if return_code is None:
        footer = "运行中..."
        footer_attr = curses.color_pair(CP_FOOTER_RUN) | curses.A_BOLD
    elif return_code == 0:
        footer = "已完成，退出码=0"
        footer_attr = curses.color_pair(CP_FOOTER_OK) | curses.A_BOLD
    else:
        footer = f"已完成，退出码={return_code}"
        footer_attr = curses.color_pair(CP_FOOTER_FAIL) | curses.A_BOLD
    try:
        # 填充整行背景色
        stdscr.addnstr(height - 1, 0, footer.ljust(width - 1), width - 1, footer_attr)
    except curses.error:
        pass

    stdscr.refresh()


def main() -> int:
    if not sys.stdout.isatty() or not sys.stdin.isatty():
        print("pascc_tui.py 需要在交互式终端中运行。", file=sys.stderr)
        return 2

    args = parse_args()
    project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    cmd = build_command(args, project_root)

    try:
        return curses.wrapper(run_ui, cmd, project_root, args.log_level, args.output)
    except FileNotFoundError as exc:
        print(f"无法启动 pascc: {exc}", file=sys.stderr)
        print("请先执行 scripts/build.sh 生成 build/pascc，或用 --pascc-bin 指定路径。", file=sys.stderr)
        return 127


if __name__ == "__main__":
    raise SystemExit(main())