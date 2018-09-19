/*
* heading.h
*
* This file contains all the includes that are needed by the project to work
*
*  Created on: Jul 23, 2015
*      Author: jjpalacios
*/
#ifndef HEADING_H_
#define HEADING_H_

#include <algorithm>
#include <cmath>
#include <ctime>
#include <time.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <cstring>
#include <vector>
#include <queue>
#include <map>
#include <list>

#include "FJSPException.h"
#include "RandomMT.h"

#define Infd std::numeric_limits<double>::max()
#define Inff std::numeric_limits<float>::max()
#define Infi std::numeric_limits<int>::max()
#define AccuracyError 0.00001

#ifndef NULL
#define NULL   ((void *) 0)
#endif

#if defined(_WIN32)

static const std::string FSEP = "\\";
typedef long long off64_t;

#define NOMINMAX
#include <direct.h>
#include <windows.h>

static void makeDir(const char* dirname) {
	_mkdir(dirname);
}

static char* getWorkDir(char* buffer, int maxLength) {
	return _getcwd(buffer, maxLength);
}

#else //#######################################################################

static const std::string FSEP = "/";
#include <sys/stat.h>

static void makeDir(const char* dirname) {
	mkdir(dirname, 0777); // notice that 777 is different than 0777
}

#include <unistd.h>

static char* getWorkDir(char* buffer, int maxLength) {
	return getcwd(buffer, maxLength);
}

#define localtime_s(x, y) localtime_r(y, x)

#endif

// Auxiliary function to convert a value to string
template<typename T>
inline std::string valueToString(T value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

// Auxiliary function to convert a string to upper case
inline std::string toUpper(std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

// Auxiliary function to convert a string to lower case
inline std::string toLower(std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

// Auxiliary function to truncate a real numbers to integer
inline int truncateToInteger(double x) {
	return (int)std::floor(x + AccuracyError);
}

// Auxiliary function to truncate a real numbers to integer
inline int roundToCeil(double x) {
	return (int)std::ceil(x - AccuracyError);
}

// Auxiliary function to round real numbers to itneger
inline int roundToInt(double a) {
	if (a > 0)
		return (int)std::round(a + AccuracyError);
	return (int)std::round(a - AccuracyError);
}

// Auxiliary function to round real numbers
static double round(double a, const int nDigits = 0) {
	double sol = a;
	int pow = 1;
	for (int i = 0; i < nDigits; i++)
		pow *= 10;
	sol *= pow;
	return std::round(sol) / pow;
}


// Function to compare double numbers up to certain accuracy
static int compareDouble(const double a, const double b) {
	if (a < b - AccuracyError)
		return -1;
	if (a > b + AccuracyError)
		return 1;
	return 0;
}

#endif /* HEADING_H_ */
