/*
* TFN.cpp
*
*  Created on: Nov 29, 2016
*      Author: Juan Jose Palacios
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
	}
	return "N/A";
}


Interval::Maximum Interval::getMaximum(std::string str) {
	if (toUpper(str).compare("COMPONENT") == 0)
		return M_COMPONENT;
	if (toUpper(str).compare("EV"))
		return M_EV;
	if (toUpper(str).compare("EXPECTEDVALUE") == 0)
		return M_EV;
	if (toUpper(str).compare("SAKAWA") == 0)
		return M_SAKAWA;
	if (toUpper(str).compare("LEI") == 0)
		return M_SAKAWA;
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
Interval Interval::operator-(const Interval &t) const {
	return Interval(this->a - t.a, this->b - t.b);
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

Interval & Interval::operator-=(const Interval &t) {
	this->a -= t.a;
	this->b -= t.b;
	return *this;
}



//=============================================================================
//		COMPARATORS
//=============================================================================
//====  Equality  =============================================================
bool Interval::isEqualTo(const Interval t, const Compare cp) const {
	switch (cp) {

	case C_COMPONENT:
		return (compareDouble(this->a, t.a) == 0
			&& compareDouble(this->b, t.b) == 0);
			
	case C_EV:
		return (compareDouble(this->expectedValue(), t.expectedValue()) == 0);

	case C_SAKAWA:
		return (compareDouble(this->expectedValue(), t.expectedValue()) == 0)
			&& (compareDouble(this->b - this->a, t.b - t.a) == 0);			
	}
	throw new FuzzyFWException("TFN", "Comparison method unknown");
}


//====  Greater than  =========================================================
bool Interval::isGreaterThan(const Interval t, const Compare cp) const {
	switch (cp) {

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
    }
	throw new FuzzyFWException("TFN", "Comparison method unknown");
}


//====  Greater or equal to  ==================================================
bool Interval::isGreaterEqualTo(const Interval t, const Compare cp) const {
	return isEqualTo(t, cp) || isGreaterThan(t, cp);
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



//====  Possibility function  =================================================
//TODO repasar
double Interval::possibility(const double r) const {
	double pivot = (this->a + this->b) / 2;
	if (compareDouble(r, this->a) <= 0)
		return 0.0;
	if (compareDouble(r, pivot) >= 0)
		return 1.0;
	return (r - this->a) / (pivot - this->a);
}



//====  Necessity function  =================================================
//TODO Repasar
double Interval::necessity(const double r) const {
	double pivot = (this->a + this->b) / 2;
	if (compareDouble(r, pivot) <= 0)
		return 0.0;
	if (compareDouble(r, this->b) >= 0)
		return 1.0;
	return (r - pivot) / (this->b - pivot);
}



//====  Credibility function  =================================================
double Interval::credibility(const double r) const {
	return (this->possibility(r) + this->necessity(r)) / 2;
}



//====  Funcion toString  =====================================================
std::string Interval::toString() const {
	return "(" + valueToString(this->a) + "," + valueToString(this->b) + ")"; 
}



//=============================================================================
//		INPUT / OUTPUT
//=============================================================================
//====  Input Operator  =======================================================
std::ifstream & operator >> (std::ifstream & is, Interval & t)
{
	char c;
	is >> c >> t.a >> c >> t.b >> c ;
	return is;
}

//====  Output Operator  ======================================================
std::ostream & operator << (std::ostream & os, const Interval & t)
{
	os << "(" << t.a << ", " << t.b <<")";
	return os;
}


}
