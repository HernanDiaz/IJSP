/*
* FuzzyProblem.cpp
*
*  Created on: May 15, 2017
*      Author: Juan Jose Palacios
*/

#include "FuzzyProblem.h"

namespace FJSP {


//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
FuzzyProblem::FuzzyProblem(const ParameterDB *params)
	: Problem(params) {
	SchedulingClassRegister::registerClasses();

	// Initialize variables
	this->nJobs = this->nMachines = this->nTasks = 0;
	this->lb_Makespan = 0.0;
	this->ub_AImin = 1.0;
	this->ub_AIavg = 1.0;

	// Load bounds parameter
	std::string paramValue = params->getStringUpper(PROBLEM_LOWERBOUNDS);
	if (paramValue.compare("YES") == 0)
		this->withBounds = true;
	else if (paramValue.compare("NO") == 0
		|| (paramValue.compare("NONE") == 0))
		this->withBounds = false;
	else if (paramValue.length() == 0) {
		std::cout << "Warning (Loading problem): Parameter \'";
		std::cout << PROBLEM_LOWERBOUNDS << "\' not found. ";
		std::cout << "User lower bounds will NOT be considered";
		this->withBounds = false;
	}
	else {
		std::string errorMsg = "Invalid value for parameter";
		errorMsg += "\'" + std::string(PROBLEM_LOWERBOUNDS) + "\': \'";
		errorMsg += paramValue + "\'";
		throw new FJSPException("Loading problem", errorMsg);
	}

	// Load due-date type
	this->duedateType = params->getStringLower(PROBLEM_DUEDATES);
	if (this->duedateType.length() == 0) {
		std::cout << "Warning (Loading problem): Parameter \'";
		std::cout << PROBLEM_DUEDATES << "\' not found. ";
		std::cout << "Due-dates will NOT be considered";
	}
}



//====  Main constructor  =====================================================
FuzzyProblem::FuzzyProblem(const ParameterDB *params,
	const char *inputFile)
	: Problem(params, inputFile) {

	// Initialize variables
	this->nJobs = this->nMachines = this->nTasks = 0;
	this->lb_Makespan = 0.0;
	this->ub_AImin = 1.0;
	this->ub_AIavg = 1.0;

	// Load bounds parameter
	std::string paramValue = params->getStringUpper(PROBLEM_LOWERBOUNDS);
	if (paramValue.compare("YES") == 0)
		this->withBounds = true;
	else if (paramValue.compare("NO") == 0
		|| (paramValue.compare("NONE") == 0))
		this->withBounds = false;
	else if (paramValue.length() == 0) {
		std::cout << "Warning (Loading problem): Parameter \'";
		std::cout << PROBLEM_LOWERBOUNDS << "\' not found. ";
		std::cout << "User lower bounds will NOT be considered";
		this->withBounds = false;
	}
	else {
		std::string errorMsg = "Invalid value for parameter";
		errorMsg += "\'" + std::string(PROBLEM_LOWERBOUNDS) + "\': \'";
		errorMsg += paramValue + "\'";
		throw new FJSPException("Loading problem", errorMsg);
	}

	// Load due-date type
	this->duedateType = params->getStringLower(PROBLEM_DUEDATES);
	if (this->duedateType.length() == 0) {
		std::cout << "Warning (Loading problem): Parameter \'";
		std::cout << PROBLEM_DUEDATES << "\' not found. ";
		std::cout << "Due-dates will NOT be considered";
	}

	this->loadFile(inputFile);
}



//====  Copy constructor  =====================================================
FuzzyProblem::FuzzyProblem(const FuzzyProblem & source)
	: Problem(source) {
	this->nJobs = source.nJobs;
	this->nMachines = source.nMachines;
	this->nTasks = source.nTasks;
	this->lb_Makespan = source.lb_Makespan;
	this->ub_AImin = source.ub_AImin;
	this->ub_AIavg = source.ub_AIavg;

	this->withBounds = source.withBounds;
	this->duedateType = source.duedateType;

	for (size_t t = 0; t < source.task.size(); t++)
		this->task.push_back(new FuzzyTask(*source.task[t]));
	for (size_t t = 0; t < source.dueDate.size(); t++)
		this->dueDate.push_back(source.dueDate[t]->clone());

	this->taskSequence = source.taskSequence;
}



//=============================================================================
//		DESTRUCTOR
//=============================================================================
FuzzyProblem::~FuzzyProblem() {
	for (size_t t = 0; t < this->task.size(); t++)
		delete this->task[t];
	for (size_t t = 0; t < this->dueDate.size(); t++)
		delete this->dueDate[t];
}



//=============================================================================
//		GET/SET METHODS
//=============================================================================
//====  Get Number of Tasks  ==================================================
unsigned int FuzzyProblem::getNumberTasks(const int job) const {
	if (job < this->nJobs && job >= 0)
		return (int)(this->taskSequence[job].size());
	std::string errorMsg = "Trying to access unexisting job: ";
	errorMsg += valueToString(job);
	throw new FJSPException("Problem", errorMsg);
}


//====  Get the different bounds  =============================================
double FuzzyProblem::getMakespanLB() const {
	if (this->withBounds)
		return this->lb_Makespan;
	return 0.0;
}

double FuzzyProblem::getAIminUB() const {
	if (this->withBounds)
		return this->ub_AImin;
	return 1.0;
}

double FuzzyProblem::getAIavgUB() const {
	if (this->withBounds)
		return this->ub_AIavg;
	return 1.0;
}


//====  Get Duedate  ==========================================================
const DueDate * FuzzyProblem::getDueDate(const int job) const {
	if (job < this->nJobs && job >= 0) {
		if (this->dueDate.size() == 0)
			return NULL;
		return this->dueDate[job];
	}
	else {
		std::string errorMsg = "Trying to access unexisting job: ";
		errorMsg += valueToString(job);
		throw new FJSPException("Problem", errorMsg);
	}
}


//====  Get Task in a job  ====================================================
int FuzzyProblem::getTaskId(const int job, const int position)
	const {
	if (job >= this->nJobs || job < 0) {
		std::string errorMsg = "Trying to access unexisting job: ";
		errorMsg += valueToString(job);
		throw new FJSPException("Problem", errorMsg);
	}
	if (position < 0) {
		std::string errorMsg = "Trying to access unexisting task";
		errorMsg += " of a job: " + valueToString(position);
		throw new FJSPException("Problem", errorMsg);
	}

	if (position >= (int)this->taskSequence[job].size())
		return -1;
	return taskSequence[job][position];
}



//====  Index access  =========================================================
const FuzzyTask * FuzzyProblem::getTask(const int taskId) const {
	if (taskId < 0 || taskId >= (int)this->task.size()) {
		std::string errorMsg = "Trying to access unexisting task: ";
		errorMsg += valueToString(taskId);
		throw new FJSPException("Problem", errorMsg);
	}
	return this->task[taskId];
}



//=============================================================================
//		METHODS
//=============================================================================
//====  Load data from file  ==================================================
void FuzzyProblem::loadFile(const char *inputFile) {
	std::string reader;
	std::stringstream lineStream;
	std::ifstream input;
	std::vector<FuzzyTask *> preTasks;
	unsigned int machine, lastTask, job;
	size_t jobSize;
	TFN duration;

	// Open the problem file
	input.open(this->problemPath);
	if (!input.is_open()) {
		std::string errorMsg;
		errorMsg = "The problem file \'" + std::string(this->problemPath);
		errorMsg += +"\' has not been found.";
		throw new FJSPException("Problem", errorMsg);
	}

	// Reset the data structures
	this->clear();

	// Read the number of jobs and machines
	getline(input, reader);
	input >> this->nJobs;
	getline(input, reader);	// Blank line
	getline(input, reader);
	input >> this->nMachines;

	// Read empty lines
	getline(input, reader);
	getline(input, reader);

	// Read the machine requirements
	for (unsigned int i = 0; i < this->nJobs; i++) {
		getline(input, reader);
		lineStream = std::stringstream(reader);
		while (!lineStream) {
			lineStream >> machine;
			preTasks.push_back(new FuzzyTask(preTasks.size(),
				i, machine));
		}
	}

	getline(input, reader);
	getline(input, reader);

	// Read the processing times of the problem and discard null tasks
	for (size_t i = 0; i < (int)preTasks.size(); i++) {
		input >> preTasks[i]->p;

		if (preTasks[i]->p.isGreaterEqualTo(TFN(0, 0, 0),
			TFN::C_COMPONENT)) {
			// Job predecessor
			job = preTasks[i]->job;
			if (this->taskSequence[job].size() > 0) {
				jobSize = this->taskSequence[job].size();
				lastTask = this->taskSequence[job][jobSize - 1];
				this->task[lastTask]->js = this->nTasks;
				preTasks[i]->jp = lastTask;
			}
			else
				preTasks[i]->jp = -1;

			// Job succecessor
			preTasks[i]->js = -1;
			this->taskSequence[job].push_back(this->nTasks);
			this->task.push_back(preTasks[i]);
			this->nTasks++;
		}
		else
			delete preTasks[i];
	}

	// Load due dates if they are present in the problem
	getline(input, reader);
	this->loadDueDates(input);

	// Load lower/upper bounds if they are present in the problem
	this->loadBounds(input);

	input.close();
	this->scalarize();
}



//====  Load due-dates from file  =============================================
void FuzzyProblem::loadDueDates(std::ifstream &input) {
	std::string reader;
	DueDate * dd;

	if (this->duedateType.length() > 0) {
		// Check due-date types
		dd = SchedulingClassRegister::getDueDateObject(this->duedateType);
		if (dd == NULL) {
			std::string errorMsg = "The introduced due-date type is not";
			errorMsg += " valid: \'" + this->duedateType + "\'";
			throw new FJSPException("Loading problem", errorMsg);
		}

		getline(input, reader); // Blank line
		input >> dd;
		this->dueDate.push_back(dd);

		for (unsigned int i = 1; i < this->nJobs; i++) {
			dd = SchedulingClassRegister::getDueDateObject(this->duedateType);
			input >> dd;
			this->dueDate.push_back(dd);
		}
		getline(input, reader);
	}
}



//====  Load lower/upper bounds from file  ====================================
void FuzzyProblem::loadBounds(std::ifstream &input) {
	std::string reader;

	if (this->withBounds) {
		getline(input, reader);
		input >> this->lb_Makespan;
		input >> this->ub_AImin;
		input >> this->ub_AIavg;
	}
}


//====  Clear  ================================================================
void FuzzyProblem::clear() {
	this->nJobs = this->nMachines = this->nTasks = 0;
	this->lb_Makespan = 0.0;
	this->ub_AImin = 1.0;
	this->ub_AIavg = 1.0;

	for (size_t t = 0; t < this->task.size(); t++)
		delete this->task[t];
	this->task.clear();
	for (size_t t = 0; t < this->dueDate.size(); t++)
		delete this->dueDate[t];
	this->dueDate.clear();

	this->taskSequence.clear();
}



//====  Scalirize  ============================================================
void FuzzyProblem::scalarize() {
	if (this->nTasks == 0)
		return;

	int minMac = this->task[0]->machine;
	for (unsigned int i = 1; i < this->nTasks; i++) {
		if (this->task[i]->machine < minMac)
			minMac = this->task[i]->machine;
	}

	for (unsigned int i = 0; i < this->nTasks; i++)
		this->task[i]->machine -= minMac;
}

}




/*
//====  Compute Lower/Upper Bounds  ===========================================
void FuzzyProblem::computeTaillardBounds() {
	int mac, job;
	double ai, sumAI;
	TFN max(0, 0, 0);

	// Maximum workload per job/machine
	this->maxLoadJob.resize(this->nJobs, TFN(0, 0, 0));
	this->maxLoadMachine.resize(this->nMachines, TFN(0, 0, 0));

	for (unsigned int i = 0; i < this->nTasks; i++) {
		mac = this->task[i].machine;
		job = this->task[i].job;

		this->maxLoadMachine[mac] += this->task[i].p;
		this->maxLoadJob[job] += this->task[i].p;

		max = maximum(max, this->maxLoadMachine[mac]);
		max = maximum(max, this->maxLoadJob[job]);
	}

	// MAKESPAN Lower Bound
	this->taiLB_Makespan = max;


	// In case there are flexible due-dates
	this->taiUB_AImin = 1.0;
	sumAI = 0.0;

	if (this->getDueDatesType() != DueDate::DueDateType::NONE) {
		for (unsigned int i = 0; i < this->nJobs; i++) {
			if (this->dueDate[i].getType() != DueDate::DueDateType::LINEAR
				|| this->dueDate[i].getType() != DueDate::DueDateType::TRAPEZOID) {

				ai = this->dueDate[i].agreementIndex(this->maxLoadJob[job]);
				if (compareDouble(ai, this->taiUB_AImin) < 0)
					this->taiUB_AImin = ai;
				sumAI += ai;
			}
		}
		this->taiUB_AIavg = sumAI / this->nJobs;
	}
}
*/
