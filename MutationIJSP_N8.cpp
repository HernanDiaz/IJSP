/*
 * MutationIJSP_N8.cpp
 *
 * Implementation of the N8-based mutation operator. See MutationIJSP_N8.h.
 *
 * The critical-block identification and the reinsertion link surgery mirror
 * NB_ParallelN8_MakespanIJSP (NeighbourhoodIJSP_N8.cpp), but without the
 * improvement clipping and with k random moves.
 *
 * @author hdiaz
 */

#include "MutationIJSP_N8.h"
#include "SharedVarsEvolutionary.h"
#include "ParameterDB.h"
#include "Decoder.h"
#include "Encoder.h"
#include "Individual.h"
#include "ScheduleIJSP.h"
#include <queue>
#include <vector>
#include <algorithm>

namespace IJSP {

namespace {

// A candidate reinsertion: put task `t` between machine-predecessor `newMp`
// and machine-successor `newMs` (either may be -1 = machine head / tail).
struct N8Move {
	int t;
	int newMp;
	int newMs;
	N8Move(int t_, int mp_, int ms_) : t(t_), newMp(mp_), newMs(ms_) { }
};


// ---- makespan = max completion time over jobs ------------------------------
FuzzyFW::Interval makespanOf(const ScheduleIJSP *s) {
	FuzzyFW::Interval mk(0.0, 0.0);
	for (size_t j = 0; j < s->lastTaskJob.size(); j++)
		mk = maximum(mk, s->getCTJob((unsigned int)j),
			FuzzyFW::Interval::M_COMPONENT);
	return mk;
}


// ---- mark every task on a critical path of G- or G+ ------------------------
// Mirrors NB_ParallelN8_MakespanIJSP::findNewNeighbours, step 2.
void identifyCritical(const ScheduleIJSP *s, const FuzzyFW::Interval &mk,
		std::vector<char> &crit) {
	std::queue<int> q;
	for (short comp = 1; comp <= 2; comp++) {
		for (size_t i = 0; i < s->lastTaskMachine.size(); i++) {
			int last = s->lastTaskMachine[i];
			if (last >= 0 &&
				s->getCTMachine((unsigned int)i).EqualComponent(mk, comp)) {
				if (!crit[last]) { crit[last] = 1; q.push(last); }
			}
		}
		while (!q.empty()) {
			int tid = q.front(); q.pop();
			const ScheduledTaskInfo &t = s->taskInfo[tid];
			if (t.mp != -1) {
				const ScheduledTaskInfo &mp = s->taskInfo[t.mp];
				if ((mp.head + mp.task->p).EqualComponent(t.head, comp)
					&& !crit[t.mp]) {
					crit[t.mp] = 1; q.push(t.mp);
				}
			}
			if (t.task->jp != -1) {
				const ScheduledTaskInfo &jp = s->taskInfo[t.task->jp];
				if ((jp.head + jp.task->p).EqualComponent(t.head, comp)
					&& !crit[t.task->jp]) {
					crit[t.task->jp] = 1; q.push(t.task->jp);
				}
			}
		}
	}
}


// ---- collect the unclipped N8 extra-block reinsertion moves ----------------
// For every critical block, move its first/last task to the opposite end of
// the block or to the machine head/tail (N8 moves A/B/C/D, no clipping).
void collectMoves(const ScheduleIJSP *s, const std::vector<char> &crit,
		std::vector<N8Move> &moves) {
	int n = (int)s->taskInfo.size();
	for (int T = 0; T < n; T++) {
		if (s->taskInfo[T].task == NULL || !crit[T]) continue;
		int mpT = s->taskInfo[T].mp;
		if (!(mpT == -1 || !crit[mpT])) continue;	// T must start a block

		int blockStart = T;
		int blockEnd = T;
		while (true) {
			int ms = s->taskInfo[blockEnd].ms;
			if (ms == -1 || !crit[ms]) break;
			blockEnd = ms;
		}
		int beforeMp = s->taskInfo[blockStart].mp;	// -1 at machine head
		int afterMs  = s->taskInfo[blockEnd].ms;	// -1 at machine tail

		// A: blockStart -> just after blockEnd
		if (afterMs != -1 && blockEnd != blockStart && afterMs != blockStart)
			moves.push_back(N8Move(blockStart, blockEnd, afterMs));
		// B: blockEnd -> just before blockStart
		if (beforeMp != -1 && beforeMp != blockEnd && blockStart != blockEnd)
			moves.push_back(N8Move(blockEnd, beforeMp, blockStart));
		// C: blockStart -> machine head
		if (beforeMp != -1) {
			int head = blockStart;
			while (s->taskInfo[head].mp != -1) head = s->taskInfo[head].mp;
			if (head != blockStart)
				moves.push_back(N8Move(blockStart, -1, head));
		}
		// D: blockEnd -> machine tail
		if (afterMs != -1) {
			int tail = blockEnd;
			while (s->taskInfo[tail].ms != -1) tail = s->taskInfo[tail].ms;
			if (tail != blockEnd)
				moves.push_back(N8Move(blockEnd, tail, -1));
		}
	}
}


// ---- link surgery for a reinsertion (mirrors N8 evaluateNeighbour) ---------
void applyMove(ScheduleIJSP *s, const N8Move &mv) {
	int T = mv.t;
	int oldMp = s->taskInfo[T].mp;
	int oldMs = s->taskInfo[T].ms;
	int machine = s->taskInfo[T].task->machine;

	// remove T from its current position
	if (oldMp != -1) s->taskInfo[oldMp].ms = oldMs;
	if (oldMs != -1) s->taskInfo[oldMs].mp = oldMp;
	else             s->lastTaskMachine[machine] = oldMp;

	// insert T between newMp and newMs
	s->taskInfo[T].mp = mv.newMp;
	s->taskInfo[T].ms = mv.newMs;
	if (mv.newMp != -1) s->taskInfo[mv.newMp].ms = T;
	if (mv.newMs != -1) s->taskInfo[mv.newMs].mp = T;
	else                s->lastTaskMachine[machine] = T;
}


// ---- recompute all heads in topological order ------------------------------
// Returns false if the (modified) machine sequences create a cycle, i.e. the
// move was infeasible. O(n) Kahn traversal (see ScheduleIJSP::verifyHeads).
bool recomputeHeads(ScheduleIJSP *s) {
	int n = (int)s->taskInfo.size();
	std::vector<int> remaining(n, 0);
	for (int t = 0; t < n; t++) {
		if (s->taskInfo[t].task == NULL) continue;
		if (s->taskInfo[t].task->jp != -1) remaining[t]++;
		if (s->taskInfo[t].mp != -1) remaining[t]++;
	}
	std::queue<int> q;
	int total = 0;
	for (int t = 0; t < n; t++)
		if (s->taskInfo[t].task != NULL) {
			total++;
			if (remaining[t] == 0) q.push(t);
		}

	int processed = 0;
	while (!q.empty()) {
		int t = q.front(); q.pop();
		processed++;
		int jp = s->taskInfo[t].task->jp;
		int mp = s->taskInfo[t].mp;
		FuzzyFW::Interval h(0.0, 0.0);
		if (jp != -1)
			h = maximum(h, s->taskInfo[jp].head + s->taskInfo[jp].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		if (mp != -1)
			h = maximum(h, s->taskInfo[mp].head + s->taskInfo[mp].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		s->taskInfo[t].head = h;

		int job = s->taskInfo[t].task->job;
		int ms = s->taskInfo[t].ms;
		int js = (s->lastTaskJob[job] == t) ? -1 : s->taskInfo[t].task->js;
		if (ms != -1 && --remaining[ms] == 0) q.push(ms);
		if (js != -1 && --remaining[js] == 0) q.push(js);
	}
	return processed == total;
}

} // anonymous namespace


//-----  setup  ---------------------------------------------------------------
void MutationIJSP_N8::setup(FuzzyFW::ParameterDB *parameters) {
	FuzzyFW::Mutation::setup(parameters);
	this->kMoves = parameters->getInteger(MUTATION_N8_MOVES, 1);
	if (this->kMoves < 1)
		this->kMoves = 1;
}


//-----  apply  ---------------------------------------------------------------
void MutationIJSP_N8::apply(FuzzyFW::Individual *individual,
		const FuzzyFW::SharedVars *svars) const {

	if (svars == NULL || individual == NULL)
		return;
	// The evolutionary algorithms always pass a SharedVarsEvolutionary (it is
	// where encoder/decoder live). SharedVars is not polymorphic, so a
	// static_cast is the correct down-cast here.
	const FuzzyFW::SharedVarsEvolutionary *esv =
		static_cast<const FuzzyFW::SharedVarsEvolutionary *>(svars);
	if (esv->decoder == NULL || esv->encoder == NULL)
		return;

	// Current schedule. Do NOT delete it: it is owned either by the individual
	// (its phenotype) or by the decoder's SGS — exactly as in EvaluationIJSP.
	FuzzyFW::Solution *solution = individual->isPhenotypeUpdated()
		? individual->getPhenotype()
		: esv->decoder->decode(individual, esv);
	ScheduleIJSP *base = dynamic_cast<ScheduleIJSP *>(solution);
	if (base == NULL || base->getScheduledTasks() < 3)
		return;

	// Work on a clone we own, so an infeasible (cyclic) trial move can be
	// discarded without corrupting the source schedule.
	ScheduleIJSP *work = dynamic_cast<ScheduleIJSP *>(base->clone());
	if (work == NULL)
		return;
	if (!recomputeHeads(work)) { delete work; return; }

	// Apply up to kMoves chained random reinsertions. The critical blocks are
	// re-identified after each accepted move (the heads, hence the critical
	// structure, change), so the moves form a perturbation chain.
	int applied = 0;
	for (int step = 0; step < this->kMoves; step++) {
		FuzzyFW::Interval mk = makespanOf(work);
		std::vector<char> crit(work->taskInfo.size(), 0);
		identifyCritical(work, mk, crit);

		std::vector<N8Move> moves;
		collectMoves(work, crit, moves);
		if (moves.empty())
			break;

		// Random order; take the first move that yields a feasible schedule.
		for (int i = (int)moves.size() - 1; i > 0; i--) {
			int j = svars->rng->getInteger(0, i);
			std::swap(moves[i], moves[j]);
		}

		bool ok = false;
		for (size_t i = 0; i < moves.size() && !ok; i++) {
			ScheduleIJSP *trial = dynamic_cast<ScheduleIJSP *>(work->clone());
			if (trial == NULL) continue;
			applyMove(trial, moves[i]);
			if (recomputeHeads(trial)) {
				delete work;
				work = trial;
				ok = true;
				applied++;
			} else {
				delete trial;
			}
		}
		if (!ok)
			break;	// no feasible move available at this step
	}
	if (applied == 0) { delete work; return; }

	// Re-encode the mutated schedule into the individual's genotype and store
	// it as the (now consistent) phenotype. setSorted(false) forces the encoder
	// to rebuild the topological order from the new machine sequences.
	work->setSorted(false);
	esv->encoder->encode(work, individual, esv);
	individual->updatePhenotype(work);	// individual takes ownership of `work`
}

} // namespace IJSP
