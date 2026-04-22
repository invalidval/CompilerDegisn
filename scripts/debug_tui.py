#!/usr/bin/env python3
"""Debug Terminal UI wrapper for pascc compilation progress.

This UI is opt-in and does not affect normal pascc CLI usage.
"""

from __future__ import annotations

import argparse
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
STAGE_ORDER = ["init", "lexer", "parser", "semantic", "codegen", "output", "compiler"]
STAGE_TITLES = {
    "init": "初始化",
    "lexer": "词法分析",
    "parser": "语法分析",
    "semantic": "语义分析",
    "codegen": "代码生成",
    "output": "写入输出",
    "compiler": "总体流程",
}


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


def run_ui(stdscr: curses.window, cmd: List[str], project_root: str, log_level: str) -> int:
    stdscr.keypad(True)
    
    curses.curs_set(0)
    curses.noecho()
    curses.cbreak()
    stdscr.nodelay(True)
    stdscr.timeout(120)

    stage_states: Dict[str, StageState] = {name: StageState() for name in STAGE_ORDER}
    stage_details: Dict[str, Deque[str]] = {name: deque(maxlen=40) for name in STAGE_ORDER}
    logs: Deque[str] = deque(maxlen=2000)
    current_stage = "init"

    log_offset = 0
    is_scrolling = False

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

    while True:
        try:
            while True:
                item = queue.get_nowait()
                if item is None:
                    done_reading = True
                    break

                if item.startswith(EVENT_PREFIX):
                    payload = item[len(EVENT_PREFIX) :]
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
                    payload = item[len(LOG_PREFIX) :]
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

        key = stdscr.getch()
        if key != -1:  # -1 表示没有按键
            # --- DEBUG: Print the key code ---
            print(f"DEBUG: Pressed key code: {key}")
            # --- END DEBUG ---
            
            if key == ord('q') or key == ord('Q'):
                if proc.poll() is None:
                    proc.terminate()
                    logs.append("[UI] 已发送终止信号")
                else:
                    break
            elif key == curses.KEY_UP:
                print("DEBUG: Recognized KEY_UP")
                if len(logs) > 0 and log_offset < len(logs) - 1:
                    log_offset += 1
                    is_scrolling = True
            elif key == curses.KEY_DOWN:
                print("DEBUG: Recognized KEY_DOWN")
                if log_offset > 0:
                    log_offset -= 1
                    if log_offset == 0:
                        is_scrolling = False
            elif key in (curses.KEY_ENTER, 10, 13):
                if proc.poll() is not None:
                    break

        if proc.poll() is not None and done_reading:
            log_offset = 0
            is_scrolling = False

        spinner_idx = (spinner_idx + 1) % len(spinner)
        draw_ui(
            stdscr,
            cmd,
            stage_states,
            stage_details,
            current_stage,
            logs,
            proc.poll(),
            spinner[spinner_idx],
            log_level,
            is_scrolling,
            log_offset
        )

    draw_ui(stdscr, cmd, stage_states, stage_details, current_stage, logs, proc.returncode, " ", log_level, False, 0)
    logs.append(f"[UI] 进程退出码: {proc.returncode}")
    draw_ui(stdscr, cmd, stage_states, stage_details, current_stage, logs, proc.returncode, " ", log_level, False, 0)

    while True:
        key = stdscr.getch()
        if key in (ord("q"), ord("Q"), 10, 13):
            break

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

    title = "pascc 编译过程可视化 (q 终止, UP/DOWN 滚动, Enter/q 退出) - DEBUG"
    stdscr.addnstr(0, 0, title, width - 1, curses.A_BOLD)

    command_text = "命令: " + " ".join(shlex.quote(p) for p in cmd)
    stdscr.addnstr(1, 0, command_text, width - 1)
    stdscr.addnstr(2, 0, f"日志级别: {log_level.upper()}", width - 1)

    split_col = max(38, width // 3)
    split_col = min(split_col, width - 20)

    stdscr.addnstr(3, 0, f"阶段状态 {spinner}", split_col - 1, curses.A_UNDERLINE)
    stdscr.addnstr(3, split_col + 1, "实时日志", width - split_col - 2, curses.A_UNDERLINE)

    row = 4
    for stage in STAGE_ORDER:
        state = stage_states[stage]
        line = f"{symbol_for_status(state.status)} {STAGE_TITLES[stage]}: {state.message}"
        stdscr.addnstr(row, 0, line, split_col - 1)
        row += 1
        if row >= height - 2:
            break

    details_stage = current_stage if current_stage in stage_details else "compiler"
    row += 1
    if row < height - 1:
        stdscr.addnstr(row, 0, f"阶段细节: {STAGE_TITLES.get(details_stage, details_stage)}", split_col - 1, curses.A_UNDERLINE)
        row += 1

    detail_rows = max(0, height - 1 - row)
    detail_lines = list(stage_details.get(details_stage, deque()))[-detail_rows:] if detail_rows > 0 else []
    for detail in detail_lines:
        stdscr.addnstr(row, 0, detail, split_col - 1)
        row += 1
        if row >= height - 1:
            break

    log_rows = height - 5
    start_index = max(0, len(logs) - log_rows - log_offset)
    end_index = min(len(logs), start_index + log_rows)
    
    visible_logs = list(logs)[start_index:end_index]

    log_col_width = width - split_col - 2
    for i, line in enumerate(visible_logs):
        stdscr.addnstr(4 + i, split_col + 1, line, max(1, log_col_width))

    scroll_status = "滚动模式" if is_scrolling else "实时模式"
    scroll_info = f"{scroll_status} | 偏移: {log_offset} | 总计: {len(logs)}"
    stdscr.addnstr(height - 2, split_col + 1, scroll_info, width - split_col - 2, curses.A_DIM)

    footer = "运行中" if return_code is None else f"已结束，退出码={return_code}"
    stdscr.addnstr(height - 1, 0, footer, width - 1, curses.A_REVERSE)

    stdscr.refresh()


def main() -> int:
    if not sys.stdout.isatty() or not sys.stdin.isatty():
        print("pascc_tui.py 需要在交互式终端中运行。", file=sys.stderr)
        return 2

    args = parse_args()
    project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    cmd = build_command(args, project_root)

    try:
        return curses.wrapper(run_ui, cmd, project_root, args.log_level)
    except FileNotFoundError as exc:
        print(f"无法启动 pascc: {exc}", file=sys.stderr)
        print("请先执行 scripts/build.sh 生成 build/pascc，或用 --pascc-bin 指定路径。", file=sys.stderr)
        return 127


if __name__ == "__main__":
    raise SystemExit(main())