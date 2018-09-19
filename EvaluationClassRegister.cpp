
#include "EvaluationClassRegister.h"

namespace FuzzyFW {

/**
 * Initialize all static variables in GeneticClassRegister
 */
	std::map<std::string, Evaluation*(*)()>	EvaluationClassRegister::EvaluationMap;
	
}
