/*
* QuantumInspiredEA.cpp
*
*  Created on: June 6, 2026
*/

#include "QuantumInspiredEA.h"
#include "ProblemIJSP.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class QuantumInspiredEA
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
QuantumInspiredEA::QuantumInspiredEA(ParameterDB *params)
	: EvolutiveAlgorithm(params) {

	GeneticClassRegister::registerClasses();

	this->samples = 0;
	this->samplesStart = -1;
	this->samplesEnd = -1;
	this->deltaTheta = 0.0;
	this->rotStart = -1.0;
	this->rotEnd = -1.0;
	this->floorProb = 0.0;
	this->floorStart = -1.0;
	this->floorEnd = -1.0;
	this->cosineSchedule = false;
	this->tauStart = 1.0;
	this->tauEnd = 1.0;
	this->usePrecedence = false;

	this->maxGenerations = Infi;
	this->maxPlateau = Infi;
	this->maxEvaluations = Infi;
	this->maxRuntime = Infi;

	this->showEvolutionTime = false;
	this->evolutionSpan = 1.0;

	this->generation = 0;
	this->iterationsNI = 0;
	this->evaluations = 0;
	this->bestSoFar = NULL;

	this->nJobs = 0;
	this->genotypeLength = 0;

	this->totalRuntime = 0;
	this->observationTime = 0;
	this->evaluationTime = 0;
	this->rotationTime = 0;
	this->nextSplit = 0.0;
}


//-----  Destructor  ----------------------------------------------------------
QuantumInspiredEA::~QuantumInspiredEA() {
	delete this->bestSoFar;
}


//-----  clearAll  ------------------------------------------------------------
void QuantumInspiredEA::clearAll() {
	delete this->bestSoFar;
	this->bestSoFar = NULL;
	this->evolutionStats.clear();
	this->amplitude.clear();
	this->pref.clear();
	this->tasksPerJob.clear();
	EvolutiveAlgorithm::clearAll();

	this->maxGenerations = Infi;
	this->maxEvaluations = Infi;
	this->maxRuntime = Infi;
	this->maxPlateau = Infi;
	this->samples = 0;
	this->samplesStart = -1;
	this->samplesEnd = -1;
	this->deltaTheta = 0.0;
	this->rotStart = -1.0;
	this->rotEnd = -1.0;
	this->floorProb = 0.0;
	this->floorStart = -1.0;
	this->floorEnd = -1.0;
	this->cosineSchedule = false;
	this->tauStart = 1.0;
	this->tauEnd = 1.0;
}



//=============================================================================
//		GET/SET METHODS
//=============================================================================
//-----  printSetupTree  ------------------------------------------------------
void QuantumInspiredEA::printSetupTree(std::ofstream & output) const {
	output << "Quantum-Inspired Evolutionary Algorithm" << std::endl;

	output << "Problem to solve:;"
		<< this->sharedVariables->problem->getName() << std::endl;

	std::vector<std::string> names;
	names = this->evaluator->getName();
	output << ";Objective Function:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	output << ";Samples per generation:;" << this->samples;
	if (this->samplesStart != this->samplesEnd)
		output << "  (schedule: " << this->samplesStart
			<< " -> " << this->samplesEnd << ")";
	output << std::endl;
	output << ";Rotation step:;" << valueToString(this->deltaTheta);
	if (compareDouble(this->rotStart, this->rotEnd) != 0)
		output << "  (schedule: " << valueToString(this->rotStart)
			<< " -> " << valueToString(this->rotEnd) << ")";
	output << std::endl;
	output << ";Floor probability:;" << valueToString(this->floorProb);
	if (compareDouble(this->floorStart, this->floorEnd) != 0)
		output << "  (schedule: " << valueToString(this->floorStart)
			<< " -> " << valueToString(this->floorEnd) << ")";
	output << std::endl;
	output << ";Model scheme:;"
		<< (this->usePrecedence ? "Precedence" : "Positional") << std::endl;
	output << ";Schedule type:;"
		<< (this->cosineSchedule ? "Cosine annealing" : "Linear") << std::endl;
	output << ";Sampling tau:;";
	if (compareDouble(this->tauStart, 1.0) == 0
		&& compareDouble(this->tauEnd, 1.0) == 0)
		output << "1.0 (raw probabilities)" << std::endl;
	else
		output << valueToString(this->tauStart) << " -> "
			<< valueToString(this->tauEnd) << std::endl;

	output << ";Stopping criteria:" << std::endl;
	output << ";;Max.Generations:;";
	if (this->maxGenerations < 0) output << "none" << std::endl;
	else output << this->maxGenerations << std::endl;

	output << ";;Max.Generations without improvement:;";
	if (this->maxPlateau < 0) output << "none" << std::endl;
	else output << this->maxPlateau << std::endl;

	output << ";;Max.Evaluations:;";
	if (this->maxEvaluations < 0) output << "none" << std::endl;
	else output << this->maxEvaluations << std::endl;

	output << ";;Time Limit:;";
	if (this->maxRuntime < 0) output << "no" << std::endl;
	else output << this->maxRuntime << std::endl;

	names = this->sharedVariables->encoder->getName();
	output << ";Codification:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->sharedVariables->decoder->getName();
	output << ";Decodification:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;
}


//-----  getStatistics  -------------------------------------------------------
std::vector< std::pair<std::string, double> > QuantumInspiredEA::getStatistics()
	const {
	std::vector< std::pair<std::string, double> > stats;
	stats.push_back(std::pair<std::string, double>
		("Number of Generations", this->generation));
	stats.push_back(std::pair<std::string, double>
		("Theoretical Evaluations", (double)this->evaluations));
	stats.push_back(std::pair<std::string, double>
		("Best solution", this->bestSoFar->getFitness()->toDouble()));
	return stats;
}


//-----  getRuntime  ----------------------------------------------------------
std::vector< std::pair<std::string, double> > QuantumInspiredEA::getRuntime()
	const {
	std::vector< std::pair<std::string, double> > times;
	double percentage, totalPerc = 0.0;

	times.push_back(std::pair<std::string, double>
		("Total runtime", (1.0*totalRuntime) / CLOCKS_PER_SEC));

	percentage = (double)observationTime / totalRuntime;
	times.push_back(std::pair<std::string, double>("Observation", 100 * percentage));
	totalPerc += percentage;

	percentage = (double)evaluationTime / totalRuntime;
	times.push_back(std::pair<std::string, double>("Evaluation", 100 * percentage));
	totalPerc += percentage;

	percentage = (double)rotationTime / totalRuntime;
	times.push_back(std::pair<std::string, double>("Rotation", 100 * percentage));
	totalPerc += percentage;

	times.push_back(std::pair<std::string, double>
		("Others", 100 - 100 * totalPerc));
	return times;
}


//-----  getEvolution  --------------------------------------------------------
std::vector< std::vector<double> > QuantumInspiredEA::getEvolution(
	std::vector< std::string > &labels) const {
	labels.clear();
	labels.push_back("Generation");
	labels.push_back("Runtime");
	labels.push_back("Best");
	labels.push_back("Avg");
	for (size_t i = 0; i < this->statistics.size(); i++)
		labels.push_back(this->statistics[i]->getName());
	return this->evolutionStats;
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  prepareToRun  --------------------------------------------------------
void QuantumInspiredEA::prepareToRun(ParameterDB *params) {
	// Loads the common parameters (evaluator + statistics)
	EvolutiveAlgorithm::prepareToRun(params);

	ParameterDB *p = this->sharedVariables->parameters;
	std::string value;

	// Encoding / decoding strategies (reuse the genetic register, untouched)
	value = p->getStringLower(QEA_ENCODING);
	this->sharedVariables->encoder = GeneticClassRegister::getEncoderObject(value);

	value = p->getStringLower(QEA_DECODING);
	this->sharedVariables->decoder = GeneticClassRegister::getDecoderObject(value);

	// QEA-specific parameters
	this->samples = p->getInteger(QEA_SAMPLES, -1);
	// Optional samples schedule (defaults to the constant samples)
	this->samplesStart = p->getInteger(QEA_SAMP_START, (int)this->samples);
	this->samplesEnd = p->getInteger(QEA_SAMP_END, (int)this->samples);
	this->deltaTheta = p->getDouble(QEA_ROTATION, -1.0);
	// Optional rotation schedule (defaults to the constant deltaTheta)
	this->rotStart = p->getDouble(QEA_ROT_START, this->deltaTheta);
	this->rotEnd = p->getDouble(QEA_ROT_END, this->deltaTheta);
	this->floorProb = p->getDouble(QEA_FLOOR, 0.0);
	// Optional floor schedule (defaults to the constant floorProb)
	this->floorStart = p->getDouble(QEA_FLOOR_START, this->floorProb);
	this->floorEnd = p->getDouble(QEA_FLOOR_END, this->floorProb);
	this->cosineSchedule =
		(p->getStringLower(QEA_SCHED_TYPE).compare("cosine") == 0);
	this->tauStart = p->getDouble(QEA_TAU_START, 1.0);
	this->tauEnd = p->getDouble(QEA_TAU_END, 1.0);
	if (this->tauStart <= 0.0) this->tauStart = 1.0;
	if (this->tauEnd <= 0.0) this->tauEnd = 1.0;
	this->usePrecedence =
		(p->getStringLower(QEA_SCHEME).compare("precedence") == 0);

	// Stopping criteria (reused names)
	this->maxGenerations = p->getInteger(QEA_GENERATIONS, -1);
	this->maxPlateau = p->getInteger(QEA_NOIMPROVE, -1);
	this->maxEvaluations = p->getInteger(QEA_EVALUATIONS, -1);
	this->maxRuntime = p->getDouble(QEA_TIME, -1.0);

	// Evolution reporting unit
	value = p->getStringLower(QEA_EVOL_METRIC);
	if (value.compare(QEA_EVOL_UNIT_GEN) == 0)
		this->showEvolutionTime = false;
	else if (value.compare(QEA_EVOL_UNIT_TIME) == 0)
		this->showEvolutionTime = true;
	else
		this->showEvolutionTime = false;
	this->evolutionSpan = p->getDouble(QEA_EVOL_SPAN, 1);

	this->checkSetup();

	this->sharedVariables->encoder->setup(p);
	this->sharedVariables->decoder->setup(p);
}


//-----  checkSetup  ----------------------------------------------------------
bool QuantumInspiredEA::checkSetup() {
	bool correct = true;
	std::string err = "";

	if (this->sharedVariables->encoder == NULL) {
		err = "Invalid codification method.";
		correct = false;
	}
	if (this->sharedVariables->decoder == NULL) {
		err = "Invalid decodification method.";
		correct = false;
	}
	if ((int)this->samples <= 0) {
		err = "Invalid number of samples (qea.samples).";
		correct = false;
	}
	if (compareDouble(this->deltaTheta, 0.0) <= 0) {
		err = "Invalid rotation step (qea.rotation).";
		correct = false;
	}
	if (compareDouble(this->floorProb, 0.0) < 0
		|| compareDouble(this->floorProb, 1.0) > 0) {
		err = "Invalid floor probability (qea.floor).";
		correct = false;
	}
	if (this->maxGenerations < 0 && this->maxEvaluations < 0
		&& this->maxRuntime < 0 && this->maxPlateau < 0) {
		err = "No stopping criteria defined.";
		correct = false;
	}

	if (!correct) {
		err += " Incorrect value or missing parameter";
		throw FuzzyFWException("Quantum-Inspired EA", err);
	}
	return true;
}


//-----  RUN !!  --------------------------------------------------------------
//=============================================================================
std::pair<Solution *, Objective *> QuantumInspiredEA::run(Problem *problem,
	std::string signature, std::string logFolder, int rngSeed) {

	// Initialize dynamic variables
	this->observationTime = 0;
	this->evaluationTime = 0;
	this->rotationTime = 0;
	this->iterationsNI = 0;
	this->evolutionStats.clear();
	this->generation = 0;
	this->evaluations = 0;
	this->nextSplit = 0.0;
	this->finished = false;

	if (this->bestSoFar != NULL) {
		delete this->bestSoFar;
		this->bestSoFar = NULL;
	}

	// Initialize the RNG and set the problem to solve
	this->sharedVariables->rng->init(rngSeed);
	this->sharedVariables->problem = problem;

	// Cache problem dimensions
	IJSP::ProblemIJSP *prob = dynamic_cast<IJSP::ProblemIJSP *>(problem);
	if (prob == NULL)
		throw FuzzyFWException("Quantum-Inspired EA",
			"The QEA currently supports only Interval Job Shop problems.");

	this->nJobs = (int)prob->getNumberJobs();
	this->genotypeLength = (int)prob->getNumberTasks();
	this->tasksPerJob.resize(this->nJobs);
	for (int j = 0; j < this->nJobs; j++)
		this->tasksPerJob[j] = (int)prob->getNumberTasks((unsigned int)j);

	// Initialize the probability model according to the chosen scheme
	if (this->usePrecedence) {
		// pref[a][b] = 0.5 everywhere (no precedence bias)
		this->pref.assign(this->nJobs, std::vector<double>(this->nJobs, 0.5));
		this->amplitude.clear();
	}
	else {
		// positional amplitudes uniform: amplitude^2 = 1/nJobs
		double initAmp = 1.0 / std::sqrt((double)this->nJobs);
		this->amplitude.assign(this->genotypeLength,
			std::vector<double>(this->nJobs, initAmp));
		this->pref.clear();
	}

	clock_t timePoint;
	clock_t startTime = clock();
	this->totalRuntime = 0;

	// Main loop  ---------------------------------------------------------
	while (true) {
		this->totalRuntime = clock() - startTime;
		if (this->stop())
			break;

		Population *currentPopulation = new Population();

		// ----- Observe K samples (K may vary along the schedule) -----
		int K = this->currentSamples();
		timePoint = clock();
		for (int k = 0; k < K; k++) {
			std::vector<int> genotype = this->observe();
			currentPopulation->addIndividual(new IndividualArrayInt(genotype));
		}
		this->observationTime += clock() - timePoint;

		// ----- Evaluate (decodes + interval fitness) -----
		timePoint = clock();
		this->evaluator->evaluatePopulation(this->sharedVariables,
			currentPopulation);
		this->evaluations += (long int)K;
		this->evaluationTime += clock() - timePoint;

		// ----- Update best so far -----
		Individual *genBest =
			currentPopulation->getBest(this->sharedVariables);
		if (this->bestSoFar == NULL
			|| genBest->getFitness()->isBetterThan(
				this->bestSoFar->getFitness())) {
			if (this->bestSoFar != NULL)
				delete this->bestSoFar;
			this->bestSoFar = genBest->clone();
			this->iterationsNI = 0;
		}
		else
			this->iterationsNI++;

		// ----- Rotate the distribution towards the best -----
		timePoint = clock();
		IndividualArrayInt *bi =
			dynamic_cast<IndividualArrayInt *>(this->bestSoFar);
		this->rotateTowards(bi->getGenotype());
		this->applyFloor();
		this->rotationTime += clock() - timePoint;

		this->generation++;

		// ----- Statistics -----
		this->totalRuntime = clock() - startTime;
		this->computeStatistics(currentPopulation);

		delete currentPopulation;
	}

	this->totalRuntime = clock() - startTime;
	this->finished = true;

	std::pair<Solution *, Objective *> returnValue;
	returnValue.first = this->bestSoFar->getPhenotype()->clone();
	returnValue.second = this->bestSoFar->getFitness()->clone();
	return returnValue;
}


//-----  stop  ----------------------------------------------------------------
bool QuantumInspiredEA::stop() {
	if (this->maxGenerations >= 0
		&& (int)this->generation >= this->maxGenerations)
		return true;
	if (this->maxEvaluations >= 0
		&& this->evaluations >= this->maxEvaluations)
		return true;
	if (this->maxPlateau >= 0
		&& (int)this->iterationsNI >= this->maxPlateau)
		return true;

	double currentRuntime = this->totalRuntime / (double)CLOCKS_PER_SEC;
	if (this->maxRuntime >= 0) {
		if (currentRuntime >= this->maxRuntime)
			return true;
		if (this->generation > 0) {
			double runtimePerGen = currentRuntime / this->generation;
			if (currentRuntime + runtimePerGen / 2 >= this->maxRuntime)
				return true;
		}
	}
	return false;
}


//-----  computeStatistics  ---------------------------------------------------
void QuantumInspiredEA::computeStatistics(Population *currentPopulation) {
	std::vector<double> stats;
	double runtime = this->totalRuntime / (double)CLOCKS_PER_SEC;

	if (!this->showEvolutionTime && this->generation < this->nextSplit)
		return;
	if (this->showEvolutionTime && runtime < this->nextSplit)
		return;

	stats.push_back(this->generation);
	stats.push_back(runtime);
	stats.push_back(this->bestSoFar->getFitness()->toDouble());
	stats.push_back(currentPopulation->getAverageFitness());

	for (size_t i = 0; i < this->statistics.size(); i++)
		stats.push_back(this->statistics[i]->
			getValue(this->sharedVariables, currentPopulation));
	this->evolutionStats.push_back(stats);

	if (this->showEvolutionTime) {
		while (runtime >= this->nextSplit)
			this->nextSplit += this->evolutionSpan;
	}
	else
		this->nextSplit += this->evolutionSpan;
}



//=============================================================================
//		QEA-SPECIFIC OPERATORS
//=============================================================================
//-----  observePositional  ---------------------------------------------------
std::vector<int> QuantumInspiredEA::observePositional() {
	std::vector<int> remaining = this->tasksPerJob;
	std::vector<int> perm;
	perm.reserve(this->genotypeLength);

	// Sampling temperature: P[j] ~ amplitude[p][j]^(2 * tau).
	// tau = 1.0 -> raw probability (baseline); tau > 1 sharpens, tau < 1 flattens.
	double tau = this->currentTau();
	bool useTau = (compareDouble(tau, 1.0) != 0);

	for (int p = 0; p < this->genotypeLength; p++) {
		// Compute weights of jobs still available at this position.
		// Weight = amplitude^2 when tau=1; weight = amplitude^(2*tau) otherwise.
		double total = 0.0;
		for (int j = 0; j < this->nJobs; j++) {
			if (remaining[j] > 0) {
				double a2 = this->amplitude[p][j] * this->amplitude[p][j];
				total += useTau ? std::pow(a2, tau) : a2;
			}
		}

		// Roulette-wheel selection restricted to available jobs
		double u = this->sharedVariables->rng->getProbability() * total;
		double acc = 0.0;
		int chosen = -1;
		for (int j = 0; j < this->nJobs; j++) {
			if (remaining[j] > 0) {
				double a2 = this->amplitude[p][j] * this->amplitude[p][j];
				acc += useTau ? std::pow(a2, tau) : a2;
				if (u <= acc) {
					chosen = j;
					break;
				}
			}
		}
		// Fallback for floating-point rounding: last available job
		if (chosen < 0) {
			for (int j = this->nJobs - 1; j >= 0; j--)
				if (remaining[j] > 0) {
					chosen = j;
					break;
				}
		}

		perm.push_back(chosen);
		remaining[chosen]--;
	}
	return perm;
}


//-----  rotatePositional  ----------------------------------------------------
void QuantumInspiredEA::rotatePositional(const std::vector<int> &bestGenotype,
	double step) {
	for (int p = 0; p < this->genotypeLength; p++) {
		int jstar = bestGenotype[p];
		double c = this->amplitude[p][jstar];
		double r2 = 1.0 - c * c;
		if (r2 < 1e-12)
			continue;					// already collapsed onto jstar
		double r = std::sqrt(r2);
		double cNew = c * std::cos(step) + r * std::sin(step);
		if (cNew > 1.0)
			cNew = 1.0;
		double rNew = std::sqrt(1.0 - cNew * cNew);
		double scale = rNew / r;
		this->amplitude[p][jstar] = cNew;
		for (int j = 0; j < this->nJobs; j++)
			if (j != jstar)
				this->amplitude[p][j] *= scale;
	}
}


//-----  applyFloorPositional  ------------------------------------------------
void QuantumInspiredEA::applyFloorPositional() {
	double f = this->currentFloor();
	if (compareDouble(f, 0.0) <= 0)
		return;
	double minAmp = std::sqrt(f);
	for (int p = 0; p < this->genotypeLength; p++) {
		bool changed = false;
		for (int j = 0; j < this->nJobs; j++)
			if (this->amplitude[p][j] < minAmp) {
				this->amplitude[p][j] = minAmp;
				changed = true;
			}
		if (changed) {
			double norm = 0.0;
			for (int j = 0; j < this->nJobs; j++)
				norm += this->amplitude[p][j] * this->amplitude[p][j];
			norm = std::sqrt(norm);
			for (int j = 0; j < this->nJobs; j++)
				this->amplitude[p][j] /= norm;
		}
	}
}



//=============================================================================
//		SCHEME DISPATCHERS
//=============================================================================
std::vector<int> QuantumInspiredEA::observe() {
	return this->usePrecedence ? this->observePrecedence()
		: this->observePositional();
}

// Schedule interpolant. t in [0,1] is the linear progress; this returns the
// fraction in [0,1] that we travel from start to end at that point.
//   linear           -> t
//   cosine annealing -> 0.5 * (1 - cos(pi * t))
// The cosine version starts near 0, accelerates through the middle, and
// decelerates as it approaches 1 (ease-in/ease-out).
static inline double scheduleProgress(double t, bool cosine) {
	if (t < 0.0) t = 0.0;
	if (t > 1.0) t = 1.0;
	if (!cosine) return t;
	return 0.5 * (1.0 - std::cos(M_PI * t));
}

double QuantumInspiredEA::currentRotation() const {
	if (compareDouble(this->rotStart, this->rotEnd) == 0)
		return this->rotStart;
	double t = 0.0;
	if (this->maxGenerations > 1)
		t = (double)this->generation / (double)(this->maxGenerations - 1);
	double f = scheduleProgress(t, this->cosineSchedule);
	return this->rotStart + (this->rotEnd - this->rotStart) * f;
}

double QuantumInspiredEA::currentFloor() const {
	if (compareDouble(this->floorStart, this->floorEnd) == 0)
		return this->floorStart;
	double t = 0.0;
	if (this->maxGenerations > 1)
		t = (double)this->generation / (double)(this->maxGenerations - 1);
	double f = scheduleProgress(t, this->cosineSchedule);
	return this->floorStart + (this->floorEnd - this->floorStart) * f;
}

int QuantumInspiredEA::currentSamples() const {
	if (this->samplesStart == this->samplesEnd)
		return this->samplesStart;
	double t = 0.0;
	if (this->maxGenerations > 1)
		t = (double)this->generation / (double)(this->maxGenerations - 1);
	double f = scheduleProgress(t, this->cosineSchedule);
	int k = (int)std::round((double)this->samplesStart
		+ ((double)this->samplesEnd - (double)this->samplesStart) * f);
	if (k < 1) k = 1;
	return k;
}

double QuantumInspiredEA::currentTau() const {
	if (compareDouble(this->tauStart, this->tauEnd) == 0)
		return this->tauStart;
	double t = 0.0;
	if (this->maxGenerations > 1)
		t = (double)this->generation / (double)(this->maxGenerations - 1);
	double f = scheduleProgress(t, this->cosineSchedule);
	return this->tauStart + (this->tauEnd - this->tauStart) * f;
}

void QuantumInspiredEA::rotateTowards(const std::vector<int> &bestGenotype) {
	double step = this->currentRotation();
	if (this->usePrecedence)
		this->rotatePrecedence(bestGenotype, step);
	else
		this->rotatePositional(bestGenotype, step);
}

void QuantumInspiredEA::applyFloor() {
	if (this->usePrecedence)
		this->applyFloorPrecedence();
	else
		this->applyFloorPositional();
}



//=============================================================================
//		PRECEDENCE SCHEME:  pref[a][b] = P(job a precedes job b)
//=============================================================================
//-----  observePrecedence  ---------------------------------------------------
std::vector<int> QuantumInspiredEA::observePrecedence() {
	std::vector<int> remaining = this->tasksPerJob;
	std::vector<int> perm;
	perm.reserve(this->genotypeLength);

	// score[j] = sum over available b != j of P(j precedes b).
	// A high score means j should be dispatched early. It only changes when a
	// job leaves the available set (its count reaches 0), so it is maintained
	// incrementally to keep observation O(L*n).
	std::vector<double> score(this->nJobs, 0.0);
	for (int j = 0; j < this->nJobs; j++) {
		if (remaining[j] <= 0) continue;
		double s = 0.0;
		for (int b = 0; b < this->nJobs; b++)
			if (b != j && remaining[b] > 0)
				s += this->pref[j][b];
		score[j] = s;
	}

	double tau = this->currentTau();
	bool useTau = (compareDouble(tau, 1.0) != 0);

	for (int p = 0; p < this->genotypeLength; p++) {
		double total = 0.0;
		for (int j = 0; j < this->nJobs; j++)
			if (remaining[j] > 0)
				total += useTau ? std::pow(score[j], tau) : score[j];

		int chosen = -1;
		if (total > 1e-12) {
			double u = this->sharedVariables->rng->getProbability() * total;
			double acc = 0.0;
			for (int j = 0; j < this->nJobs; j++) {
				if (remaining[j] > 0) {
					acc += useTau ? std::pow(score[j], tau) : score[j];
					if (u <= acc) { chosen = j; break; }
				}
			}
		}
		if (chosen < 0) {
			for (int j = this->nJobs - 1; j >= 0; j--)
				if (remaining[j] > 0) { chosen = j; break; }
		}

		perm.push_back(chosen);
		remaining[chosen]--;

		// If this job is now exhausted, drop its contribution from all scores
		if (remaining[chosen] == 0) {
			for (int j = 0; j < this->nJobs; j++)
				if (j != chosen && remaining[j] > 0)
					score[j] -= this->pref[j][chosen];
		}
	}
	return perm;
}


//-----  rotatePrecedence  ----------------------------------------------------
void QuantumInspiredEA::rotatePrecedence(const std::vector<int> &bestGenotype,
	double step) {
	// Mean position of each job in the best sequence (stable precedence signal)
	std::vector<double> sumPos(this->nJobs, 0.0);
	std::vector<int> cnt(this->nJobs, 0);
	for (int p = 0; p < (int)bestGenotype.size(); p++) {
		int j = bestGenotype[p];
		sumPos[j] += p;
		cnt[j]++;
	}
	std::vector<double> meanPos(this->nJobs, 0.0);
	for (int j = 0; j < this->nJobs; j++)
		meanPos[j] = (cnt[j] > 0) ? sumPos[j] / cnt[j] : 0.0;

	double s = std::sin(step);
	double c = std::cos(step);
	for (int a = 0; a < this->nJobs; a++) {
		for (int b = a + 1; b < this->nJobs; b++) {
			double pa = this->pref[a][b];          // P(a before b)
			double amp = std::sqrt(pa);            // amplitude
			double r = std::sqrt(1.0 - pa);
			double ampNew;
			if (meanPos[a] <= meanPos[b])
				ampNew = amp * c + r * s;          // rotate toward a-before-b
			else
				ampNew = amp * c - r * s;          // rotate toward b-before-a
			if (ampNew > 1.0) ampNew = 1.0;
			if (ampNew < 0.0) ampNew = 0.0;
			double paNew = ampNew * ampNew;
			this->pref[a][b] = paNew;
			this->pref[b][a] = 1.0 - paNew;
		}
	}
}


//-----  applyFloorPrecedence  ------------------------------------------------
void QuantumInspiredEA::applyFloorPrecedence() {
	double f = this->currentFloor();
	if (compareDouble(f, 0.0) <= 0)
		return;
	double lo = f;
	double hi = 1.0 - f;
	for (int a = 0; a < this->nJobs; a++)
		for (int b = a + 1; b < this->nJobs; b++) {
			double pa = this->pref[a][b];
			if (pa < lo) pa = lo;
			if (pa > hi) pa = hi;
			this->pref[a][b] = pa;
			this->pref[b][a] = 1.0 - pa;
		}
}


}
