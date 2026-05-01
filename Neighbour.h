/*
* Neighbourhood.h
*
*  Created on: Oct 11, 2017
*/
#pragma once

#include "Fitness.h"
#include "FitnessMO.h"
#include "Solution.h"

namespace FuzzyFW {

typedef std::pair<FuzzyFW::Solution *, FuzzyFW::Fitness *> FullSolution;

	//=============================================================================
	//
	//	Abstract class Neighbour
	//
	//=============================================================================
	/**
	* This class defines the different neighbours that can be used in a local
	* search algorithm
	*
	*
	*/
	class Neighbour {
		//=========================================================================
		//		COMMON FIELDS
		//=========================================================================
	protected:
		/*
		* Estimated quality of the neighbour
		*/
		Fitness * estimatedQuality;

		/*
		* Flag indicating if the neighbour's quaity has been estimated
		*/
		bool estimated;

		/*
		* Estimated quality of the neighbour
		*/
		FullSolution solution;

		/*
		* Flag indicating if the neighbour's quaity has been estimated
		*/
		bool evaluated;



		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/*
		* Default constructor.
		*/
		explicit Neighbour()
			: estimatedQuality(NULL), estimated(false), evaluated(false) {
			solution.first = NULL;
			solution.second = NULL;
		}

		/**
		* Copy constructor
		*/
		Neighbour(const Neighbour &source);

		/**
		* Clone method for inheriting classes
		*/
		virtual Neighbour * clone() const = 0;

		/**
		* Destructor
		*/
		virtual ~Neighbour() {
			if (this->estimatedQuality != NULL)
				delete estimatedQuality;
			if (this->solution.first != NULL)
				delete this->solution.first;
			if (this->solution.second != NULL)
				delete this->solution.second;
		}



		//=========================================================================
		//		METHODS
		//=========================================================================
		/*
		* Set a fitness as an estimated quality for the neighbour
		*/
		virtual void setEstimatedQuality(Fitness *estimation);

		/*
		* Gets the estimated quality of the neighbour
		*/
		virtual Fitness * getEstimatedQuality() {
			if (!this->estimated)
				return NULL;
			return this->estimatedQuality;
		}


		/*
		* Checks if the neighbour has been fully evaluated
		*/
		virtual bool isEstimated() const {
			return this->estimated;
		}

		/*
		* Set a the evaluted fitness of the neighbour
		*/
		virtual void setEvaluation(Solution *sol, Fitness *fitness);

		/*
		* Gets the fitness value of the neighbour
		*/
		virtual Fitness * getEvaluatedFitness() {
			if (!this->evaluated)
				return NULL;
			return this->solution.second;
		}

		/*
		* Gets the full solution evaluated
		*/
		virtual Solution * getEvaluation() {
			if (!this->evaluated)
				return NULL;
			return this->solution.first;
		}


		/*
		* Checks if the neighbour's quality has been evaluated
		*/
		virtual bool isEvaluated() const {
			return this->evaluated;
		}


		/*
		* Comparison methods: Equality
		* Indicates if two neighbours are the same
		*/
		virtual bool isEqualTo(const Neighbour *v) const = 0;

		/*
		* Comparison methods: Reverse
		* Indicates if two neighbours are exactly the opposite of each other.
		* That is, if we move to this neighbour, we end up in neighbour v.
		*/
		virtual bool isReverse(const Neighbour *v) const = 0;

	};

}
