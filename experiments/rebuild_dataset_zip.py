#!/usr/bin/env python3
"""rebuild_dataset_zip.py - Regenerate Papers/COR_Tabu/IJSP_dataset_2026.zip
with the post-bugfix Phase B data.

Strategy:
- Copy everything from the old zip UNCHANGED except:
  - results/phaseB_TS/*      → replaced with current exp4/ data (82 common stems)
  - aggregated_csvs/phaseB/* → replaced with current statistical_results_exp4/
  - scripts/python/*         → replaced with current patched scripts (and renames)
  - scripts/R/*              → replaced with current R scripts

Phase A (results/phaseA_HC/*) is preserved unchanged — validated 99.91% clean.
"""
import os, re, shutil, zipfile
from pathlib import Path

_WSL = "/mnt/c/Users/diazhernan/CLionProjects/IJSP"
_WIN = "C:/Users/diazhernan/CLionProjects/IJSP"
ROOT = Path(_WSL if os.path.isdir(_WSL) else _WIN)

OLD_ZIP = ROOT / "Papers" / "COR_Tabu" / "IJSP_dataset_2026.zip"
NEW_ZIP = ROOT / "Papers" / "COR_Tabu" / "IJSP_dataset_2026.zip.new"

EXP4_BASE = ROOT / "experiments" / "results" / "exp4"
STATS_DIR = ROOT / "experiments" / "statistical_results_exp4"
EXPDIR    = ROOT / "experiments"
FIGDIR    = ROOT / "Papers" / "COR_Tabu" / "figures"

TS_RE   = re.compile(r"^(.+)_(\d{14})_Sols\.csv$")
MAIN_RE = re.compile(r"^(.+)_(\d{14})\.csv$")

# Skip these paths from old zip (will be replaced)
SKIP_PREFIXES = (
    "results\\phaseB_TS\\",
    "results/phaseB_TS/",
    "aggregated_csvs\\phaseB\\",
    "aggregated_csvs/phaseB/",
    "scripts\\python\\",
    "scripts/python/",
    "scripts\\R\\",
    "scripts/R/",
)


def build_allow_82():
    """Intersection of stems across the 5 NB folders → 82 common."""
    allow = None
    for nb in ("N1_tuned","N2_tuned","N3_tuned","N8_tuned","Next_tuned"):
        s = set()
        for fname in os.listdir(EXP4_BASE / nb):
            m = MAIN_RE.match(fname)
            if m and "Sols" not in fname and "Robustness" not in fname and "Scenarios" not in fname:
                s.add(m.group(1))
        allow = s if allow is None else (allow & s)
    return allow


def latest_per_stem(folder, allow):
    """Returns dict: stem -> (main_csv_path, sols_csv_path) for the latest timestamp."""
    main_by_stem = {}
    for fname in os.listdir(folder):
        if "Sols" in fname or "Robustness" in fname or "Scenarios" in fname:
            continue
        m = MAIN_RE.match(fname)
        if not m: continue
        stem, ts = m.group(1), m.group(2)
        if stem not in allow: continue
        if stem not in main_by_stem or ts > main_by_stem[stem][0]:
            main_by_stem[stem] = (ts, fname)
    out = {}
    for stem, (ts, main_fname) in main_by_stem.items():
        sols_fname = main_fname.replace(".csv", "_Sols.csv")
        sols_path = folder / sols_fname
        out[stem] = {
            "main": (main_fname, folder / main_fname),
            "sols": (sols_fname, sols_path if sols_path.exists() else None),
        }
    return out


def main():
    allow_82 = build_allow_82()
    print(f"82-instance allow-list: {len(allow_82)} stems")
    assert len(allow_82) == 82, f"expected 82, got {len(allow_82)}"

    if NEW_ZIP.exists():
        NEW_ZIP.unlink()

    with zipfile.ZipFile(OLD_ZIP, "r") as zin, \
         zipfile.ZipFile(NEW_ZIP, "w", zipfile.ZIP_DEFLATED, allowZip64=True) as zout:

        # 1. Pass-through: copy all entries except the ones we're replacing
        n_copied = n_skipped = 0
        for info in zin.infolist():
            if any(info.filename.startswith(p) for p in SKIP_PREFIXES):
                n_skipped += 1
                continue
            zout.writestr(info, zin.read(info.filename))
            n_copied += 1
        print(f"Copied {n_copied} entries unchanged; skipped {n_skipped} for replacement.")

        # 2. Add fresh results/phaseB_TS data (main + Sols for 82 stems × 5 NBs)
        n_added = 0
        for nb in ("N1_tuned","N2_tuned","N3_tuned","N8_tuned","Next_tuned"):
            stems = latest_per_stem(EXP4_BASE / nb, allow_82)
            print(f"  {nb}: {len(stems)} stems")
            for stem, files in stems.items():
                # main CSV
                main_fname, main_path = files["main"]
                zout.write(main_path, f"results/phaseB_TS/{nb}/{main_fname}")
                n_added += 1
                # Sols CSV
                sols_fname, sols_path = files["sols"]
                if sols_path and sols_path.exists():
                    zout.write(sols_path, f"results/phaseB_TS/{nb}/{sols_fname}")
                    n_added += 1
        print(f"Added {n_added} phaseB_TS CSVs.")

        # 3. Add aggregated_csvs/phaseB/*
        for csv_name in ("runs_data.csv", "per_instance_stats.csv",
                          "phaseB_group_summary.csv", "convergence_data_ts.csv"):
            src = STATS_DIR / csv_name
            if src.exists():
                zout.write(src, f"aggregated_csvs/phaseB/{csv_name}")
                print(f"  added aggregated_csvs/phaseB/{csv_name}  ({src.stat().st_size} bytes)")

        # 4. Add updated scripts
        # scripts/python/
        for py_name in ("build_stats_data_exp4.py",
                         "build_convergence_data_ts.py",
                         "gen_phaseB_tables.py"):
            src = EXPDIR / py_name
            if src.exists():
                zout.write(src, f"scripts/python/{py_name}")
                print(f"  added scripts/python/{py_name}")

        # scripts/R/
        for r_name in ("combine_fig34.R", "combine_fig5.R",
                        "generate_fig6_ts.R", "generate_fig_taillard_compar.R"):
            src = FIGDIR / r_name
            if src.exists():
                zout.write(src, f"scripts/R/{r_name}")
                print(f"  added scripts/R/{r_name}")
        # get_phaseC_runtimes.R might be in experiments/
        gpc = EXPDIR / "get_phaseC_runtimes.R"
        if gpc.exists():
            zout.write(gpc, "scripts/R/get_phaseC_runtimes.R")
            print("  added scripts/R/get_phaseC_runtimes.R")

    new_size = NEW_ZIP.stat().st_size
    old_size = OLD_ZIP.stat().st_size
    print(f"\nNew zip: {NEW_ZIP}")
    print(f"  size: {new_size/1024/1024:.2f} MB (old: {old_size/1024/1024:.2f} MB)")
    print(f"  entries: counted during creation; verify with zip tools.")


if __name__ == "__main__":
    main()
