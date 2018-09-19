/*
* FuzzyFWException.h
*
*  Created on: Nov 29, 2016
*      Author: Juan Jose Palacios
*/
#pragma once

#include <iostream>

namespace FuzzyFW {


	/**
	* Class to manage exceptions
	*
	* Allows to customize text messages to be shown after an expcetion occurs
	*
	* @author jjpalacios
	*
	*/
	class FuzzyFWException : public std::exception {
	protected:
		std::string cause;

	public:
		explicit FuzzyFWException(const std::string & errorMessage) : std::exception() {
			this->cause = "Fatal error (FSch): " + errorMessage;
		}

		explicit FuzzyFWException(const std::string & where,
			const std::string & errorMessage) : std::exception() {
			this->cause = "Fatal error (FSch:" + where + "): " + errorMessage;
		}

		~FuzzyFWException() throw() {};

		//! Overload of method what to get the new message
		const char* what() const throw() {
			return this->cause.c_str();
		}
	};
}
