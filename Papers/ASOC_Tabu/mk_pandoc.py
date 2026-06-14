#!/usr/bin/env python3
"""Produce a pandoc-friendly copy of the blinded manuscript:
- \rm WORD  -> \mathrm{WORD}   (pandoc's math reader rejects plain-TeX \rm)
- point figure includes at the PNG rasterizations
"""
import re

src = open("main_blind.tex", encoding="utf-8").read()

# \rm ext  ->  \mathrm{ext}   (handles _{\rm ext}, \leq_{\rm LEX2}, etc.)
src = re.sub(r"\\rm\s+([A-Za-z0-9]+)", r"\\mathrm{\1}", src)

# Also the occasional \bf / \it inside math, if any
src = re.sub(r"\\bf\s+([A-Za-z0-9]+)", r"\\mathbf{\1}", src)

# Point figure includes at the PNG rasterizations (pandoc cannot embed PDF)
figs = ["fig1_example_graphs", "fig2_block_structure", "fig34_combined",
        "fig5_combined", "fig6_convergence", "fig_taillard_compar"]
for fg in figs:
    src = re.sub(r"(\\includegraphics(?:\[[^\]]*\])?\{)" + re.escape(fg) + r"\}",
                 r"\1figures/" + fg + ".png}", src)

with open("main_pandoc.tex", "w", encoding="utf-8") as f:
    f.write(src)

# The \input'ed table file also contains \rm (output-identical to \mathrm)
tab = "figures/tab_phaseb_groups.tex"
t = open(tab, encoding="utf-8").read()
t = re.sub(r"\\rm\s+([A-Za-z0-9]+)", r"\\mathrm{\1}", t)
open(tab, "w", encoding="utf-8").write(t)

# Report
print("remaining backslash-rm in main:", len(re.findall(r"\\rm\s", src)),
      " in table:", len(re.findall(r"\\rm\s", t)))
