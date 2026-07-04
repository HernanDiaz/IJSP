#!/usr/bin/env python3
"""
extend_instances.py — create energy-extended copies of the 82 benchmark
instances: SelectosYTaillardIntervalosEnergia/<stem>.txt = original file +
POTENCIA PASIVA section (one integer per machine).

Scheme (documented, reproducible): Pp_k ~ U{2..8}, random.Random(23) seeded
PER INSTANCE — identical to conflict_check.py / epsilon_probe.py, so all
Phase-0/1 artefacts share the same power data.
"""
import os, random, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from conflict_check import parse_instance, INSTDIR, POWER_SEED

OUTDIR = os.path.join(os.path.dirname(INSTDIR), "SelectosYTaillardIntervalosEnergia")


def main():
    os.makedirs(OUTDIR, exist_ok=True)
    stems = sorted(os.path.splitext(f)[0] for f in os.listdir(INSTDIR)
                   if f.endswith(".txt"))
    done = 0
    for stem in stems:
        n, m, _, _ = parse_instance(stem)
        rng = random.Random(POWER_SEED)
        pp = [rng.randint(2, 8) for _ in range(m)]
        with open(os.path.join(INSTDIR, stem + ".txt"), encoding="utf-8",
                  errors="replace") as fh:
            body = fh.read()
        if not body.endswith("\n"):
            body += "\n"
        body += "POTENCIA PASIVA\n" + " ".join(str(p) for p in pp) + "\n"
        with open(os.path.join(OUTDIR, stem + ".txt"), "w", encoding="utf-8",
                  newline="\n") as fh:
            fh.write(body)
        done += 1
    print(f"{done} instances extended into {OUTDIR}")


if __name__ == "__main__":
    main()
