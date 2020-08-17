/*
* SturdinessAnalyzer.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#include "MakespanMRAnalyzer.h"

#include <iostream>
using namespace std;


namespace PostExecution {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//====  Default constructor  ==================================================
	MakespanMRAnalyzer::MakespanMRAnalyzer() {}

	void MakespanMRAnalyzer::close() {
		this->writer.close();
	}

	void MakespanMRAnalyzer::analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective, const  FuzzyFW::ParameterDB *params, int numRun) {
		this->analyzeObjectiveFunction(
			this->castProblem(problem),
			this->castSchedule(solution),
			this->castFitness(objective),
			params,
			numRun
		);
	}

	void MakespanMRAnalyzer::open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature) {
		this->writer.open(outputPrefix + signature);
	}

	IJSP::ScheduleIJSP * MakespanMRAnalyzer::castSchedule(FuzzyFW::Solution* solution) {
		IJSP::ScheduleIJSP * schedule =
			dynamic_cast<IJSP::ScheduleIJSP *>(solution);
		if (schedule == NULL) {
			std::string errorMsg = "The Robustness analyzer can be only used on Interval Problems.";
			throw new PostExecutionException("RobustnessAnalyzer", errorMsg);
		}
		return schedule;
	}

	IJSP::ProblemIJSP* MakespanMRAnalyzer::castProblem(FuzzyFW::Problem* problem) {
		IJSP::ProblemIJSP * problemIJSP =
			dynamic_cast<IJSP::ProblemIJSP *>(problem);
		if (problemIJSP == NULL) {
			std::string errorMsg = "The Robustness analyzer can be only used on Interval Problems.";
			throw new PostExecutionException("RobustnessAnalyzer", errorMsg);
		}
		return problemIJSP;
	}

	FuzzyFW::FitnessInterval * MakespanMRAnalyzer::castFitness(FuzzyFW::Fitness* objective) {
		FuzzyFW::FitnessInterval * fitness =
			dynamic_cast<FuzzyFW::FitnessInterval *>(objective);
		if (fitness == NULL) {
			std::string errorMsg = "The Robustness analyzer can only use a IJSP tardiness evaluator to analyze IJSP tardiness.";
			throw new PostExecutionException("RobustnessAnalyzer", errorMsg);
		}
		return fitness;
	}

void MakespanMRAnalyzer::analyzeObjectiveFunction(const IJSP::ProblemIJSP *problemIJSP, IJSP::ScheduleIJSP * schedule, FuzzyFW::FitnessInterval * fitness, const FuzzyFW::ParameterDB *params, int numRun)
{
	this->writer.write("Run " + numRun);
	this->writer.endline();
	this->writer.write("Completion times:");
	this->writer.endline();
	std::vector<std::vector<double>*> tailsDistro(problemIJSP->getNumberTasks());
	std::vector<double> makespanDistro;
	for (unsigned int i = 0; i < problemIJSP->getNumberJobs(); i++) {
		this->writer.write(schedule->getCTJob(i).a);
		this->writer.write(schedule->getCTJob(i).b);
		this->writer.endline();
	}
	for (unsigned int i = 0; i < problemIJSP->getNumberTasks(); i++) {
		this->calculateTailDistribution(schedule->getTaskOrder()[i], schedule, tailsDistro);
	}
	this->writer.write("Task Times Distributions:");
	this->writer.endline();
	for (int i = 0; i < tailsDistro.size(); i++) {
		FuzzyFW::Interval tail = schedule->taskInfo[i].head + schedule->taskInfo[i].task->p;
		this->writer.write(tail.a);
		this->writer.write(tail.b);
		double sum = 0;
		double avg = 0;
		double var = 0;
		for (int j = 0; j < tailsDistro[i]->size(); j++) {
			this->writer.write((*tailsDistro[i])[j]);
			sum += (*tailsDistro[i])[j];
			avg += (*tailsDistro[i])[j] * (tail.a + j);
		}
		for (int j = 0; j < tailsDistro[i]->size(); j++) {
			var += (*tailsDistro[i])[j] * ((tail.a + j) - avg)*((tail.a + j) - avg);
		}
		this->writer.write(sum);
		this->writer.write(avg);
		this->writer.write(var);
		this->writer.endline();
	}
	//Makespan distribution output
	calculateMakespanDistribution(schedule, tailsDistro, makespanDistro);
	this->writer.write("Makespan:");
	this->writer.endline();
	this->writer.write(fitness->getValue().a);
	this->writer.write(fitness->getValue().b);
	double sum = 0;
	double avg = 0;
	double var = 0;
	for (int j = 0; j < makespanDistro.size(); j++) {
		this->writer.write(makespanDistro[j]);
		sum += makespanDistro[j];
		avg += makespanDistro[j] * (fitness->getValue().a + j);
	}
	for (int j = 0; j < makespanDistro.size(); j++) {
		var += makespanDistro[j] * ((fitness->getValue().a + j) - avg)*((fitness->getValue().a + j) - avg);
	}
	this->writer.write(sum);
	this->writer.write(avg);
	this->writer.write(var);
	this->writer.endline();
	this->writer.endline();
}


void MakespanMRAnalyzer::calculateMakespanDistribution(const IJSP::ScheduleIJSP * schedule, const std::vector<std::vector<double>*> & tailsDistro, std::vector<double> & makespanDistro) {
	std::vector<int> lastTasks = orderLastTasks(schedule);
	if (lastTasks.size() == 1) {
		makespanDistro = (*tailsDistro[lastTasks[0]]);
		return;
	}
	else if (lastTasks.size() >= 2) {
		FuzzyFW::Interval tailA = schedule->taskInfo[lastTasks[0]].head + schedule->taskInfo[lastTasks[0]].task->p;
		FuzzyFW::Interval tailB = schedule->taskInfo[lastTasks[1]].head + schedule->taskInfo[lastTasks[1]].task->p;
		makespanDistro = combineDistributions(
			(*tailsDistro[lastTasks[0]]),
			(*tailsDistro[lastTasks[1]]),
			tailA,
			tailB);
		FuzzyFW::Interval makespanTail(max(tailA.a, tailB.a), max(tailA.b, tailB.b));

		for (int i = 2; i < lastTasks.size(); i++) {
			tailB = schedule->taskInfo[lastTasks[i]].head + schedule->taskInfo[lastTasks[i]].task->p;
			makespanDistro = combineDistributions(
				makespanDistro,
				(*tailsDistro[lastTasks[i]]),
				makespanTail,
				tailB);
			makespanTail = FuzzyFW::Interval(max(makespanTail.a, tailB.a), max(makespanTail.b, tailB.b));
		}
	}
}

std::vector<int> MakespanMRAnalyzer::orderLastTasks(const IJSP::ScheduleIJSP * schedule) {
	//ordering criterion >B >A
	std::vector<int> orderedTasks(schedule->lastTaskMachine.size());
	for (int i = 0; i < schedule->lastTaskMachine.size(); i++) {
		bool inserted = false;
		int token = schedule->lastTaskMachine[i];
		for (int j = 0; j < i && !inserted; j++) {
			FuzzyFW::Interval tailA = schedule->taskInfo[orderedTasks[j]].head + schedule->taskInfo[orderedTasks[j]].task->p;
			FuzzyFW::Interval tailB = schedule->taskInfo[token].head + schedule->taskInfo[token].task->p;
				if (tailB.b > tailA.b || (tailB.b == tailA.b) && (tailB.a > tailA.a)){
					int aux = token;
					token = orderedTasks[j];
					orderedTasks[j] = aux;
				}
			}
		orderedTasks[i] = token;
		}
	return orderedTasks;
	}


	void MakespanMRAnalyzer::calculateTailDistribution(int taskId, IJSP::ScheduleIJSP * schedule, std::vector<std::vector<double>*> & tailsDistro){
		FuzzyFW::Interval tail = schedule->taskInfo[taskId].head + schedule->taskInfo[taskId].task->p;
		//cout << endl << "Taskid: " << taskId << endl;
		int mp = schedule->taskInfo[taskId].mp;
		int jp = schedule->taskInfo[taskId].task->jp;
		//there are no predecessors
		if (mp < 0 && jp < 0) {
			tailsDistro[taskId] = new vector<double>(tail.b-tail.a + 1, 1/(tail.b-tail.a +1));
			return;
		}

		std::vector<double> prevtailDist = getPreviousTailDistribution(schedule, taskId, tailsDistro);
		std::vector<double> currentTailDistInit(schedule->taskInfo[taskId].task->p.b - schedule->taskInfo[taskId].task->p.a +1, 1 / (schedule->taskInfo[taskId].task->p.b - schedule->taskInfo[taskId].task->p.a + 1));
		
		tailsDistro[taskId] = new vector<double>(tail.b - tail.a + 1, 0);
		for (int i = 0; i < prevtailDist.size(); i++) {
			for (int j = 0; j < currentTailDistInit.size(); j++) {
				(*tailsDistro[taskId])[i+j] += prevtailDist[i] * currentTailDistInit[j];
			}
		}
	}

	std::vector<double> MakespanMRAnalyzer::combineDistributions(const std::vector<double> & distA,const  std::vector<double> & distB, const FuzzyFW::Interval & tailA, const FuzzyFW::Interval & tailB) {
		//all predecessors > 0, there is no intersection
		if (tailA.a >= tailB.b) {
			return distA;
		}
		if (tailB.a >= tailA.b) {
			return distB;
		}
		//There is intersection
		double maxA = tailB.a > tailA.a ? tailB.a : tailA.a;
		if ((tailB.b > tailA.b) || ((tailB.b == tailA.b) && (tailB.a >= tailA.a))) {
			std::vector<double> prevtailDist(tailB.b - maxA + 1, 0);
			for (int i = 0; i <= tailB.b - tailB.a; i++) {
				for (int j = 0; j <= tailA.b - tailA.a; j++) {
					int val = max(tailB.a + i, tailA.a + j);
					prevtailDist[val - maxA] += distB[i] * distA[j];
				}
			}
			return prevtailDist;
		}
		std::vector<double> prevtailDist(tailA.b - maxA + 1, 0);
		for (int i = 0; i <= tailA.b - tailA.a; i++) {
			for (int j = 0; j <= tailB.b - tailB.a; j++) {
				int val = max(tailA.a + i, tailB.a + j);
				prevtailDist[val - maxA] += distA[i] * distB[j];
			}
		}
		return prevtailDist;
	}

	std::vector<double> MakespanMRAnalyzer::getPreviousTailDistribution(IJSP::ScheduleIJSP * schedule, const int taskId, std::vector<std::vector<double>*> & tailsDistro) {
		FuzzyFW::Interval mpTail;
		std::vector<double>* mpDist = 0;
		FuzzyFW::Interval jpTail;
		std::vector<double>* jpDist = 0;

		int mp = schedule->taskInfo[taskId].mp;
		if (mp >= 0) {
			mpTail = schedule->taskInfo[mp].head + schedule->taskInfo[mp].task->p;
			mpDist = tailsDistro[mp];
		}

		int jp = schedule->taskInfo[taskId].task->jp;
		if (jp >= 0) {
			jpTail = schedule->taskInfo[jp].head + schedule->taskInfo[jp].task->p;
			jpDist = tailsDistro[jp];
		}
		//there is only job predecessor
		if (mp < 0 && jp > 0) {
			return (*jpDist);
		}
		//there is only machine predecessor
		if (mp > 0 && jp < 0) {
			return (*mpDist);
		}
		return this->combineDistributions((*jpDist), (*mpDist), jpTail, mpTail);
	}
	
}