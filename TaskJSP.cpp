/*
* TaskJSP.cpp
*
*  Created on: June 06, 2019
*      Author: Hernan Diaz
*/

#include "TaskJSP.h"

namespace JSP {

//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
TaskJSP::TaskJSP() :
	id(0), machine(-1), job(-1), jp(-1), js(-1), p(FuzzyFW::Crisp(0))
{ }

//====  Full constructor  =====================================================
TaskJSP::TaskJSP(int id, int job, int machine, int jobPred, int jobSuc)
	: id(id), machine(machine), job(job), jp(jobPred), js(jobSuc),
	p(FuzzyFW::Crisp(0))
{ }
	
//====  Duration constructor  =================================================
TaskJSP::TaskJSP(int id, int job, int machine, int jobPred,
	int jobSuc, FuzzyFW::Crisp pt) :
	id(id), machine(machine), job(job), jp(jobPred), js(jobSuc), p(pt)
{ }
	
//====  Copy constructor  =====================================================
TaskJSP::TaskJSP(const TaskJSP & source)
	: id(source.id), machine(source.machine), job(source.job),
	jp(source.jp), js(source.js), p(source.p)
{ }



//=============================================================================
//		OPERATORS
//=============================================================================
//====  Assignment overload  ==================================================
TaskJSP & TaskJSP::operator = (const TaskJSP &src) {
	this->id =	src.id;
	this->job =	src.job;
	this->machine =	src.machine;
	this->jp =	src.jp;
	this->js =	src.js;
	this->p =	src.p;
	return *this;
}

}
