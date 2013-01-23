/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2010-2013 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  See LICENSE file for license information.
 */
#ifndef _PASSENGER_LOGGING_H_
#define _PASSENGER_LOGGING_H_

#include <oxt/thread.hpp>
#include <oxt/system_calls.hpp>
#include <oxt/macros.hpp>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string>
#include <exception>
#include <stdexcept>
#include <ios>
#include <ostream>
#include <sstream>
#include <cstdio>
#include <ctime>
#include <cerrno>


namespace Passenger {

using namespace std;
using namespace boost;
using namespace oxt;


/********** Debug logging facilities **********/

extern int _logLevel;
extern int _logOutput;

int getLogLevel();
void setLogLevel(int value);
bool setDebugFile(const char *logFile = NULL);
void _prepareLogEntry(std::stringstream &sstream);
void _writeLogEntry(const std::string &str);


/**
 * Write the given expression to the log stream.
 */
#define P_LOG(level, expr) \
	do { \
		if (Passenger::_logLevel >= level) { \
			std::stringstream sstream; \
			Passenger::_prepareLogEntry(sstream); \
			sstream << expr << "\n"; \
			Passenger::_writeLogEntry(sstream.str()); \
		} \
	} while (false)

/**
 * Write the given expression, which represents a warning,
 * to the log stream.
 */
#define P_WARN(expr) P_LOG(0, expr)

/**
 * Write the given expression, which represents a warning,
 * to the log stream.
 */
#define P_INFO(expr) P_LOG(0, expr)

/**
 * Write the given expression, which represents an error,
 * to the log stream.
 */
#define P_ERROR(expr) P_LOG(-1, expr)

/**
 * Write the given expression, which represents a debugging message,
 * to the log stream.
 */
#define P_DEBUG(expr) P_TRACE(1, expr)

#ifdef PASSENGER_DEBUG
	#define P_TRACE(level, expr) P_LOG(level, expr)
#else
	#define P_TRACE(level, expr) do { /* nothing */ } while (false)
#endif


class NotExpectingExceptions {
private:
	this_thread::disable_interruption di;
	this_thread::disable_syscall_interruption dsi;
	const char *filename;
	const char *function;
	unsigned int line;

public:
	NotExpectingExceptions(const char *_filename, unsigned int _line, const char *_function) {
		filename = _filename;
		line = _line;
		function = _function;
	}

	~NotExpectingExceptions() {
		if (std::uncaught_exception()) {
			P_ERROR("Unexpected exception detected at " << filename <<
				":" << line << ", function '" << function << "'!");
		}
	}
};

/**
 * Put this in code sections where you don't expect *any* exceptions to be thrown.
 * This macro will automatically disables interruptions in the current scope,
 * and will print an error message whenever the scope exits with an exception.
 *
 * When inside critical sections, you should put this macro right after the lock
 * object so that the error message is displayed before unlocking the lock;
 * otherwise other threads may run before the error message is displayed, and
 * those threads may see an inconsistant state and crash.
 */
#define NOT_EXPECTING_EXCEPTIONS() NotExpectingExceptions __nee(__FILE__, __LINE__, __PRETTY_FUNCTION__)

} // namespace Passenger

#endif /* _PASSENGER_LOGGING_H_ */
