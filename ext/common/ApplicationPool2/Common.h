/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2011-2013 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  See LICENSE file for license information.
 */
#ifndef _PASSENGER_APPLICATION_POOL2_COMMON_H_
#define _PASSENGER_APPLICATION_POOL2_COMMON_H_

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <oxt/tracable_exception.hpp>
#include <ApplicationPool2/Options.h>
#include <Utils/StringMap.h>

namespace tut {
	struct ApplicationPool2_PoolTest;
}

namespace Passenger {
namespace ApplicationPool2 {

using namespace std;
using namespace boost;
using namespace oxt;

class Pool;
class SuperGroup;
class Group;
class Process;
class Session;

/**
 * The result of a Pool::disableProcess/Group::disable() call. Some values are only
 * returned by the functions, some values are only passed to the Group::disable()
 * callback, some values appear in all cases.
 */
enum DisableResult {
	// The process has been successfully disabled.
	// Returned by functions and passed to the callback.
	DR_SUCCESS,
	
	// The disabling of the process was canceled before completion.
	// The process still exists.
	// Only passed to the callback.
	DR_CANCELED,
	
	// Nothing happened: the requested process does not exist (anymore)
	// or was already disabled.
	// Returned by functions and passed to the callback.
	DR_NOOP,
	
	// The disabling of the process failed: an error occurred.
	// Returned by functions and passed to the callback.
	DR_ERROR,

	// Indicates that the process cannot be disabled immediately
	// and that the callback will be called later.
	// Only returned by functions.
	DR_DEFERRED
};

typedef boost::shared_ptr<Pool> PoolPtr;
typedef boost::shared_ptr<SuperGroup> SuperGroupPtr;
typedef boost::shared_ptr<Group> GroupPtr;
typedef boost::shared_ptr<Process> ProcessPtr;
typedef boost::shared_ptr<Session> SessionPtr;
typedef boost::shared_ptr<tracable_exception> ExceptionPtr;
typedef StringMap<SuperGroupPtr> SuperGroupMap;
typedef boost::function<void (const SessionPtr &session, const ExceptionPtr &e)> GetCallback;
typedef boost::function<void (const ProcessPtr &process, DisableResult result)> DisableCallback;
typedef boost::function<void ()> Callback;

struct GetWaiter {
	Options options;
	GetCallback callback;
	
	GetWaiter(const Options &o, const GetCallback &cb)
		: options(o),
		  callback(cb)
	{
		options.persist(o);
	}
};

struct Ticket {
	boost::mutex syncher;
	boost::condition_variable cond;
	SessionPtr session;
	ExceptionPtr exception;
};

struct SpawnerConfig {
	// Used by SmartSpawner and DirectSpawner.
	/** A random generator to use. */
	RandomGeneratorPtr randomGenerator;

	// Used by DummySpawner and SpawnerFactory.
	unsigned int concurrency;
	unsigned int spawnerCreationSleepTime;
	unsigned int spawnTime;

	SpawnerConfig(const RandomGeneratorPtr &randomGenerator = RandomGeneratorPtr())
		: concurrency(1),
		  spawnerCreationSleepTime(0),
		  spawnTime(0)
	{
		if (randomGenerator != NULL) {
			this->randomGenerator = randomGenerator;
		} else {
			this->randomGenerator = boost::make_shared<RandomGenerator>();
		}
	}
};

typedef boost::shared_ptr<SpawnerConfig> SpawnerConfigPtr;

ExceptionPtr copyException(const tracable_exception &e);
void rethrowException(const ExceptionPtr &e);

} // namespace ApplicationPool2
} // namespace Passenger

#endif /* _PASSENGER_APPLICATION_POOL2_COMMON_H_ */
