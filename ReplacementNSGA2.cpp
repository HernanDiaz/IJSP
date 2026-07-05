/*
 * ReplacementNSGA2.cpp
 *
 *  Created on: Jul 5, 2026
 *      Author: hdiaz
 */

#include "ReplacementNSGA2.h"
#include <algorithm>
#include <limits>

namespace FuzzyFW {

//-----  Dominance over the product of the two component orders  --------------
bool ReplacementNSGA2::dominates(const Individual *a, const Individual *b) {
	const FitnessLexicographic *fa =
		dynamic_cast<const FitnessLexicographic *>(a->getFitness());
	const FitnessLexicographic *fb =
		dynamic_cast<const FitnessLexicographic *>(b->getFitness());
	if (fa == NULL || fb == NULL) {
		throw FuzzyFWException("Replacement", "NSGA-II replacement requires "
			"a lexicographic (makespan, energy) fitness. Use the "
			"ijsp.makespan-energy evaluation.");
	}
	bool strict = false;
	for (unsigned int k = 0; k < 2; k++) {
		Fitness *ak = fa->getFitness(k);
		Fitness *bk = fb->getFitness(k);
		if (bk->isBetterThan(ak))
			return false;		// worse in component k
		if (ak->isBetterThan(bk))
			strict = true;
	}
	return strict;
}


//-----  Survival selection  ---------------------------------------------------
void ReplacementNSGA2::apply(Population *oldPopulation,
	Population *newPopulation, const SharedVars *svars) const {

	const double INF = std::numeric_limits<double>::infinity();
	std::vector<Individual *> pool;
	for (unsigned int i = 0; i < oldPopulation->size(); i++)
		pool.push_back(oldPopulation->getIndividual(i));
	for (unsigned int i = 0; i < newPopulation->size(); i++)
		pool.push_back(newPopulation->getIndividual(i));
	size_t n = pool.size();
	size_t mu = newPopulation->size();

	// Feed the archive (single per-generation choke point)
	if (this->archive != NULL) {
		for (size_t i = 0; i < n; i++) {
			const FitnessLexicographic *f = dynamic_cast<
				const FitnessLexicographic *>(pool[i]->getFitness());
			if (f != NULL && pool[i]->isPhenotypeUpdated())
				this->archive->offer(pool[i]->getPhenotype()->toString(), f);
		}
	}

	// Non-dominated sorting (simple O(n^2) domination counting)
	std::vector<int> domCount(n, 0);
	std::vector< std::vector<size_t> > dominated(n);
	for (size_t i = 0; i < n; i++)
		for (size_t j = 0; j < n; j++) {
			if (i == j) continue;
			if (dominates(pool[i], pool[j]))
				dominated[i].push_back(j);
			else if (dominates(pool[j], pool[i]))
				domCount[i]++;
		}

	std::vector<size_t> order;			// pool indices, front by front
	std::vector<size_t> current, next;
	for (size_t i = 0; i < n; i++)
		if (domCount[i] == 0)
			current.push_back(i);
	std::vector<size_t> frontStart;		// offsets of each front in 'order'
	while (!current.empty() && order.size() < n) {
		frontStart.push_back(order.size());
		// Crowding distance inside the front (interval midpoints)
		size_t fn = current.size();
		std::vector<double> dist(fn, 0.0);
		if (fn <= 2)
			std::fill(dist.begin(), dist.end(), INF);
		else {
			for (int obj = 0; obj < 2; obj++) {
				auto val = [&](size_t k) {
					const FitnessLexicographic *f = dynamic_cast<
						const FitnessLexicographic *>(pool[current[k]]->getFitness());
					const FitnessInterval *c = dynamic_cast<
						const FitnessInterval *>(f->getFitness(obj));
					return (c->getValue().a + c->getValue().b) / 2.0;
				};
				std::vector<size_t> idx(fn);
				for (size_t k = 0; k < fn; k++) idx[k] = k;
				std::sort(idx.begin(), idx.end(),
					[&](size_t x, size_t y) { return val(x) < val(y); });
				double range = val(idx[fn - 1]) - val(idx[0]);
				dist[idx[0]] = dist[idx[fn - 1]] = INF;
				if (range <= 0) continue;
				for (size_t k = 1; k + 1 < fn; k++)
					dist[idx[k]] += (val(idx[k + 1]) - val(idx[k - 1])) / range;
			}
		}
		// Append the front ordered by decreasing crowding
		std::vector<size_t> byCrowd(fn);
		for (size_t k = 0; k < fn; k++) byCrowd[k] = k;
		std::sort(byCrowd.begin(), byCrowd.end(),
			[&](size_t x, size_t y) { return dist[x] > dist[y]; });
		for (size_t k = 0; k < fn; k++)
			order.push_back(current[byCrowd[k]]);
		// Build the next front
		next.clear();
		for (size_t k = 0; k < fn; k++)
			for (size_t d = 0; d < dominated[current[k]].size(); d++) {
				size_t j = dominated[current[k]][d];
				if (--domCount[j] == 0)
					next.push_back(j);
			}
		current.swap(next);
	}

	// Survivors: first mu of 'order', CLONED (originals are owned by the
	// populations and released below — no cross-population pointer games)
	std::vector<Individual *> survivors;
	for (size_t k = 0; k < mu && k < order.size(); k++)
		survivors.push_back(pool[order[k]]->clone());

	for (size_t i = 0; i < mu; i++) {
		Individual *replaced = newPopulation->replaceIndividual(
			(unsigned int)i, survivors[i]);
		delete replaced;
	}
	newPopulation->rename();
	delete oldPopulation;	// framework contract: apply consumes it
}

}
