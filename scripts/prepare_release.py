#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import datetime as _dt
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Optional, Sequence, Union

# ---------- Paths ----------
REPO_ROOT = Path(__file__).resolve().parents[1]
VERSION_FILE = REPO_ROOT / "APP_VERSION"
APPVERSION_HEADER = REPO_ROOT / "src/appversion.h"
DOXYFILE = REPO_ROOT / "Doxyfile"
CHANGELOG = REPO_ROOT / "CHANGELOG.md"
PATCH_REFMAN = REPO_ROOT / "manual" / "patch_refman_title.sh"
LATEX_DIR = REPO_ROOT / "docs" / "latex"
DEVELOPER_MANUAL = REPO_ROOT / "manual" / "SyntaxTutor-Developer-Manual.pdf"

SEMVER_RE = re.compile(r"^\d+(?:\.\d+){1,3}$")


# ---------- CLI ----------
def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    today = _dt.date.today().isoformat()
    parser = argparse.ArgumentParser(
        description="Prepare a release."
    )
    parser.add_argument("version", help="Release version (p. ej. 1.1.0)")
    parser.add_argument(
        "--date",
        default=today,
        help="Release date for CHANGELOG and manual (ISO, today by default)",
    )
    parser.add_argument(
        "--skip-docs",
        action="store_true",
        help="Skip docs and developer manual generation",
    )
    parser.add_argument(
        "--skip-changelog",
        action="store_true",
        help="Skip CHANGELOG entry generation",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Simulate actions without writing to any file",
    )
    return parser.parse_args(argv)


# ---------- Tools ----------
def validate_version(version: str) -> None:
    if not SEMVER_RE.match(version):
        raise SystemExit(
            f"Invalid version number '{version}'"
        )


def write_file(path: Path, content: str, dry_run: bool) -> None:
    if dry_run:
        print(f"[dry-run] Write in {path}:")
        for line in content.splitlines():
            print(f"    {line}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")
    print(f"Updated {path.relative_to(REPO_ROOT)}")


def update_version_file(version: str, dry_run: bool) -> None:
    write_file(VERSION_FILE, f"{version}\n", dry_run)


def update_appversion_header(version: str, dry_run: bool) -> None:
    template = f"""#ifndef APPVERSION_H
#define APPVERSION_H

#include <QString>

namespace SyntaxTutor::Version {{
inline QString current() {{
    return QStringLiteral("{version}");
}}

inline const char* raw() {{
    return "{version}";
}}
}} // namespace SyntaxTutor::Version
#endif // APPVERSION_H
"""
    write_file(APPVERSION_HEADER, template, dry_run)


def update_doxyfile(version: str, dry_run: bool) -> None:
    if not DOXYFILE.exists():
        raise RuntimeError(f"{DOXYFILE} not found")
    contents = DOXYFILE.read_text(encoding="utf-8")
    new_contents, count = re.subn(
        r"^(PROJECT_NUMBER\s*=\s*).*$",
        rf"\g<1>{version}",
        contents,
        flags=re.MULTILINE,
    )
    if count == 0:
        raise RuntimeError("PROJECT_NUMBER not found in Doxyfile")
    write_file(DOXYFILE, new_contents, dry_run)


def ensure_changelog_entry(version: str, release_date: str, dry_run: bool) -> None:
    heading = f"## [{version}] - {release_date}"
    if CHANGELOG.exists():
        contents = CHANGELOG.read_text(encoding="utf-8")
    else:
        contents = "# Changelog\n\n"

    if heading in contents:
        print(f"ℹ CHANGELOG already contains {version}")
        return

    entry = (
        f"{heading}\n"
        "### Added\n"
        "- _Description._\n\n"
        "### Changed\n"
        "- _Description._\n\n"
        "### Fixed\n"
        "- _Description._\n\n"
    )

    insert_at = contents.find("## [")
    if insert_at == -1:
        new_contents = contents.rstrip() + "\n\n" + entry
    else:
        new_contents = contents[:insert_at] + entry + contents[insert_at:]

    write_file(CHANGELOG, new_contents, dry_run)


def run_command(
    command: Sequence[Union[str, Path]],
    *,
    cwd: Optional[Path] = None,
    env=None,
    dry_run: bool,
    allow_failure=False
) -> None:
    command = [str(c) for c in command]
    display = " ".join(command)
    location = f" (cwd={cwd})" if cwd else ""
    if dry_run:
        print(f"[dry-run] Run: {display}{location}")
        return

    print(f"> Running: {display}{location}")
    try:
        result = subprocess.run(command, cwd=cwd, env=env)
    except FileNotFoundError as exc:
        raise SystemExit(f"Command not found: {command[0]}") from exc

    if result.returncode != 0:
        if allow_failure:
            print(f"> Warning: command exited with code {result.returncode}, but allow_failure set to true...")
        else:
            raise SystemExit(f"Command failed: {display}{location}")

def _check_tool(cmd: str) -> None:
    if shutil.which(cmd) is None:
        raise SystemExit(f"Requisito no encontrado en PATH: {cmd}")


def build_documentation(version: str, release_date: str, dry_run: bool) -> None:
    _check_tool("doxygen")
    _check_tool("pdflatex")

    run_command(["doxygen", "Doxyfile"], cwd=REPO_ROOT, dry_run=dry_run)

    env = os.environ.copy()
    env["SYNTAXTUTOR_VERSION"] = version
    env["SYNTAXTUTOR_RELEASE_DATE"] = release_date

    if not PATCH_REFMAN.exists():
        raise SystemExit(f"Not found {PATCH_REFMAN}")
    if os.name != "nt" and not os.access(PATCH_REFMAN, os.X_OK):
        if dry_run:
            print(f"[dry-run] Make it executable {PATCH_REFMAN}")
        else:
            PATCH_REFMAN.chmod(PATCH_REFMAN.stat().st_mode | 0o111)

    run_command([PATCH_REFMAN], cwd=REPO_ROOT, env=env, dry_run=dry_run)

    run_command(
        ["pdflatex", "-interaction=nonstopmode", "refman.tex"],
        cwd=LATEX_DIR,
        dry_run=dry_run,
        allow_failure=True)

    run_command(
        ["makeindex", "refman.idx"], cwd=LATEX_DIR, dry_run=dry_run, allow_failure=True)

    run_command(
        ["pdflatex", "-interaction=nonstopmode", "refman.tex"],
        cwd=LATEX_DIR,
        dry_run=dry_run,
        allow_failure=True)


    pdf_source = LATEX_DIR / "refman.pdf"
    if dry_run:
        print(f"[dry-run] Move {pdf_source} to {DEVELOPER_MANUAL}")
        return

    if not pdf_source.exists():
        raise SystemExit(f"Expected {pdf_source}")

    DEVELOPER_MANUAL.parent.mkdir(parents=True, exist_ok=True)
    shutil.move(str(pdf_source), DEVELOPER_MANUAL)
    print(f"Updated {DEVELOPER_MANUAL.relative_to(REPO_ROOT)}")


# ---------- main ----------
def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv or sys.argv[1:])
    validate_version(args.version)

    try:
        update_version_file(args.version, args.dry_run)
        update_appversion_header(args.version, args.dry_run)
        update_doxyfile(args.version, args.dry_run)
        if not args.skip_changelog:
            ensure_changelog_entry(args.version, args.date, args.dry_run)
        if not args.skip_docs:
            build_documentation(args.version, args.date, args.dry_run)
    except RuntimeError as exc:
        raise SystemExit(str(exc)) from exc

    print("Done.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
