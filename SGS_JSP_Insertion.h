/*
* SGS_JSP_Insertion.h
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/

#pragma once

#include "SGS_JSP.h"

namespace JSP {

//=============================================================================
//
//	Class SGS_JSP_Insertion
//
//=============================================================================
/**
* The insertion SGS is introduced and detailed in the following paper:
* "Schedule generation schemes for job shop problems with fuzziness.
* JJ Palacios, CR Vela, I Gonz�lez-Rodr�guez, J Puente - Proceedings of the
* Twenty-first European Conference on Artificial Intelligence, 687-692,
* 2014"
*
* In brief, it uses an insertion strategy to find holes in the schedule
* that can be used to introduce the new task with no possibility of
* delaying any other.
*
* @author hdiaz
*
*/

class SGS_JSP_Insertion : public SGS_JSP
{
	//=========================================================================
	//		FIELDS
	//=========================================================================

	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	SGS_JSP_Insertion(const FuzzyFW::ParameterDB *params = NULL)
		: SGS_JSP(params) { }; // Nothing new here


	/*
	* Copy constructor
	*/
	SGS_JSP_Insertion(const SGS_JSP_Insertion &source)
		: SGS_JSP(source) { }; // Here neither


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_JSP * clone() const {
		return new SGS_JSP_Insertion(*this);
	}


	/*
	* Destructor... not much to destroy though
	*/
	virtual ~SGS_JSP_Insertion() { }



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the name and configuration of the SGS
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Insertion");
		return setup;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
protected:
	virtual void postBuild() override;


	/*
	* Looks for a space in the current schedule where the task can be
	* scheduled without delaying any other in any scenario. If there is no
	* such a space, schedules the new task at the end.
	*/
	virtual FuzzyFW::Crisp scheduleTask(const TaskJSP *task,
		const int taskIdx);
};

}
