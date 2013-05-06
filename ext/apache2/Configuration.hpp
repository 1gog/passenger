/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2010-2013 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  See LICENSE file for license information.
 */
#ifndef _PASSENGER_CONFIGURATION_HPP_
#define _PASSENGER_CONFIGURATION_HPP_

#include "Utils.h"
#include "Logging.h"
#include "ServerInstanceDir.h"
#include "Constants.h"

/* The APR headers must come after the Passenger headers. See Hooks.cpp
 * to learn why.
 */
#include "Configuration.h"

#include <set>
#include <string>

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

/**
 * @defgroup Configuration Apache module configuration
 * @ingroup Core
 * @{
 */

namespace Passenger {

using namespace std;

	
/**
 * Per-directory configuration information.
 *
 * Use the getter methods to query information, because those will return
 * the default value if the value is not specified.
 */
struct DirConfig {
	enum Threeway { ENABLED, DISABLED, UNSET };
	enum SpawnMethod { SM_UNSET, SM_SMART, SM_DIRECT };
	
	Threeway enabled;
	
	std::set<std::string> railsBaseURIs;
	std::set<std::string> rackBaseURIs;
	
	/** The Ruby interpreter to use. */
	const char *ruby;

	/** The Python interpreter to use. */
	const char *python;
	
	/** The environment (RAILS_ENV/RACK_ENV/WSGI_ENV) under which
	 * applications should operate. */
	const char *environment;
	
	/** The path to the application's root (for example: RAILS_ROOT
	 * for Rails applications, directory containing 'config.ru'
	 * for Rack applications). If this value is NULL, the default
	 * autodetected path will be used.
	 */
	const char *appRoot;
	
	string appGroupName;
	
	/** The spawn method to use. */
	SpawnMethod spawnMethod;
	
	/** See PoolOptions for more info. */
	const char *user;
	/** See PoolOptions for more info. */
	const char *group;
	
	/**
	 * The idle timeout, in seconds, of preloader processes.
	 * May also be 0 (which indicates that the application spawner should
	 * never idle timeout) or -1 (which means that the value is not specified,
	 * and the default value should be used).
	 */
	long maxPreloaderIdleTime;
	
	/**
	 * The maximum number of requests that the spawned application may process
	 * before exiting. A value of 0 means unlimited.
	 */
	unsigned long maxRequests;
	
	/** Indicates whether the maxRequests option was explicitly specified
	 * in the directory configuration. */
	bool maxRequestsSpecified;
	
	/**
	 * The minimum number of processes for a group that should be kept in
	 * the pool when cleaning idle processes. Defaults to 0.
	 */
	unsigned long minInstances;
	
	/**
	 * Indicates whether the minInstances option was explicitly specified
	 * in the directory configuration. */
	bool minInstancesSpecified;
	
	/** Whether symlinks in the document root path should be resolved.
	 * The implication of this is documented in the users guide, section
	 * "How Phusion Passenger detects whether a virtual host is a web application".
	 */
	Threeway resolveSymlinksInDocRoot;
	
	/** Whether high performance mode should be turned on. */
	Threeway highPerformance;
	
	/**
	 * Whether encoded slashes in URLs should be supported. This however conflicts
	 * with mod_rewrite support because of a bug/limitation in Apache, so it's one
	 * or the other.
	 */
	Threeway allowEncodedSlashes;
	
	/**
	 * Throttle the number of stat() calls on files like
	 * restart.txt to the once per given number of seconds.
	 */
	unsigned long statThrottleRate;
	
	/** Indicates whether the statThrottleRate option was
	 * explicitly specified in the directory configuration. */
	bool statThrottleRateSpecified;
	
	/** The directory in which Passenger should look for
	 * restart.txt. NULL means that the default directory
	 * should be used.
	 */
	const char *restartDir;
	
	/**
	 * The directory in which Passenger should place upload buffer
	 * files. NULL means that the default directory should be used.
	 */
	const char *uploadBufferDir;
	
	string unionStationKey;
	
	vector<string> unionStationFilters;
	
	/**
	 * Whether Phusion Passenger should show friendly error pages.
	 */
	Threeway friendlyErrorPages;
	
	/**
	 * Whether analytics logging should be enabled.
	 */
	Threeway unionStationSupport;
	
	/**
	 * Whether response buffering support is enabled.
	 */
	Threeway bufferResponse;

	const char *concurrencyModel;
	unsigned int threadCount;
	bool threadCountSpecified;
	
	/*************************************/
	
	/*
	 * The maximum number of instances that may be spawned
	 * for the corresponding application.
	 */
	unsigned long maxInstances;
	
	/** Indicates whether the maxInstances option was explicitly specified
	 * in the directory configuration. */
	bool maxInstancesSpecified;
	
	/**
	 * The maximum amount of time (in seconds) that the current application
	 * may spend on a request.
	 */
	unsigned long maxRequestTime;
	
	/** Indicates whether the maxRequestTime option was explicitly.
	 * specified in the directory configuration. */
	bool maxRequestTimeSpecified;
	
	/**
	 * The maximum amount of memory (in MB) the spawned application may use.
	 * A value of 0 means unlimited.
	 */
	unsigned long memoryLimit;
	
	/** Indicates whether the memoryLimit option was explicitly specified
	 * in the directory configuration. */
	bool memoryLimitSpecified;
	
	/** Whether rolling restarts should be used. */
	Threeway rollingRestarts;

	/** Whether deployment error resiatnce should be enabled. */
	Threeway resistDeploymentErrors;

	/** Whether debugger support should be enabled. */
	Threeway debugger;
	
	/*************************************/
	
	bool isEnabled() const {
		return enabled != DISABLED;
	}
	
	string getAppRoot(const StaticString &documentRoot) const {
		if (appRoot == NULL) {
			if (resolveSymlinksInDocRoot == DirConfig::ENABLED) {
				return extractDirName(resolveSymlink(documentRoot));
			} else {
				return extractDirName(documentRoot);
			}
		} else {
			return appRoot;
		}
	}
	
	StaticString getUser() const {
		if (user != NULL) {
			return user;
		} else {
			return "";
		}
	}
	
	StaticString getGroup() const {
		if (group != NULL) {
			return group;
		} else {
			return "";
		}
	}
	
	StaticString getEnvironment() const {
		if (environment != NULL) {
			return environment;
		} else {
			return "production";
		}
	}
	
	StaticString getAppGroupName(const StaticString &appRoot) const {
		if (appGroupName.empty()) {
			return appRoot;
		} else {
			return appGroupName;
		}
	}
	
	StaticString getSpawnMethodString() const {
		switch (spawnMethod) {
		case SM_SMART:
			return "smart";
		case SM_DIRECT:
			return "direct";
		default:
			return "smart";
		}
	}
	
	unsigned long getMaxRequests() const {
		if (maxRequestsSpecified) {
			return maxRequests;
		} else {
			return 0;
		}
	}
	
	unsigned long getMinInstances() const {
		if (minInstancesSpecified) {
			return minInstances;
		} else {
			return 1;
		}
	}
	
	bool highPerformanceMode() const {
		return highPerformance == ENABLED;
	}
	
	bool allowsEncodedSlashes() const {
		return allowEncodedSlashes == ENABLED;
	}
	
	unsigned long getStatThrottleRate() const {
		if (statThrottleRateSpecified) {
			return statThrottleRate;
		} else {
			return 0;
		}
	}
	
	StaticString getRestartDir() const {
		if (restartDir != NULL) {
			return restartDir;
		} else {
			return "";
		}
	}
	
	string getUploadBufferDir(const ServerInstanceDir::GenerationPtr &generation) const {
		if (uploadBufferDir != NULL) {
			return uploadBufferDir;
		} else {
			return generation->getPath() + "/buffered_uploads";
		}
	}
	
	bool showFriendlyErrorPages() const {
		return friendlyErrorPages != DISABLED;
	}
	
	bool useUnionStation() const {
		return unionStationSupport == ENABLED;
	}

	bool getBufferResponse() const {
		return bufferResponse == ENABLED;
	}

	const char *getConcurrencyModel() const {
		if (concurrencyModel != NULL) {
			return concurrencyModel;
		} else {
			return "process";
		}
	}

	unsigned int getThreadCount() const {
		if (threadCountSpecified) {
			return threadCount;
		} else {
			return 1;
		}
	}
	
	string getUnionStationFilterString() const {
		if (unionStationFilters.empty()) {
			return string();
		} else {
			string result;
			vector<string>::const_iterator it;
			
			for (it = unionStationFilters.begin(); it != unionStationFilters.end(); it++) {
				if (it != unionStationFilters.begin()) {
					result.append(1, '\1');
				}
				result.append(*it);
			}
			return result;
		}
	}
	
	/*************************************/
	
	unsigned long getMaxInstances() const {
		if (maxInstancesSpecified) {
			return maxInstances;
		} else {
			return 0;
		}
	}
	
	unsigned long getMaxRequestTime() const {
		if (maxRequestTimeSpecified) {
			return maxRequestTime;
		} else {
			return 0;
		}
	}
	
	unsigned long getMemoryLimit() const {
		if (memoryLimitSpecified) {
			return memoryLimit;
		} else {
			return 0;
		}
	}
	
	bool useRollingRestarts() const {
		return rollingRestarts == ENABLED;
	}

	bool shouldResistDeploymentErrors() const {
		return resistDeploymentErrors == ENABLED;
	}

	bool useDebugger() const {
		return debugger == ENABLED;
	}
};


/**
 * Server-wide (global, not per-virtual host) configuration information.
 *
 * Use the getter methods to query information, because those will return
 * the default value if the value is not specified.
 */
struct ServerConfig {
	/** The Passenger root folder. */
	const char *root;

	/** The default Ruby interpreter to use. */
	const char *defaultRuby;
	
	/** The log verbosity. */
	int logLevel;
	
	/** A file to print debug messages to, or NULL to just use STDERR. */
	const char *debugLogFile;
	
	/** The maximum number of simultaneously alive application
	 * instances. */
	unsigned int maxPoolSize;
	
	/** The maximum number of simultaneously alive Rails application
	 * that a single Rails application may occupy. */
	unsigned int maxInstancesPerApp;
	
	/** The maximum number of seconds that an application may be
	 * idle before it gets terminated. */
	unsigned int poolIdleTime;
	
	/** Whether user switching support is enabled. */
	bool userSwitching;
	
	/** See PoolOptions for more info. */
	string defaultUser;
	/** See PoolOptions for more info. */
	string defaultGroup;
	
	/** The temp directory that Passenger should use. */
	string tempDir;
	
	string unionStationGatewayAddress;
	int unionStationGatewayPort;
	string unionStationGatewayCert;
	string unionStationProxyAddress;
	
	/** Directory in which analytics logs should be saved. */
	string analyticsLogUser;
	string analyticsLogGroup;
	
	set<string> prestartURLs;
	
	ServerConfig() {
		root               = NULL;
		defaultRuby        = DEFAULT_RUBY;
		logLevel           = DEFAULT_LOG_LEVEL;
		debugLogFile       = NULL;
		maxPoolSize        = DEFAULT_MAX_POOL_SIZE;
		maxInstancesPerApp = DEFAULT_MAX_INSTANCES_PER_APP;
		poolIdleTime       = DEFAULT_POOL_IDLE_TIME;
		userSwitching      = true;
		defaultUser        = DEFAULT_WEB_APP_USER;
		tempDir            = getSystemTempDir();
		unionStationGatewayAddress = DEFAULT_UNION_STATION_GATEWAY_ADDRESS;
		unionStationGatewayPort    = DEFAULT_UNION_STATION_GATEWAY_PORT;
		unionStationGatewayCert    = string();
		unionStationProxyAddress   = string();
		analyticsLogUser   = DEFAULT_ANALYTICS_LOG_USER;
		analyticsLogGroup  = DEFAULT_ANALYTICS_LOG_GROUP;
	}
	
	/** Called after the configuration files have been loaded, inside
	 * the control process.
	 */
	void finalize() {
		if (defaultGroup.empty()) {
			struct passwd *userEntry = getpwnam(defaultUser.c_str());
			if (userEntry == NULL) {
				throw ConfigurationException(
					string("The user that PassengerDefaultUser refers to, '") +
					defaultUser + "', does not exist.");
			}
			
			struct group *groupEntry = getgrgid(userEntry->pw_gid);
			if (groupEntry == NULL) {
				throw ConfigurationException(
					string("The option PassengerDefaultUser is set to '" +
					defaultUser + "', but its primary group doesn't exist. "
					"In other words, your system's user account database "
					"is broken. Please fix it."));
			}
			
			defaultGroup = groupEntry->gr_name;
		}
	}
};

extern ServerConfig serverConfig;


} // namespace Passenger

/**
 * @}
 */

#endif /* _PASSENGER_CONFIGURATION_HPP_ */
