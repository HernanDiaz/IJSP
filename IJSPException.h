/*
* IJSPException.h
*
*  Created on: June 18, 2019
*      Author: Hernan Diaz
*/
#pragma once

#include <iostream>

namespace IJSP {


	/**
	* Class to manage exceptions
	*
	* Allows to customize text messages to be shown after an expcetion occurs
	*
	* @author hdiaz
	*
	*/
	class IJSPException : public std::exception {
	protected:
		std::string cause;

	public:
		explicit IJSPException(const std::string & errorMessage) : std::exception() {
			this->cause = "Fatal error (FSch): " + errorMessage;
		}

		explicit IJSPException(const std::string & where,
			const std::string & errorMessage) : std::exception() {
			this->cause = "Fatal error (FSch:" + where + "): " + errorMessage;
		}

		~IJSPException() throw() {};

		//! Overload of method what to get the new message
		const char* what() const throw() {
			return this->cause.c_str();
		}
	};
}
