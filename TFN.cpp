/*
* TFN.cpp
*
*  Created on: Nov 29, 2016
*      Author: Juan Jose Palacios
*/

#include "TFN.h"

namespace FJSP {

//=============================================================================
//		ENUMS TO STRING
//=============================================================================
//==== Comparison initialization  =============================================
std::string TFN::getComparison(TFN::Compare cmp) {
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

TFN::Compare TFN::getComparison(std::string str) {
	if (toUpper(str).compare("COMPONENT"))
		return C_COMPONENT;
	if (toUpper(str).compare("EV"))
		return C_EV;
	if (toUpper(str).compare("EXPECTEDVALUE"))
		return C_EV;
	if (toUpper(str).compare("SAKAWA"))
		return C_SAKAWA;
	if (toUpper(str).compare("LEI"))
		return C_SAKAWA;
	if (toUpper(str).compare("RANKING"))
		return C_SAKAWA;
	return C_Err;
}


//==== Maximum initialization  ================================================
std::string TFN::getMaximum(TFN::Maximum mxm) {
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


TFN::Maximum TFN::getMaximum(std::string str) {
	if (toUpper(str).compare("COMPONENT"))
		return M_COMPONENT;
	if (toUpper(str).compare("EV"))
		return M_EV;
	if (toUpper(str).compare("EXPECTEDVALUE"))
		return M_EV;
	if (toUpper(str).compare("SAKAWA"))
		return M_SAKAWA;
	if (toUpper(str).compare("LEI"))
		return M_SAKAWA;
	if (toUpper(str).compare("RANKING"))
		return M_SAKAWA;
	return M_Err;
}





//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
TFN::TFN() {
	a = b = c = 0;
}


//====  Main Constructor  =====================================================
TFN::TFN(double a1, double a2, double a3) {
	this->a = a1;
	this->b = a2;
	this->c = a3;
}


//====  Copy constructor  =====================================================
TFN::TFN(const TFN & source) {
	this->a = source.a;
	this->b = source.b;
	this->c = source.c;
}



//=============================================================================
//		OPERATORS
//=============================================================================
//====  Addition/substraction component by component  =========================
TFN TFN::operator+(const TFN &t) const {
	return TFN(this->a + t.a, this->b + t.b, this->c + t.c);
}
TFN TFN::operator-(const TFN &t) const {
	return TFN(this->a - t.a, this->b - t.b, this->c - t.c);
}


//====  Asignment overload  ===================================================
TFN & TFN::operator=(const TFN &t) {
	this->a = t.a;
	this->b = t.b;
	this->c = t.c;
	return *this;
}


//====  Adition/Substraction + assignment  ====================================
TFN & TFN::operator+=(const TFN &t) {
	this->a += t.a;
	this->b += t.b;
	this->c += t.c;
	return *this;
}
TFN & TFN::operator-=(const TFN &t) {
	this->a -= t.a;
	this->b -= t.b;
	this->c -= t.c;
	return *this;
}



//=============================================================================
//		COMPARATORS
//=============================================================================
//====  Equality  =============================================================
bool TFN::isEqualTo(const TFN t, const Compare cp) const {
	switch (cp) {

	case C_COMPONENT:
		return (compareDouble(this->a, t.a) == 0
			&& compareDouble(this->b, t.b) == 0
			&& compareDouble(this->c, t.c) == 0);

	case C_EV:
		return (compareDouble(this->expectedValue(), t.expectedValue()) == 0);

	case C_SAKAWA:
		if (compareDouble(this->expectedValue(), t.expectedValue()) == 0
			&& compareDouble(this->b, t.b) == 0
			&& compareDouble(this->c - this->a, t.c - t.a) == 0)
			return true;
		return false;
	}
}


//====  Greater than  =========================================================
bool TFN::isGreaterThan(const TFN t, const Compare cp) const {
	switch (cp) {

	case C_COMPONENT:
		return (compareDouble(this->a, t.a) > 0
			&& compareDouble(this->b, t.b) > 0
			&& compareDouble(this->c, t.c) > 0);

	case C_EV:
		return (compareDouble(this->expectedValue(), t.expectedValue()) > 0);

	case C_SAKAWA:
		if (compareDouble(this->expectedValue(), t.expectedValue()) > 0)
			return true;
		else if (compareDouble(this->expectedValue(), t.expectedValue()) == 0) {
			if (compareDouble(this->b, t.b) > 0)
				return true;
			else if (compareDouble(this->b, t.b) == 0) {
				if (compareDouble(this->c - this->a, t.c - t.a) > 0)
					return true;
			}
		}
		return false;
	}
}


//====  Greater or equal to  ==================================================
bool TFN::isGreaterEqualTo(const TFN t, const Compare cp) const {
	switch (cp) {

	case C_COMPONENT:
		return (compareDouble(this->a, t.a) >= 0
			&& compareDouble(this->b, t.b) >= 0
			&& compareDouble(this->c, t.c) >= 0);

	case C_EV:
		return (compareDouble(this->expectedValue(), t.expectedValue()) >= 0);

	case C_SAKAWA:
		if (compareDouble(this->expectedValue(), t.expectedValue()) > 0)
			return true;
		else if (compareDouble(this->expectedValue(), t.expectedValue()) == 0) {
			if (compareDouble(this->b, t.b) > 0)
				return true;
			else if (compareDouble(this->b, t.b) == 0) {
				if (compareDouble(this->c - this->a, t.c - t.a) >= 0)
					return true;
			}
		}
		return false;
	}
}


//====  Lesser than  ==========================================================
bool TFN::isLesserThan(const TFN t, const Compare cp) const {
	switch (cp) {

	case C_COMPONENT:
		return (compareDouble(this->a, t.a) < 0
			&& compareDouble(this->b, t.b) < 0
			&& compareDouble(this->c, t.c) < 0);

	case C_EV:
		return (compareDouble(this->expectedValue(), t.expectedValue()) < 0);

	case C_SAKAWA:
		if (compareDouble(this->expectedValue(), t.expectedValue()) < 0)
			return true;
		else if (compareDouble(this->expectedValue(), t.expectedValue()) == 0) {
			if (compareDouble(this->b, t.b) < 0)
				return true;
			else if (compareDouble(this->b, t.b) == 0) {
				if (compareDouble(this->c - this->a, t.c - t.a) < 0)
					return true;
			}
		}
		return false;
	}
}


//====  Lesser or equal to  ===================================================
bool TFN::isLesserEqualTo(const TFN t, const Compare cp) const {
	switch (cp) {

	case C_COMPONENT:
		return (compareDouble(this->a, t.a) <= 0
			&& compareDouble(this->b, t.b) <= 0
			&& compareDouble(this->c, t.c) <= 0);

	case C_EV:
		return (compareDouble(this->expectedValue(), t.expectedValue()) <= 0);

	case C_SAKAWA:
		if (compareDouble(this->expectedValue(), t.expectedValue()) < 0)
			return true;
		else if (compareDouble(this->expectedValue(), t.expectedValue()) == 0) {
			if (compareDouble(this->b, t.b) < 0)
				return true;
			else if (compareDouble(this->b, t.b) == 0) {
				if (compareDouble(this->c - this->a, t.c - t.a) <= 0)
					return true;
			}
		}
		return false;
	}
}



//====  Equality (one component)  =============================================
bool TFN::EqualComponent(const TFN t, const unsigned int component) const {
	if (component == 1) return (compareDouble(this->a, t.a) == 0);
	if (component == 2) return (compareDouble(this->b, t.b) == 0);
	if (component == 3) return (compareDouble(this->c, t.c) == 0);
	return false;
}



//=============================================================================
//		METHODS
//=============================================================================
//====  Maximum/minimim methods  ==============================================
TFN maximum(const TFN & x, const TFN & y, const TFN::Maximum mt) {
	TFN sol;
	switch (mt) {

	case TFN::M_COMPONENT:
		sol.a = std::max(x.a, y.a);
		sol.b = std::max(x.b, y.b);
		sol.c = std::max(x.c, y.c);

	case TFN::M_EV:
		if (x.isGreaterThan(y, TFN::C_EV))
			sol = x;
		else sol = y;

	case TFN::M_SAKAWA:
		if (x.isGreaterThan(y, TFN::C_SAKAWA))
			sol = x;
		else sol = y;
	}
	return sol;
}

TFN minimum(const TFN & x, const TFN & y, const TFN::Maximum mt) {
	TFN sol;
	switch (mt) {

	case TFN::M_COMPONENT:
		sol.a = std::min(x.a, y.a);
		sol.b = std::min(x.b, y.b);
		sol.c = std::min(x.c, y.c);

	case TFN::M_EV:
		if (x.isLesserThan(y, TFN::C_EV))
			sol = x;
		else sol = y;

	case TFN::M_SAKAWA:
		if (x.isLesserThan(y, TFN::C_SAKAWA))
			sol = x;
		else sol = y;
	}
	return sol;
}



//====  Funcion expectedValue  ================================================
double TFN::expectedValue() const {
	return (this->a + 2 * this->b + this->c) / 4.0;
}



//====  Funcion toString  =====================================================
std::string TFN::toString() const {
	std::string str;
	str = "(" + valueToString(this->a);
	str += "," + valueToString(this->b);
	str += "," + valueToString(this->c) + ")"; 
	return str;
}



//=============================================================================
//		INPUT / OUTPUT
//=============================================================================
//====  Input Operator  =======================================================
std::ifstream & operator >> (std::ifstream & is, TFN & t)
{
	char c;
	is >> c >> t.a >> c >> t.b >> c >> t.c >> c;
	return is;
}

//====  Output Operator  ======================================================
std::ostream & operator << (std::ostream & os, const TFN & t)
{
	os << "(" << t.a << ", " << t.b << ", " << t.c << ")";
	return os;
}


}
