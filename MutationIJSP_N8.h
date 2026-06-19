/*
 * MutationIJSP_N8.h
 *
 * N8-based mutation operator for the IJSP.
 *
 * A structure-aware perturbation: it applies `k` RANDOM extra-block reinsertion
 * moves taken from the N8 neighbourhood (move a critical-block boundary task to
 * the opposite end of its own block, or to a machine extreme — see
 * NeighbourhoodIJSP_N8). Two deliberate differences with the N8 local-search
 * neighbourhood make it a mutation rather than a search step:
 *   - NO improvement clipping: a mutation must be free to worsen the solution
 *     (and at a local optimum the clipped neighbourhood may be empty);
 *   - the moves are chosen uniformly at random, not by quality.
 *
 * `k` (parameter `mutation.n8.moves`, default 1) is the perturbation strength:
 * the critical blocks are re-identified after each accepted move, so the k moves
 * form a chain. k=1 is easily undone by an N2 local search; k>=2 gives a
 * stronger ILS-style "kick".
 *
 * Cost per call: decode -> [identify critical blocks (O(n) BFS) -> one random
 * reinsertion] x k -> re-encode. No move estimation/evaluation, so it is far
 * cheaper than an N8 search step and negligible next to a tabu-search local
 * search. It is meant to complement an N2-based local search: the moves it makes
 * are exactly the extra-block reinsertions N2 cannot reach in a single step.
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

#define MUTATION_N8_MOVES "mutation.n8.moves"	// perturbation strength k (>=1)

class MutationIJSP_N8 : public FuzzyFW::Mutation {
protected:
	// Number of chained random N8 moves applied per mutation (perturbation
	// strength). Read from `mutation.n8.moves`; default 1.
	int kMoves;

public:
	explicit MutationIJSP_N8(FuzzyFW::ParameterDB *parameters = NULL)
		: FuzzyFW::Mutation(), kMoves(1) {
		if (parameters != NULL)
			this->setup(parameters);
	}

	virtual void setup(FuzzyFW::ParameterDB *parameters);

	virtual void apply(FuzzyFW::Individual *individual,
		const FuzzyFW::SharedVars *svars) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		name.push_back("IJSP-N8-reinsertion (random, unclipped, k="
			+ std::to_string(this->kMoves) + ")");
		return name;
	}
};

} // namespace IJSP
