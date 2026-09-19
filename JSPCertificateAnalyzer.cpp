/*
* JSPCertificateAnalyzer.cpp
*
*  Created on: Sep 17, 2026
*      Author: Hernan Diaz Rodriguez
*/
#include "JSPCertificateAnalyzer.h"

namespace PostExecution {

//====  Open the certificate file  ============================================
void JSPCertificateAnalyzer::open(FuzzyFW::Problem *problem,
	std::string outputPrefix, std::string signature) {
	std::string outputName = outputPrefix + signature + "_Certificate.csv";

	this->output.open(outputName.c_str());
	if (!this->output.is_open()) {
		std::string errorMsg = "The certificate file \'" + outputName;
		errorMsg += "\' could not be created.";
		throw PostExecutionException("Certificate", errorMsg);
	}

	this->output << "run;task;job;operation;machine;start;duration;"
		<< "completion" << std::endl;
}

//====  Write one solution  ===================================================
void JSPCertificateAnalyzer::analyze(FuzzyFW::Problem *problem,
	FuzzyFW::Solution *solution, FuzzyFW::Fitness *objective,
	const FuzzyFW::ParameterDB *params, int numRun) {
	JSP::ProblemJSP *problemJSP;
	JSP::ScheduleJSP *schedule;

	problemJSP = dynamic_cast<JSP::ProblemJSP *>(problem);
	if (problemJSP == NULL) {
		std::string errorMsg = "The certificate analyzer can only be used on";
		errorMsg += " interval job shop problems.";
		throw PostExecutionException("Certificate", errorMsg);
	}

	schedule = dynamic_cast<JSP::ScheduleJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "The certificate analyzer can only be used on";
		errorMsg += " interval job shop schedules.";
		throw PostExecutionException("Certificate", errorMsg);
	}

	// The operation index inside its job is not stored in the task, so it is
	// recovered by walking the job chain from the task's predecessors.
	for (unsigned int t = 0; t < problemJSP->getNumberTasks(); t++) {
		const JSP::TaskJSP *task = problemJSP->getTask(t);
		const FuzzyFW::Crisp &start = schedule->taskInfo[t].head;
		FuzzyFW::Crisp completion = start + task->p;
		unsigned int operation = 0;

		for (int pred = task->jp; pred >= 0;
			pred = problemJSP->getTask(pred)->jp)
			operation++;

		// One row per task. Times are exact integers, so the row carries the
		// whole truth about the task and there are no upper endpoints left to
		// append, as there were while durations were intervals.
		this->output << numRun + 1 << ";" << t << ";" << task->job << ";"
			<< operation << ";" << task->machine << ";"
			<< start.v << ";" << task->p.v << ";" << completion.v
			<< std::endl;
	}
}

//====  Close the certificate file  ===========================================
void JSPCertificateAnalyzer::close() {
	if (this->output.is_open())
		this->output.close();
}

}
