#!/usr/bin/env python3
"""Build a self-contained, FLAT LaTeX manuscript for Elsevier upload:
  - bibliography inlined from the .bbl (no .bib / BibTeX needed)
  - figures referenced from the base folder (no figures/ subdir)
  - the \\input'ed table flattened too
Everything then lives in one directory, which is how Editorial Manager
compiles uploaded source files.

Usage: python3 mk_flat.py [blind|nonblind]   (default: blind)
  blind    -> main_blind.tex + main_blind.bbl + references_blind
              (double-anonymized: \\author{} empty, self/repo cites withheld)
  nonblind -> main.tex       + main.bbl       + references
              (author block + affiliation + real citations)
"""
import sys

variant = sys.argv[1] if len(sys.argv) > 1 else "blind"
if variant == "blind":
    base, bib = "main_blind", "references_blind"
elif variant == "nonblind":
    base, bib = "main", "references"
else:
    sys.exit("usage: mk_flat.py [blind|nonblind]")

tex = open(base + ".tex", encoding="utf-8").read()
bbl = open(base + ".bbl", encoding="utf-8").read().strip()

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
cmd = r"\bibliography{%s}" % bib
assert tex.count(cmd) == 1
tex = tex.replace(cmd, "\x00BBL\x00")
tex = tex.replace("\x00BBL\x00", bbl)

open("submission_flat/main.tex", "w", encoding="utf-8").write(tex)
print("wrote submission_flat/main.tex (flat, fully self-contained) [variant=%s]" % variant)
print("  graphicspath ./:", r"\graphicspath{{./}}" in tex)
print("  table inlined (no \\input left):", r"\input{" not in tex,
      "| label present:", r"\label{tab:phaseb_groups}" in tex)
print("  bbl inlined:", r"\begin{thebibliography}" in tex,
      "| bibitems:", tex.count(r"\bibitem"))
print("  author block present:", r"\author[uo]" in tex,
      "| 'withheld' occurrences:", tex.count("withheld"))
