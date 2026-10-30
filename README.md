# C++ Log Analyzer

This repository contains the planned foundation for an offline C++17 command-line
log analyzer. The project is intentionally being implemented incrementally.

## Phase 0: Domain contract

The following decisions are fixed for version 1:

- A record has the form
  `YYYY-MM-DD HH:MM:SS|SEVERITY|COMPONENT|MESSAGE`.
- Timestamps are timezone-unspecified diagnostic timestamps. The application does
  not convert them to local time or UTC.
- Calendar dates and clock values are validated strictly.
- Severity is one of `INFO`, `WARN`, or `ERROR`.
- The first three pipe characters are structural separators. Additional pipes
  remain part of the message.
- Components and messages must not be empty. A blank line is a malformed record.
- A trailing pipe after the third separator is preserved as part of the message.
- Lenient reading is the default: malformed records are skipped and reported.
  `--strict` stops processing at the first malformed record.
- Repeated errors are identical message strings from `ERROR` records that occur
  more than once. No fuzzy matching or root-cause inference is performed.
- Repeated CLI options are rejected. Filtering options may be combined.
- An empty file is valid and produces an empty analysis. Lenient input with no
  valid records produces an empty analysis plus diagnostics.

The timestamp contract deliberately does not assign a timezone because these
logs are diagnostic records rather than instants intended for conversion.

## Planned structure

```text
include/                 Public domain and processing interfaces
src/                     Core implementation and CLI entry point
tests/                   Google Test unit and integration tests
samples/                 Representative input logs
.github/workflows/       Continuous integration
```

## Build prerequisites

- CMake 3.16 or newer
- A C++17-capable GCC or Clang toolchain
- Internet access on the first configure if Google Test is not installed
  locally; CMake will fetch the pinned Google Test release

The initial implementation targets Ubuntu Linux or WSL2. The commands below
are also suitable for a native Linux environment.

## Configure, build, and test

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

On a multi-configuration generator such as Visual Studio, specify the
configuration when building and testing:

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

The domain model, parser, file reader, analysis engine, report formatter, and
CLI orchestration are implemented.

The analysis engine supports severity and component filters, counts records by
severity, counts errors by component, detects repeated error messages, and
calculates the timestamp range. The formatter renders those results and reader
diagnostics in deterministic text output.

## CLI usage

```text
log_analyzer --input PATH [options]
```

Available options:

- `--input PATH` (required): input log file
- `--severity INFO|WARN|ERROR`: include only one severity
- `--component NAME`: include only one component
- `--strict`: stop at the first malformed record
- `--output PATH`: write the report to a file instead of standard output
- `--help`: print usage information

In lenient mode, malformed records are omitted from analysis and included in
the report's parse diagnostics section. Fatal input/output failures and strict
validation failures are written to standard error.

Example:

```bash
./build/log_analyzer --input samples/sample.log --severity ERROR
```

## Quality checks

The project enables `-Wall -Wextra -Wpedantic` for GCC and Clang, and `/W4` for
MSVC. Compilation commands are exported to `compile_commands.json`.

On GCC or Clang, configure a sanitizer build separately:

```bash
cmake -S . -B build-sanitized \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_SANITIZERS=ON
cmake --build build-sanitized
ctest --test-dir build-sanitized --output-on-failure
```

If `clang-tidy` is installed, enable it during configuration:

```bash
cmake -S . -B build-tidy \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_CLANG_TIDY=ON
cmake --build build-tidy
```

`ENABLE_SANITIZERS` intentionally fails on unsupported compilers instead of
silently producing an unsanitized build. `ENABLE_CLANG_TIDY` similarly requires
`clang-tidy` to be available.

## Debugging

The supported debugging environment is Linux or WSL2 with GDB. A reproducible
debug session can be started after a Debug build with:

```bash
gdb --args ./build/log_analyzer \
  --input samples/sample.log \
  --severity ERROR
```

Useful first commands are:

```text
break main
run
backtrace
quit
```

No application defect was discovered during the current Windows validation
run; therefore this phase does not claim a GDB defect fix. GDB investigation
should be recorded here when a reproducible defect is found on Linux or WSL2.

## Continuous integration

GitHub Actions runs the documented configure, build, and CTest commands for
pushes and pull requests using Ubuntu. The workflow is located at
`.github/workflows/ci.yml`.

## Exit codes

The application contract reserves these exit codes:

- `0`: successful execution
- `1`: fatal input/output or processing failure
- `2`: invalid command-line usage
- `3`: strict-mode data validation failure

## Scope limitations

Version 1 is an offline diagnostic utility using simulated logs. It does not
provide hardware, ECU, RTOS, vehicle, networking, database, GUI, multithreading,
plugin, or external AI integration.
