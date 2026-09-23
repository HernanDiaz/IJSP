/*
 * CreationJSP_Base.h
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */
#pragma once

#include "Creation.h"
#include "Encoder.h"
#include "JSPClassRegister.h"
#include "CrispTime.h"
#include "Fitness.h"
#include "ProblemJSP.h"
#include "JSPException.h"
#include <memory>
#include "SGS_JSP.h"

namespace JSP {

	// Creation parameters defined in this header file
#define CREATION_SGS "creation.sgs"
#define CREATION_RANDOM_RATIO "creation.randomratio"
// I-016. How the random creation draws the next job of a sequence.
//   "job"     (default, unchanged): uniformly among the jobs that still have
//             operations left, however many they have left
//   "uniform" : in proportion to the operations each job has left, which makes
//             every sequence of the instance equally likely
// The default is biased. A job that happens to be drawn early runs out early,
// and the tail of the sequence fills with the last operations of the few jobs
// left behind, in blocks, which decodes badly. Measured 2026-09-23 at
// generation 0, three runs each: the default averages 2269 on ta23 and 2952 on
// ta45, uniformly random sequences 2117 and 2753, 6 to 7 % better, at the same
// diversity. The scouts draw through this same creation, 500 to 800 times a
// run, so the bias reaches far more than the first population.
#define CREATION_RANDOM_DRAW "creation.random.draw"


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
		std::unique_ptr<SGS_JSP> sgs;
		double randomRatio;
		bool uniformDraw;      // I-016


		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		explicit CreationRandomSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: sgsLabel(CREATION_SGS), Creation(parameters), randomRatio(0),
			  uniformDraw(false) { }

		CreationRandomSchedule(const CreationRandomSchedule &source)
			: Creation(source), sgsLabel(CREATION_SGS), randomRatio(source.randomRatio),
			  uniformDraw(source.uniformDraw) { }

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
