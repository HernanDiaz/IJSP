#!/usr/bin/env python3
"""Splice the regenerated TA-rows into supp_taillard_tables.tex."""
import csv, re, os, shutil
from pathlib import Path

_WSL = "/mnt/c/Users/diazhernan/CLionProjects/IJSP"
_WIN = "C:/Users/diazhernan/CLionProjects/IJSP"
ROOT = Path(_WSL if os.path.isdir(_WSL) else _WIN)
STATS = ROOT / "experiments" / "statistical_results_exp4" / "per_instance_stats.csv"
SUPP  = ROOT / "Papers" / "COR_Tabu" / "supplementary" / "supp_taillard_tables.tex"

TAILLARD_LBS = {
    "tai15_15":[1231,1244,1218,1175,1224,1238,1227,1217,1274,1241],
    "tai20_15":[1357,1367,1342,1345,1339,1360,1462,1377,1332,1348],
    "tai20_20":[1642,1561,1518,1644,1558,1591,1652,1603,1583,1528],
    "tai30_15":[1764,1774,1788,1828,2007,1819,1771,1673,1795,1651],
    "tai30_20":[1906,1884,1809,1948,1997,1957,1807,1912,1931,1833],
    "tai50_15":[2760,2756,2717,2839,2679,2781,2943,2885,2655,2723],
    "tai50_20":[2868,2869,2755,2702,2725,2845,2825,2784,3071,2995],
}
SIZE_ORDER = ["tai15_15","tai20_15","tai20_20","tai30_15","tai30_20","tai50_15","tai50_20"]
SIZE_LABEL = {"tai15_15":r"$15\times15$","tai20_15":r"$20\times15$",
              "tai20_20":r"$20\times20$","tai30_15":r"$30\times15$",
              "tai30_20":r"$30\times20$","tai50_15":r"$50\times15$",
              "tai50_20":r"$50\times20$"}

# TA index -> (size, k, stem)
ta_map = {}
ta_idx = 1
for sz in SIZE_ORDER:
    for k in range(1, 11):
        ta_map[ta_idx] = (sz, k, f"{sz}_{k:02d}.F.15_01")
        ta_idx += 1

# Load per_instance_stats
stats = {}
with open(STATS) as fh:
    for row in csv.DictReader(fh):
        stats[(row["neigh"], row["instance"])] = {
            "best_E": float(row["best_E"]),
            "avg_E":  float(row["avg_E"]),
            "med_rt": float(row["med_rt"]) if row["med_rt"] != "NA" else None,
        }

def re_pct(mk, lb): return (mk - lb)/lb*100.0

# Parse existing rows for prior-art
text = SUPP.read_text(encoding="utf-8")
ROW_RE = re.compile(r"TA(\d+)\s*&\s*(\d+)\s*&\s*"
                    r"([-\d.]+)\s*&\s*([-\d.]+)\s*&\s*([-\d.]+)\s*&\s*"
                    r"([-\d.]+)\s*&\s*([-\d.]+)\s*&\s*([-\d.]+)")
prior_art = {}
for m in ROW_RE.finditer(text):
    ta = int(m.group(1))
    prior_art[ta] = {
        "lb": int(m.group(2)),
        "fE_best": float(m.group(3)), "fE_avg": float(m.group(4)), "fE_t": float(m.group(5)),
        "ES_best": float(m.group(6)), "ES_avg": float(m.group(7)), "ES_t": float(m.group(8)),
    }

def make_row(ta, first_of_class):
    sz, k, stem = ta_map[ta]
    lb = TAILLARD_LBS[sz][k-1]
    pa = prior_art[ta]
    n2 = stats[("n2", stem)]; n8 = stats[("n8", stem)]
    n2_best = re_pct(n2["best_E"], lb); n2_avg = re_pct(n2["avg_E"], lb); n2_t = n2["med_rt"]
    n8_best = re_pct(n8["best_E"], lb); n8_avg = re_pct(n8["avg_E"], lb); n8_t = n8["med_rt"]
    avgs = {"fE": pa["fE_avg"], "ES": pa["ES_avg"], "n2": n2_avg, "n8": n8_avg}
    winner = min(avgs, key=avgs.get)
    def bold(val, key):
        s = f"{val:.2f}"
        return r"\textbf{" + s + "}" if key == winner else s
    prefix = SIZE_LABEL[sz] if first_of_class else ""
    return (f"{prefix} & TA{ta} & {lb} & "
            f"{pa['fE_best']:.2f} & {bold(pa['fE_avg'], 'fE')} & {pa['fE_t']:.1f} & "
            f"{pa['ES_best']:.2f} & {bold(pa['ES_avg'], 'ES')} & {pa['ES_t']:.1f} & "
            f"{n2_best:.2f} & {bold(n2_avg, 'n2')} & {int(round(n2_t)) if n2_t else '--'} & "
            f"{n8_best:.2f} & {bold(n8_avg, 'n8')} & {int(round(n8_t)) if n8_t else '--'} \\\\")

# Build new row blocks (with hlines between size classes)
def build_block(ta_start, ta_end):
    lines = []
    prev_sz = None
    for ta in range(ta_start, ta_end+1):
        sz = ta_map[ta][0]
        first = (sz != prev_sz)
        if first and prev_sz is not None:
            lines.append(r"\hline")
        lines.append(make_row(ta, first))
        prev_sz = sz
    return "\n".join(lines)

block_I  = build_block(1, 40)
block_II = build_block(41, 70)

# Backup
shutil.copy(SUPP, SUPP.with_suffix(SUPP.suffix + ".preBugfix2"))

# Splice: find each table's row region and replace.
# Anchor: from "\hline" right after "Size & Inst. & LB..." header
# to the closing "\hline\n\end{tabular}"
def splice(text, block, header_marker, end_marker):
    h_idx = text.find(header_marker)
    if h_idx < 0:
        raise RuntimeError(f"header not found: {header_marker[:40]}")
    # Find first \hline after the column header
    first_hline = text.find(r"\hline", h_idx)
    if first_hline < 0:
        raise RuntimeError("first \\hline not found")
    # First data starts after that \hline + newline
    data_start = text.find("\n", first_hline) + 1
    # End: last \hline before \end{tabular} (closing rule line)
    et_idx = text.find(end_marker, data_start)
    if et_idx < 0:
        raise RuntimeError("end marker not found")
    last_hline = text.rfind(r"\hline", data_start, et_idx)
    if last_hline < 0:
        raise RuntimeError("last \\hline before end_marker not found")
    return text[:data_start] + block + "\n" + text[last_hline:]

# Two tables differ only by the LABEL line; use \label{tab:supp_taillard_i} and _ii as anchors
text2 = splice(text, block_I,
               header_marker="\\label{tab:supp_taillard_i}",
               end_marker="\\end{tabular}")
# For Table II, anchor on its label
text3 = splice(text2, block_II,
               header_marker="\\label{tab:supp_taillard_ii}",
               end_marker="\\end{tabular}")

SUPP.write_text(text3, encoding="utf-8")
print(f"Patched {SUPP}.")
print(f"Backup at {SUPP.with_suffix(SUPP.suffix + '.preBugfix2')}.")

# Print a diff summary on TA54 specifically
print()
print("=== TA54 N_8 (critical sanity check — was -2.08 / -1.07 with bug) ===")
stem = "tai50_15_04.F.15_01"
n8 = stats[("n8", stem)]; lb = 2839
print(f"  Best E = {n8['best_E']:.2f} -> RE = {(n8['best_E']-lb)/lb*100:.4f}%")
print(f"  Avg E  = {n8['avg_E']:.2f} -> RE = {(n8['avg_E']-lb)/lb*100:.4f}%")
print(f"  Med rt = {n8['med_rt']:.1f}s")
