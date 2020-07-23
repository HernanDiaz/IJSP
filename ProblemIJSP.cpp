/*
* ProblemIJSP.cpp
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/

#include "ProblemIJSP.h"

namespace IJSP {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//====  Default constructor  ==================================================
	ProblemIJSP::ProblemIJSP(const FuzzyFW::ParameterDB *params)
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
	ProblemIJSP::ProblemIJSP(const FuzzyFW::ParameterDB *params,
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
	ProblemIJSP::ProblemIJSP(const char *inputFile)
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
	void ProblemIJSP::setup(const FuzzyFW::ParameterDB *params) {
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
			throw new IJSPException("Loading problem", errorMsg);
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
	ProblemIJSP::ProblemIJSP(const ProblemIJSP & source)
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
			this->task.push_back(new TaskIJSP(*source.task[t]));
		for (size_t t = 0; t < source.dueDate.size(); t++)
			this->dueDate.push_back(source.dueDate[t]->clone());

		this->taskSequence = source.taskSequence;
	}



	//=============================================================================
	//		DESTRUCTOR
	//=============================================================================
	ProblemIJSP::~ProblemIJSP() {
		for (size_t t = 0; t < this->task.size(); t++)
			delete this->task[t];
		for (size_t t = 0; t < this->dueDate.size(); t++)
			delete this->dueDate[t];
	}



	//=============================================================================
	//		GET/SET METHODS
	//=============================================================================
	//====  Get Number of Tasks  ==================================================
	unsigned int ProblemIJSP::getNumberTasks(const unsigned int job) const {
		if (job < this->nJobs && job >= 0)
			return (int)(this->taskSequence[job].size());
		std::string errorMsg = "Trying to access unexisting job: ";
		errorMsg += valueToString(job);
		throw new IJSPException("Problem", errorMsg);
	}


	//====  Get the different bounds  =============================================
	double ProblemIJSP::getMakespanLB() const {
		if (this->withBounds)
			return this->lb_Makespan;
		return 0.0;
	}

	double ProblemIJSP::getAIminUB() const {
		if (this->withBounds)
			return this->ub_AImin;
		return 1.0;
	}

	double ProblemIJSP::getAIavgUB() const {
		if (this->withBounds)
			return this->ub_AIavg;
		return 1.0;
	}


	//====  Get Duedate  ==========================================================
	FuzzyFW::TimeWindow * ProblemIJSP::getTimeWindow(const unsigned int job)
		const {
		if (job < this->nJobs && job >= 0) {
			if (this->dueDate.size() == 0)
				return NULL;
			return this->dueDate[job];
		}
		else {
			std::string errorMsg = "Trying to access unexisting job: ";
			errorMsg += valueToString(job);
			throw new IJSPException("Problem", errorMsg);
		}
	}


	//====  Get Task in a job  ====================================================
	int ProblemIJSP::getTaskId(const unsigned int job, const int position)
		const {
		if (job >= this->nJobs || job < 0) {
			std::string errorMsg = "Trying to access unexisting job: ";
			errorMsg += valueToString(job);
			throw new IJSPException("Problem", errorMsg);
		}
		if (position < 0) {
			std::string errorMsg = "Trying to access unexisting task";
			errorMsg += " of a job: " + valueToString(position);
			throw new IJSPException("Problem", errorMsg);
		}

		if (position >= (int)this->taskSequence[job].size())
			return -1;
		return taskSequence[job][position];
	}



	//====  Index access  =========================================================
	TaskIJSP * ProblemIJSP::getTask(const unsigned int taskId) const {
		if (taskId < 0 || taskId >= (int)this->task.size()) {
			std::string errorMsg = "Trying to access unexisting task: ";
			errorMsg += valueToString(taskId);
			throw new IJSPException("Problem", errorMsg);
		}
		return this->task[taskId];
	}



	//=============================================================================
	//		METHODS
	//=============================================================================
	//====  Load data from file  ==================================================
	void ProblemIJSP::loadFile(const char *inputFile) {
		std::string reader;
		std::stringstream lineStream;
		std::ifstream input;
		std::vector<TaskIJSP *> preTasks;
		unsigned int machine, lastTask, job;
		size_t jobSize;
		FuzzyFW::Interval duration;

		if (!this->isSetup) {
			std::string errorMsg;
			errorMsg = "The problem file cannot be read before the reading";
			errorMsg += " parameters are loaded.";
			throw new IJSPException("Problem", errorMsg);
		}

		if (inputFile != NULL)
			this->updatePath(inputFile);

		// Open the problem file
		input.open(this->problemPath);
		if (!input.is_open()) {
			std::string errorMsg;
			errorMsg = "The problem file \'" + std::string(this->problemPath);
			errorMsg += +"\' has not been found.";
			throw new IJSPException("Problem", errorMsg);
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
				preTasks.push_back(new TaskIJSP(preTasks.size(),
					i, machine));
			}
			lineStream.clear();
		}

		getline(input, reader);

		// Read the processing times of the problem and discard null tasks
		this->taskSequence.resize(this->nJobs);
		for (size_t i = 0; i < (int)preTasks.size(); i++) {
			input >> preTasks[i]->p;

			if (preTasks[i]->p.isGreaterEqualTo(FuzzyFW::Interval(0, 0),
				FuzzyFW::Interval::C_COMPONENT)) {
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

		this->saveFile(inputFile);
		this->scalarize();
	}



	//=============================================================================
	//		METHODS
	//=============================================================================
	//====  Save data to file  ==================================================
	void ProblemIJSP::saveFile(const char *outputFile) {
		std::ofstream output;
	

		if (!this->isSetup) {
			std::string errorMsg;
			errorMsg = "The problem file cannot be written before the reading";
		   throw new IJSPException("Problem", errorMsg);
		}

		// Open the problem file

		output.open(this->problemPath);
		if (!output.is_open()) {
			std::string errorMsg;
			errorMsg = "The problem file \'" + std::string(outputFile);
			errorMsg += +"\' has not been found.";
			throw new IJSPException("Problem", errorMsg);
		}

	   // Write the number of jobs and machines
		output << std::endl;
		output << this->nJobs << std::endl;
		output << std::endl;
		output << this->nMachines << std::endl;
		output << std::endl;
			
		// Write the machine requirements
		for (unsigned int i = 0; i < this->task.size(); i++) {
		    	output << this->task[i]->machine<<"\t";
				if ((i+1)%this->nMachines == 0) {
					output << std::endl;
				}
	    }
		output << std::endl;

		// Write intervals
		for (unsigned int i = 0; i < this->task.size(); i++) {
	     		output << this->task[i]->p<<"\t";
				if ((i+1)%this->nMachines == 0) {
					output << std::endl;
				}
		}
		output << std::endl;

		// Write due dates
		for (unsigned int i = 0; i < this->dueDate.size(); i++) {
			output << this->dueDate[i] << "\t";;
		}
		output << std::endl;

		output.close();
	}



	//====  Load due-dates from file  =============================================
	void ProblemIJSP::loadDueDates(std::ifstream &input) {
		std::string reader;
		FuzzyFW::TimeWindow * tw;

		if (this->duedateType.length() > 0) {
			// Check due-date types
			tw = FuzzyFW::TimeWindowClassRegister::
				getDueDateObject(this->duedateType);
			if (tw == NULL) {
				std::string errorMsg = "The introduced due-date type is not";
				errorMsg += " valid: \'" + this->duedateType + "\'";
				throw new IJSPException("Loading problem", errorMsg);
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
	void ProblemIJSP::loadBounds(std::ifstream &input) {
		std::string reader;

		if (this->withBounds) {
			getline(input, reader);
			input >> this->lb_Makespan;
			input >> this->ub_AImin;
			input >> this->ub_AIavg;
		}
	}


	//====  Clear  ================================================================
	void ProblemIJSP::clear() {
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
	void ProblemIJSP::scalarize() {
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
