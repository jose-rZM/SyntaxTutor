#!/bin/bash

REFMAN="docs/latex/refman.tex"
TMP="docs/latex/refman_tmp.tex"

echo "Patching $REFMAN..."

awk '
BEGIN { in_titlepage = 0 }
{
    if ($0 ~ /\\begin{titlepage}/) {
        in_titlepage = 1
        print "\\begin{titlepage}"
        print "    \\centering"
        print "    \\vspace*{1cm}"
        print "    \\includegraphics[width=0.25\\textwidth]{img/logouma.png}\\\\"
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
        print "    \\textbf{Version:} 1.0.2\\\\"
        print "    \\textbf{Date:} 2025\\\\"
        print "    \\textbf{Author:} jose-rzm at GitHub"
        print "    \\end{flushright}"
        print "    \\vfill"
        print "    \\includegraphics[width=0.15\\textwidth]{img/syntaxtutor.png}"
        next
    }
    if ($0 ~ /\\end{titlepage}/) {
        in_titlepage = 0
        print "\\end{titlepage}"
        next
    }
    if (!in_titlepage) print $0
}
' "$REFMAN" > "$TMP" && mv "$TMP" "$REFMAN"

echo "Titlepage updated."
