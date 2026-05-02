/*
 * SharedVarsEvolutionary.cpp
 *
 * Destructor defined here so that Encoder and Decoder are complete types
 * when 'delete encoder' / 'delete decoder' are emitted.  The header uses
 * forward declarations to avoid a circular include with Encoder.h.
 */
#include "SharedVarsEvolutionary.h"
#include "Encoder.h"
#include "Decoder.h"

namespace FuzzyFW {

SharedVarsEvolutionary::~SharedVarsEvolutionary() {
	delete encoder;
	delete decoder;
}

}
