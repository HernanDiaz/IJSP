/*
* FVRPException.h
*
*  Created on: Oct 27, 2017
*/
#pragma once

#include <iostream>

namespace FVRP {


	/**
	* Class to manage exceptions
	*
	* Allows to customize text messages to be shown after an expcetion occurs
	*
	*
	*/
	class FVRPException : public std::exception {
	protected:
		std::string cause;

	public:
		explicit FVRPException(const std::string & errorMessage) : std::exception() {
			this->cause = "Fatal error (FVRP): " + errorMessage;
		}

		explicit FVRPException(const std::string & where,
			const std::string & errorMessage) : std::exception() {
			this->cause = "Fatal error (FVRP:" + where + "): " + errorMessage;
		}

		~FVRPException() throw() {};

		//! Overload of method what to get the new message
		const char* what() const throw() {
			return this->cause.c_str();
		}
	};
}
