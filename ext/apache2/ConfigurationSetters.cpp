/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2010-2013 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  See LICENSE file for license information.
 */

/*
 * ConfigurationSetters.cpp is automatically generated from ConfigurationSetters.cpp.erb,
 * using definitions from lib/phusion_passenger/apache2/config_options.rb.
 * Edits to ConfigurationSetters.cpp will be lost.
 *
 * To update ConfigurationSetters.cpp:
 *   rake apache2
 *
 * To force regeneration of ConfigurationSetters.cpp:
 *   rm -f ext/apache2/ConfigurationSetters.cpp
 *   rake ext/apache2/ConfigurationSetters.cpp
 */



	
		static const char *
		cmd_passenger_ruby(cmd_parms *cmd, void *pcfg, const char *arg) {
			DirConfig *config = (DirConfig *) pcfg;
			config->ruby = arg;
			return NULL;
		}
	
	
		static const char *
		cmd_passenger_python(cmd_parms *cmd, void *pcfg, const char *arg) {
			DirConfig *config = (DirConfig *) pcfg;
			config->python = arg;
			return NULL;
		}
	
	
		static const char *
		cmd_passenger_nodejs(cmd_parms *cmd, void *pcfg, const char *arg) {
			DirConfig *config = (DirConfig *) pcfg;
			config->nodejs = arg;
			return NULL;
		}
	
	
		static const char *
		cmd_passenger_min_instances(cmd_parms *cmd, void *pcfg, const char *arg) {
			DirConfig *config = (DirConfig *) pcfg;
			char *end;
			long result;

			result = strtol(arg, &end, 10);
			if (*end != '\0') {
				string message = "Invalid number specified for ";
				message.append(cmd->directive->directive);
				message.append(".");

				char *messageStr = (char *) apr_palloc(cmd->temp_pool,
					message.size() + 1);
				memcpy(messageStr, message.c_str(), message.size() + 1);
				return messageStr;
			
				} else if (result < 0) {
					string message = "Value for ";
					message.append(cmd->directive->directive);
					message.append(" must be greater than or equal to 0.");

					char *messageStr = (char *) apr_palloc(cmd->temp_pool,
						message.size() + 1);
					memcpy(messageStr, message.c_str(), message.size() + 1);
					return messageStr;
			
			} else {
				config->minInstances = (int) result;
				return NULL;
			}
		}
	
	
		static const char *
		cmd_passenger_user(cmd_parms *cmd, void *pcfg, const char *arg) {
			DirConfig *config = (DirConfig *) pcfg;
			config->user = arg;
			return NULL;
		}
	
	
		static const char *
		cmd_passenger_group(cmd_parms *cmd, void *pcfg, const char *arg) {
			DirConfig *config = (DirConfig *) pcfg;
			config->group = arg;
			return NULL;
		}
	
	
		static const char *
		cmd_passenger_max_requests(cmd_parms *cmd, void *pcfg, const char *arg) {
			DirConfig *config = (DirConfig *) pcfg;
			char *end;
			long result;

			result = strtol(arg, &end, 10);
			if (*end != '\0') {
				string message = "Invalid number specified for ";
				message.append(cmd->directive->directive);
				message.append(".");

				char *messageStr = (char *) apr_palloc(cmd->temp_pool,
					message.size() + 1);
				memcpy(messageStr, message.c_str(), message.size() + 1);
				return messageStr;
			
				} else if (result < 0) {
					string message = "Value for ";
					message.append(cmd->directive->directive);
					message.append(" must be greater than or equal to 0.");

					char *messageStr = (char *) apr_palloc(cmd->temp_pool,
						message.size() + 1);
					memcpy(messageStr, message.c_str(), message.size() + 1);
					return messageStr;
			
			} else {
				config->maxRequests = (int) result;
				return NULL;
			}
		}
	
	
		static const char *
		cmd_passenger_start_timeout(cmd_parms *cmd, void *pcfg, const char *arg) {
			DirConfig *config = (DirConfig *) pcfg;
			char *end;
			long result;

			result = strtol(arg, &end, 10);
			if (*end != '\0') {
				string message = "Invalid number specified for ";
				message.append(cmd->directive->directive);
				message.append(".");

				char *messageStr = (char *) apr_palloc(cmd->temp_pool,
					message.size() + 1);
				memcpy(messageStr, message.c_str(), message.size() + 1);
				return messageStr;
			
				} else if (result < 1) {
					string message = "Value for ";
					message.append(cmd->directive->directive);
					message.append(" must be greater than or equal to 1.");

					char *messageStr = (char *) apr_palloc(cmd->temp_pool,
						message.size() + 1);
					memcpy(messageStr, message.c_str(), message.size() + 1);
					return messageStr;
			
			} else {
				config->startTimeout = (int) result;
				return NULL;
			}
		}
	
	
		static const char *
		cmd_passenger_high_performance(cmd_parms *cmd, void *pcfg, const char *arg) {
			DirConfig *config = (DirConfig *) pcfg;
			config->highPerformance =
				arg ?
				DirConfig::ENABLED :
				DirConfig::DISABLED;
			return NULL;
		}
	
	
		static const char *
		cmd_passenger_enabled(cmd_parms *cmd, void *pcfg, const char *arg) {
			DirConfig *config = (DirConfig *) pcfg;
			config->enabled =
				arg ?
				DirConfig::ENABLED :
				DirConfig::DISABLED;
			return NULL;
		}
	
	
		static const char *
		cmd_passenger_max_request_queue_size(cmd_parms *cmd, void *pcfg, const char *arg) {
			DirConfig *config = (DirConfig *) pcfg;
			char *end;
			long result;

			result = strtol(arg, &end, 10);
			if (*end != '\0') {
				string message = "Invalid number specified for ";
				message.append(cmd->directive->directive);
				message.append(".");

				char *messageStr = (char *) apr_palloc(cmd->temp_pool,
					message.size() + 1);
				memcpy(messageStr, message.c_str(), message.size() + 1);
				return messageStr;
			
				} else if (result < 0) {
					string message = "Value for ";
					message.append(cmd->directive->directive);
					message.append(" must be greater than or equal to 0.");

					char *messageStr = (char *) apr_palloc(cmd->temp_pool,
						message.size() + 1);
					memcpy(messageStr, message.c_str(), message.size() + 1);
					return messageStr;
			
			} else {
				config->maxRequestQueueSize = (int) result;
				return NULL;
			}
		}
	
	
		static const char *
		cmd_passenger_load_shell_envvars(cmd_parms *cmd, void *pcfg, const char *arg) {
			DirConfig *config = (DirConfig *) pcfg;
			config->loadShellEnvvars =
				arg ?
				DirConfig::ENABLED :
				DirConfig::DISABLED;
			return NULL;
		}
	

