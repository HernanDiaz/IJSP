#!/usr/bin/env python3
"""rebuild_supp_taillard_tables.py - Regenerate the supplementary per-instance
Taillard tables (TA1-TA70) from corrected per_instance_stats.csv. Keeps the
fEABC and ESABC columns verbatim from the existing tex file (they come from
cited papers and are unchanged), and replaces TS-N_2 + TS-N_8 columns with
authoritative values from cor_tabu_2026/statistical_results.

Outputs the new tex content to stdout (and saves a backup of the current file).
"""
import csv, re, os
from pathlib import Path

_WSL = "/mnt/c/Users/diazhernan/CLionProjects/IJSP"
_WIN = "C:/Users/diazhernan/CLionProjects/IJSP"
ROOT = Path(_WSL if os.path.isdir(_WSL) else _WIN)
STATS = ROOT / "experiments" / "cor_tabu_2026/statistical_results" / "per_instance_stats.csv"
SUPP  = ROOT / "Papers" / "COR_Tabu" / "supplementary" / "supp_taillard_tables.tex"

# TA index -> instance stem
TAILLARD_LBS = {
    "tai15_15": [1231,1244,1218,1175,1224,1238,1227,1217,1274,1241],
    "tai20_15": [1357,1367,1342,1345,1339,1360,1462,1377,1332,1348],
    "tai20_20": [1642,1561,1518,1644,1558,1591,1652,1603,1583,1528],
    "tai30_15": [1764,1774,1788,1828,2007,1819,1771,1673,1795,1651],
    "tai30_20": [1906,1884,1809,1948,1997,1957,1807,1912,1931,1833],
    "tai50_15": [2760,2756,2717,2839,2679,2781,2943,2885,2655,2723],
    "tai50_20": [2868,2869,2755,2702,2725,2845,2825,2784,3071,2995],
}
SIZE_ORDER = ["tai15_15","tai20_15","tai20_20","tai30_15","tai30_20","tai50_15","tai50_20"]
SIZE_LABEL = {"tai15_15":r"$15\times15$", "tai20_15":r"$20\times15$",
              "tai20_20":r"$20\times20$", "tai30_15":r"$30\times15$",
              "tai30_20":r"$30\times20$", "tai50_15":r"$50\times15$",
              "tai50_20":r"$50\times20$"}

# Build TA_index -> (size_class, k_within_class, stem)
ta_map = {}
ta_idx = 1
for sz in SIZE_ORDER:
    for k in range(1, 11):
        stem = f"{sz}_{k:02d}.F.15_01"
        ta_map[ta_idx] = (sz, k, stem)
        ta_idx += 1

# Load per_instance_stats
stats = {}  # (nb, stem) -> {best_E, avg_E, med_rt}
with open(STATS) as fh:
    for row in csv.DictReader(fh):
        stats[(row["neigh"], row["instance"])] = {
            "best_E": float(row["best_E"]),
            "avg_E":  float(row["avg_E"]),
            "med_rt": float(row["med_rt"]) if row["med_rt"] != "NA" else None,
        }

def re_pct(makespan, lb):
    return (makespan - lb) / lb * 100.0

# Read the existing supp file to extract fEABC/ESABC values per TA
text = SUPP.read_text(encoding="utf-8")
# Parse rows: lines starting with " & TA" or "$N\times M$ & TAk"
ROW_RE = re.compile(r"TA(\d+)\s*&\s*(\d+)\s*&\s*"
                    r"([-\d.]+)\s*&\s*([-\d.]+)\s*&\s*([-\d.]+)\s*&\s*"  # fEABC Best Avg t
                    r"([-\d.]+)\s*&\s*([-\d.]+)\s*&\s*([-\d.]+)")        # ESABC Best Avg t
prior_art = {}
for m in ROW_RE.finditer(text):
    ta = int(m.group(1))
    prior_art[ta] = {
        "lb":       int(m.group(2)),
        "fE_best":  float(m.group(3)),
        "fE_avg":   float(m.group(4)),
        "fE_t":     float(m.group(5)),
        "ES_best":  float(m.group(6)),
        "ES_avg":   float(m.group(7)),
        "ES_t":     float(m.group(8)),
    }
print(f"Extracted {len(prior_art)} prior-art rows.")

# Compose new table rows
def emit_row(ta, first_of_class):
    sz, k, stem = ta_map[ta]
    lb = TAILLARD_LBS[sz][k-1]
    pa = prior_art.get(ta)
    if pa is None:
        return f"  [missing prior-art for TA{ta}]"
    n2 = stats.get(("n2", stem))
    n8 = stats.get(("n8", stem))
    if n2 is None or n8 is None:
        return f"  [missing TS data for TA{ta} ({stem})]"
    # RE values
    n2_best = re_pct(n2["best_E"], lb)
    n2_avg  = re_pct(n2["avg_E"], lb)
    n2_t    = n2["med_rt"]
    n8_best = re_pct(n8["best_E"], lb)
    n8_avg  = re_pct(n8["avg_E"], lb)
    n8_t    = n8["med_rt"]
    # Determine row-minimum Avg
    avgs = {"fE": pa["fE_avg"], "ES": pa["ES_avg"], "n2": n2_avg, "n8": n8_avg}
    winner = min(avgs, key=avgs.get)
    def bold(val, key):
        s = f"{val:.2f}"
        if key == winner:
            return r"\textbf{" + s + "}"
        return s
    # Build row
    prefix = SIZE_LABEL[sz] if first_of_class else ""
    return (f"{prefix} & TA{ta} & {lb} & "
            f"{pa['fE_best']:.2f} & {bold(pa['fE_avg'], 'fE')} & {pa['fE_t']:.1f} & "
            f"{pa['ES_best']:.2f} & {bold(pa['ES_avg'], 'ES')} & {pa['ES_t']:.1f} & "
            f"{n2_best:.2f} & {bold(n2_avg, 'n2')} & {int(round(n2_t)) if n2_t else '--'} & "
            f"{n8_best:.2f} & {bold(n8_avg, 'n8')} & {int(round(n8_t)) if n8_t else '--'} \\\\")

print("\n=== TABLE I (TA1-TA40) NEW ROWS ===\n")
prev_sz = None
for ta in range(1, 41):
    sz = ta_map[ta][0]
    print(emit_row(ta, first_of_class=(sz != prev_sz)))
    if (ta % 10) == 0:
        print(r"\hline")
    prev_sz = sz

print("\n=== TABLE II (TA41-TA70) NEW ROWS ===\n")
prev_sz = None
for ta in range(41, 71):
    sz = ta_map[ta][0]
    print(emit_row(ta, first_of_class=(sz != prev_sz)))
    if (ta % 10) == 0:
        print(r"\hline")
    prev_sz = sz
