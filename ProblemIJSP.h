#pragma once

/*
* Problem IJSP.h
*
*  Created on: June 18, 2019
*      Author: Hernan Diaz 
*/
#pragma once

#include "TaskIJSP.h"
#include "Problem.h"
#include "IJSPException.h"


namespace IJSP {

	/* Indicates if the problem file has bounds included (yes/no)
	 * If omited, "no" will be taken as default answer
	 */
#define PROBLEM_LOWERBOUNDS "problem.hasbounds"

	 /* Indicates the type of due-date used in the file.
	  * If omited, due-dates will not be considered
	  */
#define PROBLEM_DUEDATES "problem.duedates"



	  //=========================================================================
	  //
	  //	Class ProblemIJSP
	  //
	  //=========================================================================
	  /**
	  * Objects of this class contain all data defining an instance of a Interval
	  * Problem (IJSP in this version). The data is contained withing a file
	  * that must be provided to the class, either at creation time or using a
	  * specific method. The files contain all the same format:
	  *
	  * Text label
	  * number of jobs
	  * Text label
	  * number of machines or resources
	  * Text label
	  * A matrix containing for each job i and operation j, the machine where it
	  * must be performed, being i the rows and j the columns
	  * Text label
	  * A matrix containing for each job i and operation j, the processing
	  * time of the operation, being i the rows and j the columns
	  *
	  * Now if the file has duedates, another piece of data will appear on the file:
	  *
	  * Text label
	  * An array of due-dates
	  *
	  * Finally, if there are known bounds for the problem, they will appear as:
	  *
	  * Text label
	  * An array with the bounds in this order: E[Cmax], AImin, AImax
	  *
	  *
	  * @author hdiaz
	  *
	  */

	class ProblemIJSP : public FuzzyFW::Problem {
		//=====================================================================
		//		FIELDS
		//=====================================================================
	protected:
		/**
		* Lower/Upper Bounds for different objective functions
		*/
		double lb_Makespan;
		double ub_AImin;
		double ub_AIavg;

		/**
		* Number of jobs
		*/
		unsigned int nJobs;

		/**
		* Number of machines or resources
		*/
		unsigned int nMachines;

		/**
		* Total number of tasks to be processed
		*/
		unsigned int nTasks;

		/**
		* Array of tasks that constitute the instance
		*/
		std::vector<TaskIJSP *> task;

		/**
		* Due-dates of the problem
		*/
		std::vector<FuzzyFW::TimeWindow *> dueDate;

		/**
		* Sequence of task in each job
		*/
		std::vector< std::vector<unsigned int> > taskSequence;

		/**
		* The input file has bounds
		*/
		char withBounds;


		/**
		* Type of due-dates contained in the file. Empty string if none
		*/
		std::string duedateType;



		//=====================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=====================================================================
	public:
		/**
		* Default constructor
		*/
		ProblemIJSP(const FuzzyFW::ParameterDB *params = NULL);


		/**
		* Sets the problem file, but does not load it
		*/
		ProblemIJSP(const char *inputFile);


		/**
		* Main constructor
		* Requires the file of parameters to check if there are bounds and/or
		* due-dates and therefore make a better reading of the input file.
		*/
		ProblemIJSP(const FuzzyFW::ParameterDB *params, const char *inputFile);


		/**
		* Loads the parameters describing the problem
		*/
		virtual void setup(const FuzzyFW::ParameterDB *params);


		/**
		* Copy constructor
		*/
		ProblemIJSP(const ProblemIJSP & source);


		/**
		* Clone operator for the case of inheritance
		*/
		virtual Problem* clone() const {
			return new ProblemIJSP(*this);
		}


		//=====================================================================
		//		DESTRUCTOR
		//=====================================================================
	public:
		~ProblemIJSP();



		//=====================================================================
		//		GET/SET METHODS
		//=====================================================================
	public:
		/**
		* Get the number of jobs in the problem
		*/
		unsigned int getNumberJobs() const {
			return this->nJobs;
		}


		/**
		* Get the number of machines in the problem
		*/
		unsigned int getNumberMachines() const {
			return this->nMachines;
		}


		/**
		* Get the number of operations in the problem
		*/
		unsigned int getNumberTasks() const {
			return this->nTasks;
		}


		/**
		* Get the number of operations in a specific job
		*/
		unsigned int getNumberTasks(const unsigned int job) const;


		/**
		* Check if there are lower/upper bounds
		*/
		char hasBounds() const {
			return this->withBounds;
		}


		/**
		* Get the different lower/upper bounds of the problem
		*/
		double getMakespanLB() const;
		double getAIminUB() const;
		double getAIavgUB() const;


		/**
		* Indicates if due-dates are present in the problem
		*/
		bool hasDueDates() const {
			return this->dueDate.size() > 0;
		}

		/**
		* Returns number of dueDates
		*/
		int dueDatesSize() const {
			return this->dueDate.size();
		}



		/**
		* Get the due-date of a given job.
		* Returns NULL if there is no due-date for this job
		*/
		FuzzyFW::TimeWindow * getTimeWindow(const unsigned int job) const;


		/**
		* Get the Id of the task in a specific position of a job.
		* Returns -1 if the task does not exist
		*/
		int getTaskId(const unsigned int job, const int position) const;


		/**
		* Gets a specific task
		*/
		TaskIJSP * getTask(const unsigned int taskId) const;


		//=====================================================================
		//		OPERATORS
		//=====================================================================
	public:
		/**
		* Index access to the tasks. Tasks are constant, so no changes
		* allowed
		*/
		const TaskIJSP * operator[](const int taskId) const {
			return this->getTask(taskId);
		}



		//=====================================================================
		//		METHODS
		//=====================================================================
	public:
		/**
		* Load all data from the stored file
		*/
		virtual void loadFile(const char *inputFile = NULL);

		virtual void saveFile(const char *outputFile = NULL);


	protected:
		/**
		* Auxiliary method: Load the due-dates from the file
		* The file reader must be pointing at the due-date section
		*/
		void loadDueDates(std::ifstream &input);


		/**
		* Auxiliary method: Load the lower/upper bounds from the file
		* The file reader must be pointing at the bounds section
		*/
		void loadBounds(std::ifstream &input);


		/**
		* Clears all the structures in the class. Used mainly for reset
		* purposes or re-read a file.
		*/
		virtual void clear();


		/**
		* Scalarizes the identifiers of the tasks. Some problems enumerate the
		* elements begining by number 0, whereas other begin
		* using the number 1. This method standarizes the numeration starting
		* always in 0.
		*/
		void scalarize();
	};
}
