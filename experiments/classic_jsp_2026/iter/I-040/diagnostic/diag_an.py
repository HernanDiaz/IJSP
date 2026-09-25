import statistics as st
for inst in ("ta23", "ta45"):
    rows = [tuple(float(x) for x in l.split()) for l in open("/tmp/diag/run_%s/chains.tsv" % inst)]
    n = len(rows)
    imp = [r for r in rows if r[4] < r[0]]
    calls = sum(r[5] for r in rows)
    extra = sum(r[5] - 1 for r in rows)
    print("== %s: %d steps, improve %.1f%%, calls %d, calls beyond the first %.1f%% of all" % (inst, n, 100.0 * len(imp) / n, calls, 100.0 * extra / calls))
    g_imp = sorted(r[3] - r[0] for r in imp)
    q = lambda v, p: v[min(len(v) - 1, int(p * len(v)))]
    print("   gap after first call (afterFirst - source), improving chains: q50 %.0f q90 %.0f q95 %.0f q99 %.0f max %.0f" % (q(g_imp, .5), q(g_imp, .9), q(g_imp, .95), q(g_imp, .99), g_imp[-1]))
    g_all = sorted(r[3] - r[0] for r in rows)
    print("   gap after first call, all chains: q25 %.0f q50 %.0f q75 %.0f q90 %.0f" % (q(g_all, .25), q(g_all, .5), q(g_all, .75), q(g_all, .9)))
    # improvements over the incumbent (records of the run) and where they come from
    bsf = [r for r in rows if r[4] < r[1]]
    print("   steps that beat the run's incumbent: %d, their gap after first call: %s" % (len(bsf), sorted(r[3] - r[0] for r in bsf)[:12]))
    for t in (0, 5, 10, 20, 40):
        cut = [r for r in rows if r[3] - r[0] > t]
        lost = [r for r in cut if r[4] < r[0]]
        saved = sum(r[5] - 1 for r in cut)
        print("   cut if gap1 > %3d: cut %.1f%% of chains, saves %.1f%% of calls, loses %.1f%% of improvements, loses %d incumbent gains"
              % (t, 100.0 * len(cut) / n, 100.0 * saved / calls, 100.0 * len(lost) / max(1, len(imp)), sum(1 for r in cut if r[4] < r[1])))
    ch = [r[5] for r in rows]
    print("   calls per step: mean %.2f, q50 %d, q90 %d, max %d" % (st.mean(ch), sorted(ch)[n // 2], sorted(ch)[int(.9 * n)], max(ch)))
