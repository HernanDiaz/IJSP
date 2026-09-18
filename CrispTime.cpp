/*
* CrispTime.cpp
*
*  Created on: September 18, 2026
*      Author: Hernan Diaz Rodriguez
*/

#include "CrispTime.h"
#include <iostream>
using namespace std;

namespace FuzzyFW {

	//=========================================================================
	//		METHODS
	//=========================================================================
	std::string Crisp::toString() const {
		std::ostringstream out;
		out << this->v;
		return out.str();
	}


	//=========================================================================
	//		INPUT / OUTPUT
	//=========================================================================
	/**
	* The converted Taillard instances in TaillardJSP/ write each duration as
	* the degenerate interval "(94, 94)". Both endpoints are read and required
	* to agree: an instance file holding a proper interval is not a crisp
	* problem, and taking one endpoint silently would turn that mistake into a
	* plausible-looking answer.
	*/
	std::ifstream & operator >> (std::ifstream & is, Crisp & t) {
		char c;
		double a, b;

		// Peek past whitespace to tell "(a, b)" from a bare number.
		while (is.good() && std::isspace(is.peek()))
			is.get();

		if (is.peek() == '(') {
			is >> c >> a >> c >> b >> c;
			if (a != b)
				throw FuzzyFWException("Crisp",
					"The instance holds a proper interval; this solver "
					"reads crisp processing times only");
		}
		else {
			is >> a;
			b = a;
		}

		t.v = static_cast<int>(a);
		return is;
	}

	std::ostream & operator << (std::ostream & os, const Crisp & t) {
		os << t.v;
		return os;
	}

}
