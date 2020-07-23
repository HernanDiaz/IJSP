/*
* RobustnessException.h
*
*  Created on: June 18, 2020
*      Author: Hernan Diaz
*/
#pragma once

#include <iostream>

namespace PostExecution {


	/**
	* Class to manage exceptions
	*
	* Allows to customize text messages to be shown after an expcetion occurs
	*
	* @author hdiaz
	*
	*/
	class RobustnessException : public std::exception {
	protected:
		std::string cause;

	public:
		explicit RobustnessException(const std::string & errorMessage) : std::exception() {
			this->cause = "Fatal error (FSch): " + errorMessage;
		}

		explicit RobustnessException(const std::string & where,
			const std::string & errorMessage) : std::exception() {
			this->cause = "Fatal error (FSch:" + where + "): " + errorMessage;
		}

		~RobustnessException() throw() {};

		//! Overload of method what to get the new message
		const char* what() const throw() {
			return this->cause.c_str();
		}
	};
}
