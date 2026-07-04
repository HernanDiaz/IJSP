#!/usr/bin/env python3
"""
rightshift.py — timing optimisation for a FIXED processing order (Phase 2b).

NPE is not regular: for a fixed order, delaying tasks (right-shifts) can
close idle gaps without touching the makespan. By the window identity,
NPE_i = sum_k Pk*(C_i(k) - S_i(k)) - const, so the optimal timing wants each
machine's last completion as EARLY as possible and its first start as LATE
as possible, subject to the precedence system and Cmax_i preservation.

Two implementations, per interval component i (lower/upper independently):

- exact_rs_npe: exact optimum. For fixed order the problem is linear over a
  difference-constraint system (network matrix, integral optima) — solved
  with CP-SAT in milliseconds. This is the per-order analogue of the paper's
  LP post-processing (cf. Afsar et al. 2022).

- heuristic_rs_npe: HER-like single backward pass: machine-last tasks stay
  semi-active (never worsen C(k)); every other task is delayed to the
  minimum of its successors' starts. O(tasks), the candidate for the C++
  in-loop operator. Guaranteed semi_active >= heuristic >= exact.

Shared helpers build per-component semi-active heads from a task order.
"""
import os, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))


def build_structures(order, n, m, mach, dur, comp):
    """Semi-active heads for component comp (0=lower, 1=upper) + topology.
    Returns (heads, p, jp, js, mp, ms, mfirst, mlast, cmax)."""
    ntasks = n * m
    p = [0] * ntasks
    jp = [-1] * ntasks
    js = [-1] * ntasks
    mp = [-1] * ntasks
    ms = [-1] * ntasks
    heads = [0] * ntasks
    mac_last = [-1] * m
    mfirst = [-1] * m
    job_last = [-1] * n
    for tid in order:
        j, pos = divmod(tid, m)
        k = mach[j][pos]
        p[tid] = dur[j][pos][comp]
        if job_last[j] != -1:
            jp[tid] = job_last[j]
            js[job_last[j]] = tid
        if mac_last[k] != -1:
            mp[tid] = mac_last[k]
            ms[mac_last[k]] = tid
        else:
            mfirst[k] = tid
        h = 0
        if jp[tid] != -1:
            h = heads[jp[tid]] + p[jp[tid]]
        if mp[tid] != -1:
            h = max(h, heads[mp[tid]] + p[mp[tid]])
        heads[tid] = h
        job_last[j] = tid
        mac_last[k] = tid
    cmax = max(heads[t] + p[t] for t in job_last)
    return heads, p, jp, js, mp, ms, mfirst, mac_last, cmax


def machine_of(tid, m, mach):
    j, pos = divmod(tid, m)
    return mach[j][pos]


def busy_constant(order, m, mach, p, pp):
    """sum_k Pk * sum of processing times on machine k (order-invariant)."""
    busy = [0] * m
    for tid in order:
        busy[machine_of(tid, m, mach)] += p[tid]
    return sum(pw * b for pw, b in zip(pp, busy))


def npe_from_starts(starts, p, mfirst, mlast, pp, const):
    """Gap-form NPE = window form minus the busy constant."""
    window = sum(pw * ((starts[l] + p[l]) - starts[f])
                 for pw, f, l in zip(pp, mfirst, mlast) if f != -1)
    return window - const


def semi_active_npe(order, n, m, mach, dur, pp, comp):
    heads, p, _, _, _, _, mfirst, mlast, cmax = \
        build_structures(order, n, m, mach, dur, comp)
    const = busy_constant(order, m, mach, p, pp)
    return npe_from_starts(heads, p, mfirst, mlast, pp, const), cmax


def heuristic_rs_npe(order, n, m, mach, dur, pp, comp):
    """Backward pass: machine-last tasks pinned semi-active; others delayed
    to min(successor starts) - p. Never moves a task left."""
    heads, p, jp, js, mp, ms, mfirst, mlast, cmax = \
        build_structures(order, n, m, mach, dur, comp)
    starts = list(heads)
    is_mlast = [False] * (n * m)
    for l in mlast:
        if l != -1:
            is_mlast[l] = True
    for tid in reversed(order):
        if is_mlast[tid]:
            continue
        cap = cmax
        if js[tid] != -1:
            cap = min(cap, starts[js[tid]])
        if ms[tid] != -1:
            cap = min(cap, starts[ms[tid]])
        starts[tid] = max(starts[tid], cap - p[tid])
    const = busy_constant(order, m, mach, p, pp)
    return npe_from_starts(starts, p, mfirst, mlast, pp, const), cmax


def exact_rs_npe(order, n, m, mach, dur, pp, comp, time_limit=10):
    """Exact optimal timing for the fixed order (difference-constraint LP,
    solved with CP-SAT; integral by total unimodularity)."""
    from ortools.sat.python import cp_model
    heads, p, jp, _, mp, _, mfirst, mlast, cmax = \
        build_structures(order, n, m, mach, dur, comp)
    ntasks = n * m
    mdl = cp_model.CpModel()
    s = [mdl.NewIntVar(0, cmax - p[t], f"s{t}") for t in range(ntasks)]
    for t in range(ntasks):
        if jp[t] != -1:
            mdl.Add(s[t] >= s[jp[t]] + p[jp[t]])
        if mp[t] != -1:
            mdl.Add(s[t] >= s[mp[t]] + p[mp[t]])
    obj = []
    for pw, f, l in zip(pp, mfirst, mlast):
        if f != -1:
            obj.append(pw * (s[l] + p[l]))
            obj.append(-pw * s[f])
    mdl.Minimize(sum(obj))
    solver = cp_model.CpSolver()
    solver.parameters.max_time_in_seconds = time_limit
    solver.parameters.num_search_workers = 1
    st = solver.Solve(mdl)
    if st not in (cp_model.OPTIMAL, cp_model.FEASIBLE):
        raise RuntimeError(f"exact RS solve failed: {solver.StatusName(st)}")
    window = sum(pw * ((solver.Value(s[l]) + p[l]) - solver.Value(s[f]))
                 for pw, f, l in zip(pp, mfirst, mlast) if f != -1)
    npe = window - busy_constant(order, m, mach, p, pp)
    return int(npe), cmax, st == cp_model.OPTIMAL


def rs_summary(order, n, m, mach, dur, pp):
    """(semi, heur, exact) NPE midpoints across the two components."""
    out = {}
    for name, fn in (("semi", semi_active_npe), ("heur", heuristic_rs_npe)):
        lo, _ = fn(order, n, m, mach, dur, pp, 0)
        hi, _ = fn(order, n, m, mach, dur, pp, 1)
        out[name] = (lo + hi) / 2
    lo, _, _ = exact_rs_npe(order, n, m, mach, dur, pp, 0)
    hi, _, _ = exact_rs_npe(order, n, m, mach, dur, pp, 1)
    out["exact"] = (lo + hi) / 2
    return out
