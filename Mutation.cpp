/*
 * Mutation.cpp
 *
 *  Created on: Jul 27, 2015
 */

#include "Mutation.h"

namespace FuzzyFW {

//=============================================================================
//
//	Abstract class Mutation
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Apply (Population)  --------------------------------------------------
void Mutation::apply(Population *population, const  double probability,
	const SharedVars *svars) const {
	for(unsigned int i=0; i < population->size(); i++) {
		if(svars->rng->getProbability() < probability)
			apply(population->getIndividual(i), svars);
	}
}





//=============================================================================
//
//	Class MutationInsertion
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Apply (Individual)  --------------------------------------------------
void MutationInsertion::apply(Individual *individual,
		const SharedVars *svars) const {

	unsigned int origin, destination;
	IndividualArrayInt *ind;

	// Check that the length of genotype is enough
	if (individual->size() < 2)
		return;

	unsigned int nGenes = individual->size();
	
	// Choose the points to apply the mutation
	origin = destination = svars->rng->getInteger(0, nGenes - 1);
	while (origin == destination)
		destination = svars->rng->getInteger(0, nGenes - 1);

	// If the individual is made of integer values
	ind = dynamic_cast<IndividualArrayInt *>(individual);
	if(ind != NULL) {
		int gene = ind->getGene(origin);
		if (origin > destination) {
			for (unsigned int i = origin; i > destination; i--)
				ind->setGene(i, ind->getGene(i - 1));
		}
		else {
			for (unsigned int i = origin; i < destination; i++)
				ind->setGene(i, ind->getGene(i + 1));
		}
		ind->setGene(destination, gene);
	}
	else {
		std::string errorMsg = "Type of genotype not recognised";
		errorMsg += " by this operator";
		throw FuzzyFWException("Mutation", errorMsg);
	}
}





//=============================================================================
//
//	Class MutationSwap
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Apply (Individual)  --------------------------------------------------
void MutationSwap::apply(Individual *individual,
	const SharedVars *svars) const {

	unsigned int point1, point2;
	IndividualArrayInt *ind;

	// Check that the length of genotype is enough
	if (individual->size() < 2)
		return;

	unsigned int nGenes = individual->size();

	// Choose the points to apply the mutation
	point1 = point2 = svars->rng->getInteger(0, nGenes - 1);
	while (point1 == point2)
		point2 = svars->rng->getInteger(0, nGenes - 1);

	// If the insertion is on the point1 part
	ind = dynamic_cast<IndividualArrayInt *>(individual);
	if (ind != NULL) {
		int gene = ind->getGene(point1);
		ind->setGene(point1, ind->getGene(point2));
		ind->setGene(point2, gene);
	}
	else {
		std::string errorMsg = "Type of individual not recognised";
		errorMsg += " by this operator";
		throw FuzzyFWException("Mutation", errorMsg);
	}
}





//=============================================================================
//
//	Class MutationInversion
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Apply (Individual)  --------------------------------------------------
void MutationInversion::apply(Individual *individual,
	const SharedVars *svars) const {

	unsigned int left, right;
	IndividualArrayInt * ind;

	// Check that the length of genotype is enough
	if (individual->size() < 2)
		return;

	unsigned int nGenes = individual->size();

	// Choose the points to apply the mutation
	left = right = svars->rng->getInteger(0, nGenes - 1);
	while (left == right)
		right = svars->rng->getInteger(0, nGenes - 1);
	if (right < left)
		std::swap(left, right);


	// If the insertion is on the left part	
	ind = dynamic_cast<IndividualArrayInt *>(individual);
	if (ind != NULL) {
		int gene;
		while (left < right) {
			gene = ind->getGene(left);
			ind->setGene(left, ind->getGene(right));
			ind->setGene(right, gene);
			left++;
			right--;
		}
	}
	else {
		std::string errorMsg = "Type of individual not recognised";
		errorMsg += " by this operator";
		throw FuzzyFWException("Mutation", errorMsg);
	}
}


}
