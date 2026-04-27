/*
 * NeighbourhoodIJSP_N8.h
 *
 * N8 neighbourhood for Makespan minimization in IJSP.
 *
 * Extends N2 with extra-block reinsertion moves:
 *   - N2 boundary arc swaps (inherited from NB_ParallelN2_MakespanIJSP)
 *   - Extra-block reinsertion: move a critical task T to a position outside
 *     its current critical block on the same machine.
 *
 * A move is represented by NeighbourIJSP_Arc(x, y, z, tipo):
 *   tipo = 0  ->  N2 boundary swap of tasks x (mp) and y (ms of x)
 *   tipo = 1  ->  reinsertion of task x, inserted after task y (new mp),
 *                 before task z (new ms). y=-1 means insert at machine head;
 *                 z=-1 means insert at machine tail.
 *
 * Neighbourhood clipping: reinsertion moves whose heads&tails lower bound
 * is >= current makespan are discarded.
 *
 * Reference:
 *   Xie, J., Li, X., Gao, L., Gui, L. (2022). A new neighbourhood structure
 *   for job shop scheduling problems. International Journal of Production
 *   Research, 61(7), 2147-2161.
 *
 *   Adaptation to IJSP (interval processing times, dual G-/G+ extreme graphs):
 *   extra-block positions are defined with respect to critical blocks in
 *   G-(sigma) union G+(sigma).
 *
 * @author hdiaz
 */
#pragma once

#include "NeighbourhoodIJSP_Cmax.h"

namespace IJSP {

class NB_ParallelN8_MakespanIJSP : public NB_ParallelN2_MakespanIJSP {

public:
    NB_ParallelN8_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
        : NB_ParallelN2_MakespanIJSP(parameters) { }

    NB_ParallelN8_MakespanIJSP(const NB_ParallelN8_MakespanIJSP & source)
        : NB_ParallelN2_MakespanIJSP(source) { }

    virtual Neighbourhood * clone() const {
        return new NB_ParallelN8_MakespanIJSP(*this);
    }

    virtual std::vector<std::string> getName() {
        std::vector<std::string> setup;
        std::string value;
        setup.push_back("Makespan-N8");
        value = "Estimator:;";
        if (this->estimator == Estimator::NONE)
            value += NB_ESTIMATOR_NONE;
        else if (this->estimator == Estimator::ESTIM_HEADTAILS)
            value += NB_ESTIMATOR_HEADSTAILS;
        setup.push_back(value);
        return setup;
    }

    // Overrides: add extra-block reinsertion moves on top of N2
    virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);

    // Overrides: handle tipo=1 reinsertion in addition to tipo=0 swap
    virtual FuzzyFW::Fitness * evaluateNeighbour(const unsigned int idx,
        const FuzzyFW::SharedVars *svars, const bool improvement = false);

    // Overrides: dispatch estimation to N2 for tipo=0, or reinsertion estimate for tipo=1
    virtual FuzzyFW::Fitness * getEstimation(const unsigned int idx,
        const FuzzyFW::SharedVars *svars);

    // Overrides: handle tipo=1 reinsertion tail update
    virtual void acceptNeighbour(const unsigned int idx,
        const FuzzyFW::SharedVars *svars);

private:
    // Heads&tails lower-bound estimate for a reinsertion move
    // Returns estimated makespan if T (=arc->x) is inserted after newMp (=arc->y)
    // before newMs (=arc->z). Used for neighbourhood clipping.
    FuzzyFW::Interval estimateReinsertion(unsigned int T, int newMp, int newMs);
};

} // namespace IJSP
