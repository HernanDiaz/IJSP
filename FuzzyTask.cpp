/*
* FuzzyTask.cpp
*
*  Created on: May 12, 2017
*      Author: Juan Jose Palacios
*/

#include "FuzzyTask.h"

namespace FJSP {

//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
FuzzyTask::FuzzyTask() :
	id(0), machine(-1), job(-1), jp(-1), js(-1), p(TFN(0, 0, 0))
{ }

//====  Full constructor  =====================================================
FuzzyTask::FuzzyTask(int id, int job, int machine, int jobPred, int jobSuc)
	: id(id), machine(machine), job(job), jp(jobPred), js(jobSuc),
	p(TFN(0, 0, 0))
{ }
	
//====  Duration constructor  =================================================
FuzzyTask::FuzzyTask(int id, int job, int machine, int jobPred,
	int jobSuc, TFN pt) :
	id(id), machine(machine), job(job), jp(jobPred), js(jobSuc), p(pt)
{ }
	
//====  Copy constructor  =====================================================
FuzzyTask::FuzzyTask(const FuzzyTask & source)
	: id(source.id), machine(source.machine), job(source.job),
	jp(source.jp), js(source.js), p(source.p)
{ }



//=============================================================================
//		OPERATORS
//=============================================================================
//====  Assignment overload  ==================================================
FuzzyTask & FuzzyTask::operator = (const FuzzyTask &src) {
	this->id =	src.id;
	this->job =	src.job;
	this->machine =	src.machine;
	this->jp =	src.jp;
	this->js =	src.js;
	this->p =	src.p;
}

}
