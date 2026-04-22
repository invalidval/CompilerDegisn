#!/usr/bin/env python3
"""Interactive Terminal UI wrapper for pascc compilation.

This version provides an interactive configuration menu before compilation.
"""

from __future__ import annotations

import curses
import json
import os
import shlex
import subprocess
import sys
import threading
import time
from collections import deque
from dataclasses import dataclass
from pathlib import Path
from queue import Empty, Queue
from typing import Deque, Dict, List, Optional

# Import constants and functions from pascc_tui
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

# Color pair IDs
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


@dataclass
class StageState:
    status: str = "pending"
    message: str = "等待中"


def init_colors() -> None:
    """Initialize curses color pairs."""
    curses.start_color()
    curses.use_default_colors()
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


def locate_pascc(project_root: str, override: Optional[str]) -> str:
    """Locate the pascc executable."""
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


def symbol_for_status(status: str) -> str:
    """Return symbol for status."""
    if status == "done":
        return "[OK]"
    if status == "running":
        return "[..]"
    if status == "failed":
        return "[!!]"
    return "[  ]"


def read_stream(proc: subprocess.Popen, queue: Queue) -> None:
    """Read process output stream."""
    assert proc.stdout is not None
    for raw_line in iter(proc.stdout.readline, ""):
        queue.put(raw_line.rstrip("\n"))
    queue.put(None)


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
    """Draw the compilation UI."""
    stdscr.erase()
    height, width = stdscr.getmaxyx()

    if height < 20 or width < 80:
        stdscr.addstr(0, 0, "终端窗口太小，请调整到至少 80x20")
        stdscr.refresh()
        return

    title = "pascc 编译过程可视化 (q 终止, ↑↓/PgUp/PgDn 滚动, Home/End 跳转)"
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

    row = 4
    for stage in STAGE_ORDER:
        if row >= height - 8:
            break
        state = stage_states[stage]
        line = f"{symbol_for_status(state.status)} {STAGE_TITLES[stage]}: {state.message}"
        attr = color_for_status(state.status)
        stdscr.addnstr(row, 0, line[:split_col-1], split_col - 1, attr)
        row += 1

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

    scroll_status = "滚动模式 ↑↓" if is_scrolling else "实时模式 (自动跟随)"
    scroll_info = f"{scroll_status} | 偏移: {log_offset} | 总计: {len(logs)} 条日志"
    scroll_attr = curses.color_pair(CP_SCROLL_HINT) if is_scrolling else curses.A_DIM
    try:
        stdscr.addnstr(height - 2, split_col + 1, scroll_info[:width - split_col - 2],
                      width - split_col - 2, scroll_attr)
    except curses.error:
        pass

    if return_code is None:
        footer = "运行中..."
        footer_attr = curses.color_pair(CP_FOOTER_RUN) | curses.A_BOLD
    elif return_code == 0:
        footer = "已完成，退出码=0 | 按 Enter/q 返回菜单"
        footer_attr = curses.color_pair(CP_FOOTER_OK) | curses.A_BOLD
    else:
        footer = f"已完成，退出码={return_code} | 按 Enter/q 返回菜单"
        footer_attr = curses.color_pair(CP_FOOTER_FAIL) | curses.A_BOLD
    try:
        stdscr.addnstr(height - 1, 0, footer.ljust(width - 1), width - 1, footer_attr)
    except curses.error:
        pass

    stdscr.refresh()


def run_ui(stdscr: curses.window, cmd: List[str], project_root: str, log_level: str) -> int:
    """Run the compilation UI."""
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
            elif key == curses.KEY_PPAGE:
                h, _ = stdscr.getmaxyx()
                page_size = max(1, (h - 6) // 2)
                max_offset = max(0, len(logs) - 1)
                log_offset = min(max_offset, log_offset + page_size)
                is_scrolling = log_offset > 0
                need_refresh = True
            elif key == curses.KEY_NPAGE:
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

        if proc.poll() is not None and done_reading:
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


@dataclass
class CompilerConfig:
    """Configuration for the compiler."""
    input_file: str = ""
    output_file: str = ""
    log_level: str = "info"
    extra_args: str = ""
    pascc_bin: str = ""
    selected_options: List[str] = None  # Store selected options in order

    def __post_init__(self):
        if self.selected_options is None:
            self.selected_options = []


LOG_LEVELS = ["off", "error", "warn", "info", "debug"]

# Available pascc options
PASCC_OPTIONS = [
    ("--lex", "只运行到词法分析"),
    ("--dump-tokens", "转储 tokens"),
    ("--parse", "只运行到语法分析"),
    ("--semantic", "只运行到语义分析"),
    ("--dump-annotated-ast", "转储带注释的 AST"),
]


def select_compiler_options(stdscr: curses.window, current_options: List[str]) -> Optional[List[str]]:
    """Show a multi-select menu for compiler options."""
    curses.curs_set(0)
    stdscr.keypad(True)

    selected_idx = 0
    # Track which options are selected and their order
    selected_options = list(current_options)  # Copy current selections

    while True:
        stdscr.erase()
        height, width = stdscr.getmaxyx()

        # Title
        title = "选择编译器选项 (多选)"
        try:
            stdscr.addstr(1, (width - len(title)) // 2, title, curses.A_BOLD | curses.color_pair(1))
        except curses.error:
            pass

        # Instructions
        instructions = "Space 切换选择 | ↑↓ 导航 | Enter 确认 | Esc 取消 | c 清除所有"
        try:
            stdscr.addstr(3, 2, instructions, curses.color_pair(8) | curses.A_DIM)
        except curses.error:
            pass

        # Options list
        start_y = 5
        for idx, (option, description) in enumerate(PASCC_OPTIONS):
            y = start_y + idx
            if y >= height - 5:
                break

            is_selected = (idx == selected_idx)
            is_checked = option in selected_options

            # Cursor
            cursor = "▶ " if is_selected else "  "

            # Checkbox
            checkbox = "[✓]" if is_checked else "[ ]"

            # Order number
            order_num = ""
            if is_checked:
                order = selected_options.index(option) + 1
                order_num = f" ({order})"

            # Build line
            line = f"{cursor}{checkbox} {option}{order_num}"
            attr = curses.color_pair(3) | curses.A_BOLD if is_selected else curses.A_NORMAL
            if is_checked:
                attr |= curses.color_pair(2)

            try:
                stdscr.addstr(y, 4, line, attr)
            except curses.error:
                pass

            # Description
            desc_x = 40
            if desc_x < width - 10:
                try:
                    desc_attr = curses.color_pair(8) | curses.A_DIM
                    stdscr.addstr(y, desc_x, description[:width - desc_x - 2], desc_attr)
                except curses.error:
                    pass

        # Show selected options in order
        summary_y = start_y + len(PASCC_OPTIONS) + 2
        if summary_y < height - 3:
            try:
                stdscr.addstr(summary_y, 2, "已选择的选项 (按顺序):", curses.A_UNDERLINE | curses.color_pair(1))
            except curses.error:
                pass
            summary_y += 1

            if selected_options:
                summary_text = " ".join(selected_options)
                if len(summary_text) > width - 6:
                    summary_text = summary_text[:width - 9] + "..."
                try:
                    stdscr.addstr(summary_y, 4, summary_text, curses.color_pair(3))
                except curses.error:
                    pass
            else:
                try:
                    stdscr.addstr(summary_y, 4, "(无)", curses.A_DIM)
                except curses.error:
                    pass

        # Footer
        footer = f"已选择 {len(selected_options)} 个选项"
        try:
            stdscr.addnstr(height - 1, 0, footer.ljust(width - 1), width - 1, curses.A_REVERSE | curses.color_pair(8))
        except curses.error:
            pass

        stdscr.refresh()

        # Handle input
        key = stdscr.getch()

        if key == 27:  # ESC
            return None
        elif key == curses.KEY_UP:
            selected_idx = (selected_idx - 1) % len(PASCC_OPTIONS)
        elif key == curses.KEY_DOWN:
            selected_idx = (selected_idx + 1) % len(PASCC_OPTIONS)
        elif key == ord(' '):  # Space - toggle selection
            option = PASCC_OPTIONS[selected_idx][0]
            if option in selected_options:
                selected_options.remove(option)
            else:
                selected_options.append(option)
        elif key in (curses.KEY_ENTER, 10, 13):  # Enter - confirm
            return selected_options
        elif key in (ord('c'), ord('C')):  # Clear all
            selected_options.clear()


def find_pascal_files(project_root: str) -> List[str]:
    """Find all Pascal files in the test directory."""
    test_dir = os.path.join(project_root, "test")
    pascal_files = []

    if os.path.isdir(test_dir):
        for root, _, files in os.walk(test_dir):
            for file in files:
                if file.endswith(('.pas', '.p')):
                    full_path = os.path.join(root, file)
                    rel_path = os.path.relpath(full_path, project_root)
                    pascal_files.append(rel_path)

    return sorted(pascal_files)


def draw_welcome_screen(stdscr: curses.window, selected_idx: int, config: CompilerConfig,
                       pascal_files: List[str], file_scroll: int, file_selected: int,
                       in_file_browser: bool, project_root: str) -> None:
    """Draw the welcome/configuration screen."""
    stdscr.erase()
    height, width = stdscr.getmaxyx()

    if height < 20 or width < 80:
        stdscr.addstr(0, 0, "终端窗口太小，请调整到至少 80x20")
        stdscr.refresh()
        return

    # Title
    title = "═══ PASCC 编译器交互式界面 ═══"
    try:
        stdscr.addstr(1, (width - len(title)) // 2, title, curses.A_BOLD | curses.color_pair(1))
    except curses.error:
        pass

    subtitle = "Pascal-S 到 C 的编译器"
    try:
        stdscr.addstr(2, (width - len(subtitle)) // 2, subtitle, curses.color_pair(8))
    except curses.error:
        pass

    # Instructions
    if in_file_browser:
        instruction = "文件浏览: ↑↓ 选择，Enter/Space 确认，Esc 返回，PgUp/PgDn 翻页"
    else:
        instruction = "使用 ↑↓ 导航，← → 调整选项，Enter 选择/编辑，q 退出"
    try:
        stdscr.addstr(4, 2, instruction, curses.color_pair(8) | curses.A_DIM)
    except curses.error:
        pass

    # Menu items
    menu_start_y = 6
    options_summary = f"{len(config.selected_options)} 个选项" if config.selected_options else "无"
    menu_items = [
        ("输入文件", config.input_file or "未选择"),
        ("输出文件", config.output_file or "自动生成"),
        ("编译器选项", options_summary),
        ("日志级别", config.log_level.upper()),
        ("额外参数", config.extra_args or "无"),
        ("", ""),  # Separator
        ("开始编译", ""),
        ("退出", ""),
    ]

    for idx, (label, value) in enumerate(menu_items):
        y = menu_start_y + idx
        if y >= height - 2:
            break

        if not label:  # Separator
            continue

        is_selected = (idx == selected_idx) and not in_file_browser

        if is_selected:
            try:
                stdscr.addstr(y, 4, "▶", curses.color_pair(3) | curses.A_BOLD)
            except curses.error:
                pass

        label_attr = curses.A_BOLD if is_selected else curses.A_NORMAL
        if idx >= len(menu_items) - 2:  # Action items
            label_attr |= curses.color_pair(2) if idx == len(menu_items) - 2 else curses.color_pair(4)
        else:
            label_attr |= curses.color_pair(1) if is_selected else curses.A_NORMAL

        try:
            stdscr.addstr(y, 6, f"{label}", label_attr)
        except curses.error:
            pass

        if value and idx < len(menu_items) - 2:
            value_attr = curses.color_pair(3) if is_selected else curses.color_pair(8)
            value_text = f": {value}"
            max_value_len = width - 30
            if len(value_text) > max_value_len:
                value_text = value_text[:max_value_len-3] + "..."
            try:
                stdscr.addstr(y, 20, value_text, value_attr)
            except curses.error:
                pass

    # File browser (if input file is selected or in browser mode)
    if (selected_idx == 0 or in_file_browser) and pascal_files:
        browser_y = menu_start_y + len(menu_items) + 1
        if browser_y < height - 3:
            browser_title = "可用的 Pascal 文件 (按 Enter 进入浏览模式):" if not in_file_browser else "可用的 Pascal 文件 (浏览模式):"
            try:
                title_attr = curses.A_UNDERLINE | curses.color_pair(1)
                if in_file_browser:
                    title_attr |= curses.A_BOLD
                stdscr.addstr(browser_y, 2, browser_title, title_attr)
            except curses.error:
                pass
            browser_y += 1

            visible_height = min(10, height - browser_y - 2)

            # Auto-scroll to keep selected file visible
            if in_file_browser:
                if file_selected < file_scroll:
                    file_scroll = file_selected
                elif file_selected >= file_scroll + visible_height:
                    file_scroll = file_selected - visible_height + 1

            start_idx = file_scroll
            end_idx = min(start_idx + visible_height, len(pascal_files))

            for i in range(start_idx, end_idx):
                if browser_y >= height - 2:
                    break
                file_name = pascal_files[i]
                display_name = file_name
                if len(display_name) > width - 12:
                    display_name = "..." + display_name[-(width-15):]

                is_file_selected = (i == file_selected and in_file_browser)

                # Show cursor for selected file
                cursor = "▶ " if is_file_selected else "  "
                file_attr = curses.color_pair(3) | curses.A_BOLD if is_file_selected else curses.color_pair(8)

                try:
                    stdscr.addstr(browser_y, 4, f"{cursor}{i+1}. {display_name}", file_attr)
                except curses.error:
                    pass
                browser_y += 1

            if len(pascal_files) > visible_height:
                scroll_info = f"(显示 {start_idx+1}-{end_idx}/{len(pascal_files)})"
                if in_file_browser:
                    scroll_info += " | ↑↓ 导航, PgUp/PgDn 翻页"
                try:
                    stdscr.addstr(browser_y, 4, scroll_info, curses.A_DIM)
                except curses.error:
                    pass

    # Footer
    if in_file_browser:
        footer = "Enter/Space 确认选择 | Esc 返回菜单 | 数字键快速选择"
    else:
        footer = "提示: 在输入文件上按 Enter 进入浏览模式 | q 退出"
    try:
        stdscr.addnstr(height - 1, 0, footer.ljust(width-1), width - 1, curses.A_REVERSE | curses.color_pair(8))
    except curses.error:
        pass

    stdscr.refresh()

    return file_scroll  # Return updated scroll position


def get_text_input(stdscr: curses.window, prompt: str, default: str = "") -> Optional[str]:
    """Get text input from user."""
    height, width = stdscr.getmaxyx()

    # Create input window
    input_height = 7
    input_width = min(60, width - 4)
    input_y = (height - input_height) // 2
    input_x = (width - input_width) // 2

    input_win = curses.newwin(input_height, input_width, input_y, input_x)
    input_win.box()

    try:
        input_win.addstr(1, 2, prompt[:input_width-4], curses.A_BOLD)
        input_win.addstr(3, 2, "输入: ", curses.A_BOLD)
        input_win.addstr(5, 2, "Enter 确认 | Esc 取消", curses.A_DIM)
    except curses.error:
        pass

    curses.curs_set(1)
    curses.echo()

    input_win.refresh()

    # Get input
    text = default
    try:
        input_win.addstr(3, 9, text)
    except curses.error:
        pass
    input_win.refresh()

    result = None
    while True:
        key = input_win.getch()

        if key == 27:  # ESC
            break
        elif key in (10, 13):  # Enter
            result = text
            break
        elif key in (curses.KEY_BACKSPACE, 127, 8):
            if text:
                text = text[:-1]
                try:
                    input_win.addstr(3, 9, " " * (input_width - 11))
                    input_win.addstr(3, 9, text)
                except curses.error:
                    pass
                input_win.refresh()
        elif 32 <= key <= 126:  # Printable characters
            if len(text) < input_width - 12:
                text += chr(key)
                try:
                    input_win.addstr(3, 9, text)
                except curses.error:
                    pass
                input_win.refresh()

    curses.noecho()
    curses.curs_set(0)

    del input_win
    stdscr.touchwin()
    stdscr.refresh()

    return result


def run_config_menu(stdscr: curses.window, project_root: str) -> Optional[CompilerConfig]:
    """Run the interactive configuration menu."""
    curses.curs_set(0)
    curses.noecho()
    curses.cbreak()
    stdscr.keypad(True)
    init_colors()

    config = CompilerConfig()
    pascal_files = find_pascal_files(project_root)

    selected_idx = 0
    file_scroll = 0
    file_selected = 0
    in_file_browser = False
    max_menu_idx = 7  # 0-4: config items, 5: separator, 6: start, 7: exit

    while True:
        file_scroll = draw_welcome_screen(stdscr, selected_idx, config, pascal_files,
                                         file_scroll, file_selected, in_file_browser, project_root)

        key = stdscr.getch()

        # Handle file browser mode
        if in_file_browser:
            if key == 27:  # ESC - exit file browser
                in_file_browser = False
            elif key == curses.KEY_UP:
                if file_selected > 0:
                    file_selected -= 1
            elif key == curses.KEY_DOWN:
                if file_selected < len(pascal_files) - 1:
                    file_selected += 1
            elif key == curses.KEY_PPAGE:  # Page Up
                file_selected = max(0, file_selected - 10)
            elif key == curses.KEY_NPAGE:  # Page Down
                file_selected = min(len(pascal_files) - 1, file_selected + 10)
            elif key == curses.KEY_HOME:
                file_selected = 0
            elif key == curses.KEY_END:
                file_selected = len(pascal_files) - 1
            elif key in (curses.KEY_ENTER, 10, 13, ord(' ')):  # Enter or Space
                if pascal_files:
                    config.input_file = pascal_files[file_selected]
                    in_file_browser = False
            elif 49 <= key <= 57:  # Number keys 1-9
                num = key - 48
                if 0 < num <= len(pascal_files):
                    config.input_file = pascal_files[num - 1]
                    file_selected = num - 1
                    in_file_browser = False
        # Handle normal menu mode
        else:
            if key == ord('q') or key == ord('Q'):
                return None
            elif key == curses.KEY_UP:
                selected_idx = (selected_idx - 1) % (max_menu_idx + 1)
                if selected_idx == 5:  # Skip separator
                    selected_idx = 4
            elif key == curses.KEY_DOWN:
                selected_idx = (selected_idx + 1) % (max_menu_idx + 1)
                if selected_idx == 5:  # Skip separator
                    selected_idx = 6
            elif key == curses.KEY_LEFT:
                if selected_idx == 3:  # Log level
                    current_idx = LOG_LEVELS.index(config.log_level)
                    config.log_level = LOG_LEVELS[(current_idx - 1) % len(LOG_LEVELS)]
            elif key == curses.KEY_RIGHT:
                if selected_idx == 3:  # Log level
                    current_idx = LOG_LEVELS.index(config.log_level)
                    config.log_level = LOG_LEVELS[(current_idx + 1) % len(LOG_LEVELS)]
            elif key in (curses.KEY_ENTER, 10, 13):
                if selected_idx == 0:  # Input file - enter browser mode
                    if pascal_files:
                        in_file_browser = True
                        # Set initial selection to current file if it exists
                        if config.input_file and config.input_file in pascal_files:
                            file_selected = pascal_files.index(config.input_file)
                        else:
                            file_selected = 0
                    else:
                        # No files found, allow manual input
                        result = get_text_input(stdscr, "输入文件路径", config.input_file)
                        if result is not None:
                            config.input_file = result
                elif selected_idx == 1:  # Output file
                    result = get_text_input(stdscr, "输出文件路径 (留空自动生成)", config.output_file)
                    if result is not None:
                        config.output_file = result
                elif selected_idx == 2:  # Compiler options
                    result = select_compiler_options(stdscr, config.selected_options)
                    if result is not None:
                        config.selected_options = result
                    # Reinitialize colors after returning from sub-menu
                    init_colors()
                elif selected_idx == 4:  # Extra args
                    result = get_text_input(stdscr, "额外参数 (如 --dump-ast)", config.extra_args)
                    if result is not None:
                        config.extra_args = result
                elif selected_idx == 6:  # Start compilation
                    if not config.input_file:
                        # Show error
                        try:
                            stdscr.addstr(0, 0, "错误: 必须选择输入文件!", curses.color_pair(4) | curses.A_BOLD)
                        except curses.error:
                            pass
                        stdscr.refresh()
                        curses.napms(1500)
                    else:
                        return config
                elif selected_idx == 7:  # Exit
                    return None
            elif key == ord('i') or key == ord('I'):  # 'i' for manual input
                if selected_idx == 0:
                    result = get_text_input(stdscr, "输入文件路径", config.input_file)
                    if result is not None:
                        config.input_file = result


def main() -> int:
    """Main entry point."""
    if not sys.stdout.isatty() or not sys.stdin.isatty():
        print("此程序需要在交互式终端中运行。", file=sys.stderr)
        return 2

    project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    last_exit_code = 0

    try:
        while True:
            # Run configuration menu
            config = curses.wrapper(run_config_menu, project_root)

            if config is None:
                print("已退出。")
                return last_exit_code

            # Build command
            pascc_bin = locate_pascc(project_root, config.pascc_bin)
            cmd = [pascc_bin, "-i", config.input_file]

            if config.output_file:
                cmd.extend(["-o", config.output_file])

            # Add selected compiler options in order
            if config.selected_options:
                cmd.extend(config.selected_options)

            if config.extra_args:
                cmd.extend(shlex.split(config.extra_args))

            # Run the compilation UI
            last_exit_code = curses.wrapper(run_ui, cmd, project_root, config.log_level)

            # After compilation, the UI will return to the menu automatically
            # The loop continues, showing the config menu again

    except FileNotFoundError as exc:
        print(f"无法启动 pascc: {exc}", file=sys.stderr)
        print("请先执行 scripts/build.sh 生成 build/pascc。", file=sys.stderr)
        return 127
    except KeyboardInterrupt:
        print("\n已取消。")
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
