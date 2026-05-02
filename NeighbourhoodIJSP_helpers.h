/*
 * NeighbourhoodIJSP_helpers.h
 *
 *  Created on: May 2, 2026
 *      Author: hdiaz
 */
#pragma once

#include <vector>
#include <string>

#ifndef NB_ESTIMATOR_NONE
#define NB_ESTIMATOR_NONE "none"
#define NB_ESTIMATOR_HEADSTAILS "heads&tails"
#endif

namespace IJSP {
namespace {

inline std::vector<std::string> buildNBName(const std::string &id, bool headsTails) {
	std::vector<std::string> setup;
	setup.push_back(id);
	setup.push_back(std::string("Estimator:;") +
		(headsTails ? NB_ESTIMATOR_HEADSTAILS : NB_ESTIMATOR_NONE));
	return setup;
}

} // anonymous namespace
} // namespace IJSP
