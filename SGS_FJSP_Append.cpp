/*
* SGS_FJSP_Append.cpp
*
*  Created on: June 1, 2017
*/

#include "SGS_FJSP_Append.h"

namespace FJSP {

//=============================================================================
//
//	Class SGS_FJSP_Append
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
SGS_FJSP_Append::SGS_FJSP_Append(const FuzzyFW::ParameterDB *params)
	: maximumLabel(FJSP_SGS_APPEND_MAXIMUM), tfnMaximum(FuzzyFW::TFN::M_COMPONENT),
	SGS_FJSP(params) {
}


//=====  Copy constructor  ====================================================
SGS_FJSP_Append::SGS_FJSP_Append(const SGS_FJSP_Append &source)
	: maximumLabel(source.maximumLabel),
	tfnMaximum(source.tfnMaximum),
	SGS_FJSP(source) { }


//=====  Setup method  ========================================================
void SGS_FJSP_Append::setup(const FuzzyFW::ParameterDB *params) {
	SGS_FJSP::setup(params);

	// Load maximum type parameter
	std::string maxName = params->getStringUpper(this->maximumLabel);
	if (maxName.length() == 0) {
		std::string errorMsg = this->maximumLabel + " parameter not found.";
		throw FJSPException("SGS", errorMsg);
	}
	this->tfnMaximum = FuzzyFW::TFN::getMaximum(maxName);
	if (this->tfnMaximum == FuzzyFW::TFN::M_Err) {
		std::string errorMsg = "Invalid value for parameter";
		errorMsg += "\'" + this->maximumLabel + "\': \'";
		errorMsg += maxName + "\'";
		throw FJSPException("SGS", errorMsg);
	}
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Build schedule  ======================================================
ScheduleFJSP * SGS_FJSP_Append::buildSchedule(const FuzzyFW::SharedVars * svars,
	std::vector<int> &order) {

	ProblemFJSP * fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy Problems.";
		throw FJSPException("SGS", errorMsg);
	}

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new ScheduleFJSP(fuzzyProb);
		this->isCreated = true;
	}

	for (size_t i = 0; i < order.size(); i++) {
		this->scheduleTask((*fuzzyProb)[order[i]], order[i]);
	}

	return this->schedule;
}



//=====  Schedule a task  =====================================================
FuzzyFW::TFN SGS_FJSP_Append::scheduleTask(const TaskFJSP *task,
	const int taskIdx) {

	// Machine predecessor
	int mp = this->schedule->lastTaskMachine[task->machine];

	// Job predecessor
	int jp = this->schedule->lastTaskJob[task->job];

	// Check if this task can be scheduled
	if (task->jp != jp) {
		std::string errorMsg;
		errorMsg = "Job precedence constraint is being violated. Scheduling ";
		errorMsg += "task " + valueToString(taskIdx) + " after task ";
		errorMsg += valueToString(jp);
		throw FJSPException("SGS", errorMsg);
	}

	// Starting time
	FuzzyFW::TFN Stime = FuzzyFW::TFN(0, 0, 0);
	if (jp != -1)
		Stime = this->schedule->taskInfo[jp].head +
		this->schedule->taskInfo[jp].task->p;

	if (mp != -1)
		Stime = maximum(Stime, this->schedule->taskInfo[mp].head +
			this->schedule->taskInfo[mp].task->p, this->tfnMaximum);

	// Update the schedule
	this->schedule->addTask(taskIdx, Stime, -1);
	return Stime;
}





//=============================================================================
//
//	Class SGS_FJSP_Dense
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
SGS_FJSP_Dense::SGS_FJSP_Dense(const FuzzyFW::ParameterDB *params)
	: compareLabel(FJSP_SGS_DENSE_COMPARISON),
	tfnCompare(FuzzyFW::TFN::C_EV),
	SGS_FJSP_Append(params) {
}


//=====  Copy constructor  ====================================================
SGS_FJSP_Dense::SGS_FJSP_Dense(const SGS_FJSP_Dense &source)
	: compareLabel(source.compareLabel),
	tfnCompare(source.tfnCompare),
	SGS_FJSP_Append(source) { }


//=====  Setup method  ========================================================
void SGS_FJSP_Dense::setup(const FuzzyFW::ParameterDB *params) {
	SGS_FJSP_Append::setup(params);

	// Load comparison type parameter
	std::string cmpName = params->getStringUpper(this->compareLabel);
	if (cmpName.length() == 0) {
		std::string errorMsg = this->compareLabel + " parameter not found.";
		throw FJSPException("SGS", errorMsg);
	}
	this->tfnCompare = FuzzyFW::TFN::getComparison(cmpName);
	if (this->tfnCompare == FuzzyFW::TFN::C_Err) {
		std::string errorMsg = "Invalid value for parameter";
		errorMsg += "\'" + this->compareLabel + "\': \'";
		errorMsg += cmpName + "\'";
		throw FJSPException("SGS", errorMsg);
	}
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Build schedule  ======================================================
ScheduleFJSP * SGS_FJSP_Dense::buildSchedule(const FuzzyFW::SharedVars * svars,
	std::vector<int> &order) {

	int chosenTask, jobPred, taskIdx;
	const TaskFJSP *task;
	FuzzyFW::TFN Stime, minS;
	
	unsigned int nTasks = (unsigned int)order.size();
	int nJobs;
	std::vector<int> situation(order.size());

	ProblemFJSP * fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy Problems.";
		throw FJSPException("SGS", errorMsg);
	}

	nJobs = fuzzyProb->getNumberJobs();

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new ScheduleFJSP(fuzzyProb);
		this->isCreated = true;
	}


	// Check the position of each task in the permutation
	// NOTE: Empirical results show that this speeds up the method
	for (size_t i = 0; i < order.size(); i++)
		situation[i] = i;


	// Look for the task that can be scheduled the earliest
	while (schedule->getScheduledTasks() < nTasks) {
		minS = FuzzyFW::TFN(Infd, Infd, Infd);
		for (int job = 0; job < nJobs; job++) {

			// Get the next task to be scheduled in the job
			jobPred = this->schedule->lastTaskJob[job];
			if (jobPred != -1)
				taskIdx = this->schedule->taskInfo[jobPred].task->js;
			else if (fuzzyProb->getNumberTasks(job))
				taskIdx = fuzzyProb->getTaskId(job, 0);
			else
				continue;

			task = (*fuzzyProb)[taskIdx];

			Stime = maximum(schedule->getCTJob(job),
				schedule->getCTMachine(task->machine),
				this->tfnMaximum);

			if (Stime.isLesserThan(minS, this->tfnCompare)) {
				minS = Stime;
				chosenTask = taskIdx;
			}
			// If there is a tie, choose the task that is more on the lost
			else if (Stime.isEqualTo(minS, this->tfnCompare)
				&& situation[taskIdx] < situation[chosenTask]) {
				minS = Stime;
				chosenTask = taskIdx;
			}
		}

		task = (*fuzzyProb)[taskIdx];
		this->scheduleTask(task, chosenTask);
	}

	return this->schedule;
}





//=============================================================================
//
//	Class SGS_FJSP_fGYT1
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
SGS_FJSP_fGYT1::SGS_FJSP_fGYT1(const FuzzyFW::ParameterDB *params)
	: deltaLabel(FJSP_SGS_GYT_DELTA),
	delta(1.0),
	SGS_FJSP_Append(params) {

	this->tfnMaximum = FuzzyFW::TFN::M_COMPONENT;
}


//=====  Copy constructor  ====================================================
SGS_FJSP_fGYT1::SGS_FJSP_fGYT1(const SGS_FJSP_fGYT1 &source)
	: deltaLabel(source.deltaLabel),
	delta(source.delta),
	SGS_FJSP_Append(source) { }



//=====  Setup method  ========================================================
void SGS_FJSP_fGYT1::setup(const FuzzyFW::ParameterDB *params) {
	// Don't call the super-class method. We do not need a maximum
	// in this class
	// SGS_FJSP_Append::setup(params);

	// Load delta parameter
	this->delta = params->getDouble(this->deltaLabel);
	if (compareDouble(this->delta, -1.0) == 0) {
		std::cout << "Warning! Parameter \'" << this->deltaLabel;
		std::cout << "\' not found. Value 1.0 taken by default." << std::endl;
		this->delta = 1.0;
	}
	else if (compareDouble(delta, 0.0) < 0 || compareDouble(delta, 1.0) > 0) {
		std::string errorMsg = "Invalid value for parameter";
		errorMsg += "\'" + this->deltaLabel + "\': \'";
		errorMsg += valueToString(this->delta) + "\'";
		throw FJSPException("SGS", errorMsg);
	}

	this->tfnMaximum = FuzzyFW::TFN::M_COMPONENT;
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Build schedule  ======================================================
ScheduleFJSP * SGS_FJSP_fGYT1::buildSchedule(const FuzzyFW::SharedVars * svars,
	std::vector<int> &order) {

	int jobPred, taskIdx, chosenTask;
	const TaskFJSP *task;
	FuzzyFW::TFN Stime, Ctime, threshold;
	FuzzyFW::TFN minS, minC;

	unsigned int nTasks = (unsigned int)order.size();
	int nJobs;

	std::vector<unsigned int> situation(nTasks);
	std::vector<int> available;

	ProblemFJSP * fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy Problems.";
		throw FJSPException("SGS", errorMsg);
	}

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new ScheduleFJSP(fuzzyProb);
		this->isCreated = true;
	}

	// Keep the position of each task in the permutation
	// NOTE: Empirical results show that this speeds up the method
	nJobs = fuzzyProb->getNumberJobs();
	for (size_t i = 0; i < order.size(); i++)
		situation[order[i]] = i;


	while (this->schedule->getScheduledTasks() < nTasks) {
		// Look for the minimum completion time
		minC = FuzzyFW::TFN(Infd, Infd, Infd);
		minS = FuzzyFW::TFN(Infd, Infd, Infd);
		available.clear();

		for (int job = 0; job < nJobs; job++) {
			// Get the next task to be scheduled in the job
			jobPred = this->schedule->lastTaskJob[job];
			if (jobPred != -1)
				taskIdx = this->schedule->taskInfo[jobPred].task->js;
			else if (fuzzyProb->getNumberTasks(job) > 0)
				taskIdx = fuzzyProb->getTaskId(job, 0);
			else
				taskIdx = -1;

			if(taskIdx < 0)
				continue;
			task = (*fuzzyProb)[taskIdx];

			Stime = maximum(schedule->getCTJob(job),
				schedule->getCTMachine(task->machine),
				this->tfnMaximum);
			Ctime = Stime + task->p;

			minS = minimum(minS, Stime, FuzzyFW::TFN::M_COMPONENT);
			minC = minimum(minC, Ctime, FuzzyFW::TFN::M_COMPONENT);
			if (!Stime.isGreaterEqualTo(minC, FuzzyFW::TFN::C_COMPONENT))
				available.push_back(taskIdx);
		}


		// Calculate the threshold to build the conflict set
		threshold = FuzzyFW::TFN(minS.a + this->delta * (minC.a - minS.a),
			minS.b + this->delta * (minC.b - minS.b),
			minS.c + this->delta * (minC.c - minS.c));

		// Look for the tasks that can begin before the threshold and
		// choose the one that is more on the left
		chosenTask = -1;
		for (size_t i = 0; i < available.size(); i++) {
			task = (*fuzzyProb)[available[i]];

			Stime = maximum(schedule->getCTJob(task->job),
				schedule->getCTMachine(task->machine),
				this->tfnMaximum);

			// Is lesser than threshold in at least, one component
			if (!Stime.isGreaterEqualTo(threshold, FuzzyFW::TFN::C_COMPONENT)) {
				if (chosenTask < 0
					|| situation[available[i]] < situation[chosenTask])
					chosenTask = available[i];
			}
		}

		scheduleTask((*fuzzyProb)[chosenTask], chosenTask);
	}

	return this->schedule;
}





//=============================================================================
//
//	Class SGS_FJSP_fGYT2
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
SGS_FJSP_fGYT2::SGS_FJSP_fGYT2(const FuzzyFW::ParameterDB *params)
	: SGS_FJSP_Append(params) {

	this->tfnMaximum = FuzzyFW::TFN::M_COMPONENT;
}


//=====  Setup method  ========================================================
void SGS_FJSP_fGYT2::setup(const FuzzyFW::ParameterDB *params) {
	// Don't call the super-class method. We do not need a maximum
	// in this class
	// SGS_FJSP_Append::setup(params);

	this->tfnMaximum = FuzzyFW::TFN::M_COMPONENT;
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Build schedule  ======================================================
ScheduleFJSP * SGS_FJSP_fGYT2::buildSchedule(const FuzzyFW::SharedVars * svars,
	std::vector<int> &order) {

	int jobPred, taskIdx, chosenTask;
	bool conflict;
	const TaskFJSP *task;
	FuzzyFW::TFN Stime, Ctime;
	FuzzyFW::TFN minS, minC;

	unsigned int nTasks = (unsigned int)order.size();
	int nJobs;

	std::vector<unsigned int> situation(nTasks);
	std::vector<unsigned int> available;
	std::vector<FuzzyFW::TFN> setAstar;

	ProblemFJSP * fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy Problems.";
		throw FJSPException("SGS", errorMsg);
	}

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new ScheduleFJSP(fuzzyProb);
		this->isCreated = true;
	}

	// Keep the position of each task in the permutation
	// NOTE: Empirical results show that this speeds up the method
	nJobs = fuzzyProb->getNumberJobs();
	for (size_t i = 0; i < order.size(); i++)
		situation[order[i]] = i;


	while (this->schedule->getScheduledTasks() < nTasks) {
		// Look for the minimum completion time
		minC = FuzzyFW::TFN(Infd, Infd, Infd);
		minS = FuzzyFW::TFN(Infd, Infd, Infd);
		available.clear();

		for (int job = 0; job < nJobs; job++) {
			// Get the next task to be scheduled in the job
			jobPred = this->schedule->lastTaskJob[job];
			if (jobPred != -1)
				taskIdx = this->schedule->taskInfo[jobPred].task->js;
			else if (fuzzyProb->getNumberTasks(job) > 0)
				taskIdx = fuzzyProb->getTaskId(job, 0);
			else
				taskIdx = -1;

			if(taskIdx < 0)
				continue;
			task = (*fuzzyProb)[taskIdx];

			Stime = maximum(schedule->getCTJob(job),
				schedule->getCTMachine(task->machine),
				this->tfnMaximum);
			Ctime = Stime + task->p;

			minS = minimum(minS, Stime, FuzzyFW::TFN::M_COMPONENT);

			// The completion time is the minimum so far
			if (Ctime.isLesserThan(minC, FuzzyFW::TFN::C_COMPONENT)) {
				minC = Ctime;
				setAstar.clear();
				setAstar.push_back(Ctime);
			}
			// The completion time can donate a component to the minimum
			else if (!Ctime.isGreaterThan(minC, FuzzyFW::TFN::C_COMPONENT)) {
				minC = minimum(minC, Ctime, FuzzyFW::TFN::M_COMPONENT);
				setAstar.push_back(Ctime);
			}

			// Update list of available tasks for next step
			if (!Stime.isGreaterEqualTo(minC, FuzzyFW::TFN::C_COMPONENT))
				available.push_back(taskIdx);
		}

		// Look for the tasks that can begin before the threshold and
		// choose the one that is more on the left
		chosenTask = -1;
		for (size_t i = 0; i < available.size(); i++) {
			task = (*fuzzyProb)[available[i]];

			Stime = maximum(schedule->getCTJob(task->job),
				schedule->getCTMachine(task->machine),
				this->tfnMaximum);

			conflict = true;
			for (size_t j = 0; j < setAstar.size(); j++) {
				if (Stime.isGreaterEqualTo(setAstar[j],
					FuzzyFW::TFN::C_COMPONENT))
					conflict = false;
				break;
			}

			// If is the left-most operation in the order
			if (conflict) {
				if (chosenTask < 0
					|| situation[available[i]] < situation[chosenTask])
					chosenTask = available[i];
			}
		}

		scheduleTask((*fuzzyProb)[chosenTask], chosenTask);
	}

	return this->schedule;
}

}
