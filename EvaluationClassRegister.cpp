
#include "EvaluationClassRegister.h"

namespace FJSP {

/**
 * Initialize all static variables in GeneticClassRegister
 */
	std::map<std::string, Evaluation*(*)()>	EvaluationClassRegister::EvaluationMap;
	
}
