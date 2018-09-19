
#include "GeneticClassRegister.h"

namespace FJSP {

/**
 * Initialize all static variables in GeneticClassRegister
 */
	std::map<std::string, Encoder*(*)()>	GeneticClassRegister::EncoderMap;
	std::map<std::string, Decoder*(*)()>	GeneticClassRegister::DecoderMap;
	std::map<std::string, Creation*(*)()>	GeneticClassRegister::CreationMap;
	std::map<std::string, Crossover*(*)()>	GeneticClassRegister::CrossoverMap;
	std::map<std::string, Mutation*(*)()>	GeneticClassRegister::MutationMap;
	std::map<std::string, Selection*(*)()>	GeneticClassRegister::SelectionMap;
	std::map<std::string, Replacement*(*)()>	GeneticClassRegister::ReplacementMap;

}
