#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CLOX_BIN="$ROOT_DIR/clox"

if [[ ! -x "$CLOX_BIN" ]]; then
  echo "clox binary not found. Run: make clox"
  exit 1
fi

PASS=0
FAIL=0
TMP_STDOUT="/tmp/clox_example_stdout.txt"
TMP_STDERR="/tmp/clox_example_stderr.txt"
trap 'rm -f "$TMP_STDOUT" "$TMP_STDERR"' EXIT

run_case() {
  local file="$1"
  local expected="$2"
  local input_data="${3:-}"

  local rc
  if [[ -n "$input_data" ]]; then
    rc=0
    timeout 20s "$CLOX_BIN" "$ROOT_DIR/$file" <<<"$input_data" >"$TMP_STDOUT" 2>"$TMP_STDERR" || rc=$?
  else
    rc=0
    timeout 20s "$CLOX_BIN" "$ROOT_DIR/$file" </dev/null >"$TMP_STDOUT" 2>"$TMP_STDERR" || rc=$?
  fi

  local ok=0
  case "$expected" in
    ok)
      [[ "$rc" -eq 0 ]] && ok=1
      ;;
    compile_error)
      [[ "$rc" -eq 65 ]] && ok=1
      ;;
    runtime_error)
      [[ "$rc" -eq 70 ]] && ok=1
      ;;
    nonzero)
      [[ "$rc" -ne 0 ]] && ok=1
      ;;
  esac

  if [[ "$ok" -eq 1 ]]; then
    echo "PASS $file (exit=$rc expected=$expected)"
    PASS=$((PASS + 1))
  else
    echo "FAIL $file (exit=$rc expected=$expected)"
    if [[ -s "$TMP_STDOUT" ]]; then
      echo "[stdout]"
      cat "$TMP_STDOUT"
    fi
    if [[ -s "$TMP_STDERR" ]]; then
      echo "[stderr]"
      cat "$TMP_STDERR"
    fi
    FAIL=$((FAIL + 1))
  fi
}

run_case "examples/anonymous_functions.lox" "ok"
run_case "examples/classes.lox" "ok"
run_case "examples/closures.lox" "ok"
run_case "examples/control_flow.lox" "ok"
run_case "examples/example_fibonacci.lox" "ok"
run_case "examples/example_quiz.lox" "ok" $'4\nParis\nno'
run_case "examples/example_user_db.lox" "ok" $'Alice\nalice@example.com\nBob\nbob@example.com'
run_case "examples/fibonacci.lox" "ok"
run_case "examples/functions.lox" "ok"
run_case "examples/inheritance.lox" "ok"
run_case "examples/loops.lox" "ok"
run_case "examples/native_input.lox" "ok" $'24\nyes'
run_case "examples/native_lox.lox" "ok"
run_case "examples/native_scan.lox" "ok" $'Aymane'
run_case "examples/operators.lox" "ok"
run_case "examples/static_methods.lox" "ok"
run_case "examples/super.lox" "ok"

run_case "examples/errors/parse_error.lox" "compile_error"
run_case "examples/errors/runtime_error_division_by_zero.lox" "runtime_error"
run_case "examples/errors/runtime_error_undefined_variable.lox" "runtime_error"
run_case "examples/errors/type_error.lox" "runtime_error"

echo "Summary: pass=$PASS fail=$FAIL total=$((PASS + FAIL))"

if [[ "$FAIL" -ne 0 ]]; then
  exit 1
fi
