#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
REFMAN="${REPO_ROOT}/docs/latex/refman.tex"
TMP="${REPO_ROOT}/docs/latex/refman_tmp.tex"
VERSION_FILE="${REPO_ROOT}/VERSION"

if [[ -n "${SYNTAXTUTOR_VERSION}" ]]; then
    VERSION="${SYNTAXTUTOR_VERSION}"
elif [[ -f "${VERSION_FILE}" ]]; then
    VERSION="$(tr -d '\n\r ' < "${VERSION_FILE}")"
else
    echo "Unable to determine SyntaxTutor version. Set SYNTAXTUTOR_VERSION or create VERSION file." >&2
    exit 1
fi

if [[ -z "${VERSION}" ]]; then
    echo "Empty version string." >&2
    exit 1
fi

RELEASE_DATE="${SYNTAXTUTOR_RELEASE_DATE:-$(date +%Y-%m-%d)}"

echo "Patching ${REFMAN} with version ${VERSION} (${RELEASE_DATE})..."

awk -v version="${VERSION}" -v release_date="${RELEASE_DATE}" '
BEGIN { in_titlepage = 0 }
{
    if ($0 ~ /\\begin{titlepage}/) {
        in_titlepage = 1
        print "\\begin{titlepage}"
        print "    \\centering"
        print "    \\vspace*{1cm}"
        print "    \\includegraphics[width=0.25\\textwidth]{logouma.png}\\\\"
        print "    \\vspace{0.5cm}"
        print "    {\\scshape\\Large University of Malaga\\\\}"
        print "    {\\scshape School of Computer Science and Engineering\\\\}"
        print "    \\vspace{1.5cm}"
        print "    {\\Huge\\bfseries SyntaxTutor\\\\}"
        print "    \\vspace{0.4cm}"
        print "    {\\Large Developer Manual\\\\}"
        print "    \\vspace{1.2cm}"
        print "    \\rule{0.7\\linewidth}{0.5pt}"
        print "    \\vfill"
        print "    \\begin{flushright}"
        print "    \\textbf{Version:} " version "\\\\"
        print "    \\textbf{Date:} " release_date "\\\\"
        print "    \\textbf{Author:} jose-rzm at GitHub"
        print "    \\end{flushright}"
        print "    \\vfill"
        print "    \\includegraphics[width=0.15\\textwidth]{syntaxtutor.png}"
        next
    }
    if ($0 ~ /\\end{titlepage}/) {
        in_titlepage = 0
        print "\\end{titlepage}"
        next
    }
    if (!in_titlepage) print $0
}
' "${REFMAN}" > "${TMP}" && mv "${TMP}" "${REFMAN}"

echo "Titlepage updated."
