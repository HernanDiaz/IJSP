/*
 * MutationIJSP_N8.h
 *
 * N8-based mutation operator for the IJSP.
 *
 * A structure-aware perturbation: it applies a RANDOM extra-block reinsertion
 * move taken from the N8 neighbourhood (move a critical-block boundary task to
 * the opposite end of its own block, or to a machine extreme — see
 * NeighbourhoodIJSP_N8). Two deliberate differences with the N8 local-search
 * neighbourhood make it a mutation rather than a search step:
 *   - NO improvement clipping: a mutation must be free to worsen the solution
 *     (and at a local optimum the clipped neighbourhood may be empty);
 *   - only ONE move is taken (chosen uniformly at random, not the best).
 *
 * Cost: decode -> identify critical blocks (O(n) BFS) -> one random reinsertion
 * -> re-encode. No move estimation/evaluation, so it is far cheaper than an N8
 * search step and negligible next to a tabu-search local search. It is meant
 * to complement an N2-based local search: the moves it makes are exactly the
 * extra-block reinsertions that N2 cannot reach in a single step (ILS-style
 * perturbation).
 *
 * Works at the schedule level (decode -> move -> re-encode) so, unlike the
 * generic genotype mutations (Insertion/Swap/Inversion), it is IJSP-specific.
 * It plugs into any algorithm through the standard Mutation::apply interface;
 * no algorithm code changes are required.
 *
 * @author hdiaz
 */
#pragma once

#include "Mutation.h"

namespace IJSP {

class MutationIJSP_N8 : public FuzzyFW::Mutation {
public:
	explicit MutationIJSP_N8(FuzzyFW::ParameterDB *parameters = NULL)
		: FuzzyFW::Mutation(parameters) { }

	virtual void apply(FuzzyFW::Individual *individual,
		const FuzzyFW::SharedVars *svars) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		name.push_back("IJSP-N8-reinsertion (random, unclipped)");
		return name;
	}
};

} // namespace IJSP
