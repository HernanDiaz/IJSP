/*
* NeighbourhoodFVRP_TW.cpp
*
*  Created on: Nov 27, 2017
*/

#include "NeighbourhoodFVRP_TW.h"

namespace FVRP {

//=============================================================================
//
//	Class NB_Vidal_N1_FVRP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
NB_Vidal_FVRP_TW_FD::NB_Vidal_FVRP_TW_FD(const NB_Vidal_FVRP_TW_FD & source)
	: Neighbourhood(source), estimator(source.estimator),
	estimatorLabel(source.estimatorLabel),
	minSeqSizeLabel(source.minSeqSizeLabel), minSeqSize(source.minSeqSize),
	maxSeqSizeLabel(source.maxSeqSizeLabel), maxSeqSize(source.maxSeqSize),
	metricDemandLabel(source.metricDemandLabel),
	metricDemand(source.metricDemand),
	thresholdDemandLabel(source.thresholdDemandLabel),
	demandThreshold(source.demandThreshold)
{
	for (size_t i = 0; i < source.neighbours.size(); i++) {
		if (source.neighbours[i] != NULL)
			neighbours.push_back(new NeighbourFVRP_SegSwap(*source.neighbours[i]));
		else
			neighbours.push_back(NULL);
	}

	durationMatrix.resize(source.durationMatrix.size());
	stockMatrix.resize(source.stockMatrix.size());
	distanceMatrix.resize(source.distanceMatrix.size());
	timeWarpMatrix.resize(source.timeWarpMatrix.size());
	earlyMatrix.resize(source.earlyMatrix.size());
	lateMatrix.resize(source.lateMatrix.size());

	for (size_t i = 0; i < durationMatrix.size(); i++)
		durationMatrix[i] = source.durationMatrix[i];
	for (size_t i = 0; i < stockMatrix.size(); i++)
		stockMatrix[i] = source.stockMatrix[i];
	for (size_t i = 0; i < distanceMatrix.size(); i++)
		distanceMatrix[i] = source.distanceMatrix[i];
	for (size_t i = 0; i < timeWarpMatrix.size(); i++)
		timeWarpMatrix[i] = source.timeWarpMatrix[i];
	for (size_t i = 0; i < earlyMatrix.size(); i++)
		earlyMatrix[i] = source.earlyMatrix[i];
	for (size_t i = 0; i < lateMatrix.size(); i++)
		lateMatrix[i] = source.lateMatrix[i];
}



//-----  Setup method  --------------------------------------------------------
void NB_Vidal_FVRP_TW_FD::setup(FuzzyFW::ParameterDB *parameters) {
	Neighbourhood::setup(parameters);

	std::string estimatorValue =
		parameters->getStringLower(this->estimatorLabel);

	if (estimatorValue.length() < 1 ||
		estimatorValue.compare(NB_FVRP_ESTIMATOR_NONE) == 0)
		this->estimator = Estimator::NONE;
	else if (estimatorValue.compare(NB_FVRP_ESTIMATOR_CONST) == 0)
		this->estimator = Estimator::ESTIM_CONST;
	else {
		std::string errorMsg = "Estimation method unknown: \'";
		errorMsg += estimatorValue + "\'";
		throw new FVRPException("Neighbourhood", errorMsg);
	}

	this->minSeqSize = parameters->getInteger(this->minSeqSizeLabel, -1);
	if (this->minSeqSize == -1) {
		std::cout << "Warning! Minimum sequence size not specified for the";
		std::cout << " neighbourhood. Value " << NB_FVRP_N1_MINSIZE_DEFAULT;
		std::cout << " will be used by default";
		this->minSeqSize = NB_FVRP_N1_MINSIZE_DEFAULT;
	}
	this->maxSeqSize = parameters->getInteger(this->maxSeqSizeLabel, -1);
	if (this->maxSeqSize == -1) {
		std::cout << "Warning! Minimum sequence size not specified for the";
		std::cout << " neighbourhood. Value " << NB_FVRP_N1_MAXSIZE_DEFAULT;
		std::cout << " will be used by default";
		this->minSeqSize = NB_FVRP_N1_MAXSIZE_DEFAULT;
	}

	// Load penalty value for time warps
	penalty = parameters->getDouble(this->penaltyLabel, -1.0);
	if (penalty < 0) {
		std::string errorMsg = this->penaltyLabel + " parameter not found.";
		throw new FVRPException("Evaluation", errorMsg);
	}

	// Load metric for feasibility on demand
	std::string value = parameters->getStringLower(this->metricDemandLabel);
	if (value == NB_FVRP_DEMAND_NEC)
		this->metricDemand = DemandType::FD_NECESSITY;
	else if (value == NB_FVRP_DEMAND_POS)
		this->metricDemand = DemandType::FD_POSSIBILITY;
	else if (value == NB_FVRP_DEMAND_CRED)
		this->metricDemand = DemandType::FD_CREDIBILITY;
	else if (value == NB_FVRP_DEMAND_EV)
		this->metricDemand = DemandType::FD_EV;
	else {
		std::string errorMsg = "Unkown value for parameter \'";
		errorMsg += this->metricDemandLabel + "\' or parameter not found.";
		throw new FVRPException("SGS", errorMsg);
	}

	this->demandThreshold = parameters->getDouble(this->thresholdDemandLabel, -1.0);
	if (this->demandThreshold < 0 && this->metricDemand != DemandType::FD_EV) {
		std::string errorMsg = "Parameter \'" + this->thresholdDemandLabel;
		errorMsg += "\' not found";
		throw new FVRPException("SGS", errorMsg);
	}
}



//-----  Destructor  ----------------------------------------------------------
NB_Vidal_FVRP_TW_FD::~NB_Vidal_FVRP_TW_FD() {
	for (size_t i = 0; i < neighbours.size(); i++) {
		if (neighbours[i] != NULL)
			delete neighbours[i];
	}
	neighbours.clear();
	for (size_t i = 0; i < durationMatrix.size(); i++)
		durationMatrix[i].clear();
	durationMatrix.clear();
	for (size_t i = 0; i < stockMatrix.size(); i++)
		stockMatrix[i].clear();
	stockMatrix.clear();
	for (size_t i = 0; i < distanceMatrix.size(); i++)
		distanceMatrix[i].clear();
	distanceMatrix.clear();
	for (size_t i = 0; i < timeWarpMatrix.size(); i++)
		timeWarpMatrix[i].clear();
	timeWarpMatrix.clear();
	for (size_t i = 0; i < earlyMatrix.size(); i++)
		earlyMatrix[i].clear();
	earlyMatrix.clear();
	for (size_t i = 0; i < lateMatrix.size(); i++)
		lateMatrix[i].clear();
	lateMatrix.clear();
}





//=============================================================================
//		METHODS
//=============================================================================
//-----  Set the initial solution to the problem  -----------------------------
void NB_Vidal_FVRP_TW_FD::setInitialSolution(FuzzyFW::Solution *solution,
	FuzzyFW::Fitness *fitness, const FuzzyFW::SharedVars *svars) {

	ProblemFVRP *problem;
	double delta, deltaTW, deltaWT;
	
	if (this->route != NULL)
		delete this->route;
	this->route = dynamic_cast<RouteFVRP *>(solution);
	if (this->route == NULL) {
		std::string errorMsg = "Type of solution not valid for this type";
		errorMsg += " of neighbourhood. Only Fuzzy VRP solutions are allowed.";
		throw new FVRPException("Neighbourhood", errorMsg);
	}

	if (this->currentFitness != NULL)
		delete this->currentFitness;
	this->currentFitness =
		dynamic_cast<FuzzyFW::FitnessDouble *>(fitness);
	if (this->currentFitness == NULL) {
		std::string errorMsg = "The fitness of the solution is not the ";
		errorMsg += "total cost";
		throw new FVRPException("Neighbourhood", errorMsg);
	}

	problem = dynamic_cast<ProblemFVRP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to fuzzy VRP problems";
		throw new FVRPException("Neighbourhood", errorMsg);
	}

	// CREATE THE SEGMENT MATRICES
	// Clean them if they have data
	for (size_t i = 0; i < this->durationMatrix.size(); i++)
		this->durationMatrix[i].clear();
	this->durationMatrix.clear();
	for (size_t i = 0; i < this->stockMatrix.size(); i++)
		this->stockMatrix[i].clear();
	this->stockMatrix.clear();
	for (size_t i = 0; i < this->distanceMatrix.size(); i++)
		this->distanceMatrix[i].clear();
	this->distanceMatrix.clear();
	for (size_t i = 0; i < this->timeWarpMatrix.size(); i++)
		this->timeWarpMatrix[i].clear();
	this->timeWarpMatrix.clear();
	for (size_t i = 0; i < this->earlyMatrix.size(); i++)
		this->earlyMatrix[i].clear();
	this->earlyMatrix.clear();
	for (size_t i = 0; i < this->lateMatrix.size(); i++)
		this->lateMatrix[i].clear();
	this->lateMatrix.clear();

	// Create the matrices
	this->durationMatrix.resize(problem->getNumberCustomers()+1);
	this->stockMatrix.resize(problem->getNumberCustomers()+1);
	this->distanceMatrix.resize(problem->getNumberCustomers()+1);
	this->timeWarpMatrix.resize(problem->getNumberCustomers() + 1);
	this->earlyMatrix.resize(problem->getNumberCustomers() + 1);
	this->lateMatrix.resize(problem->getNumberCustomers() + 1);
	this->segments.resize(problem->getNumberCustomers() + 1);

	for (unsigned int i = 0; i <= problem->getNumberCustomers(); i++) {
		this->durationMatrix[i].resize(problem->getNumberCustomers(), 0.0);
		this->stockMatrix[i].resize(problem->getNumberCustomers(),
			FuzzyFW::TFN(0, 0, 0));
		this->distanceMatrix[i].resize(problem->getNumberCustomers(), 0.0);
		this->timeWarpMatrix[i].resize(problem->getNumberCustomers(), 0.0);
		this->earlyMatrix[i].resize(problem->getNumberCustomers(), 0.0);
		this->lateMatrix[i].resize(problem->getNumberCustomers(),
			problem->getTimeHorizon());
		this->segments[i].resize(problem->getNumberCustomers(), -1);
	}
	

	// Fill the matrix
	unsigned int current, pred, counter;
	FuzzyFW::TFN duration, stock, distance;
	std::vector<unsigned int> visitedNodes;
	

	for (unsigned int i = 0; i < this->route->getNumberVehicles(); i++) {
		// Get last in the route
		current = this->route->getLastCustomer(i);
		counter = 0;

		// Compute the value to go back to depot and the diagonal
		this->durationMatrix[current][current] = 
			getCrispValue(problem->getServiceTime(current));
		this->stockMatrix[current][current] = problem->getDemand(current);
		this->earlyMatrix[current][current] = this->getEarlyValue(current);
		this->lateMatrix[current][current] = this->getLateValue(current);

		// Compute values from current, to the depot
		delta = this->durationMatrix[current][current]
			- this->timeWarpMatrix[current][current]
			+ getCrispValue(problem->getTravelTime(current, 0));
		deltaWT = std::max(this->earlyMatrix[0][0] - delta - this->lateMatrix[current][current], 0.0);
		deltaTW = std::max(this->earlyMatrix[current][current] + delta - this->lateMatrix[0][0], 0.0);

		this->durationMatrix[current][0] = this->durationMatrix[current][current]
			+ this->durationMatrix[0][0]
			+ getCrispValue(problem->getTravelTime(current, 0))
			+ deltaWT;
		this->stockMatrix[current][0] = this->stockMatrix[current][current]
			+ this->stockMatrix[0][0];
		this->distanceMatrix[current][0] = problem->getDistance(current, 0);
		this->segments[current][0] = current;
		this->segments[current][1] = 0;
		counter++;

		// Store the visited customers
		visitedNodes.clear();
		visitedNodes.push_back(0);
		visitedNodes.push_back(current);

		// Iterate over the predecessors
		pred = this->route->node[current].pred;
		while (current != 0) {
			// Update diagonal
			this->durationMatrix[pred][pred] =
				getCrispValue(problem->getServiceTime(pred));
			this->stockMatrix[pred][pred] = problem->getDemand(pred);
			this->earlyMatrix[pred][pred] = this->getEarlyValue(pred);
			this->lateMatrix[pred][pred] = this->getLateValue(pred);
			this->segments[pred][0] = pred;

			// Update segments from pred
			for (size_t j = visitedNodes.size() - 1; j >= 0; j++) {
				// Segment [pred] o [current, visitedNodes[j]]
				delta = this->durationMatrix[pred][pred]
					- this->timeWarpMatrix[pred][pred]
					+ getCrispValue(problem->getTravelTime(pred, current));
				deltaWT = std::max(this->earlyMatrix[current][visitedNodes[j]]
					- delta - this->lateMatrix[pred][pred], 0.0);
				deltaTW = std::max(this->earlyMatrix[pred][pred] + delta
					- this->lateMatrix[current][visitedNodes[j]], 0.0);

				this->durationMatrix[pred][visitedNodes[j]] = this->durationMatrix[pred][pred]
					+ this->durationMatrix[current][visitedNodes[j]]
					+ getCrispValue(problem->getTravelTime(pred, current))
					+ deltaWT;
				this->stockMatrix[pred][visitedNodes[j]] = this->stockMatrix[pred][pred]
					+ this->stockMatrix[current][visitedNodes[j]];
				this->distanceMatrix[pred][visitedNodes[j]] = this->distanceMatrix[pred][pred]
					+ problem->getDistance(pred, current)
					+ this->distanceMatrix[current][visitedNodes[j]];

				this->segments[pred][j + 1] = visitedNodes[j];
			}
			visitedNodes.push_back(current);
			current = pred;
			pred = this->route->node[current].pred;
		}
	}
}





//-----  Find enighbours  -----------------------------------------------------
unsigned int NB_Vidal_FVRP_TW_FD::findNewNeighbours(const FuzzyFW::SharedVars *svars) {

	unsigned int pos1, pos2;
	unsigned int start1, end1, start2, end2;
	double currentCost;

	currentCost = currentFitness->getValue();

	// For each found route
	for (unsigned int v1 = 0; v1 < this->route->getNumberVehicles(); v1++) {

		// Find the first element of the route
		start1 = this->route->getFirstCustomer(v1);
		pos1 = this->minSeqSize;

		while (start1 != 0 && this->segments[start1][pos1 - 1] != -1) {
			pos1 = this->minSeqSize;
			end1 = this->segments[start1][pos1 - 1];

			while (pos1 <= this->maxSeqSize && end1 > 0) {

				// Find all the segments to mate with
				for (unsigned int v2 = v1; v2 < this->route->getNumberVehicles(); v2++) {
					if (v1 == v2)
						start2 = this->route[v1].node[end1].succ;
					else
						start2 = this->route->getFirstCustomer(v2);
					while (start2 != 0) {
						pos2 = this->minSeqSize;
						if (pos2 < this->route->getRouteSize(v1))
							end2 = this->segments[start2][pos2 - 1];
						else
							end2 = 0;
						while (pos2 <= this->maxSeqSize && end2 != 0) {
							if (this->numNeighbours < this->neighbours.size()) {
								this->neighbours[this->numNeighbours]->setFirstSegment(v1, start1, end1);
								this->neighbours[this->numNeighbours]->setSecondSegment(v2, start2, end2);
							}
							else
								this->neighbours.push_back(
									new NeighbourFVRP_SegSwap(v1, start1, end1, v2, start2, end2));
						}
					}
				}

				pos1++;
				end1 = this->segments[start1][pos1 - 1];
			}

			start1 = this->route->node[start1].succ;
		}
	}

	return this->numNeighbours;
}




//-----  Fully evaluate the neighbour  ----------------------------------------
FuzzyFW::Fitness * NB_Vidal_FVRP_TW_FD::evaluateNeighbour(const unsigned int idx,
	const FuzzyFW::SharedVars *svars, const bool improvement) {

	RouteFVRP * newRoute;
	unsigned int pred, current;
	unsigned int firstNode1, firstNode2;
	char feasible = true;

	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw new FVRPException("Neighbourhood", errorMsg);
	}

	ProblemFVRP *problem = dynamic_cast<ProblemFVRP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to fuzzy VRP problems";
		throw new FVRPException("Neighbourhood", errorMsg);
	}

	NeighbourFVRP_SegSwap *neigh = this->neighbours[idx];
	if (neigh->initial1 < 0)
		return NULL;

	newRoute = dynamic_cast<RouteFVRP *>(this->route->clone());
	firstNode1 = this->route->getFirstCustomer(neigh->vehicle1);
	firstNode2 = this->route->getFirstCustomer(neigh->vehicle2);

	// Introduce the second sequence in place of the first
	pred = this->route->node[neigh->initial1].pred;
	current = neigh->initial2;

	while (current != 0) {
		if (pred != 0)
			this->route->node[pred].succ = current;
		else
			firstNode1 = current;
		this->route->node[current].pred = pred;
		this->route->node[current].arrivalTime =
			this->route->node[pred].arrivalTime
			+ problem->getServiceTime(pred)
			+ problem->getTravelTime(pred, current);
		this->route->node[current].vechicle = neigh->vehicle1;
		this->route->node[current].stockSpent =
			this->route->node[pred].stockSpent
			+ problem->getDemand(pred);
		if (compareDouble(this->getDemandMetric(this->route->node[current].stockSpent,
			problem->getMaxCapacity()),
			this->demandThreshold) < 0) {
			feasible = false;
			if (improvement) {
				this->neighbours[idx]->setEvaluation(newRoute,
					new FuzzyFW::FitnessDouble(Infd, false));

				return this->neighbours[idx]->getEvaluatedFitness();
			}
		}


		if (current != neigh->final2) {
			pred = current;
			current = this->route->node[current].succ;
		}
		else {
			pred = current;
			current = this->route->node[neigh->final1].succ;
		}
	}

	// Introduce the first sequence in place of the second
	pred = this->route->node[neigh->initial2].pred;
	current = neigh->initial1;

	while (current != 0) {
		if (pred != 0)
			this->route->node[pred].succ = current;
		else
			firstNode2 = current;
		this->route->node[current].pred = pred;
		this->route->node[current].arrivalTime =
			this->route->node[pred].arrivalTime
			+ problem->getServiceTime(pred)
			+ problem->getTravelTime(pred, current);
		this->route->node[current].vechicle = neigh->vehicle1;
		this->route->node[current].stockSpent =
			this->route->node[pred].stockSpent
			+ problem->getDemand(pred);

		if (current != neigh->final1) {
			pred = current;
			current = this->route->node[current].succ;
		}
		else {
			pred = current;
			current = this->route->node[neigh->final2].succ;
		}
	}

	// Update vehicle routes and time warps
	this->route->updateRoute(neigh->vehicle1, firstNode1);
	if (neigh->vehicle1 != neigh->vehicle2)
		this->route->updateRoute(neigh->vehicle2, firstNode2);

	// Calculate time warps and costs
	double cost = 0.0;
	double vt, late, tw;
	for (unsigned int v = 0; v < route->getNumberVehicles(); v++) {
		current = this->route->getFirstCustomer(v);
		tw = 0.0;
		while (current != 0) {
			vt = getCrispValue(this->route->node[current].arrivalTime) - tw;
			late = this->getLateValue(current);
			tw += std::max(vt - late, 0.0);
			current = this->route->node[current].succ;
		}
		current = this->route->getLastCustomer(v);
		vt = getCrispValue(this->route->node[current].arrivalTime)
			+ getCrispValue(this->route->node[current].cst->serviceTime)
			+ getCrispValue(problem->getTravelTime(current, 0))
			- tw;
		late = this->getLateValue(0);
		tw += std::max(vt - late, 0.0);

		cost = cost + getCrispValue(route->getRouteTime(v));
		cost += this->penalty*tw;
	}

	if (!feasible)
		this->neighbours[idx]->setEvaluation(newRoute,
			new FuzzyFW::FitnessDouble(Infd, false));
	else
		this->neighbours[idx]->setEvaluation(newRoute,
			new FuzzyFW::FitnessDouble(cost, false));

	return this->neighbours[idx]->getEvaluatedFitness();
}




//-----  Accept the neighbour  ------------------------------------------------
void NB_Vidal_FVRP_TW_FD::acceptNeighbour(const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {
	unsigned int v;

	// Update the current solution
	if (!this->neighbours[idx]->isEvaluated())
		this->evaluateNeighbour(idx, svars, false);
	if (this->route != NULL)
		delete this->route;
	if (this->currentFitness != NULL)
		delete this->currentFitness;
	this->route = dynamic_cast<RouteFVRP *>
		(this->neighbours[idx]->getEvaluation()->clone());
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessDouble *>
		(this->neighbours[idx]->getEvaluatedFitness()->clone());

	// Take the problem
	ProblemFVRP *problem = dynamic_cast<ProblemFVRP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to fuzzy VRP problems";
		throw new FVRPException("Neighbourhood", errorMsg);
	}

	NeighbourFVRP_SegSwap *neigh = this->neighbours[idx];

	// Fill the matrix
	double delta, deltaTW, deltaWT;
	unsigned int current, pred, counter;
	FuzzyFW::TFN duration, stock, distance;
	std::vector<unsigned int> visitedNodes;

	std::vector<unsigned int> vehicles;
	vehicles.push_back(neigh->vehicle1);
	if(neigh->vehicle1 != neigh->vehicle2)
		vehicles.push_back(neigh->vehicle2);

	for (size_t t = 0; t < vehicles.size(); t++) {
		v = vehicles[t];

		// Get last in the route
		// Get last in the route
		current = this->route->getLastCustomer(v);
		counter = 0;

		// Compute values from current, to the depot
		delta = this->durationMatrix[current][current]
			- this->timeWarpMatrix[current][current]
			+ getCrispValue(problem->getTravelTime(current, 0));
		deltaWT = std::max(this->earlyMatrix[0][0] - delta - this->lateMatrix[current][current], 0.0);
		deltaTW = std::max(this->earlyMatrix[current][current] + delta - this->lateMatrix[0][0], 0.0);

		this->durationMatrix[current][0] = this->durationMatrix[current][current]
			+ this->durationMatrix[0][0]
			+ getCrispValue(problem->getTravelTime(current, 0))
			+ deltaWT;
		this->stockMatrix[current][0] = this->stockMatrix[current][current]
			+ this->stockMatrix[0][0];
		this->distanceMatrix[current][0] = problem->getDistance(current, 0);
		this->segments[current][0] = current;
		this->segments[current][1] = 0;
		counter++;

		// Store the visited customers
		visitedNodes.clear();
		visitedNodes.push_back(0);
		visitedNodes.push_back(current);

		// Iterate over the predecessors
		pred = this->route->node[current].pred;
		while (current != 0) {
			// Update segments from pred
			for (size_t j = visitedNodes.size() - 1; j >= 0; j++) {
				// Segment [pred] o [current, visitedNodes[j]]
				delta = this->durationMatrix[pred][pred]
					- this->timeWarpMatrix[pred][pred]
					+ getCrispValue(problem->getTravelTime(pred, current));
				deltaWT = std::max(this->earlyMatrix[current][visitedNodes[j]]
					- delta - this->lateMatrix[pred][pred], 0.0);
				deltaTW = std::max(this->earlyMatrix[pred][pred] + delta
					- this->lateMatrix[current][visitedNodes[j]], 0.0);

				this->durationMatrix[pred][visitedNodes[j]] = this->durationMatrix[pred][pred]
					+ this->durationMatrix[current][visitedNodes[j]]
					+ getCrispValue(problem->getTravelTime(pred, current))
					+ deltaWT;
				this->stockMatrix[pred][visitedNodes[j]] = this->stockMatrix[pred][pred]
					+ this->stockMatrix[current][visitedNodes[j]];
				this->distanceMatrix[pred][visitedNodes[j]] = this->distanceMatrix[pred][pred]
					+ problem->getDistance(pred, current)
					+ this->distanceMatrix[current][visitedNodes[j]];

				this->segments[pred][j + 1] = visitedNodes[j];
			}
			visitedNodes.push_back(current);
			current = pred;
			pred = this->route->node[current].pred;
		}
	}

}




//-----  Get the estimation  --------------------------------------------------
FuzzyFW::Fitness * NB_Vidal_FVRP_TW_FD::getEstimation(const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {

	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FVRPException("Neighbourhood", errorMsg);
	}

	ProblemFVRP *problem = dynamic_cast<ProblemFVRP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to fuzzy VRP problems";
		throw new FVRPException("Neighbourhood", errorMsg);
	}

	NeighbourFVRP_SegSwap *neigh = this->neighbours[idx];
	if (neigh->initial1 < 0)
		return NULL;


	unsigned int start, end;
	double cost;

	if (neigh->vehicle1 == neigh->vehicle2) {
		// Segment 1:
		start = 0;
		end = this->route->node[neigh->initial1].pred;
		cost = this->durationMatrix[start][end];

		// Segment 2:
		start = neigh->initial2;
		cost = cost + this->durationMatrix[start][neigh->final2];
		cost = cost + this->getCrispValue(problem->getTravelTime(end, start));
		end = neigh->final2;

		// Segment 3:
		start = this->route->node[neigh->final1].succ;
		cost = cost + this->getCrispValue(problem->getTravelTime(end, start));
		end = this->route->node[neigh->initial2].pred;
		cost = cost + this->durationMatrix[start][end];

		// Segment 4:
		start = neigh->initial1;
		cost = cost + this->durationMatrix[start][neigh->final1];
		cost = cost + this->getCrispValue(problem->getTravelTime(end, start));
		end = neigh->final1;

		// Segment 5:
		start = this->route->node[neigh->final2].succ;
		cost = cost + this->getCrispValue(problem->getTravelTime(end, start));
		cost = cost + this->durationMatrix[start][0];
	}

	else {
		// Segment 1:
		start = 0;
		end = this->route->node[neigh->initial1].pred;
		cost = this->durationMatrix[start][end];

		// Segment 2:
		start = neigh->initial2;
		cost = cost + this->durationMatrix[start][neigh->final2];
		cost = cost + this->getCrispValue(problem->getTravelTime(end, start));
		end = neigh->final2;

		// Segment 3:
		start = this->route->node[neigh->final1].succ;
		cost = cost + this->getCrispValue(problem->getTravelTime(end, start));
		cost = cost + this->durationMatrix[start][0];

		// Segment 4:
		start = 0;
		end = this->route->node[neigh->initial2].pred;
		cost = cost + this->durationMatrix[start][end];

		// Segment 5:
		start = neigh->initial1;
		cost = cost + this->durationMatrix[start][neigh->final1];
		cost = cost + this->getCrispValue(problem->getTravelTime(end, start));
		end = neigh->final1;

		// Segment 6:
		start = this->route->node[neigh->final2].succ;
		cost = cost + this->getCrispValue(problem->getTravelTime(end, start));
		cost = cost + this->durationMatrix[start][0];
	}

	for (unsigned int i = 0; i < this->route->getNumberVehicles(); i++) {
		if (i != neigh->vehicle1 && i != neigh->vehicle2)
			cost = cost + this->getCrispValue(this->route->getRouteTime(i));
	}

	this->neighbours[idx]->setEstimatedQuality(new FuzzyFW::FitnessDouble(cost));
	return this->neighbours[idx]->getEstimatedQuality();
}



//-----  Discard a neighbour  --------------------------------------------------
void NB_Vidal_FVRP_TW_FD::discardNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FVRPException("Neighbourhood", errorMsg);
	}
	delete this->neighbours[idx];
	this->neighbours[idx] = NULL;
	//std::swap(this->neighbours[idx], this->neighbours[this->numNeighbours - 1]);
	//this->numNeighbours--;
}


//-----  Gets an specific neighbour  ------------------------------------------
FuzzyFW::Neighbour* NB_Vidal_FVRP_TW_FD::getNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FVRPException("Neighbourhood", errorMsg);
	}
	return this->neighbours[idx];
}


//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_Vidal_FVRP_TW_FD::sortByEstimation(
	const FuzzyFW::SharedVars *svars) {

	if (this->estimator == Estimator::NONE)
		return;
	this->quickSort(0, this->numNeighbours - 1, svars->rng);
}


//-----  Quick sort  ----------------------------------------------------------
void NB_Vidal_FVRP_TW_FD::quickSort(const int left, const int right,
	FuzzyFW::Random *rng) {

	int pivot, pos;

	if (left >= right)
		return;

	pivot = rng->getInteger(left, right);

	std::swap(this->neighbours[pivot], this->neighbours[right]);
	pos = left;
	for (int i = left; i < right; i++) {
		if (this->neighbours[i]->getEstimatedQuality()->isBetterThan(
			this->neighbours[right]->getEstimatedQuality())) {
			std::swap(this->neighbours[i], this->neighbours[pos]);
			pos++;
		}
	}

	std::swap(neighbours[pos], neighbours[right]);
	this->quickSort(left, pos - 1, rng);
	this->quickSort(pos + 1, right, rng);
}



//-----  Get the early value  -------------------------------------------------
double NB_Vidal_FVRP_TW_FD::getEarlyValue(const unsigned int customerId) {
	const FuzzyFW::TimeWindowCrisp *twc;

	twc = dynamic_cast<const FuzzyFW::TimeWindowCrisp *>
		(this->route->node[customerId].cst->timeWindow);
	if (twc == NULL) {
		std::string errorMsg = "Time Wndows must be crisp to use this";
		errorMsg += " neighbourhood";
		throw new FVRPException("Neighbourhood", errorMsg);
	}

	return twc->earlyTime;
}


//-----  Get the late value  --------------------------------------------------
double NB_Vidal_FVRP_TW_FD::getLateValue(const unsigned int customerId) {
	const FuzzyFW::TimeWindowCrisp *twc;

	twc = dynamic_cast<const FuzzyFW::TimeWindowCrisp *>
		(this->route->node[customerId].cst->timeWindow);
	if (twc == NULL) {
		std::string errorMsg = "Time Wndows must be crisp to use this";
		errorMsg += " neighbourhood";
		throw new FVRPException("Neighbourhood", errorMsg);
	}

	return twc->lateTime;
}


//=====  Get the demand metric  ===============================================
double NB_Vidal_FVRP_TW_FD::getDemandMetric(FuzzyFW::TFN demand, double capacity) {
	if (this->metricDemand == DemandType::FD_NECESSITY)
		return demand.necessity(capacity);
	if (this->metricDemand == DemandType::FD_POSSIBILITY)
		return demand.possibility(capacity);
	if (this->metricDemand == DemandType::FD_CREDIBILITY)
		return demand.credibility(capacity);
	if (this->metricDemand == DemandType::FD_EV)
		return compareDouble(demand.expectedValue(), capacity) <= 0;
	return -1.0;
}


}
