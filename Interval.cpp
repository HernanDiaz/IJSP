/*
* Interval.cpp
*
*  Created on: July 29, 2019
*      Author: Hernan Diaz Rodriguez
*/

#include "Interval.h"

namespace FuzzyFW {

	//=============================================================================
	//		ENUMS TO STRING
	//=============================================================================
	//==== Comparison initialization  =============================================
	std::string Interval::getComparison(Interval::Compare cmp) {
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
		}
		return "N/A";
	}

	Interval::Compare Interval::getComparison(std::string str) {
		if (toUpper(str).compare("COMPONENT") == 0)
			return C_COMPONENT;
		if (toUpper(str).compare("EV") == 0)
			return C_EV;
		if (toUpper(str).compare("EXPECTEDVALUE") == 0)
			return C_EV;
		if (toUpper(str).compare("SAKAWA") == 0)
			return C_SAKAWA;
		if (toUpper(str).compare("LEI") == 0)
			return C_SAKAWA;
		if (toUpper(str).compare("RANKING") == 0)
			return C_SAKAWA;
		if (toUpper(str).compare("JIANG") == 0)
			return C_JIANG;
		if (toUpper(str).compare("LEX1") == 0)
			return C_LEX1;
		if (toUpper(str).compare("LEX2") == 0)
			return C_LEX2;
		if (toUpper(str).compare("YX") == 0)
			return C_YX;
		return C_Err;
	}


	//==== Maximum initialization  ================================================
	std::string Interval::getMaximum(Interval::Maximum mxm) {
		switch (mxm) {
		case M_COMPONENT:
			return "Component";
		case M_EV:
			return "Expected Value";
		case M_SAKAWA:
			return "Ranking Sakawa";
		case M_JIANG:
			return "Ranking Jiang";
		}
		return "N/A";
	}


	Interval::Maximum Interval::getMaximum(std::string str) {
		if (toUpper(str).compare("COMPONENT") == 0)
			return M_COMPONENT;
		if (toUpper(str).compare("EV") == 0)
			return M_EV;
		if (toUpper(str).compare("EXPECTEDVALUE") == 0)
			return M_EV;
		if (toUpper(str).compare("SAKAWA") == 0)
			return M_SAKAWA;
		if (toUpper(str).compare("LEI") == 0)
			return M_SAKAWA;
		if (toUpper(str).compare("JIANG") == 0)
			return M_JIANG;
		if (toUpper(str).compare("RANKING") == 0)
			return M_SAKAWA;
		return M_Err;
	}





	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//====  Default constructor  ==================================================
	Interval::Interval() {
		a = b = 0;
	}


	//====  Main Constructor  =====================================================
	Interval::Interval(double a1, double a2) {
		this->a = a1;
		this->b = a2;
	}


	//====  Copy constructor  =====================================================
	Interval::Interval(const Interval & source) {
		this->a = source.a;
		this->b = source.b;
	}
	

	//=============================================================================
	//		OPERATORS
	//=============================================================================
	//====  Addition/substraction component by component  =========================
	Interval Interval::operator+(const Interval &t) const {
		return Interval(this->a + t.a, this->b + t.b);
	}
	
	//====  Asignment overload  ===================================================
	Interval & Interval::operator=(const Interval &t) {
		this->a = t.a;
		this->b = t.b;
		return *this;
	}

	//====  Adition/Substraction + assignment  ====================================
	Interval & Interval::operator+=(const Interval &t) {
		this->a += t.a;
		this->b += t.b;
		return *this;
	}
	

	//=============================================================================
	//		COMPARATORS
	//=============================================================================
	//====  Equality  =============================================================
	bool Interval::isEqualTo(const Interval t, const Compare cp) const {
		switch (cp) {
		case C_JIANG:
			// A = pdB iff P(A>=B) = 0.5
			return this->Lei2011Formula(t) == 0.5;

		case C_COMPONENT:
		case C_LEX1:
		case C_LEX2:
			return (compareDouble(this->a, t.a) == 0
				&& compareDouble(this->b, t.b) == 0);

		case C_EV:
			return (compareDouble(this->expectedValue(), t.expectedValue()) == 0);

		case C_SAKAWA:
			return (compareDouble(this->expectedValue(), t.expectedValue()) == 0)
				&& (compareDouble(this->b - this->a, t.b - t.a) == 0);
		case C_YX:
			return (compareDouble(this->a+this->b, t.a+t.b) == 0)
				&& (compareDouble(this->b - this->a, t.b - t.a) == 0);

		}
		throw new FuzzyFWException("Interval", "Comparison method unknown");
	}


	//====  Greater than  =========================================================
	bool Interval::isGreaterThan(const Interval t, const Compare cp) const {
		switch (cp) {
		case C_JIANG:
			//A > pdB iff P(A<=B) <0.5
			return this->Lei2011Formula(t) < 0.5;

		case C_COMPONENT:
			return (compareDouble(this->a, t.a) > 0
				&& compareDouble(this->b, t.b) > 0);

		case C_EV:
			return (compareDouble(this->expectedValue(), t.expectedValue()) > 0);

		case C_SAKAWA:
			if (compareDouble(this->expectedValue(), t.expectedValue()) > 0)
				return true;
			return (compareDouble(this->expectedValue(), t.expectedValue()) == 0)
				&& (compareDouble(this->b - this->a, t.b - t.a) > 0);

		case C_LEX1: return !(compareDouble(this->a, t.a) < 0
			|| ((compareDouble(this->a, t.a) == 0) && (compareDouble(this->b, t.b) <= 0)));

		case C_LEX2:return !(compareDouble(this->b, t.b) < 0
			|| ((compareDouble(this->b, t.b) == 0) && (compareDouble(this->a, t.a) <= 0)));

		case C_YX: return !(compareDouble(this->a + this->b, t.a + t.b) < 0
			|| ((compareDouble(this->a + this->b, t.a + t.b) == 0) && (compareDouble(this->b - this->a, t.b - t.a) < 0)));

		}
		throw new FuzzyFWException("Interval", "Comparison method unknown");

	}


	//====  Greater or equal to  ==================================================
	bool Interval::isGreaterEqualTo(const Interval t, const Compare cp) const {
		switch (cp) {

		case C_JIANG:
			//A > pdB iff P(A>=B) < 0.5 && A = pdB iff P(A>=B) = 0.5
			return this->Lei2011Formula(t) <= 0.5;
			
		case C_COMPONENT:
			return (compareDouble(this->a, t.a) >= 0
				&& compareDouble(this->b, t.b) >= 0);

		case C_EV:
			return (compareDouble(this->expectedValue(), t.expectedValue()) >= 0);

		case C_SAKAWA:
			if (compareDouble(this->expectedValue(), t.expectedValue()) > 0)
				return true;
			return (compareDouble(this->expectedValue(), t.expectedValue()) == 0)
				&& (compareDouble(this->b - this->a, t.b - t.a) >= 0);

		case C_LEX1: return !(compareDouble(this->a, t.a) < 0
			|| ((compareDouble(this->a, t.a) == 0) && (compareDouble(this->b, t.b) < 0)));
			
		case C_LEX2:return !(compareDouble(this->b, t.b) < 0
			|| ((compareDouble(this->b, t.b) == 0) && (compareDouble(this->a, t.a) < 0)));

		case C_YX: return !(compareDouble(this->a+this->b, t.a+t.b) < 0
			|| ((compareDouble(this->a + this->b, t.a + t.b) == 0) && (compareDouble(this->b-this->a, t.b-t.a) < 0)));

		}
		throw new FuzzyFWException("Interval", "Comparison method unknown");
	}


	//====  Lesser than  ==========================================================
	bool Interval::isLesserThan(const Interval t, const Compare cp) const {
		return !isGreaterEqualTo(t, cp);
	}


	//====  Lesser or equal to  ===================================================
	bool Interval::isLesserEqualTo(const Interval t, const Compare cp) const {
		return !isGreaterThan(t, cp);
	}



	//====  Equality (one component)  =============================================
	bool Interval::EqualComponent(const Interval t, const unsigned int component) const {
		if (component == 1) return (compareDouble(this->a, t.a) == 0);
		if (component == 2) return (compareDouble(this->b, t.b) == 0);
		return false;
	}



	//=============================================================================
	//		METHODS
	//=============================================================================
	//====  Maximum/minimim methods  ==============================================
	Interval maximum(const Interval & x, const Interval & y, const Interval::Maximum mt) {
		Interval sol;
		switch (mt) {

		case Interval::M_COMPONENT:
			sol.a = std::max(x.a, y.a);
			sol.b = std::max(x.b, y.b);
			break;

		case Interval::M_EV:
			if (x.isGreaterThan(y, Interval::C_EV))
				sol = x;
			else sol = y;
			break;

		case Interval::M_SAKAWA:
			if (x.isGreaterThan(y, Interval::C_SAKAWA))
				sol = x;
			else sol = y;
			break;

		case Interval::M_JIANG:
			if (x.isGreaterThan(y, Interval::C_JIANG))
				sol = x;
			else sol = y;
			break;
		}
		return sol;
	}

	Interval minimum(const Interval & x, const Interval & y, const Interval::Maximum mt) {
		Interval sol;
		switch (mt) {

		case Interval::M_COMPONENT:
			sol.a = std::min(x.a, y.a);
			sol.b = std::min(x.b, y.b);
			break;

		case Interval::M_EV:
			if (x.isLesserThan(y, Interval::C_EV))
				sol = x;
			else sol = y;
			break;

		case Interval::M_SAKAWA:
			if (x.isLesserThan(y, Interval::C_SAKAWA))
				sol = x;
			else sol = y;
			break;

		case Interval::M_JIANG:
			if (x.isLesserThan(y, Interval::C_JIANG))
				sol = x;
			else sol = y;
			break;
		}
		return sol;
	}



	//====  Function expectedValue  ===============================================
	double Interval::expectedValue() const {
		return (this->a + this->b) / 2.0;
	}



	//====  Membership function  ==================================================
	//TODO repasar esto
	double Interval::membership(const double r) const {
		double pivot = (this->a + this->b) / 2;
		if (compareDouble(r, this->a) <= 0
			|| compareDouble(r, this->b) >= 0)
			return 0.0;
		if (compareDouble(r, pivot) <= 0)
			return (r - this->a) / (pivot - this->a);
		return (r - this->b) / (pivot - this->b);
	}


	//====  Funcion toString  =====================================================
	std::string Interval::toString() const {
		return "(" + valueToString(this->a) + ", " + valueToString(this->b) + ")";
	}

	double Interval::Lei2011Formula(const Interval & t) const
	{
		//i) Al >= Br
		if (compareDouble(this->a, t.b) >= 0) return 0;
		//vi) Ar <= Bl
		if (compareDouble(this->b, t.a) <= 0) return 1;
		else {
			double longA = this->b - this->a;
			double longB = t.b - t.a;

			//ii) Bl <= Al < Br <= Ar

			if (compareDouble(t.a, this->a) <= 0) {
				if (compareDouble(t.b, this->b) <= 0)
					return (0.5* ((t.b - this->a) / longA)*((t.b - this->a) / longB));
				//v) Bl <= Al < Ar < Br
				return (((t.b - this->b) / longB) + 0.5 * longA / longB);
			}
			//iv) Al < Bl <= Ar < Br
			if (compareDouble(this->b, t.b) < 0)
				return ((t.a - this->a) / longA) + (((this->b - t.a) / longA)*((t.b - this->b) / longB))
				+ 0.5*((this->b - t.a) / longA)*((this->b - t.a) / longB);
			//iii) Al < Bl < Br <= Ar
			return (((t.a - this->a) / longA) + 0.5 * longB / longA);
		}
	}



	//=============================================================================
	//		INPUT / OUTPUT
	//=============================================================================
	//====  Input Operator  =======================================================
	std::ifstream & operator >> (std::ifstream & is, Interval & t)
	{
		char c;
		//TODO adaptation to load data from fuzzy problems
		double d;
		is >> c >> t.a >> c >> d >> c >> t.b >> c;
		return is;
	}

	//====  Output Operator  ======================================================
	std::ostream & operator << (std::ostream & os, const Interval & t)
	{
		os << "(" << t.a << ", " << t.b << ")";
		return os;
	}


}
