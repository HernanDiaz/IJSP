/*
 * CreationIJSP_helpers.h
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */
#pragma once

#include "Fitness.h"
#include "CrispTime.h"

namespace {

inline bool fiBetter(const FuzzyFW::Crisp& a, const FuzzyFW::Crisp& b) {
	FuzzyFW::FitnessCrisp fa(a, false), fb(b, false);
	return fa.isBetterThan(&fb);
}

inline bool fiWorse(const FuzzyFW::Crisp& a, const FuzzyFW::Crisp& b) {
	FuzzyFW::FitnessCrisp fa(a, false), fb(b, false);
	return fa.isWorseThan(&fb);
}

} // anonymous namespace
