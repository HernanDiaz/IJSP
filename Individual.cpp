/*
* Individual.cpp
*
*  Created on: June 23, 2017
*      Author: Juan Jose Palacios
*/

#include "Individual.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class Individual
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
Individual::Individual(const int idx)
	: evaluated(false), phenotypeUpdated(false),
	phenotype(NULL), fitness(NULL), id(idx) { }


//-----  Copy constructor  ----------------------------------------------------
Individual::Individual(const Individual &source)
: evaluated(source.evaluated), phenotypeUpdated(source.phenotypeUpdated),
id(source.id) {
	if (source.phenotype != NULL)
		this->phenotype = source.phenotype->clone();
	else
		this->phenotype = NULL;

	if (source.fitness != NULL)
		this->fitness = source.fitness->clone();
	else
		this->fitness = NULL;
}


//-----  Destructor  ----------------------------------------------------------
Individual::~Individual() {
	if (this->fitness != NULL)
		delete this->fitness;
	if (this->phenotype != NULL)
		delete this->phenotype;
}



//=============================================================================
//		GET/SET METHODS
//=============================================================================
//-----  Update Fitness  ------------------------------------------------------
void Individual::updateFitness(Fitness * fitness) {
	if (this->fitness != NULL)
		delete this->fitness;
	this->fitness = fitness;
	this->evaluated = true;
}

//-----  Get Fitness  ---------------------------------------------------------
Fitness * Individual::getFitness() const {
	if(this->evaluated)
		return this->fitness;
	std::string errorMsg = "Acceso al fitness de un individuo que no ha ";
	errorMsg += "evaluado.";
	throw new FuzzyFWException("Fitness", errorMsg);
}


//----- Update phenotype  -----------------------------------------------------
void Individual::updatePhenotype(Solution *solution) {
	if (this->phenotype != NULL)
		delete this->phenotype;
	this->phenotype = solution;
	this->phenotypeUpdated = true;
}



//=============================================================================
//
//	Class IndividualInteger
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
IndividualArrayInt::IndividualArrayInt(const std::vector<int> &genes)
	: Individual() {
	this->genotype = genes;
}


//=====  Copy constructor  ====================================================
IndividualArrayInt::IndividualArrayInt(const IndividualArrayInt &source)
	: Individual(source) {
	this->genotype = source.genotype;
}



//=============================================================================
//		GET/SET METHODS
//=============================================================================
//-----  Set Genotype  --------------------------------------------------------
void IndividualArrayInt::setGenotype(std::vector<int> & genes) {
	this->genotype.clear();
	this->genotype = genes;
	this->evaluated = false;
	this->phenotypeUpdated = false;
}


//-----  Update Genotype  -----------------------------------------------------
void IndividualArrayInt::updateGenotype(std::vector<int> & genes) {
	this->genotype.clear();
	this->genotype = genes;
}


//-----  Set Gene  ------------------------------------------------------------
void IndividualArrayInt::setGene(const unsigned int index, int value) {
	if (index < 0 || index > this->size())
		throw new FuzzyFWException("Individual", "Access to a non-existing gene");
	this->genotype[index] = value;
	this->evaluated = false;
	this->phenotypeUpdated = false;
}


//-----  Get Gene  ------------------------------------------------------------
int IndividualArrayInt::getGene(const unsigned int index) const {
	if (index < 0 || index > this->size())
		throw new FuzzyFWException("Individual", "Access to a non-existing gene");
	return this->genotype[index];
}


//----- Method toString  ------------------------------------------------------
std::string IndividualArrayInt::toString() const {
	std::string str;
	if (this->size() <= 0)
		return "";

	str = valueToString(this->genotype[0]);
	for (size_t i = 1; i < this->genotype.size(); i++)
		str += " " + valueToString(this->genotype[i]);
	return str;
}





//=============================================================================
//		METHODS
//=============================================================================
//-----  Euclidean Distance  --------------------------------------------------
double IndividualArrayInt::euclideanDistance(const Individual *ind) const {
	double diff;
	double sum = 0.0;

	if (ind->getType() != Individual::ARRAY_INTEGER) {
		std::string errorMsg = "Cannot measure the distance between ";
		errorMsg += "individuals of different types";
		throw new FuzzyFWException("Individual", errorMsg);
	}

	const IndividualArrayInt * ind2 =
		dynamic_cast<const IndividualArrayInt *>(ind);

	if (this->genotype.size() != ind2->size()) {
		std::string errorMsg = "Cannot measure the distance between ";
		errorMsg += "individuals of different size";
		throw new FuzzyFWException("Individual", errorMsg);
	}

	for (size_t i = 0; i < this->genotype.size(); i++) {
		diff = (double)(ind2->getGene(i) - this->getGene(i));
		sum += diff * diff;
	}
	return std::sqrt(sum);
}



//-----  Hamming Distance  ----------------------------------------------------
double IndividualArrayInt::hammingDistance(const Individual *ind) const {
	if (ind->getType() != Individual::ARRAY_INTEGER) {
		std::string errorMsg = "Cannot measure the distance between ";
		errorMsg += "individuals of different types";
		throw new FuzzyFWException("Individual", errorMsg);
	}

	const IndividualArrayInt * ind2 =
		dynamic_cast<const IndividualArrayInt *>(ind);

	if (this->genotype.size() != ind2->size())
		return Infd;
	unsigned int count = 0;
	for (unsigned int i = 0; i < this->genotype.size(); i++) {
		if (this->getGene(i) != ind2->getGene(i))
			count++;
	}
	return ((double)count) / this->genotype.size();
}



//-----  Kendall-tau Distance  ------------------------------------------------
double IndividualArrayInt::kendallTauDistance(const Individual *ind) const {
	if (ind->getType() != Individual::ARRAY_INTEGER) {
		std::string errorMsg = "Cannot measure the distance between ";
		errorMsg += "individuals of different types";
		throw new FuzzyFWException("Individual", errorMsg);
	}

	unsigned int count = 0, length, N;
	const IndividualArrayInt * ind2 =
		dynamic_cast<const IndividualArrayInt *>(ind);
	length = this->genotype.size();

	N = length * (length - 1) / 2;
	for (unsigned int i = 0; i < length; i++) {
		for (unsigned int j = i + 1; j < length; j++) {
			if ((this->getGene(i) < this->getGene(j)) && (ind2->getGene(i) > ind2->getGene(j)))
				count++;
			if ((this->getGene(i) > this->getGene(j)) && (ind2->getGene(i) < ind2->getGene(j)))
				count++;
		}
	}
	return ((double)count) / N;
}



}
