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
	IJSP::ProblemIJSP *problemIJSP;
	IJSP::ScheduleIJSP *schedule;

	problemIJSP = dynamic_cast<IJSP::ProblemIJSP *>(problem);
	if (problemIJSP == NULL) {
		std::string errorMsg = "The certificate analyzer can only be used on";
		errorMsg += " interval job shop problems.";
		throw PostExecutionException("Certificate", errorMsg);
	}

	schedule = dynamic_cast<IJSP::ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "The certificate analyzer can only be used on";
		errorMsg += " interval job shop schedules.";
		throw PostExecutionException("Certificate", errorMsg);
	}

	// The operation index inside its job is not stored in the task, so it is
	// recovered by walking the job chain from the task's predecessors.
	for (unsigned int t = 0; t < problemIJSP->getNumberTasks(); t++) {
		const IJSP::TaskIJSP *task = problemIJSP->getTask(t);
		const FuzzyFW::Interval &start = schedule->taskInfo[t].head;
		FuzzyFW::Interval completion = start + task->p;
		unsigned int operation = 0;

		for (int pred = task->jp; pred >= 0;
			pred = problemIJSP->getTask(pred)->jp)
			operation++;

		this->output << numRun + 1 << ";" << t << ";" << task->job << ";"
			<< operation << ";" << task->machine << ";"
			<< start.a << ";" << task->p.a << ";" << completion.a;

		// For a genuine interval instance the upper endpoints differ from the
		// lower ones; append them so the row stays a complete description.
		if (start.a != start.b || task->p.a != task->p.b)
			this->output << ";" << start.b << ";" << task->p.b << ";"
				<< completion.b;

		this->output << std::endl;
	}
}

//====  Close the certificate file  ===========================================
void JSPCertificateAnalyzer::close() {
	if (this->output.is_open())
		this->output.close();
}

}
