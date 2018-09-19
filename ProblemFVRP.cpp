/*
* ProblemFVRP.cpp
*
*  Created on: Oct 27, 2017
*      Author: Juan Jose Palacios
*/

#include "ProblemFVRP.h"

namespace FVRP {


//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
ProblemFVRP::ProblemFVRP(const FuzzyFW::ParameterDB *params)
	: FuzzyFW::Problem(params) {

	// Initialize variables
	this->nCustomers = 0;
	this->capacity = 0;
	this->fleetSize = 0;
	this->timeHorizon = 0.0;
	this->timeWindowType = "";
	this->travelTimeType = "";
	this->demandType = "";
	this->serviceTimeType = "";

	// Set the depot
	this->customer.push_back(new CustomerFVRP());
	
	// Load parameters
	if (params != NULL)
		this->setup(params);
}



//====  Main constructor  =====================================================
ProblemFVRP::ProblemFVRP(const FuzzyFW::ParameterDB *params,
	const char *inputFile)
	: FuzzyFW::Problem(params, inputFile) {

	// Initialize variables
	this->nCustomers = 0;
	this->capacity = 0;
	this->fleetSize = 0;
	this->timeHorizon = 0.0;
	this->timeWindowType = "";
	this->travelTimeType = "";
	this->demandType = "";
	this->serviceTimeType = "";

	// Load parameters
	if (params != NULL)
		this->setup(params);

	this->loadFile(inputFile);
}



//====  Main constructor  =====================================================
ProblemFVRP::ProblemFVRP(const char *inputFile)
	: FuzzyFW::Problem(inputFile) {

	// Initialize variables
	this->nCustomers = 0;
	this->capacity = 0;
	this->fleetSize = 0;
	this->timeHorizon = 0.0;
	this->timeWindowType = "";
	this->travelTimeType = "";
	this->demandType = "";
	this->serviceTimeType = "";

	// Is the problem loaded
	this->isSetup = false;
}


//====  Setup method  =========================================================
void ProblemFVRP::setup(const FuzzyFW::ParameterDB *params) {
	Problem::setup(params);

	// Load time windows type
	this->timeWindowType = params->getStringLower(FVRP_PROBLEM_DUEDATES);
	if (this->timeWindowType.length() == 0) {
		std::cout << "Warning (Loading problem): Parameter \'";
		std::cout << FVRP_PROBLEM_DUEDATES << "\' not found. ";
		std::cout << "Time windows will NOT be considered";
	}

	// Load travel times type
	this->travelTimeType = params->getStringLower(FVRP_PROBLEM_TRAVELTIMES);
	if (this->travelTimeType.length() == 0) {
		std::cout << "Warning (Loading problem): Parameter \'";
		std::cout << FVRP_PROBLEM_TRAVELTIMES << "\' not found. ";
		std::cout << "Travel times are considered equal to the distance";
	}

	// Load demand type
	this->demandType = params->getStringLower(FVRP_PROBLEM_DEMAND);
	if (this->demandType.length() == 0) {
		std::string errorMsg = "Parameter \'";
		std::cout << FVRP_PROBLEM_DEMAND << "\' not found. ";
		throw new FVRPException("Problem loading", errorMsg);
	}


	// Load demand type
	this->serviceTimeType = params->getStringLower(FVRP_PROBLEM_SERVICETIME);
	if (this->demandType.length() == 0) {
		std::string errorMsg = "Parameter \'";
		std::cout << FVRP_PROBLEM_SERVICETIME << "\' not found. ";
		throw new FVRPException("Problem loading", errorMsg);
	}
}


//====  Copy constructor  =====================================================
ProblemFVRP::ProblemFVRP(const ProblemFVRP & source)
	: Problem(source) {
	this->nCustomers = source.nCustomers;
	this->capacity = source.capacity;
	this->timeHorizon = source.timeHorizon;
	this->fleetSize = source.fleetSize;

	this->travelTime.resize(source.travelTime.size());
	for (size_t i = 0; i < source.travelTime.size(); i++) {
		for (size_t j = 0; j < source.travelTime[i].size(); j++)
			this->travelTime[i].push_back(source.travelTime[i][j]);
	}

	this->customer.resize(source.customer.size());
	for (size_t i = 0; i < source.customer.size(); i++)
		this->customer.push_back(new CustomerFVRP(*source.customer[i]));
	this->timeWindowType = source.timeWindowType;
}



//=============================================================================
//		DESTRUCTOR
//=============================================================================
ProblemFVRP::~ProblemFVRP() {
	for (size_t t = 0; t < this->customer.size(); t++)
		delete this->customer[t];
}



//=============================================================================
//		GET/SET METHODS
//=============================================================================
//====  Get Travel Times  =====================================================
FuzzyFW::TFN ProblemFVRP::getTravelTime(const unsigned int origin,
	const unsigned int destination) const {

	if (origin > this->nCustomers || origin < 0) {
		std::string errorMsg = "Trying to access unexisting customer: ";
		errorMsg += valueToString(origin);
		throw new FVRPException("Problem", errorMsg);
	}
	if (destination > this->nCustomers || destination < 0) {
		std::string errorMsg = "Trying to access unexisting customer: ";
		errorMsg += valueToString(destination);
		throw new FVRPException("Problem", errorMsg);
	}

	return this->travelTime[origin][destination];
}


//====  Get Distance  =========================================================
double ProblemFVRP::getDistance(const unsigned int origin,
	const unsigned int destination) const {

	if (origin > this->nCustomers || origin < 0) {
		std::string errorMsg = "Trying to access unexisting customer: ";
		errorMsg += valueToString(origin);
		throw new FVRPException("Problem", errorMsg);
	}
	if (destination > this->nCustomers || destination < 0) {
		std::string errorMsg = "Trying to access unexisting customer: ";
		errorMsg += valueToString(destination);
		throw new FVRPException("Problem", errorMsg);
	}

	return this->distance[origin][destination];
}



//====  Get Service Time  =====================================================
FuzzyFW::TFN ProblemFVRP::getServiceTime(const unsigned int customerId)
const {
	if (customerId <= this->nCustomers && customerId >= 0) {
		return this->customer[customerId]->serviceTime;
	}
	else {
		std::string errorMsg = "Trying to access unexisting customer: ";
		errorMsg += valueToString(customerId);
		throw new FVRPException("Problem", errorMsg);
	}
}


//====  Get Time Window  ==========================================================
const FuzzyFW::TimeWindow * ProblemFVRP::getTimeWindow(
	const unsigned int customerId) const {
	if (customerId <= this->nCustomers && customerId >= 0) {
		if (!this->hasTimeWindows())
			return NULL;
		return this->customer[customerId]->timeWindow;
	}
	else {
		std::string errorMsg = "Trying to access unexisting customer: ";
		errorMsg += valueToString(customerId);
		throw new FVRPException("Problem", errorMsg);
	}
}


//====  Get Demand  ===========================================================
FuzzyFW::TFN ProblemFVRP::getDemand(const unsigned int customerId) const {

	if (customerId <= this->nCustomers && customerId >= 0) {
		return this->customer[customerId]->demand;
	}
	else {
		std::string errorMsg = "Trying to access unexisting customer: ";
		errorMsg += valueToString(customerId);
		throw new FVRPException("Problem", errorMsg);
	}
}


//====  Get Customer  =========================================================
CustomerFVRP * ProblemFVRP::getCustomer(const unsigned int customerId) const {

	if (customerId <= this->nCustomers && customerId >= 0) {
		return this->customer[customerId];
	}
	else {
		std::string errorMsg = "Trying to access unexisting customer: ";
		errorMsg += valueToString(customerId);
		throw new FVRPException("Problem", errorMsg);
	}
}





//=============================================================================
//		METHODS
//=============================================================================
//====  Load data from file  ==================================================
void ProblemFVRP::loadFile(const char *inputFile) {

	std::pair<std::string, std::string> xmlValues;
	std::string reader;
	std::stringstream lineStream;
	std::ifstream input;
	FuzzyFW::TFN tfnValue;
	double dValue;

	if (!this->isSetup) {
		std::string errorMsg;
		errorMsg = "The problem file cannot be read before the reading";
		errorMsg += " parameters are loaded.";
		throw new FVRPException("Problem", errorMsg);
	}

	if (inputFile != NULL)
		this->updatePath(inputFile);

	// Open the problem file
	input.open(this->problemPath);
	if (!input.is_open()) {
		std::string errorMsg;
		errorMsg = "The problem file \'" + std::string(this->problemPath);
		errorMsg += +"\' has not been found.";
		throw new FVRPException("Problem", errorMsg);
	}

	// Reset the data structures
	this->clear();
	// Allocates an extra position for the depot data
	this->customer.resize(this->nCustomers + 1);
	this->travelTime.resize(this->nCustomers + 1);
	this->distance.resize(this->nCustomers + 1);
	for(unsigned int i=0; i < this->nCustomers; i++) {
		this->travelTime.resize(this->nCustomers + 1);
		this->distance.resize(this->nCustomers +1);
	}


	// Read the XMLs values
	getline(input, reader);
	while (!input.eof()) {
		xmlValues = this->getXMLValue(reader);

		if (toUpper(xmlValues.first) == "DIMENSION") {
			this->nCustomers = atoi(xmlValues.second.c_str());
		}
		else if (toUpper(xmlValues.first) == "VEHICLE_CAPACITY") {
			this->capacity = atoi(xmlValues.second.c_str());
		}
		else if (toUpper(xmlValues.first) == "TIME_HORIZON") {
			this->timeHorizon = atof(xmlValues.second.c_str());
		}
		else if (toUpper(xmlValues.first) == "FLEET_SIZE") {
			this->fleetSize = atoi(xmlValues.second.c_str());
		}
		else if (toUpper(xmlValues.first) == "VERTICES") {

			// Order: Id, X, Y, TravelTime, Demand, TimeWindow, ServiceTime
			for (unsigned int i = 0; i <= this->nCustomers; i++) {
				// Load id
				input >> this->customer[i]->id;
				this->customer[i]->id = i;
				input >> this->customer[i]->x;
				input >> this->customer[i]->y;

				// Travel times not considered in this version
				
				// Load demand
				if (toUpper(this->demandType).compare("TFN") == 0)
					input >> this->customer[i]->demand;
				else {
					input >> dValue;
					this->customer[i]->demand = FuzzyFW::TFN(dValue, dValue, dValue);
				}

				// Load Time Window
				this->customer[i]->timeWindow = this->loadTimeWindow(input);

				// Load service time
				if (toUpper(this->demandType).compare("TFN") == 0)
					input >> this->customer[i]->serviceTime;
				else {
					input >> dValue;
					this->customer[i]->serviceTime = FuzzyFW::TFN(dValue, dValue, dValue);
				}
			}
		}
		getline(input, reader);
	}

	// Calculate distances and travel times between the customers
	for (unsigned int i = 0; i < this->nCustomers; i++) {
		for (unsigned int j = 0; j < this->nCustomers; j++) {
			dValue = (this->customer[i]->x - this->customer[j]->x)
				*(this->customer[i]->x - this->customer[j]->x);
			dValue += (this->customer[i]->y - this->customer[j]->y)
				*(this->customer[i]->y - this->customer[j]->y);
			this->distance[i][j] = std::sqrt(dValue);

			// Travel times not considered in this version.
			// They are assumed to be equal to the distance
			this->travelTime[i][j] = FuzzyFW::TFN(dValue, dValue, dValue);
		}
	}

	// Error control
	std::string errorMsg = "";
	if (this->nCustomers <= 0) {
		errorMsg += "Invalid number of customers or value not found. ";
	}
	if (this->capacity <= 0) {
		errorMsg += "Invalid vehicle capacity or value not found. ";
	}
	if (this->timeHorizon <= 0) {
		std::cout << "Warning! Time horizon not found in the problem data. ";
		std::cout << "Time horizon not used for his problem" << std::endl;
		this->timeHorizon = Infd;
	}
	if (this->fleetSize <= 0) {
		std::cout << "Warning! Fleet size not found in the problem data. ";
		std::cout << "Unlimited number of vehicles available." << std::endl;
		this->fleetSize = Infi;
	}
}



//====  Load time windows from file  ==========================================
FuzzyFW::TimeWindow * ProblemFVRP::loadTimeWindow(std::ifstream &input) {
	std::string reader;
	FuzzyFW::TimeWindow * tw;
	FuzzyFW::TimeWindowCrisp * twc;

	if (this->timeWindowType.length() > 0) {
		// Check due-date types
		tw = FuzzyFW::TimeWindowClassRegister::
			getDueDateObject(this->timeWindowType);
		if (tw == NULL) {
			std::string errorMsg = "The introduced type of Time Window is not";
			errorMsg += " valid: \'" + this->timeWindowType + "\'";
			throw new FVRPException("Loading problem", errorMsg);
		}

		if (tw->getType() == FuzzyFW::TimeWindow::CRISP) {
			twc = dynamic_cast<FuzzyFW::TimeWindowCrisp *>(tw);
			input >> twc->earlyTime >> twc->lateTime;
		}
		else
			input >> tw;
		return tw;
	}
	return NULL;
}



//====  Read XML label  =======================================================
std::pair<std::string, std::string> ProblemFVRP::getXMLValue(std::string line) {
	std::string value = "";
	std::string name = "";
	unsigned int position=0;

	while (line[position] != '<')
		position++;
	position++;
	while (line[position] != '>') {
		name += line[position];
		position++;
	}
	position++;
	while (line[position] != '<') {
		value += line[position];
		position++;
	}
	return std::pair<std::string, std::string>(name, value);
}


//====  Clear  ================================================================
void ProblemFVRP::clear() {
	this->nCustomers = this->capacity = 0;
	this->fleetSize = 0;

	for (size_t i = 0; i < travelTime.size(); i++)
		travelTime[i].clear();
	travelTime.clear();

	for (size_t t = 0; t < this->customer.size(); t++)
		delete this->customer[t];
	this->customer.clear();
	this->customer.push_back(new CustomerFVRP());
}


}
