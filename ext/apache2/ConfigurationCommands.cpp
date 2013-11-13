/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2010-2013 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  See LICENSE file for license information.
 */

/*
 * ConfigurationCommands.cpp is automatically generated from ConfigurationCommands.cpp.erb,
 * using definitions from lib/phusion_passenger/apache2/config_options.rb.
 * Edits to ConfigurationCommands.cpp will be lost.
 *
 * To update ConfigurationCommands.cpp:
 *   rake apache2
 *
 * To force regeneration of ConfigurationCommands.c:
 *   rm -f ext/apache2/ConfigurationCommands.cpp
 *   rake ext/apache2/ConfigurationCommands.cpp
 */




	AP_INIT_TAKE1("PassengerRuby",
		(Take1Func) cmd_passenger_ruby,
		NULL,
		OR_OPTIONS | ACCESS_CONF | RSRC_CONF,
		"The Ruby interpreter to use."),

	AP_INIT_TAKE1("PassengerPython",
		(Take1Func) cmd_passenger_python,
		NULL,
		OR_OPTIONS | ACCESS_CONF | RSRC_CONF,
		"The Python interpreter to use."),

	AP_INIT_TAKE1("PassengerNodejs",
		(Take1Func) cmd_passenger_nodejs,
		NULL,
		OR_OPTIONS | ACCESS_CONF | RSRC_CONF,
		"The Node.js command to use."),

	AP_INIT_TAKE1("PassengerMinInstances",
		(Take1Func) cmd_passenger_min_instances,
		NULL,
		OR_LIMIT | ACCESS_CONF | RSRC_CONF,
		"The minimum number of application instances to keep when cleaning idle instances."),

	AP_INIT_TAKE1("PassengerUser",
		(Take1Func) cmd_passenger_user,
		NULL,
		ACCESS_CONF | RSRC_CONF,
		"The user that Ruby applications must run as."),

	AP_INIT_TAKE1("PassengerGroup",
		(Take1Func) cmd_passenger_group,
		NULL,
		ACCESS_CONF | RSRC_CONF,
		"The group that Ruby applications must run as."),

	AP_INIT_FLAG("PassengerErrorOverride",
		(FlagFunc) cmd_passenger_error_override,
		NULL,
		OR_ALL,
		"Allow Apache to handle error response."),

	AP_INIT_TAKE1("PassengerMaxRequests",
		(Take1Func) cmd_passenger_max_requests,
		NULL,
		OR_LIMIT | ACCESS_CONF | RSRC_CONF,
		"The maximum number of requests that an application instance may process."),

	AP_INIT_TAKE1("PassengerStartTimeout",
		(Take1Func) cmd_passenger_start_timeout,
		NULL,
		OR_LIMIT | ACCESS_CONF | RSRC_CONF,
		"A timeout for application startup."),

	AP_INIT_FLAG("PassengerHighPerformance",
		(FlagFunc) cmd_passenger_high_performance,
		NULL,
		OR_ALL,
		"Enable or disable Passenger's high performance mode."),

	AP_INIT_FLAG("PassengerEnabled",
		(FlagFunc) cmd_passenger_enabled,
		NULL,
		OR_ALL,
		"Enable or disable Phusion Passenger."),

	AP_INIT_TAKE1("PassengerMaxRequestQueueSize",
		(Take1Func) cmd_passenger_max_request_queue_size,
		NULL,
		OR_ALL,
		"The maximum number of queued requests."),

	AP_INIT_FLAG("PassengerLoadShellEnvvars",
		(FlagFunc) cmd_passenger_load_shell_envvars,
		NULL,
		OR_OPTIONS | ACCESS_CONF | RSRC_CONF,
		"Whether to load environment variables from the shell before running the application."),

