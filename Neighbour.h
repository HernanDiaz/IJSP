/*
* Neighbourhood.h
*
*  Created on: Oct 11, 2017
*      Author: jjpalacios
*/
#ifndef LS_NEIGHBOUR_H_
#define LS_NEIGHBOUR_H_

#include "Fitness.h"
#include "FuzzySchedule.h"

namespace FJSP {

	//=============================================================================
	//
	//	Abstract class Neighbour
	//
	//=============================================================================
	/**
	* This class defines the different neighbours that can be used in a local
	* search algorithm
	*
	* @author jjpalacios
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
		bool isEstimated;



		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/*
		* Default constructor.
		*/
		explicit Neighbour()
			: estimatedQuality(NULL), isEstimated(false) { }

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
			if (!this->isEstimated)
				return NULL;
			return this->estimatedQuality;
		}


		/*
		* Checks if the neighbour's quality has been estimated
		*/
		virtual bool hasEstimation() const {
			return this->isEstimated;
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





	//=============================================================================
	//
	//	Class NeighbourFJSP_Arc
	//
	//=============================================================================
	/**
	* This class defines a specific type of neighbour for FJSP problems.
	* It defines a neighbour as a reversal of a disjuntive arc in the graph
	* associated to a solution. The class will keep the origin of the original arc
	* as well as the destination
	*
	* @author jjpalacios
	*
	*/
	class NeighbourFJSP_Arc : public Neighbour {
		//=========================================================================
		//		FIELDS
		//=========================================================================
	public:
		/*
		* Origin node of the arc
		*/
		unsigned int x;

		/*
		* Destination node of the arc
		*/
		unsigned int y;

		/*
		* Internal data structures
		* Certain estimators will compute values that are useful
		* later for the full evaluation
		*/
		TFN newHeadX;
		TFN newHeadY;
		TFN newTailX;
		TFN newTailY;

	protected:
		/*
		* Indicates if the inner strcutres have been updated
		*/
		char headsUpdated;





		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/*
		* Default constructor.
		*/
		explicit NeighbourFJSP_Arc()
			: Neighbour(), x(0), y(0), newHeadX(TFN(-1, -1, -1)),
			newHeadY(TFN(-1, -1, -1)), newTailX(TFN(-1, -1, -1)),
			newTailY(TFN(-1, -1, -1)), headsUpdated(false) { }
		
		/*
		* Main constructor.
		*/
		explicit NeighbourFJSP_Arc(const int x, const int y)
			: Neighbour(), x(x), y(y), newHeadX(TFN(-1, -1, -1)),
			newHeadY(TFN(-1, -1, -1)), newTailX(TFN(-1, -1, -1)),
			newTailY(TFN(-1, -1, -1)), headsUpdated(false) { }

		/**
		* Copy constructor
		*/
		NeighbourFJSP_Arc(const NeighbourFJSP_Arc &source);

		/**
		* Clone method for inheriting classes
		*/
		virtual Neighbour * clone() const {
			return new NeighbourFJSP_Arc(*this);
		}

		/**
		* Destructor. Nothing to do here
		*/
		virtual ~NeighbourFJSP_Arc() { }



		//=========================================================================
		//		METHODS
		//=========================================================================
	public:
		/*
		* Sets the nodes of the arc
		*/
		virtual void setValues(const unsigned int x, const unsigned int y);

		/*
		* Comparison methods: Equality
		* Indicates if two neighbours are the same
		*/
		virtual bool isEqualTo(const Neighbour *v) const;

		/*
		* Comparison methods: Reverse
		* Indicates if two neighbours are exactly the opposite of each other.
		* That is, if we move to this neighbour, we end up in neighbour v.
		*/
		virtual bool isReverse(const Neighbour *v) const;


		/*
		* Intermediate state before fully evaluating the solution.
		* It computes the new head and tail values for the nodes that are
		* implicated in the neighbour
		*/
		void updateHeadsTails(FuzzySchedule *solution);
	};

}

#endif /* LS_NEIGHBOUR_H_ */
