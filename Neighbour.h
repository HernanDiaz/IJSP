/*
* Neighbourhood.h
*
*  Created on: Oct 11, 2017
*      Author: jjpalacios
*/
#pragma once

#include "Fitness.h"

namespace FuzzyFW {

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

}
