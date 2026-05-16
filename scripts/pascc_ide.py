#!/usr/bin/env python3
"""PASCC IDE - A lightweight terminal-based IDE for Pascal-S compiler.

Features:
- Code editor with syntax highlighting
- File browser
- Integrated compilation with real-time feedback
- Error navigation
- Quick compile and run
"""

from __future__ import annotations

import curses
import json
import os
import re
import shlex
import subprocess
import sys
import threading
import time
from collections import deque
from dataclasses import dataclass, field
from pathlib import Path
from queue import Empty, Queue
from typing import Deque, Dict, List, Optional, Tuple

# Color pair IDs
CP_TITLE = 1
CP_OK = 2
CP_RUNNING = 3
CP_FAILED = 4
CP_PENDING = 5
CP_KEYWORD = 6
CP_COMMENT = 7
CP_STRING = 8
CP_NUMBER = 9
CP_NORMAL = 10
CP_CURSOR_LINE = 11
CP_LINE_NUM = 12
CP_STATUS_BAR = 13
CP_ERROR_LINE = 14
CP_BORDER = 15

# Pascal-S keywords
KEYWORDS = {
    'program', 'var', 'const', 'procedure', 'function', 'begin', 'end',
    'if', 'then', 'else', 'while', 'do', 'for', 'to', 'downto',
    'array', 'of', 'integer', 'real', 'boolean', 'char',
    'true', 'false', 'not', 'and', 'or', 'div', 'mod',
    'read', 'write', 'break'
}


def init_colors() -> None:
    """Initialize curses color pairs."""
    curses.start_color()
    curses.use_default_colors()
    curses.init_pair(CP_TITLE, curses.COLOR_CYAN, -1)
    curses.init_pair(CP_OK, curses.COLOR_GREEN, -1)
    curses.init_pair(CP_RUNNING, curses.COLOR_YELLOW, -1)
    curses.init_pair(CP_FAILED, curses.COLOR_RED, -1)
    curses.init_pair(CP_PENDING, curses.COLOR_WHITE, -1)
    curses.init_pair(CP_KEYWORD, curses.COLOR_BLUE, -1)
    curses.init_pair(CP_COMMENT, curses.COLOR_GREEN, -1)
    curses.init_pair(CP_STRING, curses.COLOR_YELLOW, -1)
    curses.init_pair(CP_NUMBER, curses.COLOR_MAGENTA, -1)
    curses.init_pair(CP_NORMAL, curses.COLOR_WHITE, -1)
    curses.init_pair(CP_CURSOR_LINE, curses.COLOR_BLACK, curses.COLOR_WHITE)
    curses.init_pair(CP_LINE_NUM, curses.COLOR_CYAN, -1)
    curses.init_pair(CP_STATUS_BAR, curses.COLOR_BLACK, curses.COLOR_CYAN)
    curses.init_pair(CP_ERROR_LINE, curses.COLOR_WHITE, curses.COLOR_RED)
    curses.init_pair(CP_BORDER, curses.COLOR_CYAN, -1)


@dataclass
class EditorState:
    """State of the text editor."""
    lines: List[str] = field(default_factory=lambda: [""])
    cursor_x: int = 0
    cursor_y: int = 0
    scroll_x: int = 0
    scroll_y: int = 0
    file_path: Optional[str] = None
    modified: bool = False
    error_lines: Dict[int, str] = field(default_factory=dict)  # line_num -> error_msg


@dataclass
class CompileResult:
    """Result of compilation."""
    success: bool
    output: str
    errors: List[Tuple[int, int, str]] = field(default_factory=list)  # (line, col, msg)


class TextEditor:
    """Simple text editor component."""

    def __init__(self, state: EditorState):
        self.state = state

    def load_file(self, file_path: str) -> bool:
        """Load a file into the editor."""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                self.state.lines = f.read().splitlines()
                if not self.state.lines:
                    self.state.lines = [""]
            self.state.file_path = os.path.abspath(file_path)
            self.state.modified = False
            self.state.cursor_x = 0
            self.state.cursor_y = 0
            self.state.scroll_x = 0
            self.state.scroll_y = 0
            self.state.error_lines.clear()
            return True
        except Exception as e:
            return False

    def save_file(self, file_path: Optional[str] = None) -> bool:
        """Save the current buffer to a file."""
        target = file_path or self.state.file_path
        if not target:
            return False

        try:
            with open(target, 'w', encoding='utf-8') as f:
                f.write('\n'.join(self.state.lines))
            self.state.file_path = os.path.abspath(target)
            self.state.modified = False
            return True
        except Exception:
            return False

    def insert_char(self, ch: str) -> None:
        """Insert a character at cursor position."""
        line = self.state.lines[self.state.cursor_y]
        self.state.lines[self.state.cursor_y] = (
            line[:self.state.cursor_x] + ch + line[self.state.cursor_x:]
        )
        self.state.cursor_x += 1
        self.state.modified = True

    def delete_char(self) -> None:
        """Delete character before cursor (backspace)."""
        if self.state.cursor_x > 0:
            line = self.state.lines[self.state.cursor_y]
            self.state.lines[self.state.cursor_y] = (
                line[:self.state.cursor_x - 1] + line[self.state.cursor_x:]
            )
            self.state.cursor_x -= 1
            self.state.modified = True
        elif self.state.cursor_y > 0:
            # Join with previous line
            prev_line = self.state.lines[self.state.cursor_y - 1]
            curr_line = self.state.lines[self.state.cursor_y]
            self.state.lines[self.state.cursor_y - 1] = prev_line + curr_line
            del self.state.lines[self.state.cursor_y]
            self.state.cursor_y -= 1
            self.state.cursor_x = len(prev_line)
            self.state.modified = True

    def delete_char_forward(self) -> None:
        """Delete character at cursor (delete key)."""
        line = self.state.lines[self.state.cursor_y]
        if self.state.cursor_x < len(line):
            self.state.lines[self.state.cursor_y] = (
                line[:self.state.cursor_x] + line[self.state.cursor_x + 1:]
            )
            self.state.modified = True
        elif self.state.cursor_y < len(self.state.lines) - 1:
            # Join with next line
            next_line = self.state.lines[self.state.cursor_y + 1]
            self.state.lines[self.state.cursor_y] += next_line
            del self.state.lines[self.state.cursor_y + 1]
            self.state.modified = True

    def insert_newline(self) -> None:
        """Insert a newline at cursor position."""
        line = self.state.lines[self.state.cursor_y]
        self.state.lines[self.state.cursor_y] = line[:self.state.cursor_x]
        self.state.lines.insert(self.state.cursor_y + 1, line[self.state.cursor_x:])
        self.state.cursor_y += 1
        self.state.cursor_x = 0
        self.state.modified = True

    def move_cursor(self, dx: int, dy: int, height: int, width: int) -> None:
        """Move cursor by delta, with bounds checking."""
        # Move vertically
        self.state.cursor_y = max(0, min(len(self.state.lines) - 1, self.state.cursor_y + dy))

        # Move horizontally
        line_len = len(self.state.lines[self.state.cursor_y])
        self.state.cursor_x = max(0, min(line_len, self.state.cursor_x + dx))

        # Adjust scroll to keep cursor visible
        if self.state.cursor_y < self.state.scroll_y:
            self.state.scroll_y = self.state.cursor_y
        elif self.state.cursor_y >= self.state.scroll_y + height:
            self.state.scroll_y = self.state.cursor_y - height + 1

        if self.state.cursor_x < self.state.scroll_x:
            self.state.scroll_x = self.state.cursor_x
        elif self.state.cursor_x >= self.state.scroll_x + width:
            self.state.scroll_x = self.state.cursor_x - width + 1

    def goto_line(self, line_num: int) -> None:
        """Go to a specific line number (1-indexed)."""
        target = max(0, min(len(self.state.lines) - 1, line_num - 1))
        self.state.cursor_y = target
        self.state.cursor_x = 0
        self.state.scroll_y = max(0, target - 5)  # Center the line


def tokenize_line(line: str) -> List[Tuple[str, int]]:
    """Simple tokenizer for syntax highlighting. Returns (token, color_pair)."""
    tokens = []
    i = 0

    while i < len(line):
        # Skip whitespace
        if line[i].isspace():
            tokens.append((line[i], CP_NORMAL))
            i += 1
            continue

        # Comments
        if line[i] == '{':
            end = line.find('}', i)
            if end == -1:
                tokens.append((line[i:], CP_COMMENT))
                break
            tokens.append((line[i:end+1], CP_COMMENT))
            i = end + 1
            continue

        # String literals (single quotes)
        if line[i] == "'":
            j = i + 1
            while j < len(line) and line[j] != "'":
                j += 1
            if j < len(line):
                j += 1
            tokens.append((line[i:j], CP_STRING))
            i = j
            continue

        # Numbers
        if line[i].isdigit():
            j = i
            while j < len(line) and (line[j].isdigit() or line[j] == '.'):
                j += 1
            tokens.append((line[i:j], CP_NUMBER))
            i = j
            continue

        # Identifiers and keywords
        if line[i].isalpha() or line[i] == '_':
            j = i
            while j < len(line) and (line[j].isalnum() or line[j] == '_'):
                j += 1
            word = line[i:j]
            color = CP_KEYWORD if word.lower() in KEYWORDS else CP_NORMAL
            tokens.append((word, color))
            i = j
            continue

        # Operators and punctuation
        tokens.append((line[i], CP_NORMAL))
        i += 1

    return tokens


def draw_editor(win: curses.window, editor: TextEditor, show_errors: bool = True) -> None:
    """Draw the editor content."""
    win.erase()
    height, width = win.getmaxyx()

    state = editor.state
    line_num_width = 5

    # Draw lines
    for i in range(height):
        line_idx = state.scroll_y + i
        if line_idx >= len(state.lines):
            break

        line = state.lines[line_idx]
        is_error_line = line_idx in state.error_lines

        # Line number
        line_num_str = f"{line_idx + 1:4d} "
        try:
            if is_error_line:
                win.addstr(i, 0, line_num_str, curses.color_pair(CP_FAILED) | curses.A_BOLD)
            else:
                win.addstr(i, 0, line_num_str, curses.color_pair(CP_LINE_NUM) | curses.A_DIM)
        except curses.error:
            pass

        # Line content
        visible_line = line[state.scroll_x:state.scroll_x + width - line_num_width]

        if is_error_line and show_errors:
            # Highlight error line
            try:
                win.addstr(i, line_num_width, visible_line.ljust(width - line_num_width),
                          curses.color_pair(CP_ERROR_LINE))
            except curses.error:
                pass
        else:
            # Syntax highlighting
            tokens = tokenize_line(visible_line)
            x = line_num_width
            for token, color in tokens:
                if x >= width:
                    break
                try:
                    win.addstr(i, x, token[:width - x], curses.color_pair(color))
                except curses.error:
                    pass
                x += len(token)

    # Position cursor
    cursor_screen_y = state.cursor_y - state.scroll_y
    cursor_screen_x = line_num_width + state.cursor_x - state.scroll_x

    if 0 <= cursor_screen_y < height and line_num_width <= cursor_screen_x < width:
        try:
            win.move(cursor_screen_y, cursor_screen_x)
        except curses.error:
            pass

    win.refresh()


def draw_output_panel(win: curses.window, output_lines: List[str], scroll: int = 0) -> None:
    """Draw the output/compilation panel."""
    win.erase()
    height, width = win.getmaxyx()

    # Title
    try:
        win.addstr(0, 1, " 编译输出 ", curses.color_pair(CP_TITLE) | curses.A_BOLD)
    except curses.error:
        pass

    # Output lines
    start_idx = max(0, len(output_lines) - height + 1 + scroll)
    end_idx = start_idx + height - 1

    for i, line_idx in enumerate(range(start_idx, min(end_idx, len(output_lines)))):
        if i + 1 >= height:
            break

        line = output_lines[line_idx]
        if len(line) > width - 2:
            line = line[:width - 5] + "..."

        # Color based on content
        color = CP_NORMAL
        if "[ERROR]" in line or "Error" in line:
            color = CP_FAILED
        elif "[WARN]" in line or "Warning" in line:
            color = CP_RUNNING
        elif "[OK]" in line or "Success" in line:
            color = CP_OK

        try:
            win.addstr(i + 1, 1, line, curses.color_pair(color))
        except curses.error:
            pass

    win.refresh()


def draw_status_bar(win: curses.window, editor: TextEditor, message: str = "") -> None:
    """Draw the status bar."""
    win.erase()
    height, width = win.getmaxyx()

    state = editor.state

    # File info
    filename = Path(state.file_path).name if state.file_path else "[New File]"
    modified = "*" if state.modified else ""
    file_info = f" {filename}{modified} "

    # Cursor position
    pos_info = f" Ln {state.cursor_y + 1}, Col {state.cursor_x + 1} "

    # Message or help
    if message:
        help_text = f" {message} "
    else:
        help_text = " F2:Save | F5:Compile | F9:Run | Ctrl+X:Cmd | Ctrl+Q:Quit "

    # Draw status bar
    try:
        win.addstr(0, 0, file_info, curses.color_pair(CP_STATUS_BAR) | curses.A_BOLD)
        win.addstr(0, len(file_info), help_text[:width - len(file_info) - len(pos_info)],
                  curses.color_pair(CP_STATUS_BAR))
        win.addstr(0, width - len(pos_info), pos_info, curses.color_pair(CP_STATUS_BAR) | curses.A_BOLD)
    except curses.error:
        pass

    win.refresh()


def compile_file(file_path: str, project_root: str) -> CompileResult:
    """Compile a Pascal file and return results."""
    pascc_bin = os.path.join(project_root, "build", "pascc")
    if not os.path.exists(pascc_bin):
        pascc_bin = "pascc"

    output_file = file_path.rsplit(".", 1)[0] + ".c"
    cmd = [pascc_bin, "-i", file_path, "-o", output_file]

    try:
        result = subprocess.run(
            cmd,
            cwd=project_root,
            capture_output=True,
            text=True,
            timeout=10
        )

        output = result.stdout + result.stderr
        errors = []

        # Parse errors (format: "Error at Line:Col - message")
        for line in output.split('\n'):
            m = re.match(r'Error at (\d+):(\d+) - (.*)', line)
            if m:
                errors.append((int(m.group(1)), int(m.group(2)), m.group(3)))

        return CompileResult(
            success=(result.returncode == 0),
            output=output,
            errors=errors
        )
    except subprocess.TimeoutExpired:
        return CompileResult(success=False, output="Compilation timeout")
    except Exception as e:
        return CompileResult(success=False, output=f"Error: {str(e)}")


def run_compiled_file(c_file: str, project_root: str) -> str:
    """Compile C file with gcc and run it."""
    exe_file = c_file.rsplit(".", 1)[0]

    # Compile with gcc
    gcc_cmd = ["gcc", "-o", exe_file, c_file, "-lm"]
    try:
        result = subprocess.run(gcc_cmd, capture_output=True, text=True, timeout=10)
        if result.returncode != 0:
            return f"GCC compilation failed:\n{result.stderr}"
    except Exception as e:
        return f"GCC error: {str(e)}"

    # Run the executable
    try:
        result = subprocess.run([exe_file], capture_output=True, text=True, timeout=5)
        return f"=== Program Output ===\n{result.stdout}\n{result.stderr}"
    except subprocess.TimeoutExpired:
        return "Program execution timeout"
    except Exception as e:
        return f"Execution error: {str(e)}"


def get_input_dialog(stdscr: curses.window, prompt: str) -> Optional[str]:
    """Show an input dialog and get user input."""
    height, width = stdscr.getmaxyx()

    dialog_h, dialog_w = 5, 50
    dialog_y = (height - dialog_h) // 2
    dialog_x = (width - dialog_w) // 2

    dialog = curses.newwin(dialog_h, dialog_w, dialog_y, dialog_x)
    dialog.box()
    dialog.addstr(1, 2, prompt[:dialog_w - 4])
    dialog.addstr(2, 2, "> ")
    dialog.refresh()

    curses.echo()
    curses.curs_set(1)

    try:
        input_str = dialog.getstr(2, 4, dialog_w - 6).decode('utf-8')
    except:
        input_str = None

    curses.noecho()
    curses.curs_set(1)

    del dialog
    stdscr.touchwin()
    stdscr.refresh()

    return input_str


def main_ide(stdscr: curses.window, initial_file: Optional[str], project_root: str) -> int:
    """Main IDE loop."""
    curses.curs_set(1)
    curses.noecho()
    curses.cbreak()
    stdscr.keypad(True)
    init_colors()

    # Initialize editor
    editor_state = EditorState()
    editor = TextEditor(editor_state)

    if initial_file:
        editor.load_file(initial_file)

    output_lines = ["Welcome to PASCC IDE", "Press F5 to compile, F9 to run"]
    output_scroll = 0
    status_message = ""
    message_time = 0

    while True:
        height, width = stdscr.getmaxyx()

        # Layout: editor (left 2/3) | output (right 1/3)
        editor_width = (width * 2) // 3
        output_width = width - editor_width - 1

        # Create windows
        editor_win = curses.newwin(height - 1, editor_width, 0, 0)
        output_win = curses.newwin(height - 1, output_width, 0, editor_width + 1)
        status_win = curses.newwin(1, width, height - 1, 0)

        # Clear status message after 3 seconds
        if status_message and time.time() - message_time > 3:
            status_message = ""

        # Draw UI (editor last so its cursor position takes effect)
        draw_output_panel(output_win, output_lines, output_scroll)
        draw_status_bar(status_win, editor, status_message)
        draw_editor(editor_win, editor)

        # Handle input
        try:
            key = stdscr.getch()
        except:
            continue

        # Navigation
        if key == curses.KEY_UP:
            editor.move_cursor(0, -1, height - 1, editor_width - 6)
        elif key == curses.KEY_DOWN:
            editor.move_cursor(0, 1, height - 1, editor_width - 6)
        elif key == curses.KEY_LEFT:
            editor.move_cursor(-1, 0, height - 1, editor_width - 6)
        elif key == curses.KEY_RIGHT:
            editor.move_cursor(1, 0, height - 1, editor_width - 6)
        elif key == curses.KEY_HOME:
            editor.state.cursor_x = 0
        elif key == curses.KEY_END:
            editor.state.cursor_x = len(editor.state.lines[editor.state.cursor_y])
        elif key == curses.KEY_PPAGE:  # Page Up
            editor.move_cursor(0, -(height - 2), height - 1, editor_width - 6)
        elif key == curses.KEY_NPAGE:  # Page Down
            editor.move_cursor(0, height - 2, height - 1, editor_width - 6)

        # Editing
        elif key == curses.KEY_BACKSPACE or key == 127 or key == 8:
            editor.delete_char()
        elif key == curses.KEY_DC:  # Delete key
            editor.delete_char_forward()
        elif key == ord('\n') or key == 10:
            editor.insert_newline()
        elif key == 9:  # Tab
            editor.insert_char('    ')
        elif key == 24:  # Ctrl+X - Command mode
            cmd = get_input_dialog(stdscr, "Command (:w=save :q=quit :wq=save&quit)")
            if cmd:
                if cmd in ('w', 'write'):
                    # Save
                    if editor.state.file_path:
                        if editor.save_file():
                            status_message = "File saved"
                            message_time = time.time()
                    else:
                        file_path = get_input_dialog(stdscr, "Save as:")
                        if file_path and editor.save_file(file_path):
                            status_message = f"Saved as {file_path}"
                            message_time = time.time()
                elif cmd in ('q', 'quit'):
                    if editor.state.modified:
                        confirm = get_input_dialog(stdscr, "Unsaved changes! Quit anyway? (yes/no)")
                        if confirm and confirm.lower() in ('y', 'yes'):
                            break
                    else:
                        break
                elif cmd in ('wq', 'x'):
                    # Save and quit
                    if editor.state.file_path:
                        editor.save_file()
                        break
                    else:
                        file_path = get_input_dialog(stdscr, "Save as:")
                        if file_path and editor.save_file(file_path):
                            break
                elif cmd.startswith('w '):
                    # Save as
                    file_path = cmd[2:].strip()
                    if file_path and editor.save_file(file_path):
                        status_message = f"Saved as {file_path}"
                        message_time = time.time()
        elif 32 <= key <= 126:  # Printable characters
            editor.insert_char(chr(key))

        # Commands
        elif key == 19 or key == curses.KEY_F2:  # Ctrl+S or F2 - Save
            if editor.state.file_path:
                if editor.save_file():
                    status_message = "File saved"
                    message_time = time.time()
                else:
                    status_message = "Save failed"
                    message_time = time.time()
            else:
                file_path = get_input_dialog(stdscr, "Save as:")
                if file_path:
                    if editor.save_file(file_path):
                        status_message = f"Saved as {file_path}"
                        message_time = time.time()

        elif key == 15:  # Ctrl+O - Open
            file_path = get_input_dialog(stdscr, "Open file:")
            if file_path and os.path.exists(file_path):
                if editor.load_file(file_path):
                    output_lines.append(f"Opened: {file_path}")
                    status_message = "File loaded"
                    message_time = time.time()

        elif key == 7:  # Ctrl+G - Goto line
            line_str = get_input_dialog(stdscr, "Go to line:")
            if line_str and line_str.isdigit():
                editor.goto_line(int(line_str))

        elif key == 17:  # Ctrl+Q - Quit
            if editor.state.modified:
                # TODO: Add confirmation dialog
                pass
            break

        elif key == curses.KEY_F5:  # F5 - Compile
            if not editor.state.file_path:
                status_message = "Save file first"
                message_time = time.time()
                continue

            # Save before compiling
            editor.save_file()

            output_lines.clear()
            output_lines.append("=== Compiling ===")
            output_lines.append(f"File: {editor.state.file_path}")

            result = compile_file(editor.state.file_path, project_root)

            output_lines.extend(result.output.split('\n'))

            # Update error markers
            editor.state.error_lines.clear()
            for line_num, col_num, msg in result.errors:
                editor.state.error_lines[line_num - 1] = msg
                output_lines.append(f"Error at line {line_num}: {msg}")

            if result.success:
                output_lines.append("[OK] Compilation successful")
                status_message = "Compilation successful"
            else:
                output_lines.append("[FAILED] Compilation failed")
                status_message = "Compilation failed"
                # Jump to first error
                if result.errors:
                    editor.goto_line(result.errors[0][0])

            message_time = time.time()

        elif key == curses.KEY_F9:  # F9 - Run
            if not editor.state.file_path:
                status_message = "Save file first"
                message_time = time.time()
                continue

            c_file = editor.state.file_path.rsplit(".", 1)[0] + ".c"
            if not os.path.exists(c_file):
                status_message = "Compile first (F5)"
                message_time = time.time()
                continue

            output_lines.clear()
            output_lines.append("=== Running ===")
            run_output = run_compiled_file(c_file, project_root)
            output_lines.extend(run_output.split('\n'))
            status_message = "Program executed"
            message_time = time.time()

        # Scroll output panel
        elif key == ord('+') or key == ord('='):
            output_scroll = min(0, output_scroll + 1)
        elif key == ord('-') or key == ord('_'):
            output_scroll = max(-(len(output_lines) - 10), output_scroll - 1)

    return 0


def main() -> int:
    """Entry point."""
    if not sys.stdout.isatty():
        print("This program requires an interactive terminal.", file=sys.stderr)
        return 2

    project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

    initial_file = None
    if len(sys.argv) > 1:
        initial_file = sys.argv[1]
        if not os.path.isabs(initial_file):
            initial_file = os.path.join(project_root, initial_file)

    try:
        return curses.wrapper(main_ide, initial_file, project_root)
    except KeyboardInterrupt:
        print("\nExited.")
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
