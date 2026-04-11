  # Repository Guidelines

  ## Project Structure & Modules
  Source lives under `sp-altera-src-2025-11-30/`. Hardware descriptions for ACEX
  variants are inside `altera/acex/K30` and `altera/acex/K50`, MAX 7000 builds live
  in `altera/max`. Utilities (checksum, disk tools, converters) are shipped in `bin/
  `. Keep original vendor archives untouched under `altera/*/orig` when adding new
  bitstreams or reference data.

  ## Build, Test, and Development Commands
  Run ACEX builds on Windows via `altera/acex/make.cmd`, which copies the selected
  chip directory, runs Quartus/Max+II compilation, and emits `SP2_ACEX.sof` and
  `STREAM.BIN`. For MAX devices use `altera/max/make.cmd`. Post-build verification
  relies on `bin/make_num.exe <file>` to report size and running checksum before
  distributing new binaries.

  ## Coding Style & Naming
  TDF/ACF modules follow uppercase identifiers for exported pins and PascalCase for
  parameters (`MODE`, `ScreenOff`). Protect include filenames (`*.INC`, `*.MIF`)
  from renaming; cross references are case-sensitive on Windows. Batch and cmd
  scripts should remain CRLF and use `set VAR=value` with uppercase var names. For
  C utilities prefer K&R style braces as in `bin/make_num.c` and keep `printf`-based
  logging.

  ## Testing Guidelines
  Regression tests are manual: run `bin/make_num.exe` on every new bitstream plus
  a known-good artifact to ensure identical checksums. For video or AY blocks,
  flash to a board and confirm keyboard/mouse paths by toggling modes documented
  in `SP2_ACEX.TDF`. Record test notes in `compile.log` so future agents understand
  context.

  ## Commit & Pull Request Expectations
  Use imperative short commits (`Add K50 keyboard fix`) followed by context in the
  body (chip variant, tool version). Reference tracker IDs when available and attach
  build logs plus generated `.sof/.pof` artifacts for review. PR descriptions should
  outline affected modules, commands run, and hardware tested; include screenshots or
  oscilloscope captures if timing/fmax changes are claimed.