/*
* JSPCertificateAnalyzer.h
*
*  Created on: Sep 17, 2026
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include <fstream>
#include "PostExecutionAnalyzer.h"
#include "PostExecutionException.h"
#include "ScheduleJSP.h"

namespace PostExecution {

/**
* Writes a machine-checkable certificate of every solution returned by the
* algorithm: one row per task with its job, machine, start time, duration and
* completion time.
*
* The point of the certificate is that it can be validated WITHOUT running any
* of this code. An external checker only has to verify that (a) tasks of the
* same job do not overlap and respect the job order, (b) tasks sharing a
* machine do not overlap, and (c) the reported makespan is the largest
* completion time. That turns a reported makespan into a claim anybody can
* refute, which is what a result competing against published bounds needs.
*
* Start times are taken from the schedule heads, so the certificate reflects
* exactly what the algorithm built rather than a re-decoding of the genotype.
*
* Intended for crisp (classic JSP) runs, where every processing time is a
* degenerate interval [p, p] and both interval endpoints coincide. Both
* endpoints are written anyway so the file stays meaningful for genuine
* interval instances.
*/
class JSPCertificateAnalyzer : public PostExecutionAnalyzer {
	//=====================================================================
	//		FIELDS
	//=====================================================================
protected:
	std::ofstream output;

	//=====================================================================
	//		METHODS
	//=====================================================================
public:
	void open(FuzzyFW::Problem *problem, std::string outputPrefix,
		std::string signature);

	void analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution *solution,
		FuzzyFW::Fitness *objective, const FuzzyFW::ParameterDB *params,
		int numRun);

	void close();
};

}
