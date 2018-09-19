/*
* ProblemFJSP.cpp
*
*  Created on: May 15, 2017
*      Author: Juan Jose Palacios
*/

#include "ProblemFJSP.h"

namespace FJSP {


//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
ProblemFJSP::ProblemFJSP(const FuzzyFW::ParameterDB *params)
	: FuzzyFW::Problem(params) {

	// Initialize variables
	this->nJobs = this->nMachines = this->nTasks = 0;
	this->lb_Makespan = 0.0;
	this->ub_AImin = 1.0;
	this->ub_AIavg = 1.0;
	this->duedateType = "";

	// Load parameters
	if (params != NULL)
		this->setup(params);
}



//====  Main constructor  =====================================================
ProblemFJSP::ProblemFJSP(const FuzzyFW::ParameterDB *params,
	const char *inputFile)
	: FuzzyFW::Problem(params, inputFile) {

	// Initialize variables
	this->nJobs = this->nMachines = this->nTasks = 0;
	this->lb_Makespan = 0.0;
	this->ub_AImin = 1.0;
	this->ub_AIavg = 1.0;
	this->duedateType = "";

	// Load parameters
	if (params != NULL)
		this->setup(params);

	this->loadFile(inputFile);
}



//====  Main constructor  =====================================================
ProblemFJSP::ProblemFJSP(const char *inputFile)
	: FuzzyFW::Problem(inputFile) {

	// Initialize variables
	this->nJobs = this->nMachines = this->nTasks = 0;
	this->lb_Makespan = 0.0;
	this->ub_AImin = 1.0;
	this->ub_AIavg = 1.0;
	this->duedateType = "";

	// Is the problem loaded
	this->isSetup = false;
}


//====  Setup method  =========================================================
void ProblemFJSP::setup(const FuzzyFW::ParameterDB *params) {
	Problem::setup(params);

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


//====  Copy constructor  =====================================================
ProblemFJSP::ProblemFJSP(const ProblemFJSP & source)
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
		this->task.push_back(new TaskFJSP(*source.task[t]));
	for (size_t t = 0; t < source.dueDate.size(); t++)
		this->dueDate.push_back(source.dueDate[t]->clone());

	this->taskSequence = source.taskSequence;
}



//=============================================================================
//		DESTRUCTOR
//=============================================================================
ProblemFJSP::~ProblemFJSP() {
	for (size_t t = 0; t < this->task.size(); t++)
		delete this->task[t];
	for (size_t t = 0; t < this->dueDate.size(); t++)
		delete this->dueDate[t];
}



//=============================================================================
//		GET/SET METHODS
//=============================================================================
//====  Get Number of Tasks  ==================================================
unsigned int ProblemFJSP::getNumberTasks(const unsigned int job) const {
	if (job < this->nJobs && job >= 0)
		return (int)(this->taskSequence[job].size());
	std::string errorMsg = "Trying to access unexisting job: ";
	errorMsg += valueToString(job);
	throw new FJSPException("Problem", errorMsg);
}


//====  Get the different bounds  =============================================
double ProblemFJSP::getMakespanLB() const {
	if (this->withBounds)
		return this->lb_Makespan;
	return 0.0;
}

double ProblemFJSP::getAIminUB() const {
	if (this->withBounds)
		return this->ub_AImin;
	return 1.0;
}

double ProblemFJSP::getAIavgUB() const {
	if (this->withBounds)
		return this->ub_AIavg;
	return 1.0;
}


//====  Get Duedate  ==========================================================
const FuzzyFW::TimeWindow * ProblemFJSP::getTimeWindow(const unsigned int job)
const {
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
int ProblemFJSP::getTaskId(const unsigned int job, const int position)
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
const TaskFJSP * ProblemFJSP::getTask(const unsigned int taskId) const {
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
void ProblemFJSP::loadFile(const char *inputFile) {
	std::string reader;
	std::stringstream lineStream;
	std::ifstream input;
	std::vector<TaskFJSP *> preTasks;
	unsigned int machine, lastTask, job;
	size_t jobSize;
	FuzzyFW::TFN duration;

	if (!this->isSetup) {
		std::string errorMsg;
		errorMsg = "The problem file cannot be read before the reading";
		errorMsg += " parameters are loaded.";
		throw new FJSPException("Problem", errorMsg);
	}

	if (inputFile != NULL)
		this->updatePath(inputFile);

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
		lineStream.str(reader);
		while (lineStream >> machine) {
			preTasks.push_back(new TaskFJSP(preTasks.size(),
				i, machine));
		}
		lineStream.clear();
	}

	getline(input, reader);

	// Read the processing times of the problem and discard null tasks
	this->taskSequence.resize(this->nJobs);
	for (size_t i = 0; i < (int)preTasks.size(); i++) {
		input >> preTasks[i]->p;

		if (preTasks[i]->p.isGreaterEqualTo(FuzzyFW::TFN(0, 0, 0),
			FuzzyFW::TFN::C_COMPONENT)) {
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
void ProblemFJSP::loadDueDates(std::ifstream &input) {
	std::string reader;
	FuzzyFW::TimeWindow * tw;

	if (this->duedateType.length() > 0) {
		// Check due-date types
		tw = FuzzyFW::TimeWindowClassRegister::
			getDueDateObject(this->duedateType);
		if (tw == NULL) {
			std::string errorMsg = "The introduced due-date type is not";
			errorMsg += " valid: \'" + this->duedateType + "\'";
			throw new FJSPException("Loading problem", errorMsg);
		}

		getline(input, reader); // Heading
		input >> tw;
		this->dueDate.push_back(tw);

		for (unsigned int i = 1; i < this->nJobs; i++) {
			tw = FuzzyFW::TimeWindowClassRegister::
				getDueDateObject(this->duedateType);
			input >> tw;
			this->dueDate.push_back(tw);
		}
		getline(input, reader);
	}
}



//====  Load lower/upper bounds from file  ====================================
void ProblemFJSP::loadBounds(std::ifstream &input) {
	std::string reader;

	if (this->withBounds) {
		getline(input, reader);
		input >> this->lb_Makespan;
		input >> this->ub_AImin;
		input >> this->ub_AIavg;
	}
}


//====  Clear  ================================================================
void ProblemFJSP::clear() {
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
void ProblemFJSP::scalarize() {
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
void ProblemFJSP::computeTaillardBounds() {
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
