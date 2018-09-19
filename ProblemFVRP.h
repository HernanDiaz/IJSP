#pragma once

/*
* Problem.h
*
*  Created on: Oct 27, 2017
*      Author: jjpalacios
*/
#pragma once

#include "Problem.h"
#include "FVRPException.h"
#include "CustomerFVRP.h"


namespace FVRP {


/* Indicates the type of time windows for the delivery
* If omited, time windows will not be consideredof time windows
*/
#define FVRP_PROBLEM_DUEDATES "problem.timewindow"

/* Indicates if the input file contains travel times.
* If not or omited, then the travel time will be considered equal to
* the distance
*/
#define FVRP_PROBLEM_TRAVELTIMES "problem.traveltime"

/*
* Indicates the type of data for the demand (int, real, tfn)
* Mandatory
*/
#define FVRP_PROBLEM_DEMAND "problem.demand"

/*
* Indicates the type of data for the service time (int, real, tfn)
* Mandatory
*/
#define FVRP_PROBLEM_SERVICETIME "problem.servicetime"





	//=========================================================================
	//
	//	Class ProblemFVRP
	//
	//=========================================================================
	/**
	* Objects of this class contain all data defining an instance of a Fuzzy
	* Vehicle Routing Problem with it's most common constraints.
	* The data is contained withing a file that must be provided to the class,
	* either at creation time or using a specific method. 
	* The files contain all the same format:
	*
	* To be defined
	*
	*
	* @author Juan Jose Palacios
	*
	*/

	class ProblemFVRP : public FuzzyFW::Problem {
		//=====================================================================
		//		FIELDS
		//=====================================================================
	protected:
		/**
		* Number of customers
		*/
		unsigned int nCustomers;

		/**
		* Fleet size
		*/
		unsigned int fleetSize;
		
		/**
		* Maximum capacity per vehicle
		*/
		double timeHorizon;

		/**
		* Maximum capacity per vehicle
		*/
		unsigned int capacity;

		/**
		* List of custoers to visit
		*/
		std::vector<CustomerFVRP *> customer;


		/**
		* Matrix with the travel distances between customers
		* Customer 0 is the depot
		*/
		std::vector< std::vector<FuzzyFW::TFN> > travelTime;

		/**
		* Matrix with the distance between customers
		* Customer 0 is the depot
		*/
		std::vector< std::vector<double> > distance;

		/**
		* Type of time windows contained in the file. Empty string if none
		*/
		std::string timeWindowType;

		/**
		* Type of travel times, if they are present
		*/
		std::string travelTimeType;

		/**
		* Type of demand
		*/
		std::string demandType;

		/**
		* Type of travel times, if they are present
		*/
		std::string serviceTimeType;





		//=====================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=====================================================================
	public:
		/**
		* Default constructor
		*/
		ProblemFVRP(const FuzzyFW::ParameterDB *params = NULL);


		/**
		* Sets the problem file, but does not load it
		*/
		ProblemFVRP(const char *inputFile);


		/**
		* Main constructor
		* Requires the file of parameters to check if there are bounds and/or
		* due-dates and therefore make a better reading of the input file.
		*/
		ProblemFVRP(const FuzzyFW::ParameterDB *params, const char *inputFile);


		/**
		* Loads the parameters describing the problem
		*/
		virtual void setup(const FuzzyFW::ParameterDB *params);


		/**
		* Copy constructor
		*/
		ProblemFVRP(const ProblemFVRP & source);


		/**
		* Clone operator for the case of inheritance
		*/
		virtual Problem* clone() const {
			return new ProblemFVRP(*this);
		}


		//=====================================================================
		//		DESTRUCTOR
		//=====================================================================
	public:
		~ProblemFVRP();



		//=====================================================================
		//		GET/SET METHODS
		//=====================================================================
	public:
		/**
		* Get the number of customers in the problem
		*/
		unsigned int getNumberCustomers() const {
			return this->nCustomers;
		}


		/**
		* Get the maximum capacity per vehicle
		*/
		unsigned int getMaxCapacity() const {
			return this->capacity;
		}

		/**
		* Get the size of the fleet
		*/
		unsigned int getFleetSize() const {
			return this->fleetSize;
		}

		/**
		* Get the time horizon
		*/
		double getTimeHorizon() const {
			return this->timeHorizon;
		}

		/**
		* Get the travel distance between two customers
		*/
		FuzzyFW::TFN getTravelTime(const unsigned int origin,
			const unsigned int destination) const;

		/**
		* Get distance
		*/
		double getDistance(const unsigned int origin,
			const unsigned int destination) const;

		/**
		* Get the service time of the customer
		*/
		FuzzyFW::TFN getServiceTime(const unsigned int customerId) const;


		/**
		* Indicates if time windows are present in the problem
		*/
		bool hasTimeWindows() const {
			return this->timeWindowType.length() > 0;
		}


		/**
		* Get the time window of a given customer.
		* Returns NULL if there is none
		*/
		const FuzzyFW::TimeWindow * getTimeWindow(
			const unsigned int customer) const;

		/**
		* Gets the demand of a customer
		*/
		FuzzyFW::TFN getDemand(const unsigned int customerId) const;

		/**
		* Gets the customer data
		*/
		CustomerFVRP * getCustomer(const unsigned int customerId) const;
		


		//=====================================================================
		//		METHODS
		//=====================================================================
	public:
		/**
		* Load all data from the stored file
		*/
		virtual void loadFile(const char *inputFile = NULL);


	protected:
		/**
		* Auxiliary method: Load the due-dates from the file
		* The file reader must be pointing at the due-date section
		*/
		FuzzyFW::TimeWindow * loadTimeWindow(std::ifstream &input);

		/**
		* Given a string, returns the name of the XML label as the first
		* string and it's value on the second string
		* Returns an empty string if there is no XML label
		*/
		std::pair<std::string, std::string> getXMLValue(std::string line);

		/**
		* Clears all the structures in the class. Used mainly for reset
		* purposes or re-read a file.
		*/
		virtual void clear();
	};
}
