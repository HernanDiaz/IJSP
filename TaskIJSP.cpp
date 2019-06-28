/*
* TaskIJSP.cpp
*
*  Created on: June 06, 2019
*      Author: Hernan Diaz
*/

#include "TaskIJSP.h"

namespace IJSP {

//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
TaskIJSP::TaskIJSP() :
	id(0), machine(-1), job(-1), jp(-1), js(-1), p(FuzzyFW::Interval(0, 0))
{ }

//====  Full constructor  =====================================================
TaskIJSP::TaskIJSP(int id, int job, int machine, int jobPred, int jobSuc)
	: id(id), machine(machine), job(job), jp(jobPred), js(jobSuc),
	p(FuzzyFW::Interval(0, 0))
{ }
	
//====  Duration constructor  =================================================
TaskIJSP::TaskIJSP(int id, int job, int machine, int jobPred,
	int jobSuc, FuzzyFW::Interval pt) :
	id(id), machine(machine), job(job), jp(jobPred), js(jobSuc), p(pt)
{ }
	
//====  Copy constructor  =====================================================
TaskIJSP::TaskIJSP(const TaskIJSP & source)
	: id(source.id), machine(source.machine), job(source.job),
	jp(source.jp), js(source.js), p(source.p)
{ }



//=============================================================================
//		OPERATORS
//=============================================================================
//====  Assignment overload  ==================================================
TaskIJSP & TaskIJSP::operator = (const TaskIJSP &src) {
	this->id =	src.id;
	this->job =	src.job;
	this->machine =	src.machine;
	this->jp =	src.jp;
	this->js =	src.js;
	this->p =	src.p;
	return *this;
}

}
