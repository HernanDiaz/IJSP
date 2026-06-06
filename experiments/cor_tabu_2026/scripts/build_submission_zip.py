#!/usr/bin/env python3
"""build_submission_zip.py - Pack the LaTeX source for Elsevier Editorial
Manager submission.

Layout inside the zip:
    main.tex
    references.bib
    figures/
        fig1_example_graphs.pdf
        fig2_block_structure.pdf
        fig34_combined.pdf
        fig5_combined.pdf
        fig6_convergence.pdf
        fig_taillard_compar.pdf
        tab_phaseb_groups.tex
"""
import os, zipfile
from pathlib import Path

_WSL = "/mnt/c/Users/diazhernan/CLionProjects/IJSP"
_WIN = "C:/Users/diazhernan/CLionProjects/IJSP"
ROOT = Path(_WSL if os.path.isdir(_WSL) else _WIN)

COR = ROOT / "Papers" / "COR_Tabu"
OUT = COR / "COR_Tabu_source.zip"

# Files referenced in main.tex (verified by grep includegraphics + input)
FIGS_LOCAL = COR / "figures"
FIGS_ROOT  = ROOT / "figures"
FIGURES = [
    (FIGS_ROOT  / "fig1_example_graphs.pdf",  "figures/fig1_example_graphs.pdf"),
    (FIGS_ROOT  / "fig2_block_structure.pdf", "figures/fig2_block_structure.pdf"),
    (FIGS_LOCAL / "fig34_combined.pdf",       "figures/fig34_combined.pdf"),
    (FIGS_LOCAL / "fig5_combined.pdf",        "figures/fig5_combined.pdf"),
    (FIGS_LOCAL / "fig6_convergence.pdf",     "figures/fig6_convergence.pdf"),
    (FIGS_LOCAL / "fig_taillard_compar.pdf",  "figures/fig_taillard_compar.pdf"),
    (FIGS_LOCAL / "tab_phaseb_groups.tex",    "figures/tab_phaseb_groups.tex"),
]

if OUT.exists():
    OUT.unlink()

with zipfile.ZipFile(OUT, "w", zipfile.ZIP_DEFLATED) as z:
    z.write(COR / "main.tex",       "main.tex")
    z.write(COR / "references.bib", "references.bib")
    for src, dest in FIGURES:
        if not src.exists():
            print(f"  WARNING: missing {src}")
            continue
        z.write(src, dest)
        print(f"  added {dest}  ({src.stat().st_size} bytes)")

print(f"\nWrote {OUT}")
print(f"  size: {OUT.stat().st_size/1024:.1f} KB")

# Verify contents
print("\nContents:")
with zipfile.ZipFile(OUT) as z:
    for info in z.infolist():
        print(f"  {info.filename:50s} {info.file_size:>10} bytes")
