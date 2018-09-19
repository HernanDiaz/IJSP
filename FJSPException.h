/*
* SchException.h
*
*  Created on: Nov 29, 2016
*      Author: Juan Jose Palacios
*/
#ifndef UTILS_FJSP_EXCEPTION_H_
#define UTILS_FJSP_EXCEPTION_H_

#include <iostream>

namespace FJSP {


	/**
	* Class to manage exceptions
	*
	* Allows to customize text messages to be shown after an expcetion occurs
	*
	* @author jjpalacios
	*
	*/
	class FJSPException : public std::exception {
	protected:
		std::string cause;

	public:
		explicit FJSPException(const std::string & errorMessage) : std::exception() {
			this->cause = "Fatal error (FSch): " + errorMessage;
		}

		explicit FJSPException(const std::string & where,
			const std::string & errorMessage) : std::exception() {
			this->cause = "Fatal error (FSch:" + where + "): " + errorMessage;
		}

		~FJSPException() throw() {};

		//! Overload of method what to get the new message
		const char* what() const throw() {
			return this->cause.c_str();
		}
	};
}

#endif /* UTILS_FJSP_EXCEPTION_H_ */
