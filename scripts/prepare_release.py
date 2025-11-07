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

# ---------- Rutas del repositorio ----------
REPO_ROOT = Path(__file__).resolve().parents[1]
VERSION_FILE = REPO_ROOT / "VERSION"
APPVERSION_HEADER = REPO_ROOT / "appversion.h"
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
        description="Prepara una release: versionado, Doxygen y manual PDF."
    )
    parser.add_argument("version", help="Versión de release (p. ej. 1.1.0)")
    parser.add_argument(
        "--date",
        default=today,
        help="Fecha de release para CHANGELOG y manual (ISO, por defecto hoy)",
    )
    parser.add_argument(
        "--skip-docs",
        action="store_true",
        help="No regenerar Doxygen ni el manual de desarrollador",
    )
    parser.add_argument(
        "--skip-changelog",
        action="store_true",
        help="No insertar entrada de plantilla en CHANGELOG.md",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Simular acciones sin modificar archivos ni ejecutar comandos",
    )
    return parser.parse_args(argv)


# ---------- Utilidades ----------
def validate_version(version: str) -> None:
    if not SEMVER_RE.match(version):
        raise SystemExit(
            f"Versión inválida '{version}'. Usa semver: 1.1.0, 2.0, 1.0.3, etc."
        )


def write_file(path: Path, content: str, dry_run: bool) -> None:
    if dry_run:
        print(f"[dry-run] Escribir en {path}:")
        for line in content.splitlines():
            print(f"    {line}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")
    print(f"✔ Actualizado {path.relative_to(REPO_ROOT)}")


def update_version_file(version: str, dry_run: bool) -> None:
    write_file(VERSION_FILE, f"{version}\n", dry_run)


def update_appversion_header(version: str, dry_run: bool) -> None:
    template = f"""#pragma once

#include <QString>

namespace SyntaxTutor::Version {{
inline QString current() {{
    return QStringLiteral("{version}");
}}

inline const char* raw() {{
    return "{version}";
}}
}} // namespace SyntaxTutor::Version
"""
    write_file(APPVERSION_HEADER, template, dry_run)


def update_doxyfile(version: str, dry_run: bool) -> None:
    if not DOXYFILE.exists():
        raise RuntimeError(f"No se encontró {DOXYFILE}")
    contents = DOXYFILE.read_text(encoding="utf-8")
    new_contents, count = re.subn(
        r"^(PROJECT_NUMBER\s*=\s*).*$",
        rf"\g<1>{version}",
        contents,
        flags=re.MULTILINE,
    )
    if count == 0:
        raise RuntimeError("Entrada PROJECT_NUMBER no encontrada en Doxyfile")
    write_file(DOXYFILE, new_contents, dry_run)


def ensure_changelog_entry(version: str, release_date: str, dry_run: bool) -> None:
    heading = f"## [{version}] - {release_date}"
    if CHANGELOG.exists():
        contents = CHANGELOG.read_text(encoding="utf-8")
    else:
        contents = "# Changelog\n\n"

    if heading in contents:
        print(f"ℹ CHANGELOG ya contiene una entrada para {version}")
        return

    entry = (
        f"{heading}\n"
        "### Added\n"
        "- _Describe nuevas funcionalidades._\n\n"
        "### Changed\n"
        "- _Modificaciones destacables._\n\n"
        "### Fixed\n"
        "- _Bugs resueltos._\n\n"
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
) -> None:
    command = [str(c) for c in command]
    display = " ".join(command)
    location = f" (cwd={cwd})" if cwd else ""
    if dry_run:
        print(f"[dry-run] Ejecutar: {display}{location}")
        return

    print(f"▶ Ejecutando: {display}{location}")
    try:
        subprocess.run(command, cwd=cwd, env=env, check=True)
    except FileNotFoundError as exc:
        raise SystemExit(f"Comando no encontrado: {command[0]}") from exc


def _check_tool(cmd: str) -> None:
    if shutil.which(cmd) is None:
        raise SystemExit(f"Requisito no encontrado en PATH: {cmd}")


def build_documentation(version: str, release_date: str, dry_run: bool) -> None:
    # Requisitos previos
    _check_tool("doxygen")
    _check_tool("xelatex")

    run_command(["doxygen", "Doxyfile"], cwd=REPO_ROOT, dry_run=dry_run)

    env = os.environ.copy()
    env["SYNTAXTUTOR_VERSION"] = version
    env["SYNTAXTUTOR_RELEASE_DATE"] = release_date

    # Asegura que el script exista y sea ejecutable (en *nix)
    if not PATCH_REFMAN.exists():
        raise SystemExit(f"No se encontró {PATCH_REFMAN}")
    if os.name != "nt" and not os.access(PATCH_REFMAN, os.X_OK):
        if dry_run:
            print(f"[dry-run] Haría ejecutable {PATCH_REFMAN}")
        else:
            PATCH_REFMAN.chmod(PATCH_REFMAN.stat().st_mode | 0o111)

    run_command([PATCH_REFMAN], cwd=REPO_ROOT, env=env, dry_run=dry_run)

    # Compilar dos veces para referencias estables
    for _ in range(2):
        run_command(
            ["xelatex", "-interaction=nonstopmode", "refman.tex"],
            cwd=LATEX_DIR,
            dry_run=dry_run,
        )

    pdf_source = LATEX_DIR / "refman.pdf"
    if dry_run:
        print(f"[dry-run] Mover {pdf_source} -> {DEVELOPER_MANUAL}")
        return

    if not pdf_source.exists():
        raise SystemExit(f"Esperaba {pdf_source} tras compilar con xelatex")

    DEVELOPER_MANUAL.parent.mkdir(parents=True, exist_ok=True)
    shutil.move(str(pdf_source), DEVELOPER_MANUAL)
    print(f"✔ Actualizado {DEVELOPER_MANUAL.relative_to(REPO_ROOT)}")


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

    print("Release preparada correctamente.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
