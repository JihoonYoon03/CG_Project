"""Format project code through UTF-8 while keeping source files in CP949."""

import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess


ROOT = Path(__file__).resolve().parent.parent


def find_formatter():
    installed = shutil.which("clang-format")
    program_files = Path(os.environ.get("ProgramFiles", r"C:\Program Files"))
    candidates = list(
        program_files.glob("Microsoft Visual Studio/*/*/VC/Tools/Llvm/x64/bin/clang-format.exe")
    )
    candidates.append(program_files / "LLVM/bin/clang-format.exe")
    if installed:
        candidates.append(Path(installed))
    available = []
    for candidate in candidates:
        if not candidate.is_file():
            continue
        version = subprocess.check_output([str(candidate), "--version"], text=True)
        number = re.search(r"version (\d+)\.(\d+)\.(\d+)", version)
        if number:
            available.append((tuple(map(int, number.groups())), str(candidate)))
    if available:
        return max(available)[1]
    raise FileNotFoundError("Install clang-format or specify --formatter PATH")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Check without writing files")
    parser.add_argument("--formatter", help="Path to clang-format executable")
    args = parser.parse_args()
    formatter = args.formatter or find_formatter()

    sources = [ROOT / name for name in ("main.cpp", "pch.cpp", "pch.h")]
    for folder in ("Engine", "Game", "Tests", "Shaders"):
        sources.extend(
            path for path in sorted((ROOT / folder).iterdir())
            if path.suffix in (".h", ".cpp", ".vert", ".frag")
        )

    changes = []
    for path in sources:
        original = path.read_bytes()
        source = original.decode("cp949")
        # Treat GLSL as C-like code. Never send legacy-encoded bytes to the formatter.
        assumed_path = path.with_suffix(".cpp") if path.suffix in (".vert", ".frag") else path
        result = subprocess.run(
            [formatter, "--style=file", f"--assume-filename={assumed_path}"],
            input=source.encode("utf-8"),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=ROOT,
            check=False,
        )
        if result.returncode:
            print(result.stderr.decode("utf-8", errors="replace"))
            return result.returncode
        formatted = result.stdout.decode("utf-8").encode("cp949")
        if formatted != original:
            changes.append((path, formatted))

    # Finish all conversions before touching files so encoding errors stop cleanly.
    if args.check:
        for path, _ in changes:
            print(f"Needs formatting: {path.relative_to(ROOT)}")
        if changes:
            return 1
        print(f"All {len(sources)} source files match .clang-format (CP949 preserved).")
    else:
        for path, formatted in changes:
            path.write_bytes(formatted)
        print(f"Formatted {len(changes)} of {len(sources)} source files; CP949 preserved.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
