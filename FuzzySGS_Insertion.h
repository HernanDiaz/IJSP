/*
* FuzzySGS_Insertion.h
*
*  Created on: June 1, 2017
*      Author: Juan Jose Palacios
*/

#ifndef FJSPPROBLEM_FUZZYSGS_INS_H_
#define FJSPPROBLEM_FUZZYSGS_INS_H_

#include "FuzzySGS.h"
#include "SharedVars.h"


namespace FJSP {

	//=============================================================================
	//
	//	Class FuzzySGS_Insertion
	//
	//=============================================================================
	/**
	* The insertion SGS is introduced and detailed in the following paper:
	* "Schedule generation schemes for job shop problems with fuzziness.
	* JJ Palacios, CR Vela, I González-Rodríguez, J Puente - Proceedings of the
	* Twenty-first European Conference on Artificial Intelligence, 687-692,
	* 2014"
	* 
	* In brief, it uses an insertion strategy to find holes in the schedule
	* that can be used to introduce the new task with no possibility of
	* delaying any other.
	* 
	* @author Juan Jose Palacios
	*
	*/

	class FuzzySGS_Insertion : public FuzzySGS
	{
		//=====================================================================
		//		FIELDS
		//=====================================================================

		//=====================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=====================================================================
	public:
		/*
		* Default constructor
		*/
		FuzzySGS_Insertion() : FuzzySGS() { }; // Nothing new here


		/*
		* Copy constructor
		*/
		FuzzySGS_Insertion(const FuzzySGS_Insertion &source)
			: FuzzySGS(source) { }; // Here neither


		/*
		* Clone method to replicate inherited instances
		*/
		virtual FuzzySGS * clone() const {
			return new FuzzySGS_Insertion(*this);
		}


		/*
		* Destructor
		*/
		virtual ~FuzzySGS_Insertion() { }  // Daddy's responsabilty



		//=====================================================================
		//		GET/SET METHODS
		//=====================================================================
	public:
		/*
		* Get the name and configuration of the SGS
		*/
		virtual std::vector<std::string> getName() const {
			std::vector<std::string> setup;
			setup.push_back("Insertion");
			return setup;
		}
		


		//=====================================================================
		//		METHODS
		//=====================================================================
	public:
		/*
		* Takes an order of operations and creates a schedule from them.
		* This method requires several shared variables, as it is the problem
		* and the RNG
		*/
		virtual FuzzySchedule * buildSchedule(const SharedVars * const svars,
			const std::vector<int> &order);


	protected:
		/*
		* Schedules a specific task in the current schedule. There are many
		* strategies to do this, so this method must be adapted for each SGS.
		* Returns the starting time assigned to the operation
		*/
		virtual TFN scheduleTask(const FuzzyProblem *problem,
			const unsigned int task);
};





	/*
/*=============================================================================
||					CLASE SGS_Insertion
=============================================================================*/
class SGS_Insertion : public Scheduler {
public:
	//! Constructors
	SGS_Insertion() : Scheduler() {};
	SGS_Insertion(Problem *p) : Scheduler(p) {};
	SGS_Insertion(const SGS_Insertion & source)
		: Scheduler(source) {};

	//! Destructor
	~SGS_Insertion() { };

	//! Overloaded Methods
	string getName() { return "Insertion"; };
	int scheduleTask(int task);
};



/*=============================================================================
||					CLASS SGS_Dense
=============================================================================*/
class SGS_Dense : public Scheduler {
public:
	//! Constructors
	SGS_Dense() : Scheduler() {};
	SGS_Dense(Problem *p) : Scheduler(p) {};
	SGS_Dense(const SGS_Dense & source)
		: Scheduler(source) {};

	//! Destructor
	~SGS_Dense() { };

	//! Overloaded Methods
	string getName() { return "Densa"; };
	int buildSchedule(vector<int> & permutation);
};



/*=============================================================================
||					CLASS SGS_fG&T1
=============================================================================*/
class SGS_fGYT1 : public Scheduler {
public:
	//! Constructors
	SGS_fGYT1(const double delta = 1.0) : Scheduler() { _delta = delta; };
	SGS_fGYT1(Problem *p, const double delta = 1.0) : Scheduler(p) { _delta = delta; };
	SGS_fGYT1(const SGS_fGYT1 & source)
		: Scheduler(source) {
		_delta = source._delta;
	};

	//! Destructor
	~SGS_fGYT1() { };

	//! Overloaded Methods
	string getName() { return "SGS-fGYT1"; };
	int buildSchedule(vector<int> & permutation);

	double _delta;
};



/*=============================================================================
||					CLASS SGS_fG&T2
=============================================================================*/
class SGS_fGYT2 : public Scheduler {
public:
	//! Constructors
	SGS_fGYT2(const double delta = 1.0) : Scheduler() { _delta = delta; };
	SGS_fGYT2(Problem *p, const double delta = 1.0) : Scheduler(p) { _delta = delta; };
	SGS_fGYT2(const SGS_fGYT1 & source)
		: Scheduler(source) {
		_delta = source._delta;
	};

	//! Destructor
	~SGS_fGYT2() { };

	//! Overloaded Methods
	string getName() { return "SGS-fGYT2"; };
	int buildSchedule(vector<int> & permutation);

	double _delta;
};



/*=============================================================================
||					CLASE SGS_eG&T
=============================================================================*/
class SGS_eGYT : public Scheduler {
public:
	//! Constructors
	SGS_eGYT(const double delta = 1.0) : Scheduler() { _delta = delta; };
	SGS_eGYT(Problem *p, const double delta = 1.0) : Scheduler(p) { _delta = delta; };
	SGS_eGYT(const SGS_eGYT & source)
		: Scheduler(source) {
		_delta = source._delta;
	};

	//! Destructor
	~SGS_eGYT() { };

	//! Overloaded Methods
	string getName() { return "E-GYT"; };
	int buildSchedule(vector<int> & permutation);

	double _delta;
};



/*=============================================================================
||					CLASE SGS_Sakawa
=============================================================================*/
class SGS_Sakawa : public Scheduler {
public:
	//! Constructors
	SGS_Sakawa(const double delta = 1.0) : Scheduler() { _delta = delta; };
	SGS_Sakawa(Problem *p, const double delta = 1.0) : Scheduler(p) { _delta = delta; };
	SGS_Sakawa(const SGS_eGYT & source)
		: Scheduler(source) {
		_delta = source._delta;
	};

	//! Destructor
	~SGS_Sakawa() { };

	//! Overloaded Methods
	string getName() { return "Sakawa"; };
	int buildSchedule(vector<int> & permutation);

	double _delta;
};

*/

#endif /* FJSPPROBLEM_FUZZYSGS_INS_H_ */
