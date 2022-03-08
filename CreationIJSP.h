/*
 * Creation.h
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */
#pragma once

#include "Creation.h"
#include "IJSPClassRegister.h"
#include "EncoderFJSP.h"
#include "Interval.h"
#include "Fitness.h"
#include "ProblemIJSP.h"
#include "IJSPException.h"
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
		/*
		* Label to identify the SGS type
		*/
		const std::string sgsLabel;

		/*
		* SGS to create schedules from task orderings
		*/
		SGS_IJSP * sgs;



		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		 * Default constructor
		 */
		explicit CreationRandomSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: sgsLabel(CREATION_SGS), sgs(NULL), Creation(parameters) { }

		/**
		* Copy constructor
		*/
		CreationRandomSchedule(const CreationRandomSchedule &source)
			: Creation(source), sgsLabel(CREATION_SGS), sgs(NULL) { }

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

		/**
		 * Destructor
		 */
		virtual ~CreationRandomSchedule() { } 	// Nothing to destroy here


		//=========================================================================
		//		METHODS
		//=========================================================================
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

	//=============================================================================
	//
	//	Class CreationSRTIntervalMkSchedule
	//
	//=============================================================================
	/**
	 * This class generates an initial individual/population by creating a
	 * Shortest Remaining Time schedule and then codifying it with the respective strategy
	 *
	 * @author hdiaz
	 *
	 */
	class CreationSRTIntervalMkSchedule : public FuzzyFW::Creation {
	protected:
		//=============================================================================
		//		COMMON FIELDS
		//=============================================================================
		/*
		* Label to identify the SGS type
		*/
		const std::string sgsLabel;

		double randomRatio;

		/*
		* SGS to create schedules from task orderings
		*/
		SGS_IJSP * sgs;


		CreationRandomSchedule randomSchedule;


		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		 * Default constructor
		 */
		explicit CreationSRTIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: sgsLabel(CREATION_SGS), sgs(NULL), Creation(parameters), randomRatio(0) { }

		/**
		* Copy constructor
		*/
		CreationSRTIntervalMkSchedule(const CreationSRTIntervalMkSchedule &source)
			: Creation(source), sgsLabel(CREATION_SGS), sgs(NULL), randomRatio(source.randomRatio) { }

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
			return new CreationSRTIntervalMkSchedule(*this);
		}

		/**
		 * Destructor
		 */
		virtual ~CreationSRTIntervalMkSchedule() { } 	// Nothing to destroy here


		//=========================================================================
		//		METHODS
		//=========================================================================
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
			setup.push_back("SRT");
			setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
			setup.push_back(";SGS:;" + sgsName[0]);
			for (size_t i = 1; i < sgsName.size(); i++)
				setup.push_back(";" + sgsName[i]);
			return setup;
		}
	};

	//=============================================================================
	//
	//	Class CreationSRTFIntervalMkSchedule
	//
	//=============================================================================
	/**
	 * This class generates an initial individual/population by creating a
	 * Longest Remaining Time First schedule and then codifying it with the respective strategy
	 *
	 * @author hdiaz
	 *
	 */
	class CreationLRTFIntervalMkSchedule : public FuzzyFW::Creation {
	protected:
		//=============================================================================
		//		COMMON FIELDS
		//=============================================================================
		/*
		* Label to identify the SGS type
		*/
		const std::string sgsLabel;

		double randomRatio;

		/*
		* SGS to create schedules from task orderings
		*/
		SGS_IJSP * sgs;


		CreationRandomSchedule randomSchedule;


		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		 * Default constructor
		 */
		explicit CreationLRTFIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: sgsLabel(CREATION_SGS), sgs(NULL), Creation(parameters), randomRatio(0) { }

		/**
		* Copy constructor
		*/
		CreationLRTFIntervalMkSchedule(const CreationLRTFIntervalMkSchedule &source)
			: Creation(source), sgsLabel(CREATION_SGS), sgs(NULL), randomRatio(source.randomRatio) { }

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
			return new CreationLRTFIntervalMkSchedule(*this);
		}

		/**
		 * Destructor
		 */
		virtual ~CreationLRTFIntervalMkSchedule() { } 	// Nothing to destroy here


		//=========================================================================
		//		METHODS
		//=========================================================================
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
			setup.push_back("LRTF");
			setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
			setup.push_back(";SGS:;" + sgsName[0]);
			for (size_t i = 1; i < sgsName.size(); i++)
				setup.push_back(";" + sgsName[i]);
			return setup;
		}
	};



	//=============================================================================
	//
	//	Class CreationSRTFInverseIntervalMkSchedule
	//
	//=============================================================================
	/**
	 * This class generates an initial individual/population by creating a
	 * Longest Remaining Time First Inverse schedule and then codifying it with the respective strategy
	 *(gives priority to the shortest remaining)
	 * @author hdiaz
	 *
	 */
	class CreationLRTFInverseIntervalMkSchedule : public FuzzyFW::Creation {
	protected:
		//=============================================================================
		//		COMMON FIELDS
		//=============================================================================
		/*
		* Label to identify the SGS type
		*/
		const std::string sgsLabel;

		double randomRatio;

		/*
		* SGS to create schedules from task orderings
		*/
		SGS_IJSP * sgs;


		CreationRandomSchedule randomSchedule;


		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		 * Default constructor
		 */
		explicit CreationLRTFInverseIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: sgsLabel(CREATION_SGS), sgs(NULL), Creation(parameters), randomRatio(0) { }

		/**
		* Copy constructor
		*/
		CreationLRTFInverseIntervalMkSchedule(const CreationLRTFInverseIntervalMkSchedule &source)
			: Creation(source), sgsLabel(CREATION_SGS), sgs(NULL), randomRatio(source.randomRatio) { }

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
			return new CreationLRTFInverseIntervalMkSchedule(*this);
		}

		/**
		 * Destructor
		 */
		virtual ~CreationLRTFInverseIntervalMkSchedule() { } 	// Nothing to destroy here


		//=========================================================================
		//		METHODS
		//=========================================================================
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
			setup.push_back("LRTFInverse");
			setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
			setup.push_back(";SGS:;" + sgsName[0]);
			for (size_t i = 1; i < sgsName.size(); i++)
				setup.push_back(";" + sgsName[i]);
			return setup;
		}
	};


	//=============================================================================
	//
	//	Class CreationSRTFntervalMkSchedule
	//
	//=============================================================================
	/**
	 * This class generates an initial individual/population by creating a
	 * Shortest Next Task First schedule and then codifying it with the respective strategy
	 *
	 * @author hdiaz
	 *
	 */
	class CreationSNTFIntervalMkSchedule : public FuzzyFW::Creation {
	protected:
		//=============================================================================
		//		COMMON FIELDS
		//=============================================================================
		/*
		* Label to identify the SGS type
		*/
		const std::string sgsLabel;

		double randomRatio;

		/*
		* SGS to create schedules from task orderings
		*/
		SGS_IJSP * sgs;


		CreationRandomSchedule randomSchedule;


		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		 * Default constructor
		 */
		explicit CreationSNTFIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: sgsLabel(CREATION_SGS), sgs(NULL), Creation(parameters), randomRatio(0) { }

		/**
		* Copy constructor
		*/
		CreationSNTFIntervalMkSchedule(const CreationSNTFIntervalMkSchedule &source)
			: Creation(source), sgsLabel(CREATION_SGS), sgs(NULL), randomRatio(source.randomRatio) { }

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
			return new CreationSNTFIntervalMkSchedule(*this);
		}

		/**
		 * Destructor
		 */
		virtual ~CreationSNTFIntervalMkSchedule() { } 	// Nothing to destroy here


		//=========================================================================
		//		METHODS
		//=========================================================================
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
			setup.push_back("LRTF");
			setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
			setup.push_back(";SGS:;" + sgsName[0]);
			for (size_t i = 1; i < sgsName.size(); i++)
				setup.push_back(";" + sgsName[i]);
			return setup;
		}
	};

	//=============================================================================
	//
	//	Class CreationSCTFIntervalMkSchedule
	//
	//=============================================================================
	/**
	 * This class generates an initial individual/population by creating a
	 * Shortest Completion Time First schedule and then codifying it with the respective strategy
	 *
	 * @author hdiaz
	 *
	 */

	class CreationSCTFIntervalMkSchedule : public FuzzyFW::Creation {
		//=============================================================================
			//		COMMON FIELDS
			//=============================================================================
			/*
			* Label to identify the SGS type
			*/
		const std::string sgsLabel;

		double randomRatio;

		/*
		* SGS to create schedules from task orderings
		*/
		SGS_IJSP * sgs;


		CreationRandomSchedule randomSchedule;


		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		 * Default constructor
		 */
		explicit CreationSCTFIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: sgsLabel(CREATION_SGS), sgs(NULL), Creation(parameters), randomRatio(0) { }

		/**
		* Copy constructor
		*/
		CreationSCTFIntervalMkSchedule(const CreationSCTFIntervalMkSchedule &source)
			: Creation(source), sgsLabel(CREATION_SGS), sgs(NULL), randomRatio(source.randomRatio) { }

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
			return new CreationSCTFIntervalMkSchedule(*this);
		}

		/**
		 * Destructor
		 */
		virtual ~CreationSCTFIntervalMkSchedule() { } 	// Nothing to destroy here
		//=========================================================================
		//		METHODS
		//=========================================================================
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
			setup.push_back("SCTF");
			setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
			setup.push_back(";SGS:;" + sgsName[0]);
			for (size_t i = 1; i < sgsName.size(); i++)
				setup.push_back(";" + sgsName[i]);
			return setup;
		}

	};

	//=============================================================================
	//
	//	Class CreationLCTFIntervalMkSchedule
	//
	//=============================================================================
	/**
	 * This class generates an initial individual/population by creating a
	 * Longest Completion Time First schedule and then codifying it with the respective strategy
	 *
	 * @author hdiaz
	 *
	 */

	class CreationLCTFIntervalMkSchedule : public FuzzyFW::Creation {
		//=============================================================================
			//		COMMON FIELDS
			//=============================================================================
			/*
			* Label to identify the SGS type
			*/
		const std::string sgsLabel;

		double randomRatio;

		/*
		* SGS to create schedules from task orderings
		*/
		SGS_IJSP * sgs;


		CreationRandomSchedule randomSchedule;


		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		 * Default constructor
		 */
		explicit CreationLCTFIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: sgsLabel(CREATION_SGS), sgs(NULL), Creation(parameters), randomRatio(0) { }

		/**
		* Copy constructor
		*/
		CreationLCTFIntervalMkSchedule(const CreationLCTFIntervalMkSchedule &source)
			: Creation(source), sgsLabel(CREATION_SGS), sgs(NULL), randomRatio(source.randomRatio) { }

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
			return new CreationLCTFIntervalMkSchedule(*this);
		}

		/**
		 * Destructor
		 */
		virtual ~CreationLCTFIntervalMkSchedule() { } 	// Nothing to destroy here
		//=========================================================================
		//		METHODS
		//=========================================================================
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
			setup.push_back("LCTF");
			setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
			setup.push_back(";SGS:;" + sgsName[0]);
			for (size_t i = 1; i < sgsName.size(); i++)
				setup.push_back(";" + sgsName[i]);
			return setup;
		}

	};


	//=============================================================================
	//
	//	Class CreationSPJFIntervalMkSchedule
	//
	//=============================================================================
	/**
	 * This class generates an initial individual/population by creating a
	 * Shortest Planned Job First schedule and then codifying it with the respective strategy
	 * (gives priority to the jobs with the shortest makespan)
	 * @author hdiaz
	 *
	 */

	class CreationSPJFIntervalMkSchedule : public FuzzyFW::Creation {
		//=============================================================================
			//		COMMON FIELDS
			//=============================================================================
			/*
			* Label to identify the SGS type
			*/
		const std::string sgsLabel;

		double randomRatio;

		/*
		* SGS to create schedules from task orderings
		*/
		SGS_IJSP * sgs;


		CreationRandomSchedule randomSchedule;


		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		 * Default constructor
		 */
		explicit CreationSPJFIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: sgsLabel(CREATION_SGS), sgs(NULL), Creation(parameters), randomRatio(0) { }

		/**
		* Copy constructor
		*/
		CreationSPJFIntervalMkSchedule(const CreationSPJFIntervalMkSchedule &source)
			: Creation(source), sgsLabel(CREATION_SGS), sgs(NULL), randomRatio(source.randomRatio) { }

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
			return new CreationSPJFIntervalMkSchedule(*this);
		}

		/**
		 * Destructor
		 */
		virtual ~CreationSPJFIntervalMkSchedule() { } 	// Nothing to destroy here
		//=========================================================================
		//		METHODS
		//=========================================================================
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
			setup.push_back("SPJF");
			setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
			setup.push_back(";SGS:;" + sgsName[0]);
			for (size_t i = 1; i < sgsName.size(); i++)
				setup.push_back(";" + sgsName[i]);
			return setup;
		}

	};

	//=============================================================================
	//
	//	Class CreationSPJFInverseIntervalMkSchedule
	// 
	//=============================================================================
	/**
	 * This class generates an initial individual/population by creating a
	 * Shortest Planned Job First Inverse schedule and then codifying it with the respective strategy
	 * (gives priority to the jobs with the longest makespan)
	 * @author hdiaz
	 *
	 */

	class CreationSPJFInverseIntervalMkSchedule : public FuzzyFW::Creation {
		//=============================================================================
			//		COMMON FIELDS
			//=============================================================================
			/*
			* Label to identify the SGS type
			*/
		const std::string sgsLabel;

		double randomRatio;

		/*
		* SGS to create schedules from task orderings
		*/
		SGS_IJSP * sgs;


		CreationRandomSchedule randomSchedule;


		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		 * Default constructor
		 */
		explicit CreationSPJFInverseIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: sgsLabel(CREATION_SGS), sgs(NULL), Creation(parameters), randomRatio(0) { }

		/**
		* Copy constructor
		*/
		CreationSPJFInverseIntervalMkSchedule(const CreationSPJFInverseIntervalMkSchedule &source)
			: Creation(source), sgsLabel(CREATION_SGS), sgs(NULL), randomRatio(source.randomRatio) { }

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
			return new CreationSPJFInverseIntervalMkSchedule(*this);
		}

		/**
		 * Destructor
		 */
		virtual ~CreationSPJFInverseIntervalMkSchedule() { } 	// Nothing to destroy here
		//=========================================================================
		//		METHODS
		//=========================================================================
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
			setup.push_back("SPJFInverse");
			setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
			setup.push_back(";SGS:;" + sgsName[0]);
			for (size_t i = 1; i < sgsName.size(); i++)
				setup.push_back(";" + sgsName[i]);
			return setup;
		}

	};

	//=============================================================================
	//
	//	Class CreationManagerIntervalMkSchedule
	//
	//=============================================================================
	/**
	 * This class generates an initial individual/population by creating a
	 * Shortest Planned Job First schedule and then codifying it with the respective strategy
	 *
	 * @author hdiaz
	 *
	 */

	class CreationManagerIntervalMkSchedule : public FuzzyFW::Creation {
		//=============================================================================
			//		COMMON FIELDS
			//=============================================================================
			/*
			* Label to identify the SGS type
			*/
		const std::string sgsLabel;

		double randomRatio;

		/*
		* SGS to create schedules from task orderings
		*/
		SGS_IJSP * sgs;


		CreationRandomSchedule randomSchedule;
		CreationSPJFIntervalMkSchedule SPJFSchedule;
		CreationLRTFIntervalMkSchedule LRTFSchedule;

		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		 * Default constructor
		 */
		explicit CreationManagerIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: sgsLabel(CREATION_SGS), sgs(NULL), Creation(parameters), randomRatio(0) { }

		/**
		* Copy constructor
		*/
		CreationManagerIntervalMkSchedule(const CreationManagerIntervalMkSchedule &source)
			: Creation(source), sgsLabel(CREATION_SGS), sgs(NULL), randomRatio(source.randomRatio) { }

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
			return new CreationManagerIntervalMkSchedule(*this);
		}

		/**
		 * Destructor
		 */
		virtual ~CreationManagerIntervalMkSchedule() { } 	// Nothing to destroy here
		//=========================================================================
		//		METHODS
		//=========================================================================
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
			setup.push_back("Manager");
			setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
			setup.push_back(";SGS:;" + sgsName[0]);
			for (size_t i = 1; i < sgsName.size(); i++)
				setup.push_back(";" + sgsName[i]);
			return setup;
		}

	};
}
