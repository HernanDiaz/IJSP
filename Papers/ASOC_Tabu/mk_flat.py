#!/usr/bin/env python3
"""Build a self-contained, FLAT LaTeX manuscript for Elsevier upload:
  - bibliography inlined from the .bbl (no .bib / BibTeX needed)
  - figures referenced from the base folder (no figures/ subdir)
  - the \\input'ed table flattened too
Everything then lives in one directory, which is how Editorial Manager
compiles uploaded source files.
"""
import re

tex = open("main_blind.tex", encoding="utf-8").read()
bbl = open("main_blind.bbl", encoding="utf-8").read().strip()

# 1. figures from the base folder
tex = tex.replace(r"\graphicspath{{./figures/}{../../figures/}}",
                  r"\graphicspath{{./}}")
# 2. inline the \input'ed table directly (fully self-contained main.tex)
table = open("figures/tab_phaseb_groups.tex", encoding="utf-8").read().strip()
assert tex.count(r"\input{figures/tab_phaseb_groups}") == 1
tex = tex.replace(r"\input{figures/tab_phaseb_groups}", table)
# 3. inline the bibliography. Replace ONLY the real command (not the
# commented elsarticle template line "%%  \bibliography{<your bibdatabase>}").
# Function replacement keeps the .bbl backslashes literal.
assert tex.count(r"\bibliography{references_blind}") == 1
tex = tex.replace(r"\bibliography{references_blind}", "\x00BBL\x00")
tex = tex.replace("\x00BBL\x00", bbl)

open("submission_flat/main.tex", "w", encoding="utf-8").write(tex)
print("wrote submission_flat/main.tex (flat, fully self-contained)")
print("  graphicspath ./:", r"\graphicspath{{./}}" in tex)
print("  table inlined (no \\input left):", r"\input{" not in tex,
      "| label present:", r"\label{tab:phaseb_groups}" in tex)
print("  bbl inlined:", r"\begin{thebibliography}" in tex,
      "| bibitems:", tex.count(r"\bibitem"))
