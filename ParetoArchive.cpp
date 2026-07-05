/*
 * ParetoArchive.cpp
 *
 *  Created on: Jul 5, 2026
 *      Author: hdiaz
 */

#include "ParetoArchive.h"
#include <algorithm>
#include <fstream>
#include <limits>

namespace FuzzyFW {

//-----  Dominance over the product of the two component orders  --------------
// Components are ranked by the global interval comparison (e.g. LEX2), so
// the pairwise order per component is total; on the stored bounds we
// replicate it: a <= b iff NOT (b strictly better than a).
static bool lex2Better(double a_lo, double a_hi, double b_lo, double b_hi) {
	// strictly better under LEX2: smaller upper bound, tie -> smaller lower
	if (a_hi != b_hi)
		return a_hi < b_hi;
	return a_lo < b_lo;
}

bool ParetoArchive::dominates(const Entry &a, const Entry &b) {
	bool worseC = lex2Better(b.c_lo, b.c_hi, a.c_lo, a.c_hi);
	bool worseE = lex2Better(b.e_lo, b.e_hi, a.e_lo, a.e_hi);
	if (worseC || worseE)
		return false;
	bool betterC = lex2Better(a.c_lo, a.c_hi, b.c_lo, b.c_hi);
	bool betterE = lex2Better(a.e_lo, a.e_hi, b.e_lo, b.e_hi);
	return betterC || betterE;
}


//-----  Crowding distance on the midpoint plane  ------------------------------
std::vector<double> ParetoArchive::crowding() const {
	const double INF = std::numeric_limits<double>::infinity();
	size_t n = this->entries.size();
	std::vector<double> dist(n, 0.0);
	if (n <= 2) {
		std::fill(dist.begin(), dist.end(), INF);
		return dist;
	}
	for (int obj = 0; obj < 2; obj++) {
		std::vector<size_t> idx(n);
		for (size_t i = 0; i < n; i++)
			idx[i] = i;
		auto val = [&](size_t i) {
			return obj == 0
				? (this->entries[i].c_lo + this->entries[i].c_hi) / 2.0
				: (this->entries[i].e_lo + this->entries[i].e_hi) / 2.0;
		};
		std::sort(idx.begin(), idx.end(),
			[&](size_t x, size_t y) { return val(x) < val(y); });
		double range = val(idx[n - 1]) - val(idx[0]);
		dist[idx[0]] = dist[idx[n - 1]] = INF;
		if (range <= 0)
			continue;
		for (size_t k = 1; k + 1 < n; k++)
			dist[idx[k]] += (val(idx[k + 1]) - val(idx[k - 1])) / range;
	}
	return dist;
}


//-----  Offer a solution  -----------------------------------------------------
bool ParetoArchive::offer(const std::string &genotype,
	const FitnessLexicographic *fitness) {

	const FitnessInterval *c =
		dynamic_cast<const FitnessInterval *>(fitness->getFitness(0));
	const FitnessInterval *e =
		dynamic_cast<const FitnessInterval *>(fitness->getFitness(1));
	if (c == NULL || e == NULL)
		return false;

	Entry cand;
	cand.genotype = genotype;
	cand.c_lo = c->getValue().a; cand.c_hi = c->getValue().b;
	cand.e_lo = e->getValue().a; cand.e_hi = e->getValue().b;

	// Rejected if dominated (or duplicated in objective space)
	for (size_t i = 0; i < this->entries.size(); i++) {
		const Entry &m = this->entries[i];
		if (dominates(m, cand))
			return false;
		if (m.c_lo == cand.c_lo && m.c_hi == cand.c_hi
			&& m.e_lo == cand.e_lo && m.e_hi == cand.e_hi)
			return false;
	}
	// Remove the members it dominates
	for (size_t i = this->entries.size(); i-- > 0; ) {
		if (dominates(cand, this->entries[i]))
			this->entries.erase(this->entries.begin() + i);
	}
	this->entries.push_back(cand);

	// Capacity control: drop the most crowded entry
	if (this->entries.size() > this->capacity) {
		std::vector<double> dist = this->crowding();
		size_t worst = 0;
		for (size_t i = 1; i < dist.size(); i++)
			if (dist[i] < dist[worst])
				worst = i;
		this->entries.erase(this->entries.begin() + worst);
	}
	return true;
}


//-----  Dump to CSV  ----------------------------------------------------------
void ParetoArchive::dump(const std::string &path) const {
	std::ofstream out(path.c_str());
	if (!out.is_open())
		return;
	out << "cmax_lo;cmax_hi;npe_lo;npe_hi;solution" << std::endl;
	for (size_t i = 0; i < this->entries.size(); i++) {
		const Entry &m = this->entries[i];
		out << m.c_lo << ";" << m.c_hi << ";"
			<< m.e_lo << ";" << m.e_hi << ";"
			<< m.genotype << std::endl;
	}
}

}
