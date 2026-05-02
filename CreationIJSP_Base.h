/*
 * CreationIJSP_Base.h
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */
#pragma once

#include "Creation.h"
#include "Encoder.h"
#include "IJSPClassRegister.h"
#include "Interval.h"
#include "Fitness.h"
#include "ProblemIJSP.h"
#include "IJSPException.h"
#include <memory>
#include "SGS_IJSP.h"

namespace IJSP {

	// Creation parameters defined in this header file
#define CREATION_SGS "creation.sgs"
#define CREATION_RANDOM_RATIO "creation.randomratio"


//=============================================================================
//
//	Class CreationRandomSchedule
//
//=============================================================================
/**
 * This class generates an initial individual/population by creating a
 * random schedule and then codifying it with the respective strategy
 *
 * @author hdiaz
 *
 */
	class CreationRandomSchedule : public FuzzyFW::Creation {
	protected:
		//=============================================================================
		//		COMMON FIELDS
		//=============================================================================
		const std::string sgsLabel;
		std::unique_ptr<SGS_IJSP> sgs;
		double randomRatio;


		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		explicit CreationRandomSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: sgsLabel(CREATION_SGS), Creation(parameters), randomRatio(0) { }

		CreationRandomSchedule(const CreationRandomSchedule &source)
			: Creation(source), sgsLabel(CREATION_SGS), randomRatio(source.randomRatio) { }

		/**
		* Loads the needed parameters: Read the minimum/maximum
		 * values that each gene may take
		*/
		virtual void setup(FuzzyFW::ParameterDB *parameters);

		/**
		* Loads the needed parameters: Read the minimum/maximum
		* values that each gene may take
		*/
		virtual Creation * clone() const {
			return new CreationRandomSchedule(*this);
		}

		virtual ~CreationRandomSchedule() { }


		//=========================================================================
		//		METHODS
		//=========================================================================
	protected:
		bool shouldUseRandom(const FuzzyFW::SharedVarsEvolutionary *svars) const;

		std::vector<std::string> buildStrategyName(const std::string &name) const;

	public:
		/**
		* Creates an individual
		*
		* @param individualType Type of individuals to create
		* @param svars Shared variables for the algorithm
		* @return A new born individual
		*/
		virtual FuzzyFW::Individual * createIndividual(
			const FuzzyFW::SharedVarsEvolutionary *svars) const;

		/**
		* Get the name and setup of the operator
		*
		* @return A string of parameter values. The first string is the name of
		* the operator
		*/
		virtual std::vector<std::string> getName() const {
			std::vector<std::string> setup;
			std::vector<std::string> sgsName = this->sgs->getName();
			setup.push_back("Random");
			setup.push_back(";SGS:;" + sgsName[0]);
			for (size_t i = 1; i < sgsName.size(); i++)
				setup.push_back(";" + sgsName[i]);
			return setup;
		}
	};

}
