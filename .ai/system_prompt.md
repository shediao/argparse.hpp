This Content provides guidance to AI.CLI Coder when working with code in this repository.

## Build & Test Commands

```sh
# Configure with tests enabled (the default when this is the top-level project)
cmake -DARGPARSE_BUILD_TESTS=ON -B build -S .

# Build
cmake --build build

# Run all tests
ctest --test-dir build

# Run a single test by name
ctest --test-dir build -R <test_name>

# Run a single test directly (faster for iteration)
./build/tests/<test_name>
```

The `all_test` target compiles every test source into one binary — useful for a full sweep. Individual test binaries exist for each `.cc` file in `tests/`.

Formatting is enforced via `.clang-format` (Google-derived, 80-column limit, 2-space indent, AlwaysBreakBeforeMultilineStrings).

The `.clangd` configuration file enforces `-std=c++20 -Wall` for IDE/LSP support (completion, diagnostics, etc.).

## Cross-Compilation

> **Note:** Cross-compilation environment is currently only available on macOS and Linux platforms.

When directories matching `build/{platform}-{arch}` exist (e.g., `build/darwin-arm64`, `build/linux-x86_64`, `build/mingw64-x86_64`, `build/windows-arm64`), the cross-compilation environment is already configured. Build directly with:

```bash
cmake --build build/{platform}-{arch}
```

Supported build directories:

| Directory              | Target                 |
| ---------------------- | ---------------------- |
| `build/darwin-arm64`   | macOS ARM64            |
| `build/darwin-x86_64`  | macOS x86_64           |
| `build/linux-arm64`    | Linux ARM64            |
| `build/linux-x86_64`   | Linux x86_64           |
| `build/mingw64-x86_64` | Windows x86_64 (MinGW) |
| `build/windows-arm64`  | Windows ARM64 (MSVC)   |
| `build/windows-x86_64` | Windows x86_64 (MSVC)  |

When source files are added/removed or CMake options change (making it necessary to re-run CMake configuration), simply `touch CMakeLists.txt` and the next `cmake --build` will automatically re-generate the build system:

```bash
touch CMakeLists.txt
cmake --build build/{platform}-{arch}
```

## Pre-Commit Hook

The `.git/hooks/pre-commit` script automatically formats staged files before each commit:

- **C/C++ files** (`.h`, `.hpp`, `.cpp`, `.cc`, `.cxx`) — formatted with `clang-format`
- **CMake files** (`CMakeLists.txt`, `*.cmake`) — formatted with `cmake-format`

Formatted files are automatically re-staged. If either tool is not installed, the hook skips that formatter with a warning.

## CI

If the remote repository URL is `http://github.com/*` or `git@github.com:*`, then GitHub Actions (configuration files located at `.github/workflows/*.yml`) is used as CI.

Browse and manage GitHub Actions via the `gh` command:

```bash
# View recent workflow runs
gh run list

# View details of a specific run
gh run view <run-id>

# View logs of a specific run
gh run view <run-id> --log

# Manually trigger a workflow
gh workflow run <workflow-name>

# List all workflows
gh workflow list

# View workflow file contents
gh workflow view <workflow-name>
```

**When the user wants to resolve GitHub Actions failures, they should first use `gh` commands to get logs and analyze the problem**, rather than blindly guessing the cause. How to get logs:

```bash
# Get logs of the latest run (usually the failed run)
gh run list --limit 1 --json databaseId -q '.[].databaseId' | xargs gh run view --log

# Get logs of a specific run-id (including failed steps)
gh run view <run-id> --log

# Get logs of the run corresponding to a specific commit
gh run list --commit <commit-sha> --limit 1 --json databaseId -q '.[].databaseId' | xargs gh run view --log

# Only view failed runs
gh run list --status failure --limit 5

# View logs of a failed job in a run (if the run has multiple jobs)
gh run view <run-id> --log --job <job-id>
```

**When fetching logs with `gh` commands, always prefer getting only the key information rather than the full logs unless absolutely necessary.** For example:

- Use `gh run view <run-id> --log-failed` to get only failed step logs (if supported).
- Pipe through `grep` to filter for error messages, failed test names, or compilation errors: `gh run view <run-id> --log 2>&1 | grep -E '(error:|FAILED|failure)'`.
- For test failures, look for the specific test output rather than the entire build log.
- Only fetch full logs when the filtered output does not provide enough context to diagnose the issue.

After getting the logs, identify the specific failure cause based on the error messages in the logs (compilation errors, test failures, environment issues, etc.), then make targeted fixes.

**Fix Verification**: If the failure occurs on a platform different from the current machine (e.g., currently on macOS ARM64, but the failure is on Linux x86_64), and a corresponding cross-compilation environment `build/{platform}-{arch}/` exists locally, after fixing, you **must** verify the build through that cross-compilation environment to ensure the fix also passes on the target platform:

```bash
# For example: fixed a compilation error on Linux x86_64, with build/linux-x86_64/ available locally
cmake --build build/linux-x86_64
```

If the corresponding cross-compilation environment does not exist locally, just push the fix directly and let CI verify.

### CI Platforms

The CI matrix covers these targets across two workflow files:

| Workflow                   | Platforms                                                                                                                             |
| -------------------------- | ------------------------------------------------------------------------------------------------------------------------------------- |
| `cmake-multi-platform.yml` | Ubuntu (gcc, clang), Windows (MSVC, clang), macOS (clang) — plus **FreeBSD**, **OpenBSD**, and **NetBSD** VMs via `vmactions/*-vm@v1` |
| `msys2.yml`                | Windows MSYS2 (MSYS, UCRT64, MINGW64) with gcc and clang                                                                              |

All platforms run both `Release` and `Debug` builds with `ARGPARSE_BUILD_TESTS=ON`.

## Architecture

This is a **single-header, C++20, header-only** library. The entire implementation lives in `include/argparse/argparse.hpp` (~5500 lines). There are no `.cpp` files.

### Header Layout (top to bottom)

1. **Internal `expected<T,E>` implementation** (~lines 80–1795): A self-contained `std::expected`-like type used for monadic error handling throughout. Includes `unexpected<E>`, `bad_expected_access<E>`, `expected_storage<T,E>`, the main `expected<T,E>` class template, and a partial specialization `expected<T&,E>`. This is used internally but is also part of the public API.

2. **`detail` namespace utilities** (~lines 1797–2100): String helpers (`split`, `join`, `split_to_if`), `to_string`/`to_wstring` overloads, the option-name-width global, and `die`/`report_invalid_argument` error functions.

3. **Type traits and concepts** (~lines 1874–2100+): `is_optional`, `is_string`, `is_string_view`, extract-value-type machinery, and concepts like `can_from_string_without_delim` that gate which types can be bound to options/positionals.

4. **Schema classes** (~lines 2493–2916): `FlagSchema`, `OptionSchema`, `PositionalSchema`, `CommandSchema` — lightweight data objects holding names, descriptions, choices, ranges, validators, and default values. Schema objects are shared via `shared_ptr`.

5. **ArgBase hierarchy** (~lines 2917–3735):
   - `ArgBase` — virtual base for all argument types.
   - `FlagBase : ArgBase` — base for flags, holding `short_names`/`long_names` vectors and action callbacks.
   - `Flag<T> : FlagBase` — concrete flag, parametrized on the bound variable type.
   - `OptionBase : ArgBase` — base for options/positionals with schema-backed choices, ranges, validators.
   - `OptionBaseCRTP<T> : OptionBase` — CRTP layer returning derived references for chaining `.default_value()`, `.choices()`, `.range()`, `.validator()`, `.callback()`, `.env()`, `.hidden()`, `.value_placeholder()`, `.negatable()`.
   - `Option<T> : OptionBaseCRTP<Option<T>>` — concrete option.
   - `Positional<T> : OptionBaseCRTP<Positional<T>>` — concrete positional argument.
   - `OptionAlias : FlagBase` — an alias that looks like a flag but sets an underlying option to a predefined value.

6. **Tokenization** (~lines 3736–3855): `token_kind` enum, `token` struct, `token_stream` class. Handles `--`, `--opt=value` splitting, short-flag grouping (`-abc`), and recognition of known vs. unknown options. The tokenizer is the lowest-level parsing layer.

7. **`Command` class** (~lines 3856–4620): The core orchestrator. Owns `command_schema_` and a `vector<unique_ptr<ArgBase>> args_`. Provides `add_flag()`, `add_option()`, `add_positional()`, `add_alias()`, `add_command()` (for subcommands). Contains the `parse()` implementation that drives the tokenizer, dispatches to `Flag::parse()`, `Option::parse()`, `Positional::parse()`. Also contains help/usage formatting and shell completion script generation methods.

8. **`ArgParser : public Command`** (~lines 4621+): Thin subclass of `Command`. Adds the public `parse(argc, argv)` entry point (with `argv[0]`-skipping logic), `parse(vector<string>)`, Windows wide-string parse overloads, the top-level `usage()` override that includes subcommand listings, and `add_command()` (which creates and returns a child `Command`).

### Key Design Decisions

- **Schema/instance separation**: Each argument definition has a `shared_ptr<Schema>` (shared so aliases and subcommand parents can reference the same schema) and an `ArgBase` instance that owns the bound variable reference and parsing logic. This allows `parse_option_name()` to validate names across all existing definitions before creating the argument object.
- **Monadic error handling**: `parse_option_name()` and the flag/option construction chain use `expected<T, string>` with `.and_then()` chaining, avoiding exceptions for internal control flow.
- **CRTP for chaining**: `OptionBaseCRTP` enables fluent modifiers (`.default_value(...)`, `.choices(...)`, `.range(...)`) to return the concrete derived type reference.
- **Subcommands**: Subcommands are child `Command` objects stored in `ArgParser`. They cannot coexist with positional arguments. `ArgParser::parse()` returns a reference to the active `Command` (main or subcommand).
- **No exceptions for normal parse flow**: `--help`/`--version` use `exit(0)`. Validation failures throw `std::runtime_error` or `std::invalid_argument`.
- **CMake alias target**: `CMakeLists.txt` defines `argparse::argparse` as an alias for the `argparse` library target, for downstream consumers using `target_link_libraries(... argparse::argparse)`.

### Tests

Tests use Google Test (fetched via CMake FetchContent). Each `.cc` file in `tests/` becomes a test executable. An additional `all_test` target compiles all test files together. Tests also depend on `subprocess.hpp` (another header-only library) for testing subprocess behavior like `--help` exit codes.

Key test files:

- `argparse_unittest.cc` — basic flag/option/positional parsing, error cases, default help flag
- `parse_test.cc` — comprehensive parse scenarios including environment variable handling
- `optional_bind_test.cc` — `std::optional<T>` binding behavior
- `checker_test.cc` — validation with `.choices()`, `.range()`, `.validator()`
- `callback_test.cc` — callback invocation
- `env_test.cc` — environment variable fallback
- `complete_test.cc` — shell completion script generation
- `unicode_test.cc` — wide-string and Unicode handling
- `custom_type_test.cc` — user-defined types constructible from string
- `remain_positionals_test.cc` — positional argument ordering and variadic behavior
- `expected_test.cc` — internal `expected<T,E>` type correctness
- `gnucore_example_usage_test.cc` — GNU-style coreutils usage patterns
- `concept_test.cc` — C++20 concept and type-trait gating
- `parser_move_test.cc` — parser move semantics
- `fromstring_test.cc` — `from_string` conversion

# Ai Coding Guidelines

Behavioral guidelines to reduce common LLM coding mistakes. Merge with project-specific instructions as needed.

**Tradeoff:** These guidelines bias toward caution over speed. For trivial tasks, use judgment.

## 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:

- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

## 2. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

## 3. Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:

- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it - don't delete it.

When your changes create orphans:

- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.

## 4. Goal-Driven Execution

**Define success criteria. Loop until verified.**

Transform tasks into verifiable goals:

- "Add validation" → "Write tests for invalid inputs, then make them pass"
- "Fix the bug" → "Write a test that reproduces it, then make it pass"
- "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:

```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria ("make it work") require constant clarification.

---

**These guidelines are working if:** fewer unnecessary changes in diffs, fewer rewrites due to overcomplication, and clarifying questions come before implementation rather than after mistakes.
