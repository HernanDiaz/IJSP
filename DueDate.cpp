/*
* DueDate.cpp
*
*  Created on: May 12, 2017
*      Author: Juan Jose Palacios
*/

#include "DueDate.h"

namespace FJSP {

//=============================================================================
//
//	Abstract class DueDate
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Agreement index (TFN)  ===============================================
double DueDate::agreementIndex(const TFN c)  const {
	std::string errorMsg;
	errorMsg = "Agreement index cannot be computed for this type of due-dates";
	throw FJSPException("Due-Date", errorMsg);
	return -1.0;
}


//=====  Agreement index (crips value)  =======================================
double DueDate::agreementIndex(const double c)  const {
	std::string errorMsg;
	errorMsg = "Agreement index cannot be computed for this type of due-dates";
	throw FJSPException("Due-Date", errorMsg);
	return -1.0;
}


//=====  Delay (TFN)  =========================================================
TFN DueDate::delay(const TFN c)  const {
	std::string errorMsg;
	errorMsg = "Tardiness cannot be computed for this type of due-dates";
	throw FJSPException("Due-Date", errorMsg);
	return TFN(-1, -1, -1);
}


//=====  Delay (crisp value)  =================================================
double DueDate::delay(const double c)  const {
	std::string errorMsg;
	errorMsg = "Tardiness cannot be computed for this type of due-dates";
	throw FJSPException("Due-Date", errorMsg);
	return -1.0;
}



//=============================================================================
//		INPUT / OUTPUT
//=============================================================================
//====  Input operator  =======================================================
std::istream & operator>>(std::istream & is, DueDate * d) {
	d->readFromStream(is);
	return is;
}


//====  Output operator  ======================================================
std::ostream & operator<<(std::ostream & os, DueDate * d) {
	os << d->toString();
	return os;
}





//=============================================================================
//
//	Class DueDateCrisp
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
DueDateCrisp::DueDateCrisp()
	: DueDate() {
	this->duedate = -1.0;
}


//====  Main constructor  =====================================================
DueDateCrisp::DueDateCrisp(const double duedateValue)
	: DueDate() {
	if (compareDouble(duedateValue, 0.0) == -1) {
		std::string errorMsg = "Invalid due-date: ";
		errorMsg += valueToString(duedateValue) + ".";
		errorMsg += "Negative values are not allowed";
		throw FJSPException("Due-Date", errorMsg);
	}

	this->duedate = duedateValue;
	this->loadedData = true;
}


//====  Copy constructor  =====================================================
DueDateCrisp::DueDateCrisp(const DueDateCrisp &duedate)
	: DueDate(duedate) {
	this->duedate = duedate.duedate;
}


//====  Clone method  =========================================================
DueDate * DueDateCrisp::clone() const {
	return new DueDateCrisp(*this);
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Agreement index (TFN)  ===============================================
double DueDateCrisp::agreementIndex(const TFN c)  const {
	if (!this->loadedData) {
		std::string errorMsg = "Agreement index cannot be computed. ";
		errorMsg += "Invalid due-date";
		throw FJSPException("Due-Date", errorMsg);
	}

	double c1 = c.a;
	double c2 = c.b;
	double c3 = c.c;
	double dd = this->duedate;

	// c3 <= d
	if (compareDouble(c3, dd) < 1)
		return 1.0;
	// c1 >= d (and c2 >= d and c3 > d)
	if (compareDouble(c1, dd) > -1)
		return 0.0;
	// c2 >= d (and c3 > d and c1 < d)
	if (compareDouble(c2, dd) > -1)
		return ((dd - c1)*(dd - c1)) / ((c2 - c1)*(c3 - c1));

	// (c3 > d and c1 < d and c2 < d)
	else {
		return (c3*(dd - c2) + dd*(c3 - dd) - c1*(c3 - c2)) /
			((c3 - c2) * (c3 - c1));
	}

	// Case not controlled
	return -1.0;
}


//=====  Agreement index (crips value)  =======================================
double DueDateCrisp::agreementIndex(const double c)  const {
	if (!this->loadedData) {
		std::string errorMsg = "Agreement index cannot be computed. ";
		errorMsg += "Invalid due-date";
		throw FJSPException("Due-Date", errorMsg);
	}

	// c <= d
	if (compareDouble(c, this->duedate < 1))
		return 1.0;
	return 0.0;
}


//=====  Delay (TFN)  =========================================================
TFN DueDateCrisp::delay(const TFN c)  const {
	if (!this->loadedData) {
		std::string errorMsg = "Agreement index cannot be computed. ";
		errorMsg += "Invalid due-date";
		throw FJSPException("Due-Date", errorMsg);
	}
	double x = c.a - this->duedate;
	double y = c.b - this->duedate;
	double z = c.c - this->duedate;
	return TFN(x, y, z);
}


//=====  Delay (crisp value)  =================================================
double DueDateCrisp::delay(const double c)  const {
	if (!this->loadedData) {
		std::string errorMsg = "Agreement index cannot be computed. ";
		errorMsg += "Invalid due-date";
		throw FJSPException("Due-Date", errorMsg);
	}
	return c - this->duedate;
}



//=============================================================================
//		INPUT / OUTPUT
//=============================================================================
//=====  Read from a stream  ==================================================
std::istream & DueDateCrisp::readFromStream(std::istream & is) {
	is >> this->duedate;
	this->loadedData = true;
	return is;
}


//=====  Convert to string  ===================================================
std::string DueDateCrisp::toString() {
	if (!this->loadedData) {
		std::string errorMsg = "Due-date is not initialized";
		throw FJSPException("Due-Date", errorMsg);
	}
	return valueToString(this->duedate);
}





//=============================================================================
//
//	Class DueDateLinear
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
DueDateLinear::DueDateLinear()
	: DueDate() {
	this->d1 = this->d2 = -1.0;
}


//====  Main constructor  =====================================================
DueDateLinear::DueDateLinear(const double dd1, const double dd2)
	: DueDate() {
	if (compareDouble(dd1, 0.0) == -1 || compareDouble(dd2, 0.0) == -1) {
		std::string errorMsg = "Invalid due-date: ";
		errorMsg += "(" + valueToString(d1) + ",";
		errorMsg += valueToString(d2) + "). ";
		errorMsg += "Negative values are not allowed";
		throw FJSPException("Due-Date", errorMsg);
	}
	else if (compareDouble(d2, d1) == -1) {
		std::string errorMsg = "Invalid due-date: ";
		errorMsg += "(" + valueToString(d1) + ",";
		errorMsg += valueToString(d2) + "). ";
		errorMsg += "Second value must be equal or greater than the first one";
		throw FJSPException("Due-Date", errorMsg);
	}

	this->d1 = dd1;
	this->d2 = dd2;
	this->loadedData = true;
}


//====  Copy constructor  =====================================================
DueDateLinear::DueDateLinear(const DueDateLinear &duedate)
	: DueDate(duedate) {
	this->d1 = duedate.d1;
	this->d2 = duedate.d2;
}


//====  Clone method  =========================================================
DueDate * DueDateLinear::clone() const {
	return new DueDateLinear(*this);
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Agreement index (TFN)  ===============================================
double DueDateLinear::agreementIndex(const TFN c)  const {
	if (!this->loadedData) {
		std::string errorMsg = "Agreement index cannot be computed. ";
		errorMsg += "Invalid due-date";
		throw FJSPException("Due-Date", errorMsg);
	}

	double c1 = c.a;
	double c2 = c.b;
	double c3 = c.c;

	double ip1, ip2;	// Intersection points

	// c2 <= d1 and c3 <= d2
	if (compareDouble(c2, d1) < 1 && compareDouble(c3, d2) < 1)
		return 1.0;
	// c1 >= d2
	if (compareDouble(c1, d2) > -1)
		return 0.0;
	// c2 >= d1 and c3 >= d2
	if (compareDouble(c2, d1) > -1 && compareDouble(c3, d2) > -1) {
		ip1 = ((c2*d2) - (c1*d1)) / (c2 - c1 + d2 - d1);
		if (compareDouble(c1, c2) == 0)
			return ((ip1 - d2)*(d2 - c1)) / ((d1 - d2)*(c3 - c1));
		else
			return ((ip1 - c1)*(d2 - c1)) / ((c2 - c1)*(c3 - c1));
	}
	// c2 < d1 and c3 > d2
	if (compareDouble(c2, d1) < 0 && compareDouble(c3, d2) > 0) {
		ip2 = ((c3*d1) - (c2*d2)) / (c3 - c2 + d1 - d2);
		return (c3*(ip2 - c2) + d2*(c3 - ip2) - c1*(c3 - c2)) /
			((c3 - c2) * (c3 - c1));
	}
	else {
		ip1 = ((c2*d2) - (c1*d1)) / (c2 - c1 + d2 - d1);
		if (compareDouble(c1, c3) == 0)
			return (ip1 - d2) / (d1 - d2);
		else {
			ip2 = ((c3*d1) - (c2*d2)) / (c3 - c2 + d1 - d2);
			return (ip1*(c1 - d2) + ip2*(d2 - c3) + d2*(c3 - c1)) /
				((d2 - d1) * (c3 - c1));
		}
	}
	return -1.0;
}



//=====  Agreement index (crips value)  =======================================
double DueDateLinear::agreementIndex(const double c)  const {
	if (!this->loadedData) {
		std::string errorMsg = "Agreement index cannot be computed. ";
		errorMsg += "Invalid due-date";
		throw FJSPException("Due-Date", errorMsg);
	}

	// c <= d1
	if (compareDouble(c, this->d1) < 1)
		return 1.0;
	// c >= d2
	if (compareDouble(c, this->d2) > -1)
		return 0.0;
	// d1 < c < d2
	else
		return (c - d2) / (d1 - d2);
	return -1.0;
}


	
//=============================================================================
//		INPUT / OUTPUT
//=============================================================================
//=====  Read from a stream  ==================================================
std::istream & DueDateLinear::readFromStream(std::istream & is) {
	char c;
	is >> c >> this->d1 >> c >> this->d2 >> c;
	this->loadedData = true;
	return is;
}


//=====  Convert to string  ===================================================
std::string DueDateLinear::toString() {
	if (!this->loadedData) {
		std::string errorMsg = "Due-date is not initialized";
		throw FJSPException("Due-Date", errorMsg);
	}
	std::string value = "(" + valueToString(this->d1);
	value = "," + valueToString(this->d2) + ")";
	return value;
}
}
