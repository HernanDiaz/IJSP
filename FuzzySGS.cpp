/*
* FuzzySGS.cpp
*
*  Created on: June 1, 2017
*      Author: Juan Jose Palacios
*/

#include "FuzzySGS.h"

namespace FJSP {

//=============================================================================
//
//	Abstract class FuzzySGS
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
FuzzySGS::FuzzySGS(const ParameterDB *params)
	: schedule(NULL), isCreated(false) {
	if (params != NULL)
		this->setup(params);
}


//=====  Copy constructor  ====================================================
FuzzySGS::FuzzySGS(const FuzzySGS &source) {
	this->schedule = new FuzzySchedule(*source.schedule);
	this->isCreated = source.isCreated;
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Reset  ===============================================================
void FuzzySGS::reset() {
	if (this->isCreated)
		this->schedule->reset();
}





//=============================================================================
//
//	Class FuzzySGS_Insertion
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Build schedule  ======================================================
FuzzySchedule * FuzzySGS_Insertion::buildSchedule(const SharedVars * svars,
	std::vector<int> &order) {

	if (svars->problem == NULL)
		throw new FJSPException("SGS", "Problem instance not created");

	FuzzyProblem * fuzzyProb =
		dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy Problems.";
		throw new FJSPException("SGS", errorMsg);
	}

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new FuzzySchedule(fuzzyProb);
		this->isCreated = true;
	}

	for (size_t i = 0; i < order.size(); i++) {
		this->scheduleTask((*fuzzyProb)[order[i]], order[i]);
	}

	return this->schedule;
}


//=====  Schedule a task  =====================================================
TFN FuzzySGS_Insertion::scheduleTask(const FuzzyTask *task,
	const int taskIdx) {

	int mp, ms; // Machine predecessor and successor
	TFN Stime;	// Starting time
	TFN mtHead, mtPT;
	char found;	// Big gap found in the schedule
	// Comparisons are made component by component
	TFN::Compare cpComp = TFN::C_COMPONENT;
	TFN::Maximum maxComp = TFN::M_COMPONENT;

	int mac = task->machine;
	int job = task->job;

	// Check if this task can be scheduled
	if (task->jp != this->schedule->lastTaskJob[job]) {
		std::string errorMsg;
		errorMsg = "Job precedence constraint is being violated. Scheduling ";
		errorMsg += "task " + valueToString(taskIdx) + " after task ";
		errorMsg += valueToString(this->schedule->lastTaskJob[job]);
		throw new FJSPException("SGS", errorMsg);
	}

	// If I could schedule the task just after its job predecessor, who
	// would be the machine predecessor and successors...
	Stime = this->schedule->getCTJob(job);
	ms = -1;
	mp = this->schedule->lastTaskMachine[mac];
	if (mp != -1)
		mtHead = this->schedule->taskInfo[mp].head;

	while (mp != -1 && mtHead.isGreaterEqualTo(Stime, cpComp)) {
		ms = mp;
		mp = this->schedule->taskInfo[ms].mp;
		if (mp != -1)
			mtHead = this->schedule->taskInfo[mp].head;
	}

	// Update the heuristic Starting time that task could take
	if (mp != -1) {
		mtHead = this->schedule->taskInfo[mp].head;
		mtPT = this->schedule->taskInfo[mp].task->p;
		Stime = maximum(Stime, mtHead + mtPT, maxComp);
	}

	// Look for the actual minimum starting time for the task
	found = false;
	while (!found && ms != -1) {
		mtHead = this->schedule->taskInfo[ms].head;
		mtPT = this->schedule->taskInfo[ms].task->p;

		if (mtHead.isGreaterEqualTo(Stime + task->p, cpComp))
			found = true;
		else {
			mp = ms;
			Stime = maximum(Stime, mtHead + mtPT, maxComp);
			ms = this->schedule->taskInfo[mp].ms;
		}
	}

	// Update the schedule
	this->schedule->addTask(taskIdx, Stime, ms);
	return Stime;
}





//=============================================================================
//
//	Class FuzzySGS_Append
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
FuzzySGS_Append::FuzzySGS_Append(const ParameterDB *params)
	: maximumLabel(SGS_APPEND_MAXIMUM), tfnMaximum(TFN::M_COMPONENT),
	FuzzySGS(params) {
}


//=====  Copy constructor  ====================================================
FuzzySGS_Append::FuzzySGS_Append(const FuzzySGS_Append &source)
	: maximumLabel(source.maximumLabel),
	tfnMaximum(source.tfnMaximum),
	FuzzySGS(source) { }


//=====  Setup method  ========================================================
void FuzzySGS_Append::setup(const ParameterDB *params) {
	FuzzySGS::setup(params);

	// Load maximum type parameter
	std::string maxName = params->getStringUpper(this->maximumLabel);
	if (maxName.length() == 0) {
		std::string errorMsg = this->maximumLabel + " parameter not found.";
		throw new FJSPException("SGS", errorMsg);
	}
	this->tfnMaximum = TFN::getMaximum(maxName);
	if (this->tfnMaximum == TFN::M_Err) {
		std::string errorMsg = "Invalid value for parameter";
		errorMsg += "\'" + this->maximumLabel + "\': \'";
		errorMsg += maxName + "\'";
		throw new FJSPException("SGS", errorMsg);
	}
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Build schedule  ======================================================
FuzzySchedule * FuzzySGS_Append::buildSchedule(const SharedVars * svars,
	std::vector<int> &order) {

	FuzzyProblem * fuzzyProb =
		dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy Problems.";
		throw new FJSPException("SGS", errorMsg);
	}

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new FuzzySchedule(fuzzyProb);
		this->isCreated = true;
	}

	for (size_t i = 0; i < order.size(); i++) {
		this->scheduleTask((*fuzzyProb)[order[i]], order[i]);
	}

	return this->schedule;
}



//=====  Schedule a task  =====================================================
TFN FuzzySGS_Append::scheduleTask(const FuzzyTask *task,
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
		throw new FJSPException("SGS", errorMsg);
	}

	// Starting time
	TFN Stime = TFN(0, 0, 0);
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
//	Class FuzzySGS_Dense
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
FuzzySGS_Dense::FuzzySGS_Dense(const ParameterDB *params)
	: compareLabel(SGS_DENSE_COMPARISON),
	tfnCompare(TFN::C_EV),
	FuzzySGS_Append(params) {
}


//=====  Copy constructor  ====================================================
FuzzySGS_Dense::FuzzySGS_Dense(const FuzzySGS_Dense &source)
	: compareLabel(source.compareLabel),
	tfnCompare(source.tfnCompare),
	FuzzySGS_Append(source) { }


//=====  Setup method  ========================================================
void FuzzySGS_Dense::setup(const ParameterDB *params) {
	FuzzySGS_Append::setup(params);

	// Load comparison type parameter
	std::string cmpName = params->getStringUpper(this->compareLabel);
	if (cmpName.length() == 0) {
		std::string errorMsg = this->compareLabel + " parameter not found.";
		throw new FJSPException("SGS", errorMsg);
	}
	this->tfnCompare = TFN::getComparison(cmpName);
	if (this->tfnCompare == TFN::C_Err) {
		std::string errorMsg = "Invalid value for parameter";
		errorMsg += "\'" + this->compareLabel + "\': \'";
		errorMsg += cmpName + "\'";
		throw new FJSPException("SGS", errorMsg);
	}
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Build schedule  ======================================================
FuzzySchedule * FuzzySGS_Dense::buildSchedule(const SharedVars * svars,
	std::vector<int> &order) {

	int chosenTask, jobPred, taskIdx;
	const FuzzyTask *task;
	TFN Stime, minS;
	
	unsigned int nTasks = (unsigned int)order.size();
	int nJobs;
	std::vector<int> situation(order.size());

	FuzzyProblem * fuzzyProb =
		dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy Problems.";
		throw new FJSPException("SGS", errorMsg);
	}

	nJobs = fuzzyProb->getNumberJobs();

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new FuzzySchedule(fuzzyProb);
		this->isCreated = true;
	}


	// Check the position of each task in the permutation
	// NOTE: Empirical results show that this speeds up the method
	for (size_t i = 0; i < order.size(); i++)
		situation[i] = i;


	// Look for the task that can be scheduled the earliest
	while (schedule->getScheduledTasks() < nTasks) {
		minS = TFN(Infd, Infd, Infd);
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
//	Class FuzzySGS_fGYT1
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
FuzzySGS_fGYT1::FuzzySGS_fGYT1(const ParameterDB *params)
	: deltaLabel(SGS_GYT_DELTA),
	delta(1.0),
	FuzzySGS_Append(params) {

	this->tfnMaximum = TFN::M_COMPONENT;
}


//=====  Copy constructor  ====================================================
FuzzySGS_fGYT1::FuzzySGS_fGYT1(const FuzzySGS_fGYT1 &source)
	: deltaLabel(source.deltaLabel),
	delta(source.delta),
	FuzzySGS_Append(source) { }



//=====  Setup method  ========================================================
void FuzzySGS_fGYT1::setup(const ParameterDB *params) {
	// Don't call the super-class method. We do not need a maximum
	// in this class
	// FuzzySGS_Append::setup(params);

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
		throw new FJSPException("SGS", errorMsg);
	}

	this->tfnMaximum = TFN::M_COMPONENT;
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Build schedule  ======================================================
FuzzySchedule * FuzzySGS_fGYT1::buildSchedule(const SharedVars * svars,
	std::vector<int> &order) {

	int jobPred, taskIdx, chosenTask;
	const FuzzyTask *task;
	TFN Stime, Ctime, threshold;
	TFN minS, minC;

	unsigned int nTasks = (unsigned int)order.size();
	int nJobs;

	std::vector<unsigned int> situation(nTasks);
	std::vector<int> available;

	FuzzyProblem * fuzzyProb =
		dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy Problems.";
		throw new FJSPException("SGS", errorMsg);
	}

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new FuzzySchedule(fuzzyProb);
		this->isCreated = true;
	}

	// Keep the position of each task in the permutation
	// NOTE: Empirical results show that this speeds up the method
	nJobs = fuzzyProb->getNumberJobs();
	for (size_t i = 0; i < order.size(); i++)
		situation[order[i]] = i;


	while (this->schedule->getScheduledTasks() < nTasks) {
		// Look for the minimum completion time
		minC = TFN(Infd, Infd, Infd);
		minS = TFN(Infd, Infd, Infd);
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

			minS = minimum(minS, Stime, TFN::M_COMPONENT);
			minC = minimum(minC, Ctime, TFN::M_COMPONENT);
			if (!Stime.isGreaterEqualTo(minC, TFN::C_COMPONENT))
				available.push_back(taskIdx);
		}


		// Calculate the threshold to build the conflict set
		threshold = TFN(minS.a + this->delta * (minC.a - minS.a),
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
			if (!Stime.isGreaterEqualTo(threshold, TFN::C_COMPONENT)) {
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
//	Class FuzzySGS_fGYT2
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
FuzzySGS_fGYT2::FuzzySGS_fGYT2(const ParameterDB *params)
	: FuzzySGS_Append(params) {

	this->tfnMaximum = TFN::M_COMPONENT;
}


//=====  Setup method  ========================================================
void FuzzySGS_fGYT2::setup(const ParameterDB *params) {
	// Don't call the super-class method. We do not need a maximum
	// in this class
	// FuzzySGS_Append::setup(params);

	this->tfnMaximum = TFN::M_COMPONENT;
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Build schedule  ======================================================
FuzzySchedule * FuzzySGS_fGYT2::buildSchedule(const SharedVars * svars,
	std::vector<int> &order) {

	int jobPred, taskIdx, chosenTask;
	bool conflict;
	const FuzzyTask *task;
	TFN Stime, Ctime;
	TFN minS, minC;

	unsigned int nTasks = (unsigned int)order.size();
	int nJobs;

	std::vector<unsigned int> situation(nTasks);
	std::vector<unsigned int> available;
	std::vector<TFN> setAstar;

	FuzzyProblem * fuzzyProb =
		dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy Problems.";
		throw new FJSPException("SGS", errorMsg);
	}

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new FuzzySchedule(fuzzyProb);
		this->isCreated = true;
	}

	// Keep the position of each task in the permutation
	// NOTE: Empirical results show that this speeds up the method
	nJobs = fuzzyProb->getNumberJobs();
	for (size_t i = 0; i < order.size(); i++)
		situation[order[i]] = i;


	while (this->schedule->getScheduledTasks() < nTasks) {
		// Look for the minimum completion time
		minC = TFN(Infd, Infd, Infd);
		minS = TFN(Infd, Infd, Infd);
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

			minS = minimum(minS, Stime, TFN::M_COMPONENT);

			// The completion time is the minimum so far
			if (Ctime.isLesserThan(minC, TFN::C_COMPONENT)) {
				minC = Ctime;
				setAstar.clear();
				setAstar.push_back(Ctime);
			}
			// The completion time can donate a component to the minimum
			else if (!Ctime.isGreaterThan(minC, TFN::C_COMPONENT)) {
				minC = minimum(minC, Ctime, TFN::M_COMPONENT);
				setAstar.push_back(Ctime);
			}

			// Update list of available tasks for next step
			if (!Stime.isGreaterEqualTo(minC, TFN::C_COMPONENT))
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
					TFN::C_COMPONENT))
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
