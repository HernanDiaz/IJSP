/*
 * NeighbourhoodJSP_N8.cpp
 *
 * Implementation of the N8 neighbourhood for JSP makespan minimization.
 * See NeighbourhoodJSP_N8.h for full documentation.
 *
 * @author hdiaz
 */

#include "NeighbourhoodJSP_N8.h"
#include <queue>
#include <vector>

namespace JSP {

// ============================================================================
//  findNewNeighbours
//  Generates N2 boundary swaps (via parent) + extra-block reinsertion moves.
// ============================================================================
unsigned int NB_ParallelN8_MakespanJSP::findNewNeighbours(
    const FuzzyFW::SharedVars *svars)
{
    // --- 1. Get the N2 boundary arc swaps (tipo=0) --------------------------
    NB_ParallelN2_MakespanJSP::findNewNeighbours(svars);

    unsigned int nTasks = this->schedule->getScheduledTasks();
    if (nTasks < 3)
        return this->numNeighbours;

    FuzzyFW::Crisp currentMakespan = this->currentFitness->getValue();

    // --- 2. Identify critical tasks and critical-block membership ----------
    // criticalBlock[t] = true  ↔  task t is on a critical path in G⁻ or G⁺
    std::vector<char> criticalBlock(nTasks, false);
    std::queue<int> taskQueue;

    // One pass over the critical path. With interval durations this ran
    // once per endpoint, because G- and G+ are different graphs; on crisp
    // times they are the same graph, so the second pass re-asked the
    // questions the first had answered and added[] discarded the answers.
    for (size_t i = 0; i < this->schedule->lastTaskMachine.size(); i++) {
        int last = this->schedule->lastTaskMachine[i];
        if (last >= 0 &&
            this->schedule->getCTMachine(i) == currentMakespan) {
            criticalBlock[last] = true;
            taskQueue.push(last);
        }
    }
    while (!taskQueue.empty()) {
        int tid = taskQueue.front(); taskQueue.pop();
        ScheduledTaskInfo &t = this->schedule->taskInfo[tid];

        // Machine predecessor on critical path?
        if (t.mp != -1 && t.mp != t.task->jp) {
            ScheduledTaskInfo &mp = this->schedule->taskInfo[t.mp];
            if ((mp.head + mp.task->p) == t.head) {
                if (!criticalBlock[t.mp]) {
                    criticalBlock[t.mp] = true;
                    taskQueue.push(t.mp);
                }
            }
        }
        // Job predecessor on critical path?
        if (t.task->jp != -1) {
            ScheduledTaskInfo &jp = this->schedule->taskInfo[t.task->jp];
            if ((jp.head + jp.task->p) == t.head) {
                if (!criticalBlock[t.task->jp]) {
                    criticalBlock[t.task->jp] = true;
                    taskQueue.push(t.task->jp);
                }
            }
        }
    }

    // --- 3. Extra-block reinsertion moves (N8 extension) ---------------------
    //
    // For each critical block B on a machine, we try moving each boundary task
    // of B to the two positions adjacent to B on the same machine:
    //   - Position just BEFORE the block start (between block_start.mp and block_start)
    //   - Position just AFTER the block end   (between block_end and block_end.ms)
    //
    // This is the minimal set that captures the spirit of N8 (extra-block moves)
    // while keeping the neighbourhood size O(K) — same order as N2 — so that
    // Hill Climbing remains tractable on large instances.
    //
    // Neighbourhood clipping: skip the move if the reinsertion estimate is >=
    // the current makespan.
    //
    // We process blocks: a block is a maximal consecutive run of critical tasks
    // on the same machine. For each block we identify:
    //   blockStart = first critical task in the block (mp is not critical or mp==-1)
    //   blockEnd   = last  critical task in the block (ms is not critical or ms==-1)

    // Build block info for each critical task (only process block boundaries)
    std::vector<char> blockProcessed(nTasks, false);

    for (unsigned int T = 0; T < nTasks; T++) {
        if (!criticalBlock[T]) continue;

        // Only process if T is the START of a block (mp is not critical or mp==-1 or mp==jp)
        ScheduledTaskInfo &tInfo = this->schedule->taskInfo[T];
        int mp_T = tInfo.mp;
        bool isBlockStart = (mp_T == -1 || !criticalBlock[mp_T]);
        if (!isBlockStart) continue;

        int machine = tInfo.task->machine;

        // Walk forward to find block end
        int blockEnd = (int)T;
        while (true) {
            int ms_cur = this->schedule->taskInfo[blockEnd].ms;
            if (ms_cur == -1 || !criticalBlock[ms_cur]) break;
            blockEnd = ms_cur;
        }
        int blockStart = (int)T;
        int beforeBlock_mp = this->schedule->taskInfo[blockStart].mp; // -1 if at machine head
        int afterBlock_ms  = this->schedule->taskInfo[blockEnd].ms;   // -1 if at machine tail

        // --- Move A: move blockStart to just AFTER blockEnd ---
        // i.e. insert blockStart between blockEnd and afterBlock_ms
        // (equivalent to "pull the first block element to the other side")
        if (afterBlock_ms != -1) {  // there is a non-critical task after the block
            int newMp = blockEnd;
            int newMs = afterBlock_ms;
            // Skip if this is blockStart's current position (blockStart is not blockEnd.ms)
            // Also check the move makes a structural change
            if (newMp != (int)blockStart && newMs != (int)blockStart) {
                FuzzyFW::Crisp estMs = estimateReinsertion(blockStart, newMp, newMs);
                if (estMs < currentMakespan) {
                    if (this->numNeighbours < this->neighbours.size()
                        && this->neighbours[this->numNeighbours] != nullptr)
                        this->neighbours[this->numNeighbours]->setValues(
                            (unsigned int)blockStart, (unsigned int)newMp, (unsigned int)newMs, 1);
                    else
                        this->neighbours.push_back(std::make_unique<NeighbourJSP_Arc>(
                            (unsigned int)blockStart, (unsigned int)newMp, (unsigned int)newMs, 1));
                    this->numNeighbours++;
                }
            }
        }

        // --- Move B: move blockEnd to just BEFORE blockStart ---
        // i.e. insert blockEnd between beforeBlock_mp and blockStart
        if (beforeBlock_mp != -1) {  // there is a non-critical task before the block
            int newMp = beforeBlock_mp;
            int newMs = blockStart;
            if (newMp != blockEnd && newMs != blockEnd) {
                FuzzyFW::Crisp estMs = estimateReinsertion(blockEnd, newMp, newMs);
                if (estMs < currentMakespan) {
                    if (this->numNeighbours < this->neighbours.size()
                        && this->neighbours[this->numNeighbours] != nullptr)
                        this->neighbours[this->numNeighbours]->setValues(
                            (unsigned int)blockEnd, (unsigned int)newMp, (unsigned int)newMs, 1);
                    else
                        this->neighbours.push_back(std::make_unique<NeighbourJSP_Arc>(
                            (unsigned int)blockEnd, (unsigned int)newMp, (unsigned int)newMs, 1));
                    this->numNeighbours++;
                }
            }
        }

        // --- Move C: move blockStart to machine head (before all tasks) ---
        // Only if the machine head is not the block itself
        if (beforeBlock_mp != -1) {  // block is not already at machine head
            int newMp_int = -1;
            // Find the first task on the machine (different from blockStart)
            // The machine head: follow mp links from blockStart all the way back
            int machineHead = blockStart;
            while (this->schedule->taskInfo[machineHead].mp != -1)
                machineHead = this->schedule->taskInfo[machineHead].mp;
            // machineHead is the first task on the machine
            if (machineHead != blockStart) {
                FuzzyFW::Crisp estMs = estimateReinsertion(blockStart, newMp_int, machineHead);
                if (estMs < currentMakespan) {
                    if (this->numNeighbours < this->neighbours.size()
                        && this->neighbours[this->numNeighbours] != nullptr)
                        this->neighbours[this->numNeighbours]->setValues(
                            (unsigned int)blockStart, (unsigned int)(-1), (unsigned int)machineHead, 1);
                    else
                        this->neighbours.push_back(std::make_unique<NeighbourJSP_Arc>(
                            (unsigned int)blockStart, (unsigned int)(-1), (unsigned int)machineHead, 1));
                    this->numNeighbours++;
                }
            }
        }

        // --- Move D: move blockEnd to machine tail (after all tasks) ---
        if (afterBlock_ms != -1) {  // block is not already at machine tail
            int machineTail = blockEnd;
            while (this->schedule->taskInfo[machineTail].ms != -1)
                machineTail = this->schedule->taskInfo[machineTail].ms;
            if (machineTail != blockEnd) {
                FuzzyFW::Crisp estMs = estimateReinsertion(blockEnd, machineTail, -1);
                if (estMs < currentMakespan) {
                    if (this->numNeighbours < this->neighbours.size()
                        && this->neighbours[this->numNeighbours] != nullptr)
                        this->neighbours[this->numNeighbours]->setValues(
                            (unsigned int)blockEnd, (unsigned int)machineTail, (unsigned int)(-1), 1);
                    else
                        this->neighbours.push_back(std::make_unique<NeighbourJSP_Arc>(
                            (unsigned int)blockEnd, (unsigned int)machineTail, (unsigned int)(-1), 1));
                    this->numNeighbours++;
                }
            }
        }
    }

    return this->numNeighbours;
}


// ============================================================================
//  estimateReinsertion
//  Heads-and-tails lower bound for inserting task T after newMp, before newMs.
//  Uses the current tails[] array (from the parent class).
// ============================================================================
FuzzyFW::Crisp NB_ParallelN8_MakespanJSP::estimateReinsertion(
    unsigned int T, int newMp, int newMs)
{
    ScheduledTaskInfo &tInfo = this->schedule->taskInfo[T];
    int jpT = tInfo.task->jp;
    int currentMs = tInfo.ms;
    int currentMp = tInfo.mp;

    // Estimated head of T at the new position:
    //   max(head[newMp] + p[newMp],  head[jpT] + p[jpT])
    FuzzyFW::Crisp newHeadT(0);
    if (newMp != -1) {
        ScheduledTaskInfo &mpInfo = this->schedule->taskInfo[newMp];
        newHeadT = std::max(newHeadT, mpInfo.head + mpInfo.task->p);
    }
    if (jpT != -1) {
        ScheduledTaskInfo &jpInfo = this->schedule->taskInfo[jpT];
        newHeadT = std::max(newHeadT, jpInfo.head + jpInfo.task->p);
    }

    // Estimated tail of T at the new position:
    //   tail[T] as stored (a lower bound — the job-chain tail does not change)
    FuzzyFW::Crisp tailT = this->tails[T];

    // If newMs is not -1, the new machine successor of T must wait for T to finish.
    // Its estimated new head: max(newHeadT + p[T], head[newMs.jp] + p[newMs.jp])
    // We compute a lower bound on the new makespan contributed by newMs:
    FuzzyFW::Crisp makespan = newHeadT + tInfo.task->p + tailT;

    if (newMs != -1) {
        ScheduledTaskInfo &msInfo = this->schedule->taskInfo[newMs];
        int jpMs = msInfo.task->jp;
        FuzzyFW::Crisp newHeadMs = newHeadT + tInfo.task->p;
        if (jpMs != -1) {
            ScheduledTaskInfo &jpMsInfo = this->schedule->taskInfo[jpMs];
            newHeadMs = std::max(newHeadMs, jpMsInfo.head + jpMsInfo.task->p);
        }
        // Use existing tail of newMs as a lower bound
        makespan = std::max(makespan, newHeadMs + msInfo.task->p + this->tails[newMs]);
    }

    // Also consider the position opened by removing T:
    //   currentMs now follows currentMp directly.
    //   Its new estimated head: max(head[currentMp]+p[currentMp], head[jpCurrentMs]+p[jpCurrentMs])
    if (currentMs != -1) {
        ScheduledTaskInfo &csInfo = this->schedule->taskInfo[currentMs];
        int jpCs = csInfo.task->jp;
        FuzzyFW::Crisp newHeadCs(0);
        if (currentMp != -1) {
            ScheduledTaskInfo &cpInfo = this->schedule->taskInfo[currentMp];
            newHeadCs = std::max(newHeadCs, cpInfo.head + cpInfo.task->p);
        }
        if (jpCs != -1) {
            ScheduledTaskInfo &jpCsInfo = this->schedule->taskInfo[jpCs];
            newHeadCs = std::max(newHeadCs, jpCsInfo.head + jpCsInfo.task->p);
        }
        // newHeadCs could be LESS than csInfo.head (that's the improvement we hope for)
        // Use the better of the two as our lower bound estimate
        FuzzyFW::Crisp usedHead = newHeadCs;
        makespan = std::max(makespan, usedHead + csInfo.task->p + this->tails[currentMs]);
    }

    return makespan;
}


// ============================================================================
//  getEstimation
//  Dispatches to N2's heads&tails for tipo=0 arc swaps.
//  For tipo=1 reinsertion moves, uses estimateReinsertion (already computed
//  during findNewNeighbours as the clipping filter), stored as estimated quality.
// ============================================================================
FuzzyFW::Fitness * NB_ParallelN8_MakespanJSP::getEstimation(
    const unsigned int idx, const FuzzyFW::SharedVars *svars)
{
    if (idx >= this->numNeighbours || this->neighbours[idx] == nullptr) {
        std::string errorMsg = "Trying to access a non-existing neighbour";
        throw JSPException("Neighbourhood", errorMsg);
    }

    NeighbourJSP_Arc *arc = this->neighbours[idx].get();

    // For tipo=0 (N2 boundary swap), delegate to the N2 implementation
    if (arc->tipo == 0)
        return NB_ParallelN2_MakespanJSP::getEstimation(idx, svars);

    // For tipo=1 reinsertion, compute estimation if not already done
    if (!arc->isEstimated()) {
        int newMp = (arc->y == (unsigned int)(-1)) ? -1 : (int)arc->y;
        int newMs = (arc->z == (unsigned int)(-1)) ? -1 : (int)arc->z;
        FuzzyFW::Crisp estMakespan = estimateReinsertion(arc->x, newMp, newMs);
        arc->setEstimatedQuality(new FuzzyFW::FitnessCrisp(estMakespan, false));
    }
    return arc->getEstimatedQuality();
}


// ============================================================================
//  evaluateNeighbour
//  Dispatches to the N2 evaluator for tipo=0, or performs reinsertion for tipo=1.
// ============================================================================
FuzzyFW::Fitness * NB_ParallelN8_MakespanJSP::evaluateNeighbour(
    const unsigned int idx, const FuzzyFW::SharedVars *svars,
    const bool improvement)
{
    if (idx >= this->numNeighbours || this->neighbours[idx] == nullptr) {
        std::string errorMsg = "Trying to access a non-existing neighbour";
        throw JSPException("Neighbourhood", errorMsg);
    }

    NeighbourJSP_Arc *arc = this->neighbours[idx].get();

    // tipo=0: standard N2 boundary swap
    if (arc->tipo == 0)
        return NB_ParallelN2_MakespanJSP::evaluateNeighbour(idx, svars, improvement);

    // tipo=1: extra-block reinsertion of task arc->x
    //   current arc->y = new mp of T  (unsigned, -1 cast = UINT_MAX means none)
    //   current arc->z = new ms of T  (unsigned, -1 cast = UINT_MAX means none)
    unsigned int T   = arc->x;
    int newMp = (arc->y == (unsigned int)(-1)) ? -1 : (int)arc->y;
    int newMs = (arc->z == (unsigned int)(-1)) ? -1 : (int)arc->z;

    FuzzyFW::Crisp currentMakespan = this->currentFitness->getValue();
    ScheduleJSP *newSolution = new ScheduleJSP(*this->schedule);
    // Pruning bound, on the stack: a FitnessCrisp is an int and a flag, so
    // cloning it onto the heap cost an allocation, a free and a virtual
    // call per neighbour evaluated.
    FuzzyFW::FitnessCrisp lowerBound(*this->currentFitness);

    // Current neighbours of T
    int oldMp = newSolution->taskInfo[T].mp;
    int oldMs = newSolution->taskInfo[T].ms;
    int machine = newSolution->taskInfo[T].task->machine;

    // --- Remove T from its current position ---
    if (oldMp != -1)
        newSolution->taskInfo[oldMp].ms = oldMs;
    if (oldMs != -1)
        newSolution->taskInfo[oldMs].mp = oldMp;
    else
        newSolution->lastTaskMachine[machine] = oldMp;

    // --- Insert T between newMp and newMs ---
    newSolution->taskInfo[T].mp = newMp;
    newSolution->taskInfo[T].ms = newMs;
    if (newMp != -1)
        newSolution->taskInfo[newMp].ms = T;
    if (newMs != -1)
        newSolution->taskInfo[newMs].mp = T;
    else
        newSolution->lastTaskMachine[machine] = T;

    // --- Propagate head changes via BFS ---
    // Two roots of propagation:
    //  (a) oldMs: its machine predecessor changed from T to oldMp → head may decrease
    //  (b) T itself: its machine predecessor changed from oldMp to newMp → head may change
    int _nTasks = (int)newSolution->getScheduledTasks();
    int _bfsLimit = _nTasks * 20; // JSP DAG has <=2 predecessors/node: valid SPFA terminates in O(N); cycle if exceeded
    int _bfsCount = 0;
    std::queue<int> taskQueue;
    if (oldMs != -1) taskQueue.push(oldMs);
    taskQueue.push((int)T);

    while (!taskQueue.empty()) {
        if (++_bfsCount > _bfsLimit) {
            delete newSolution;
            return NULL;
        }
        int z = taskQueue.front(); taskQueue.pop();
        ScheduledTaskInfo &zi = newSolution->taskInfo[z];
        int jpz = zi.task->jp;
        int mpz = zi.mp;
        int job = zi.task->job;
        int jsz = (newSolution->lastTaskJob[job] == z) ? -1 : zi.task->js;
        int msz = zi.ms;

        FuzzyFW::Crisp newHead(0);
        if (jpz != -1 && mpz != -1)
            newHead = std::max(newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p, newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p);
        else if (mpz != -1)
            newHead = newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p;
        else if (jpz != -1)
            newHead = newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p;

        if (!(newSolution->taskInfo[z].head == newHead)) {
            newSolution->taskInfo[z].head = newHead;

            // Early termination if improvement=true and lower bound already worse
            if (improvement && jsz == -1) {
                lowerBound.setValue(newSolution->taskInfo[z].head
                    + newSolution->taskInfo[z].task->p);
                if (lowerBound.isWorseThan(currentFitness)) {
                    delete newSolution;
                    return NULL;
                }
            }
            if (msz != -1) taskQueue.push(msz);
            if (jsz != -1) taskQueue.push(jsz);
        }
    }

    // --- Compute new makespan ---
    FuzzyFW::Crisp newMakespan(0);
    for (size_t i = 0; i < newSolution->lastTaskJob.size(); i++)
        newMakespan = std::max(newMakespan, newSolution->getCTJob(i));

    newSolution->setSorted(false);
    this->neighbours[idx]->setEvaluation(newSolution,
        new FuzzyFW::FitnessCrisp(newMakespan, false));

    return this->neighbours[idx]->getEvaluatedFitness();
}


// ============================================================================
//  acceptNeighbour
//  Dispatches to N2 acceptNeighbour for tipo=0. For tipo=1, updates tails
//  after reinsertion and stores the new solution.
// ============================================================================
void NB_ParallelN8_MakespanJSP::acceptNeighbour(const unsigned int idx,
    const FuzzyFW::SharedVars *svars)
{
    if (idx >= this->numNeighbours || this->neighbours[idx] == nullptr) {
        std::string errorMsg = "Trying to access a non-existing neighbour";
        throw JSPException("Neighbourhood", errorMsg);
    }

    NeighbourJSP_Arc *arc = this->neighbours[idx].get();

    if (arc->tipo == 0) {
        NB_ParallelN2_MakespanJSP::acceptNeighbour(idx, svars);
        return;
    }

    // tipo=1: accept reinsertion
    if (!this->neighbours[idx]->isEvaluated())
        this->evaluateNeighbour(idx, svars, false);

    // Save oldMp BEFORE deleting the old schedule: after reinsertion, oldMp's
    // machine successor changed from T to oldMs, so its tail needs recomputing.
    unsigned int T = arc->x;
    int oldMp = this->schedule->taskInfo[T].mp;
    int newMp_signed = (arc->y == (unsigned int)(-1)) ? -1 : (int)arc->y;

    if (this->schedule != nullptr) delete this->schedule;
    this->schedule = dynamic_cast<ScheduleJSP *>(
        this->neighbours[idx]->getEvaluation()->clone());
    if (this->currentFitness != nullptr) delete this->currentFitness;
    this->currentFitness = dynamic_cast<FuzzyFW::FitnessCrisp *>(
        this->neighbours[idx]->getEvaluatedFitness()->clone());

    // Recompute tails for the three affected tasks:
    //   T        — its machine successor changed from oldMs to newMs
    //   oldMp    — its machine successor changed from T to oldMs
    //   newMp    — its machine successor changed from newMs to T
    std::vector<int> tailsUpdated;
    tailsUpdated.resize(this->schedule->getScheduledTasks(), 0);

    std::queue<int> taskQueue;
    auto enqueue = [&](int t) {
        if (t >= 0 && !tailsUpdated[t]) {
            tailsUpdated[t] = 1;
            taskQueue.push(t);
        }
    };

    enqueue((int)T);
    // oldMp: its machine successor is now oldMs (T was removed from there)
    if (oldMp != -1)
        enqueue(oldMp);
    // newMp: its machine successor is now T (T was inserted after it)
    if (newMp_signed != -1)
        enqueue(newMp_signed);

    while (!taskQueue.empty()) {
        int z = taskQueue.front(); taskQueue.pop();
        int msz = this->schedule->taskInfo[z].ms;
        int job  = this->schedule->taskInfo[z].task->job;
        int jsz  = (this->schedule->lastTaskJob[job] == z) ? -1
                 : this->schedule->taskInfo[z].task->js;

        FuzzyFW::Crisp newTail(0);
        bool hasSuc = false;
        if (jsz != -1 && msz != -1) {
            newTail = std::max(this->schedule->taskInfo[msz].task->p + this->tails[msz], this->schedule->taskInfo[jsz].task->p + this->tails[jsz]);
            hasSuc = true;
        } else if (msz != -1) {
            newTail = this->schedule->taskInfo[msz].task->p + this->tails[msz];
            hasSuc = true;
        } else if (jsz != -1) {
            newTail = this->schedule->taskInfo[jsz].task->p + this->tails[jsz];
            hasSuc = true;
        }

        if (hasSuc && !(this->tails[z] == newTail)) {
            this->tails[z] = newTail;
            if (this->schedule->taskInfo[z].mp != -1)
                enqueue(this->schedule->taskInfo[z].mp);
            if (this->schedule->taskInfo[z].task->jp != -1)
                enqueue(this->schedule->taskInfo[z].task->jp);
        }
    }
}

} // namespace JSP
