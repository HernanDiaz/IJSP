/*
* FuzzyTask.cpp
*
*  Created on: May 12, 2017
*      Author: Juan Jose Palacios
*/

#include "TaskFJSP.h"

namespace FJSP {

//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
TaskFJSP::TaskFJSP() :
	id(0), machine(-1), job(-1), jp(-1), js(-1), p(FuzzyFW::TFN(0, 0, 0))
{ }

//====  Full constructor  =====================================================
TaskFJSP::TaskFJSP(int id, int job, int machine, int jobPred, int jobSuc)
	: id(id), machine(machine), job(job), jp(jobPred), js(jobSuc),
	p(FuzzyFW::TFN(0, 0, 0))
{ }
	
//====  Duration constructor  =================================================
TaskFJSP::TaskFJSP(int id, int job, int machine, int jobPred,
	int jobSuc, FuzzyFW::TFN pt) :
	id(id), machine(machine), job(job), jp(jobPred), js(jobSuc), p(pt)
{ }
	
//====  Copy constructor  =====================================================
TaskFJSP::TaskFJSP(const TaskFJSP & source)
	: id(source.id), machine(source.machine), job(source.job),
	jp(source.jp), js(source.js), p(source.p)
{ }



//=============================================================================
//		OPERATORS
//=============================================================================
//====  Assignment overload  ==================================================
TaskFJSP & TaskFJSP::operator = (const TaskFJSP &src) {
	this->id =	src.id;
	this->job =	src.job;
	this->machine =	src.machine;
	this->jp =	src.jp;
	this->js =	src.js;
	this->p =	src.p;
	return *this;
}

}
