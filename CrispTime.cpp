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
	//		ENUMS TO STRING
	//=========================================================================
	// Every strategy names the same order here. The strings are still
	// recognised so that the setup files of the interval study run unchanged
	// against the crisp solver, which is what makes the two comparable.
	//=========================================================================
	std::string Crisp::getComparison(Crisp::Compare cmp) {
		switch (cmp) {
		case C_COMPONENT:
			return "Component";
		case C_EV:
			return "Expected Value";
		case C_SAKAWA:
			return "Ranking Sakawa";
		case C_JIANG:
			return "Ranking Jiang 2008";
		case C_LEX1:
			return "Lexicographical 1";
		case C_LEX2:
			return "Lexicographical 2";
		case C_YX:
			return "Xu and Yager";
		default:
			return "N/A";
		}
	}

	Crisp::Compare Crisp::getComparison(std::string str) {
		std::string s = toUpper(str);
		if (s.compare("COMPONENT") == 0) return C_COMPONENT;
		if (s.compare("EV") == 0) return C_EV;
		if (s.compare("EXPECTEDVALUE") == 0) return C_EV;
		if (s.compare("SAKAWA") == 0) return C_SAKAWA;
		if (s.compare("LEI") == 0) return C_SAKAWA;
		if (s.compare("RANKING") == 0) return C_SAKAWA;
		if (s.compare("JIANG") == 0) return C_JIANG;
		if (s.compare("LEX1") == 0) return C_LEX1;
		if (s.compare("LEX2") == 0) return C_LEX2;
		if (s.compare("YX") == 0) return C_YX;
		return C_Err;
	}

	std::string Crisp::getMaximum(Crisp::Maximum mxm) {
		switch (mxm) {
		case M_COMPONENT:
			return "Component";
		case M_EV:
			return "Expected Value";
		case M_SAKAWA:
			return "Ranking Sakawa";
		case M_JIANG:
			return "Ranking Jiang 2008";
		default:
			return "N/A";
		}
	}

	Crisp::Maximum Crisp::getMaximum(std::string str) {
		std::string s = toUpper(str);
		if (s.compare("COMPONENT") == 0) return M_COMPONENT;
		if (s.compare("EV") == 0) return M_EV;
		if (s.compare("EXPECTEDVALUE") == 0) return M_EV;
		if (s.compare("SAKAWA") == 0) return M_SAKAWA;
		if (s.compare("LEI") == 0) return M_SAKAWA;
		if (s.compare("RANKING") == 0) return M_SAKAWA;
		if (s.compare("JIANG") == 0) return M_JIANG;
		return M_Err;
	}


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
