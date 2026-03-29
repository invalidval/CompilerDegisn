#!/usr/bin/env bash
set -euo

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

if [[ ! -x ./build/pascc ]]; then
  echo "[info] build/pascc not found, running build first"
   >/dev/null
fi

run_lex_ok() {
  local file="$1"
  ./build/pascc -i "$file" --lex > /tmp/pascc_lex_stdout.txt 2>/tmp/pascc_lex_stderr.txt
}

run_lex_err() {
  local file="$1"
  set +e
  ./build/pascc -i "$file" --lex > /tmp/pascc_lex_stdout.txt 2>/tmp/pascc_lex_stderr.txt
  local ec=$?
  set -e
  if [[ $ec -eq 0 ]]; then
    echo "[fail] expected non-zero exit for $file"
    return 1
  fi
}

assert_contains() {
  local file="$1"
  local pattern="$2"
  if ! grep -E "$pattern" "$file" >/dev/null; then
    echo "[fail] pattern not found: $pattern"
    echo "[fail] file: $file"
    return 1
  fi
}

echo "[test] keyword case-insensitive"
run_lex_ok test/cases/valid/v02_keyword_mixedcase.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Keyword, ProGram, 1, 1$'

echo "[test] reserved words recognized as keywords"
run_lex_ok test/cases/valid/minimal.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Keyword, var, 2, 1$'
assert_contains /tmp/pascc_lex_stdout.txt '^Keyword, begin, 3, 1$'
assert_contains /tmp/pascc_lex_stdout.txt '^Keyword, integer, 2, 8$'

echo "[test] number recognition"
run_lex_ok test/cases/valid/v09_real_basic.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Number, 3\.1415, 4, 8$'

echo "[test] scientific notation recognition"
run_lex_ok test/cases/valid/v15_scientific_integer.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Number, 1e10, 4, 8$'

run_lex_ok test/cases/valid/v16_scientific_signed_exp.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Number, 2E-3, 4, 8$'

run_lex_ok test/cases/valid/v20_real_no_leading_zero.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Number, \.5, 4, 8$'

run_lex_ok test/cases/valid/v21_hex_dollar.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Number, \$FF, 4, 8$'

run_lex_ok test/cases/valid/v22_hex_0x.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Number, 0x1A, 4, 8$'

echo "[test] character recognition"
run_lex_ok test/cases/valid/v11_char_letter.pas
assert_contains /tmp/pascc_lex_stdout.txt "^Character, 'a', 4, 8$"

echo "[test] string recognition"
run_lex_ok test/cases/valid/v17_string_basic.pas
assert_contains /tmp/pascc_lex_stdout.txt '^String, "hi", 3, 9$'

echo "[test] comment skip with line tracking"
run_lex_ok test/cases/valid/v14_comment_brace_multiline.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Keyword, begin, 4, 1$'

echo "[test] nested comment support"
run_lex_ok test/cases/valid/v18_comment_nested.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Keyword, begin, 3, 1$'

echo "[test] directive skip support"
run_lex_ok test/cases/valid/v19_directive_basic.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Keyword, begin, 3, 1$'

echo "[test] lexical error illegal character"
run_lex_err test/cases/invalid/i01_illegal_character_at.pas
assert_contains /tmp/pascc_lex_stderr.txt "Lexical error: illegal character"
assert_contains /tmp/pascc_lex_stderr.txt '^Error at 3:3 - '

echo "[test] malformed number"
run_lex_err test/cases/invalid/i03_malformed_number_multiple_dots.pas
assert_contains /tmp/pascc_lex_stderr.txt "Lexical error: malformed number"

echo "[test] unterminated comment"
run_lex_err test/cases/invalid/i07_unterminated_comment_brace.pas
assert_contains /tmp/pascc_lex_stderr.txt "Lexical error: unterminated comment"

echo "[test] unterminated string"
run_lex_err test/cases/invalid/i08_unterminated_string_literal.pas
assert_contains /tmp/pascc_lex_stderr.txt "Lexical error: unterminated string literal"

echo "[test] unterminated directive"
run_lex_err test/cases/invalid/i09_unterminated_directive.pas
assert_contains /tmp/pascc_lex_stderr.txt "Lexical error: unterminated directive"

echo "[test] malformed hex number"
run_lex_err test/cases/invalid/i10_malformed_hex_number.pas
assert_contains /tmp/pascc_lex_stderr.txt "Lexical error: malformed number"

echo "[test] empty string recognition"
run_lex_ok test/cases/valid/v23_string_empty.pas
assert_contains /tmp/pascc_lex_stdout.txt '^String, "", 3, 9$'

echo "[test] escaped string recognition"
run_lex_ok test/cases/valid/v24_string_escape.pas
assert_contains /tmp/pascc_lex_stdout.txt '^String, "a'
assert_contains /tmp/pascc_lex_stdout.txt '3, 9$'

echo "[test] hex boundary recognition"
run_lex_ok test/cases/valid/v25_hex_boundary.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Number, 0x0, 4, 8$'
assert_contains /tmp/pascc_lex_stdout.txt '^Number, 0xFFFFFFFF, 5, 8$'
assert_contains /tmp/pascc_lex_stdout.txt '^Number, \$0, 6, 8$'
assert_contains /tmp/pascc_lex_stdout.txt '^Number, \$FFFF, 7, 8$'

echo "[test] scientific boundary recognition"
run_lex_ok test/cases/valid/v26_scientific_boundary.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Number, 1e308, 4, 8$'
assert_contains /tmp/pascc_lex_stdout.txt '^Number, 1e-309, 5, 8$'

echo "[test] comment and string mixed cases"
run_lex_ok test/cases/valid/v27_comment_mixed_cases.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Keyword, begin, 4, 1$'
assert_contains /tmp/pascc_lex_stdout.txt '^String, "\{ comment inside string \}", 5, 9$'

echo "[test] extended directives support"
run_lex_ok test/cases/valid/v28_directives_extended.pas
assert_contains /tmp/pascc_lex_stdout.txt '^Keyword, begin, 4, 1$'

echo "[test] malformed hex alphabet"
run_lex_err test/cases/invalid/i11_malformed_hex_alpha.pas
assert_contains /tmp/pascc_lex_stderr.txt "Lexical error: malformed number"

echo "[test] incomplete scientific notation"
run_lex_err test/cases/invalid/i12_invalid_scientific_incomplete.pas
assert_contains /tmp/pascc_lex_stderr.txt "Lexical error: malformed number"

echo "[test] extended unterminated directive"
run_lex_err test/cases/invalid/i13_unterminated_directive_extended.pas
assert_contains /tmp/pascc_lex_stderr.txt "Lexical error: unterminated directive"

echo "[test] invalid directive syntax"
run_lex_err test/cases/invalid/i14_invalid_directive_syntax.pas
assert_contains /tmp/pascc_lex_stderr.txt "Lexical error: invalid directive"

echo "[pass] lexer unit tests passed"
