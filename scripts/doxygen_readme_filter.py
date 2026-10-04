#!/usr/bin/env python3
"""Doxygen input filter for README.md.

GitHub switches the logo with the theme through <picture>, which Doxygen
does not understand: it prints the tags as text and leaves the image
broken. This filter swaps that block for the banner, which reads on light
and dark backgrounds and which LaTeX can include too.
"""

import re
import sys

BANNER = "![SyntaxTutor](resources/icon/lockup/syntaxtutor-banner.png)"

with open(sys.argv[1], encoding="utf-8") as source:
    text = source.read()

# The centring <p> goes too: Doxygen does not parse Markdown inside HTML.
sys.stdout.write(re.sub(r"(<p[^>]*>\s*)?<picture>.*?</picture>(\s*</p>)?",
                        BANNER, text, count=1, flags=re.DOTALL))
