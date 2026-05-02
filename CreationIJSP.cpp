/*
* Selection.cpp
*
*  Created on: June 25, 2019
*      Author: hdiaz
*/

#include "CreationIJSP.h"
#include <cmath>
#include <iostream>
#include <fstream>
namespace IJSP {
static bool fiBetter(const FuzzyFW::Interval& a, const FuzzyFW::Interval& b) {
	FuzzyFW::FitnessInterval fa(a, false), fb(b, false);
	return fa.isBetterThan(&fb);
}
static bool fiWorse(const FuzzyFW::Interval& a, const FuzzyFW::Interval& b) {
	FuzzyFW::FitnessInterval fa(a, false), fb(b, false);
	return fa.isWorseThan(&fb);
}


//=============================================================================
//
//	Class CreationRandomSchedule
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Setup method  --------------------------------------------------------
void CreationRandomSchedule::setup(FuzzyFW::ParameterDB *parameters) {
	Creation::setup(parameters);
	this->randomRatio = parameters->getDouble(CREATION_RANDOM_RATIO, 0);

	std::string sgsType = parameters->getStringLower(this->sgsLabel);
	if (sgsType.length() == 0) {
		std::string errorMsg = "SGS not found. Please, specify a SGS to use";
		errorMsg += " during the evaluation of individuals";
		throw IJSPException("Evaluation", errorMsg);
	}
	this->sgs.reset(IJSPClassRegister::getSGSObject(sgsType));
	if (this->sgs == NULL) {
		std::string errorMsg = "The introduced SGS is not";
		errorMsg += " recognised: \'" + sgsType + "\'";
		throw IJSPException("Evaluation", errorMsg);
	}
	this->sgs->setup(parameters);
}

bool CreationRandomSchedule::shouldUseRandom(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	if (this->randomRatio <= 0) return false;
	return svars->rng->getInteger(0, 100) < static_cast<int>(this->randomRatio * 100);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
FuzzyFW::Individual * CreationRandomSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	std::vector<int> permutation, count;
	int rand;
	FuzzyFW::Individual * indiv;

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw IJSPException("Creation", errorMsg);
	}

	// Find the first task of each job
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++)
		if (fuzzyProb->getNumberTasks(i) > 0)
			count.push_back(fuzzyProb->getTaskId(i, 0));

	// Build an array with repetitions
	while (count.size() > 0) {
		rand = svars->rng->getInteger(0, count.size() - 1);
		permutation.push_back(count[rand]);

		// Pass to the next task of the job
		count[rand] = (*fuzzyProb)[count[rand]]->js;
		if (count[rand] < 0) {
			for (size_t i = rand + 1; i < count.size(); i++)
				count[i - 1] = count[i];
			count.pop_back();
		}
	}

	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());

	return indiv;
}


//=============================================================================
//
//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  --------------------------------------------------- //IJSP INTERVAL MAKESPAN
FuzzyFW::Individual * CreationSRTIntervalMkSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	if (shouldUseRandom(svars))
		return CreationRandomSchedule::createIndividual(svars);
	int rand;
	std::vector<int> permutation, count;
	
	FuzzyFW::Individual * indiv;

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw IJSPException("Creation", errorMsg);
	}
		
	// Find the first task of each job and count remaining time for each job
	std::vector<FuzzyFW::Interval> jobRemainingTime(fuzzyProb->getNumberJobs());
	std::vector<int> jobOrderByRemainingTime;
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
    	unsigned int numberOfTasks = fuzzyProb->getNumberTasks(i);
		if (numberOfTasks > 0) {
			count.push_back(fuzzyProb->getTaskId(i, 0));
			//count remaining time for each job
			for (unsigned int j = 0; j < numberOfTasks; j++) {
				jobRemainingTime[i] += fuzzyProb->getTask(fuzzyProb->getTaskId(i, j))->p;
			}
			//Order the jobs from higher to lower remaining time
			int k = 0;
		    while (k < jobOrderByRemainingTime.size() && 
				fiBetter(jobRemainingTime[i], jobRemainingTime[jobOrderByRemainingTime[k]])) {
				k++;
			}
			jobOrderByRemainingTime.insert(jobOrderByRemainingTime.begin()+k,i);
		}
	}
	// Build an array with repetitions
	while (count.size() > 0) {
		int max = 3 < count.size() ? 3 : count.size();
		rand = svars->rng->getInteger(0, max*max - 1);
		int k = int(sqrt(rand));

		//rand = svars->rng->getInteger(0, count.size()*count.size()-1);
		//int k = int(sqrt(rand));
		rand = jobOrderByRemainingTime[k];
		int taskid = count[rand];
		permutation.push_back(taskid);

		// Update remaining time for the job
		jobRemainingTime[rand] -= fuzzyProb->getTask(count[rand])->p;
		// Update order of the jobs from higher to lower remaining time
		while (k < jobOrderByRemainingTime.size()-1 &&
			fiBetter(jobRemainingTime[jobOrderByRemainingTime[k]], jobRemainingTime[jobOrderByRemainingTime[k+1]])) {
			int aux = jobOrderByRemainingTime[k];
			jobOrderByRemainingTime[k] = jobOrderByRemainingTime[k + 1];
			jobOrderByRemainingTime[k + 1] = aux;
			k++;
		}
		// Pass to the next task of the job
		count[rand] = (*fuzzyProb)[count[rand]]->js;
		if (count[rand] < 0) {
			for (size_t i = rand + 1; i < count.size(); i++) {
				count[i - 1] = count[i];
				jobRemainingTime[i - 1] = jobRemainingTime[i];
			}
			for (size_t i = 0; i < jobOrderByRemainingTime.size(); i++) {
				if (jobOrderByRemainingTime[i] > rand) {
					jobOrderByRemainingTime[i]--;
				}
			}
			jobRemainingTime.pop_back();
			jobOrderByRemainingTime.pop_back();
			count.pop_back();
		}
		
	}
	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());

	return indiv;
}

//=============================================================================
//
//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  --------------------------------------------------- //IJSP INTERVAL MAKESPAN
FuzzyFW::Individual * CreationLRTFIntervalMkSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	if (shouldUseRandom(svars))
		return CreationRandomSchedule::createIndividual(svars);
	int rand;
	//debug file
	std::vector<int> permutation, count;
	//std::string finalOutputName = "LRTFdebug.txt";
	//std::ofstream outputFile;
	//outputFile.open(finalOutputName.c_str(), std::ios_base::ate);
	FuzzyFW::Individual * indiv;

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw IJSPException("Creation", errorMsg);
	}

	// Find the first task of each job and count remaining time for each job
	std::vector<FuzzyFW::Interval> jobRemainingTime(fuzzyProb->getNumberJobs());
	std::vector<int> jobOrderByRemainingTime;
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		unsigned int numberOfTasks = fuzzyProb->getNumberTasks(i);
		if (numberOfTasks > 0) {
			count.push_back(fuzzyProb->getTaskId(i, 0));
			//count remaining time for each job
			for (unsigned int j = 0; j < numberOfTasks; j++) {
				jobRemainingTime[i] += fuzzyProb->getTask(fuzzyProb->getTaskId(i, j))->p;
			}
			//Order the jobs from higher to lower remaining time
			int k = 0;
			while (k < jobOrderByRemainingTime.size() &&
				fiBetter(jobRemainingTime[i], jobRemainingTime[jobOrderByRemainingTime[k]])) {
				k++;
			}
			jobOrderByRemainingTime.insert(jobOrderByRemainingTime.begin() + k, i);
		}
	}
	/*outputFile << "Creating individual: \n";
	outputFile << "Remaining time of each job: \n";
	for (unsigned int j = 0; j < jobRemainingTime.size(); j++) {
		outputFile << jobRemainingTime[j]<<" ";
	}
	outputFile << "Remaining time (MP) of each job: \n";
	for (unsigned int j = 0; j < jobRemainingTime.size(); j++) {
		outputFile << (jobRemainingTime[j].a+jobRemainingTime[j].b)/2 << " ";
	}
	outputFile << std::endl;
	outputFile << "Job order by remaining time: \n";
	for (unsigned int j = 0; j < jobOrderByRemainingTime.size(); j++) {
		outputFile << jobOrderByRemainingTime[j] << " ";
	}
	outputFile << std::endl;
	outputFile << "Ordered: \n";
	for (unsigned int j = 0; j < jobOrderByRemainingTime.size(); j++) {
		outputFile << (jobRemainingTime[jobOrderByRemainingTime[j]].a + jobRemainingTime[jobOrderByRemainingTime[j]].b) / 2 << " ";
	}
	outputFile << std::endl;
	outputFile << "Task ids: \n";
	for (unsigned int j = 0; j < count.size(); j++) {
		outputFile << count[j] <<" ";
	}
	outputFile << std::endl;*/
	// Build an array with repetitions
	while (count.size() > 0) {
		int max = 3 < count.size() ? 3 : count.size();
		//rand = svars->rng->getInteger(0, count.size()*count.size() - 1);
		//int k = count.size() - int(sqrt(rand)) - 1;
		rand = svars->rng->getInteger(0, max*max - 1);
		int k = max - int(sqrt(rand)) - 1;
		//outputFile << "Index selected: "<< k <<" \n";;
		rand = jobOrderByRemainingTime[k]; //renombrar rand para jobId en esta linea
		//outputFile << "job id: " << rand << " \n";;
		int taskid = count[rand];
		//outputFile << "task id: " << taskid << " \n";;
		permutation.push_back(taskid);

		// Update remaining time for the job
		jobRemainingTime[rand] -= fuzzyProb->getTask(count[rand])->p;
		/*
		outputFile << "Resta:"<< (fuzzyProb->getTask(count[rand])->p.a+ fuzzyProb->getTask(count[rand])->p.b)/2 <<" \n";
		for (unsigned int j = 0; j < jobRemainingTime.size(); j++) {
			outputFile << jobRemainingTime[j] << " ";
		}*/

		// Update order of the jobs from higher to lower remaining time
		while (k < jobOrderByRemainingTime.size() - 1 &&
			fiBetter(jobRemainingTime[jobOrderByRemainingTime[k]], jobRemainingTime[jobOrderByRemainingTime[k + 1]])) {
			int aux = jobOrderByRemainingTime[k];
			jobOrderByRemainingTime[k] = jobOrderByRemainingTime[k + 1];
			jobOrderByRemainingTime[k + 1] = aux;
			k++;
		}
		// Pass to the next task of the job
		count[rand] = (*fuzzyProb)[count[rand]]->js;
		if (count[rand] < 0) {
			for (size_t i = rand + 1; i < count.size(); i++) {
				count[i - 1] = count[i];
				jobRemainingTime[i - 1] = jobRemainingTime[i];
			}
			for (size_t i = 0; i < jobOrderByRemainingTime.size(); i++) {
				if (jobOrderByRemainingTime[i] > rand) {
					jobOrderByRemainingTime[i]--;
				}
			}
			jobRemainingTime.pop_back();
			jobOrderByRemainingTime.pop_back();
			count.pop_back();
		}

		/*
		outputFile << "Remaining time of each job: \n";
		for (unsigned int j = 0; j < jobRemainingTime.size(); j++) {
			outputFile << jobRemainingTime[j] << " ";
		}
		outputFile << std::endl;
		outputFile << "Job order by remaining time: \n";
		for (unsigned int j = 0; j < jobOrderByRemainingTime.size(); j++) {
			outputFile << jobOrderByRemainingTime[j] << " ";
		}
		outputFile << std::endl;
		outputFile << "Remaining time (MP) of each job: \n";
		for (unsigned int j = 0; j < jobRemainingTime.size(); j++) {
			outputFile << (jobRemainingTime[j].a + jobRemainingTime[j].b) / 2 << " ";
		}
		outputFile << std::endl;
		outputFile << "Ordered: \n";
		for (unsigned int j = 0; j < jobOrderByRemainingTime.size(); j++) {
			outputFile << (jobRemainingTime[jobOrderByRemainingTime[j]].a + jobRemainingTime[jobOrderByRemainingTime[j]].b) / 2 << " ";
		}
		outputFile << std::endl;
		outputFile << "Task ids: \n";
		for (unsigned int j = 0; j < count.size(); j++) {
			outputFile << count[j] << " ";
		}
		outputFile << std::endl;*/
	}
	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());
	//outputFile << "End \n";
	//outputFile.close();
	return indiv;
}


//=============================================================================
//
//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  --------------------------------------------------- //IJSP INTERVAL MAKESPAN
FuzzyFW::Individual * CreationLRTFInverseIntervalMkSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	if (shouldUseRandom(svars))
		return CreationRandomSchedule::createIndividual(svars);
	int rand;
	std::vector<int> permutation, count;

	FuzzyFW::Individual * indiv;

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw IJSPException("Creation", errorMsg);
	}

	// Find the first task of each job and count remaining time for each job
	std::vector<FuzzyFW::Interval> jobRemainingTime(fuzzyProb->getNumberJobs());
	std::vector<int> jobOrderByRemainingTime;
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		unsigned int numberOfTasks = fuzzyProb->getNumberTasks(i);
		if (numberOfTasks > 0) {
			count.push_back(fuzzyProb->getTaskId(i, 0));
			//count remaining time for each job
			for (unsigned int j = 0; j < numberOfTasks; j++) {
				jobRemainingTime[i] += fuzzyProb->getTask(fuzzyProb->getTaskId(i, j))->p;
			}
			//Order the jobs from higher to lower remaining time
			int k = 0;
			while (k < jobOrderByRemainingTime.size() &&
				fiBetter(jobRemainingTime[i], jobRemainingTime[jobOrderByRemainingTime[k]])) {
				k++;
			}
			jobOrderByRemainingTime.insert(jobOrderByRemainingTime.begin() + k, i);
		}
	}
	// Build an array with repetitions
	while (count.size() > 0) {
		int max = 3 < count.size() ? 3 : count.size();
		rand = svars->rng->getInteger(0, max*max - 1);
		int k = int(sqrt(rand));
		//rand = svars->rng->getInteger(0, count.size()*count.size() - 1);
		//int k = int(sqrt(rand));
		rand = jobOrderByRemainingTime[k]; //renombrar rand para jobId en esta linea
		int taskid = count[rand];
		permutation.push_back(taskid);

		// Update remaining time for the job
		jobRemainingTime[rand] -= fuzzyProb->getTask(count[rand])->p;
		// Update order of the jobs from higher to lower remaining time
		while (k < jobOrderByRemainingTime.size() - 1 &&
			fiBetter(jobRemainingTime[jobOrderByRemainingTime[k]], jobRemainingTime[jobOrderByRemainingTime[k + 1]])) {
			int aux = jobOrderByRemainingTime[k];
			jobOrderByRemainingTime[k] = jobOrderByRemainingTime[k + 1];
			jobOrderByRemainingTime[k + 1] = aux;
			k++;
		}
		// Pass to the next task of the job
		count[rand] = (*fuzzyProb)[count[rand]]->js;
		if (count[rand] < 0) {
			for (size_t i = rand + 1; i < count.size(); i++) {
				count[i - 1] = count[i];
				jobRemainingTime[i - 1] = jobRemainingTime[i];
			}
			for (size_t i = 0; i < jobOrderByRemainingTime.size(); i++) {
				if (jobOrderByRemainingTime[i] > rand) {
					jobOrderByRemainingTime[i]--;
				}
			}
			jobRemainingTime.pop_back();
			jobOrderByRemainingTime.pop_back();
			count.pop_back();
		}

	}
	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());

	return indiv;
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  --------------------------------------------------- //IJSP INTERVAL MAKESPAN
FuzzyFW::Individual * CreationSNTFIntervalMkSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	if (shouldUseRandom(svars))
		return CreationRandomSchedule::createIndividual(svars);
	int rand;
	std::vector<int> permutation, count;

	FuzzyFW::Individual * indiv;

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw IJSPException("Creation", errorMsg);
	}

	// Find the first task of each job and count remaining time for each job
	std::vector<FuzzyFW::Interval> nextTaskTime(fuzzyProb->getNumberJobs());
	std::vector<int> jobOrderByRemainingTime;
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		unsigned int numberOfTasks = fuzzyProb->getNumberTasks(i);
		if (numberOfTasks > 0) {
			count.push_back(fuzzyProb->getTaskId(i, 0));
			//count first task duration for each job
			nextTaskTime[i] += fuzzyProb->getTask(fuzzyProb->getTaskId(i, 0))->p;
			
			//Order the jobs from higher to lower first task time
			int k = 0;
			while (k < jobOrderByRemainingTime.size() &&
				fiBetter(nextTaskTime[i], nextTaskTime[jobOrderByRemainingTime[k]])) {
				k++;
			}
			jobOrderByRemainingTime.insert(jobOrderByRemainingTime.begin() + k, i);
		}
	}
	// Build an array with repetitions
	while (count.size() > 0) {
		int max = 3 < count.size() ? 3 : count.size();
		//rand = svars->rng->getInteger(0, count.size()*count.size() - 1);
		//int k = count.size() - int(sqrt(rand)) - 1;
		rand = svars->rng->getInteger(0, max*max - 1);
		int k = max - int(sqrt(rand)) - 1;
		//std::cout <<"k= "<< k << std::endl;
		rand = jobOrderByRemainingTime[k];
		//std::cout << "jobOrderByRemainingTime" << std::endl;
		//for (int i = 0; i < jobOrderByRemainingTime.size(); i++) {
		//	std::cout << jobOrderByRemainingTime[i] << " ";
		//}

		//std::cout << std::endl;
		//std::cout << "rand= " << rand << std::endl;
		int taskid = count[rand];
		permutation.push_back(taskid);

		
		// Pass to the next task of the job
		
		//std::cout <<" count.size = "<< count.size() << std::endl;
		//std::cout << "Count= " << std::endl;
		//for (int i = 0; i < count.size(); i++) {
		//	std::cout << count[i] << " ";
		//}
		//std::cout << std::endl;
		
		//std::cout <<"count[rand]= "<< count[rand] << std::endl;
		//std::cout << (*fuzzyProb)[count[rand]]->js << std::endl;
		count[rand] = (*fuzzyProb)[count[rand]]->js;
		if (count[rand] < 0) {
			for (size_t i = rand + 1; i < count.size(); i++) {
				count[i - 1] = count[i];
				nextTaskTime[i - 1] = nextTaskTime[i];
			}
			for (size_t i = 0; i < jobOrderByRemainingTime.size(); i++) {
				if (jobOrderByRemainingTime[i] > rand) {
					jobOrderByRemainingTime[i]--;
				}
			}
			nextTaskTime.pop_back();
			jobOrderByRemainingTime.erase(jobOrderByRemainingTime.begin() + k);
			count.pop_back();
		}
		else {
			// Update next task processing time
			nextTaskTime[rand] = (*fuzzyProb)[count[rand]]->p;
			// Update order of the jobs from higher to lower remaining time
			int job = jobOrderByRemainingTime[k];
			//std::cout << "Borrando trabajo " << job <<" en la posicicion "<<k<<std::endl;
			jobOrderByRemainingTime.erase(jobOrderByRemainingTime.begin()+k);
			//std::cout << "jobOrderByRemainingTime borrado" << std::endl;
			//for (int i = 0; i < jobOrderByRemainingTime.size(); i++) {
			//	std::cout << jobOrderByRemainingTime[i] << " ";
			//}
			//std::cout << std::endl;
			//Order the jobs from higher to lower first task time
			int l = 0;
			while (l < jobOrderByRemainingTime.size() &&
				fiBetter(nextTaskTime[job], nextTaskTime[jobOrderByRemainingTime[l]])) {
				l++;
			}
			jobOrderByRemainingTime.insert(jobOrderByRemainingTime.begin() + l, job);
			//std::cout << "jobOrderByRemainingTime insertado" << std::endl;
			//for (int i = 0; i < jobOrderByRemainingTime.size(); i++) {
			//	std::cout << jobOrderByRemainingTime[i] << " ";
			//}
			//std::cout << std::endl;
		}

	}
	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());

	return indiv;
}

//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
FuzzyFW::Individual * CreationSCTFIntervalMkSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	if (shouldUseRandom(svars))
		return CreationRandomSchedule::createIndividual(svars);
	int rand;
	std::vector<int> permutation, count;

	FuzzyFW::Individual * indiv;

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw IJSPException("Creation", errorMsg);
	}
	std::vector<FuzzyFW::Interval> mMkspan(fuzzyProb->getNumberMachines());
	std::vector<FuzzyFW::Interval> jMkspan(fuzzyProb->getNumberJobs());

	// Find the first task of each job and count remaining time for each job
	std::vector<FuzzyFW::Interval> jobRemainingTime(fuzzyProb->getNumberJobs());
	std::vector<int> jobOrderByRemainingTime;
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		unsigned int numberOfTasks = fuzzyProb->getNumberTasks(i);
		if (numberOfTasks > 0) {
			count.push_back(fuzzyProb->getTaskId(i, 0));
			for (unsigned int j = 0; j < numberOfTasks; j++) {
				jobRemainingTime[i] += fuzzyProb->getTask(fuzzyProb->getTaskId(i, j))->p;
			}
			int k = 0;
			while (k < jobOrderByRemainingTime.size() &&
				fiBetter(jobRemainingTime[i], jobRemainingTime[jobOrderByRemainingTime[k]])) {
				k++;
			}
			jobOrderByRemainingTime.insert(jobOrderByRemainingTime.begin() + k, i);
		}
	}
	while (count.size() > 0) {
		int max = 3 < count.size() ? 3 : count.size();
		rand = svars->rng->getInteger(0, max*max - 1);
		int k = int(sqrt(rand));
		rand = jobOrderByRemainingTime[k];
		int taskid = count[rand];
		permutation.push_back(taskid);

		jobRemainingTime[rand] -= fuzzyProb->getTask(taskid)->p;

		const IJSP::TaskIJSP * task = fuzzyProb->getTask(taskid);
		FuzzyFW::Interval localMaxMkspan = maximum(mMkspan[task->machine], jMkspan[task->job], FuzzyFW::Interval::M_COMPONENT) + task->p;
		mMkspan[task->machine] = localMaxMkspan;
		jMkspan[task->job] = localMaxMkspan;

		while (k < jobOrderByRemainingTime.size() - 1 &&
			fiBetter(jobRemainingTime[jobOrderByRemainingTime[k]] + jMkspan[jobOrderByRemainingTime[k]], jobRemainingTime[jobOrderByRemainingTime[k + 1]] + jMkspan[jobOrderByRemainingTime[k + 1]])) {
			int aux = jobOrderByRemainingTime[k];
			jobOrderByRemainingTime[k] = jobOrderByRemainingTime[k + 1];
			jobOrderByRemainingTime[k + 1] = aux;
			k++;
		}

		count[rand] = (*fuzzyProb)[count[rand]]->js;
		if (count[rand] < 0) {
			for (size_t i = rand + 1; i < count.size(); i++) {
				count[i - 1] = count[i];
				jobRemainingTime[i - 1] = jobRemainingTime[i];
			}
			for (size_t i = 0; i < jobOrderByRemainingTime.size(); i++) {
				if (jobOrderByRemainingTime[i] > rand) {
					jobOrderByRemainingTime[i]--;
				}
			}
			jobRemainingTime.pop_back();
			jobOrderByRemainingTime.erase(jobOrderByRemainingTime.begin() + k);
			jMkspan.erase(jMkspan.begin() + k);
			mMkspan.erase(mMkspan.begin() + k);
			count.pop_back();
		}
	}
	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());

	return indiv;
}

//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
FuzzyFW::Individual * CreationLCTFIntervalMkSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	if (shouldUseRandom(svars))
		return CreationRandomSchedule::createIndividual(svars);
	int rand;
	std::vector<int> permutation, count;

	FuzzyFW::Individual * indiv;

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw IJSPException("Creation", errorMsg);
	}
	std::vector<FuzzyFW::Interval> mMkspan(fuzzyProb->getNumberMachines());
	std::vector<FuzzyFW::Interval> jMkspan(fuzzyProb->getNumberJobs());

	// Find the first task of each job and count remaining time for each job
	std::vector<FuzzyFW::Interval> jobRemainingTime(fuzzyProb->getNumberJobs());
	std::vector<int> jobOrderByRemainingTime;
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		unsigned int numberOfTasks = fuzzyProb->getNumberTasks(i);
		if (numberOfTasks > 0) {
			count.push_back(fuzzyProb->getTaskId(i, 0));
			for (unsigned int j = 0; j < numberOfTasks; j++) {
				jobRemainingTime[i] += fuzzyProb->getTask(fuzzyProb->getTaskId(i, j))->p;
			}
			int k = 0;
			while (k < jobOrderByRemainingTime.size() &&
				fiBetter(jobRemainingTime[i], jobRemainingTime[jobOrderByRemainingTime[k]])) {
				k++;
			}
			jobOrderByRemainingTime.insert(jobOrderByRemainingTime.begin() + k, i);
		}
	}
	while (count.size() > 0) {
		int max = 3 < count.size() ? 3 : count.size();
		rand = svars->rng->getInteger(0, max*max - 1);
		int k = max - int(sqrt(rand)) - 1;
		rand = jobOrderByRemainingTime[k];
		int taskid = count[rand];
		permutation.push_back(taskid);

		jobRemainingTime[rand] -= fuzzyProb->getTask(taskid)->p;

		const IJSP::TaskIJSP * task = fuzzyProb->getTask(taskid);
		FuzzyFW::Interval localMaxMkspan = maximum(mMkspan[task->machine], jMkspan[task->job], FuzzyFW::Interval::M_COMPONENT) + task->p;
		mMkspan[task->machine] = localMaxMkspan;
		jMkspan[task->job] = localMaxMkspan;

		while (k < jobOrderByRemainingTime.size() - 1 &&
			fiBetter(jobRemainingTime[jobOrderByRemainingTime[k]] + jMkspan[jobOrderByRemainingTime[k]], jobRemainingTime[jobOrderByRemainingTime[k + 1]] + jMkspan[jobOrderByRemainingTime[k + 1]])) {
			int aux = jobOrderByRemainingTime[k];
			jobOrderByRemainingTime[k] = jobOrderByRemainingTime[k + 1];
			jobOrderByRemainingTime[k + 1] = aux;
			k++;
		}

		count[rand] = (*fuzzyProb)[count[rand]]->js;
		if (count[rand] < 0) {
			for (size_t i = rand + 1; i < count.size(); i++) {
				count[i - 1] = count[i];
				jobRemainingTime[i - 1] = jobRemainingTime[i];
			}
			for (size_t i = 0; i < jobOrderByRemainingTime.size(); i++) {
				if (jobOrderByRemainingTime[i] > rand) {
					jobOrderByRemainingTime[i]--;
				}
			}
			jobRemainingTime.pop_back();
			jobOrderByRemainingTime.erase(jobOrderByRemainingTime.begin() + k);
			jMkspan.erase(jMkspan.begin() + k);
			mMkspan.erase(mMkspan.begin() + k);
			count.pop_back();
		}
	}
	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());

	return indiv;
}

//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
FuzzyFW::Individual * CreationSPJFIntervalMkSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	if (shouldUseRandom(svars))
		return CreationRandomSchedule::createIndividual(svars);
	int rand;
	std::vector<int> permutation, count;

	FuzzyFW::Individual * indiv;

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw IJSPException("Creation", errorMsg);
	}
	std::vector<FuzzyFW::Interval> mMkspan(fuzzyProb->getNumberMachines());
	std::vector<FuzzyFW::Interval> jMkspan(fuzzyProb->getNumberJobs());

	// Find the first task of each job and count remaining time for each job
	std::vector<int> jobOrderByRemainingTime(fuzzyProb->getNumberJobs());
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		unsigned int numberOfTasks = fuzzyProb->getNumberTasks(i);
		if (numberOfTasks > 0) {
			count.push_back(fuzzyProb->getTaskId(i, 0));
		}
		jobOrderByRemainingTime[i] = i;
	}
	// Build an array with repetitions

	while (count.size() > 0) {
		//Seleccionar aleatorio al principio
		int max = 3 < count.size() ? 3 : count.size();
		//rand = svars->rng->getInteger(0, count.size()*count.size() - 1);
		//int k = count.size() - int(sqrt(rand)) - 1;
		rand = svars->rng->getInteger(0, max*max - 1);
		int k = max - int(sqrt(rand)) - 1;

		rand = jobOrderByRemainingTime[k];
		/*std::cout << "jobOrderByRemainingTime" << std::endl;
		for (int i = 0; i < jobOrderByRemainingTime.size(); i++) {
				std::cout << jobOrderByRemainingTime[i] << " ";
		}
		std::cout << std::endl;
		std::cout << "rand= " << rand << std::endl;*/
		int taskid = count[rand];
		permutation.push_back(taskid);
		/*std::cout <<" count.size = "<< count.size() << std::endl;
	    std::cout << "Count= " << std::endl;
	    for (int i = 0; i < count.size(); i++) {
	    	std::cout << count[i] << " ";
	    }
	    std::cout << std::endl;*/

		// Update machine and job makespan.
		const IJSP::TaskIJSP * task = fuzzyProb->getTask(taskid);
		FuzzyFW::Interval localMaxMkspan = maximum(mMkspan[task->machine], jMkspan[task->job], FuzzyFW::Interval::M_COMPONENT) + task->p;
		mMkspan[task->machine] = localMaxMkspan;
		jMkspan[task->job] = localMaxMkspan;

		//Order from better to worst
		while (k < jobOrderByRemainingTime.size() - 1 &&
			fiWorse(jMkspan[jobOrderByRemainingTime[k]], jMkspan[jobOrderByRemainingTime[k + 1]])) {
			int aux = jobOrderByRemainingTime[k];
			jobOrderByRemainingTime[k] = jobOrderByRemainingTime[k + 1];
			jobOrderByRemainingTime[k + 1] = aux;
			k++;
		}

		/*std::cout << "jobOrderByRemainingTime ordenado" << std::endl;
		for (int i = 0; i < jobOrderByRemainingTime.size(); i++) {
			std::cout << jobOrderByRemainingTime[i] << " ";
		}
		std::cout << std::endl;*/

		// Pass to the next task of the job
		count[rand] = (*fuzzyProb)[count[rand]]->js;
		/*std::cout <<"count[rand]= "<< count[rand] << std::endl;
		if (count[rand] > 0) {
			std::cout << (*fuzzyProb)[count[rand]]->js << std::endl;
		}
		else std::cout << "(count[rand] < 0)" << std::endl;*/


		if (count[rand] < 0) {
			for (size_t i = rand + 1; i < count.size(); i++) {
				count[i - 1] = count[i];
			}
			for (size_t i = 0; i < jobOrderByRemainingTime.size(); i++) {
				if (jobOrderByRemainingTime[i] > rand) {
					jobOrderByRemainingTime[i]--;
				}
			}
			int job = jobOrderByRemainingTime[k];
			//std::cout << "Borrando trabajo " << job << " en la posicicion " << k << std::endl;
			jobOrderByRemainingTime.erase(jobOrderByRemainingTime.begin() + k);
			/*std::cout << "jobOrderByRemainingTime borrado" << std::endl;
			for (int i = 0; i < jobOrderByRemainingTime.size(); i++) {
				std::cout << jobOrderByRemainingTime[i] << " ";
			}*/
			jMkspan.erase(jMkspan.begin() + k);
			mMkspan.erase(mMkspan.begin() + k);
			count.pop_back();
		}

	}
	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());

	return indiv;
}

//=============================================================================
//
//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
FuzzyFW::Individual * CreationSPJFInverseIntervalMkSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	if (shouldUseRandom(svars))
		return CreationRandomSchedule::createIndividual(svars);
	int rand;
	std::vector<int> permutation, count;

	FuzzyFW::Individual * indiv;

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw IJSPException("Creation", errorMsg);
	}
	std::vector<FuzzyFW::Interval> mMkspan(fuzzyProb->getNumberMachines());
	std::vector<FuzzyFW::Interval> jMkspan(fuzzyProb->getNumberJobs());

	// Find the first task of each job and count remaining time for each job
	std::vector<int> jobOrderByRemainingTime(fuzzyProb->getNumberJobs());
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		unsigned int numberOfTasks = fuzzyProb->getNumberTasks(i);
		if (numberOfTasks > 0) {
			count.push_back(fuzzyProb->getTaskId(i, 0));
		}
		jobOrderByRemainingTime[i] = i;
	}
	// Build an array with repetitions

	while (count.size() > 0) {
		//Seleccionar aleatorio al principio
		//rand = svars->rng->getInteger(0, count.size()*count.size() - 1);
		//int k = int(sqrt(rand));

		int max = 3 < count.size() ? 3 : count.size();
		rand = svars->rng->getInteger(0, max*max - 1);
		int k = int(sqrt(rand));

		rand = jobOrderByRemainingTime[k];
		int taskid = count[rand];
		permutation.push_back(taskid);

		// Update machine and job makespan.
		const IJSP::TaskIJSP * task = fuzzyProb->getTask(taskid);
		FuzzyFW::Interval localMaxMkspan = maximum(mMkspan[task->machine], jMkspan[task->job], FuzzyFW::Interval::M_COMPONENT) + task->p;
		mMkspan[task->machine] = localMaxMkspan;
		jMkspan[task->job] = localMaxMkspan;

		//Order from better to worst
		while (k < jobOrderByRemainingTime.size() - 1 &&
			fiWorse(jMkspan[jobOrderByRemainingTime[k]], jMkspan[jobOrderByRemainingTime[k + 1]])) {
			int aux = jobOrderByRemainingTime[k];
			jobOrderByRemainingTime[k] = jobOrderByRemainingTime[k + 1];
			jobOrderByRemainingTime[k + 1] = aux;
			k++;
		}
		
		// Pass to the next task of the job
		count[rand] = (*fuzzyProb)[count[rand]]->js;

		if (count[rand] < 0) {
			for (size_t i = rand + 1; i < count.size(); i++) {
				count[i - 1] = count[i];
			}
			for (size_t i = 0; i < jobOrderByRemainingTime.size(); i++) {
				if (jobOrderByRemainingTime[i] > rand) {
					jobOrderByRemainingTime[i]--;
				}
			}
			int job = jobOrderByRemainingTime[k];
			jobOrderByRemainingTime.erase(jobOrderByRemainingTime.begin() + k);
			jMkspan.erase(jMkspan.begin() + k);
			mMkspan.erase(mMkspan.begin() + k);
			count.pop_back();
		}

	}
	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());

	return indiv;
}


//=============================================================================
//
//	Class CreationManagerIntervalMkSchedule
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Setup method  --------------------------------------------------------
void CreationManagerIntervalMkSchedule::setup(FuzzyFW::ParameterDB *parameters) {
	CreationRandomSchedule::setup(parameters);
	this->SPJFSchedule.setup(parameters);
	this->LRTFSchedule.setup(parameters);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
FuzzyFW::Individual * CreationManagerIntervalMkSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	int rand = svars->rng->getInteger(0, 100);
	if (rand <= 50)
		return CreationRandomSchedule::createIndividual(svars);
	if (rand % 2)
		return this->LRTFSchedule.createIndividual(svars);
	return this->SPJFSchedule.createIndividual(svars);
}
}
