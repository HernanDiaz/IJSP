/*
 * PathRelinkIJSP.h
 *
 *  Created on: Jun 10, 2026
 *      Author: hdiaz
 */
#pragma once

#include "Neighbourhood.h"
#include "NeighbourIJSP.h"

namespace IJSP {

#define PR_NEIGHBOURHOOD "pr.neighbourhood"
#define PR_NEIGHBOURHOOD_DEFAULT "ijsp.makespan.n1"
#define PR_MAX_STEPS "pr.max-steps"


//=============================================================================
//
//	Class PathRelinkIJSP
//
//=============================================================================
/**
* Path relinking operator over the disjunctive-graph orientation of IJSP
* schedules.
*
* The walk drives a Neighbourhood object (by default N1, all extreme critical
* arcs): at each step it generates the candidate arcs of the current schedule,
* keeps only those whose reversal moves the schedule TOWARDS the guiding
* solution (arcs (x,y) such that the guide sequences y before x on their
* machine), and accepts the best one according to the neighbourhood's
* estimation order. Each accepted step reverses exactly one disagreeing
* adjacent pair, so the machine-wise disagreement with the guide strictly
* decreases and the walk always terminates. Feasibility of every step follows
* from reversing extreme critical arcs only (no acyclicity check needed).
*
* The distance between two schedules is the number of disjunctive pairs
* ordered differently (machine-wise Kendall tau), computed directly from the
* public machine-predecessor/successor fields of ScheduleIJSP::taskInfo.
*
* @author hdiaz
*/
class PathRelinkIJSP {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	/**
	* Candidate-arc generator driven by the walk (owned)
	*/
	FuzzyFW::Neighbourhood *neighbourhood;

	/**
	* Hard cap on walk length (-1 = until no disagreeing arc remains)
	*/
	int maxSteps;

	/**
	* Metrics of the last walk
	*/
	unsigned int steps;
	unsigned int evaluations;


	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	explicit PathRelinkIJSP()
		: neighbourhood(NULL), maxSteps(-1), steps(0), evaluations(0) { }

	~PathRelinkIJSP() {
		delete this->neighbourhood;
	}

	/**
	* Loads the candidate neighbourhood and the walk parameters
	*/
	void setup(FuzzyFW::ParameterDB *parameters);


	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	unsigned int getSteps() const {
		return this->steps;
	}

	unsigned int getEvaluations() const {
		return this->evaluations;
	}

	std::vector<std::string> getName();


	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Position of every task within its machine sequence
	*/
	static std::vector<int> machinePositions(const ScheduleIJSP *schedule);

	/**
	* Number of disjunctive pairs ordered differently in the two schedules
	*/
	static double disagreement(const ScheduleIJSP *a, const ScheduleIJSP *b);

	/**
	* Total number of disjunctive pairs of the instance
	*/
	static double totalPairs(const ScheduleIJSP *schedule);

	/**
	* Walks from `from` towards `guide` and returns the best intermediate
	* solution found (cloned: the caller owns both pointers of the result)
	*/
	FuzzyFW::FullSolution relink(const FuzzyFW::Solution *from,
		const FuzzyFW::Fitness *fromFitness, const ScheduleIJSP *guide,
		const FuzzyFW::SharedVars *svars);
};

}
