/*
* Fitness.cpp
*
*  Created on: June 7, 2017
*      Author: Juan Jose Palacios
*/

#include "Fitness.h"

namespace FJSP {

bool Fitness::FitnessMaximize = true;

//=============================================================================
//
//	Class FitnessDouble
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Better than or equal to  =============================================
bool FitnessDouble::isBetterOrEqualTo(const Fitness * f) const {
	const FitnessDouble *fd = this->convertType(f);
	if (FitnessMaximize)
		return compareDouble(this->value, fd->value) >= 0;
	return compareDouble(this->value, fd->value) <= 0;
}


//=====  Better than  =========================================================
bool FitnessDouble::isBetterThan(const Fitness * f) const {
	const FitnessDouble *fd = this->convertType(f);
	if (FitnessMaximize)
		return compareDouble(this->value, fd->value) > 0;
	return compareDouble(this->value, fd->value) < 0;
}

//=====  Better than or equal to  =============================================
bool FitnessDouble::isEqualTo(const Fitness * f) const {
	const FitnessDouble *fd = this->convertType(f);
	return compareDouble(this->value, fd->value) == 0;
}

//=====  Better than or equal to  =============================================
bool FitnessDouble::isWorseThan(const Fitness * f) const {
	const FitnessDouble *fd = this->convertType(f);
	if (FitnessMaximize)
		return compareDouble(this->value, fd->value) < 0;
	return compareDouble(this->value, fd->value) > 0;
}

//=====  Better than or equal to  =============================================
bool FitnessDouble::isWorseOrEqualTo(const Fitness * f) const {
	const FitnessDouble *fd = this->convertType(f);
	if (FitnessMaximize)
		return compareDouble(this->value, fd->value) <= 0;
	return compareDouble(this->value, fd->value) >= 0;
}


//=====  Conversion to double  ================================================
const FitnessDouble * FitnessDouble::convertType(const Fitness *f) const {
	if (f->getType() == Fitness::Type::DOUBLE)
		return dynamic_cast<const FitnessDouble *>(f);
	throw new FJSPException("Fitness",
		"Comparison of incompatible fitness values");
}





//=============================================================================
//
//	Class FitnessInteger
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Better than or equal to  =============================================
bool FitnessInteger::isBetterOrEqualTo(const Fitness * f) const {
	const FitnessInteger *fd = this->convertType(f);
	if (FitnessMaximize)
		return this->value >= fd->value;
	return this->value <= fd->value;
}


//=====  Better than  =========================================================
bool FitnessInteger::isBetterThan(const Fitness * f) const {
	const FitnessInteger *fd = this->convertType(f);
	if (FitnessMaximize)
		return this->value > fd->value;
	return this->value < fd->value;
}

//=====  Better than or equal to  =============================================
bool FitnessInteger::isEqualTo(const Fitness * f) const {
	const FitnessInteger *fd = this->convertType(f);
	return this->value == fd->value;
}

//=====  Better than or equal to  =============================================
bool FitnessInteger::isWorseThan(const Fitness * f) const {
	const FitnessInteger *fd = this->convertType(f);
	if (FitnessMaximize)
		return this->value <= fd->value;
	return this->value >= fd->value;
}

//=====  Better than or equal to  =============================================
bool FitnessInteger::isWorseOrEqualTo(const Fitness * f) const {
	const FitnessInteger *fd = this->convertType(f);
	if (FitnessMaximize)
		return this->value < fd->value;
	return this->value > fd->value;
}


//=====  Conversion to double  ================================================
const FitnessInteger * FitnessInteger::convertType(const Fitness *f) const {
	if (f->getType() == Fitness::Type::INTEGER)
		return dynamic_cast<const FitnessInteger *>(f);
	throw new FJSPException("Fitness",
		"Comparison of incompatible fitness values");
}





//=============================================================================
//
//	Class FitnessTFN
//
//=============================================================================
TFN::Compare FitnessTFN::FitnessCompareStrategy = TFN::C_EV;

//=============================================================================
//		METHODS
//=============================================================================
//=====  Better than or equal to  =============================================
bool FitnessTFN::isBetterOrEqualTo(const Fitness * f) const {
	const FitnessTFN *ft = this->convertType(f);
	if (FitnessMaximize)
		return this->value.isGreaterEqualTo(ft->value,
			this->FitnessCompareStrategy);
	return this->value.isLesserEqualTo(ft->value,
		this->FitnessCompareStrategy);
}


//=====  Better than  =========================================================
bool FitnessTFN::isBetterThan(const Fitness * f) const {
	const FitnessTFN *ft = this->convertType(f);
	if (FitnessMaximize)
		return this->value.isGreaterThan(ft->value,
			this->FitnessCompareStrategy);
	return this->value.isLesserThan(ft->value,
		this->FitnessCompareStrategy);
}

//=====  Better than or equal to  =============================================
bool FitnessTFN::isEqualTo(const Fitness * f) const {
	const FitnessTFN *ft = this->convertType(f);
	return this->value.isEqualTo(ft->value,
			this->FitnessCompareStrategy);
}

//=====  Better than or equal to  =============================================
bool FitnessTFN::isWorseThan(const Fitness * f) const {
	const FitnessTFN *ft = this->convertType(f);
	if (FitnessMaximize)
		return this->value.isLesserThan(ft->value,
			this->FitnessCompareStrategy);
	return this->value.isGreaterThan(ft->value,
		this->FitnessCompareStrategy);
}

//=====  Better than or equal to  =============================================
bool FitnessTFN::isWorseOrEqualTo(const Fitness * f) const {
	const FitnessTFN *ft = this->convertType(f);
	if (FitnessMaximize)
		return this->value.isLesserEqualTo(ft->value,
			this->FitnessCompareStrategy);
	return this->value.isGreaterEqualTo(ft->value,
		this->FitnessCompareStrategy);
}


//=====  Conversion to double  ================================================
const FitnessTFN * FitnessTFN::convertType(const Fitness *f) const {
	if (f->getType() == Fitness::Type::FUZZY)
		return dynamic_cast<const FitnessTFN *>(f);
	throw new FJSPException("Fitness",
		"Comparison of incompatible fitness values");
}

}
