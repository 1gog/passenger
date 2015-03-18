/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2011-2015 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  See LICENSE file for license information.
 */

// This file is included inside the Pool class.

public:

struct InspectOptions {
	bool colorize;
	bool verbose;

	InspectOptions()
		: colorize(false),
		  verbose(false)
		{ }

	InspectOptions(const VariantMap &options)
		: colorize(options.getBool("colorize", false, false)),
		  verbose(options.getBool("verbose", false, false))
		{ }
};


private:

void inspectProcessList(const InspectOptions &options, stringstream &result,
	const Group *group, const ProcessList &processes) const
{
	ProcessList::const_iterator p_it;
	for (p_it = processes.begin(); p_it != processes.end(); p_it++) {
		const ProcessPtr &process = *p_it;
		char buf[128];
		char cpubuf[10];
		char membuf[10];

		snprintf(cpubuf, sizeof(cpubuf), "%d%%", (int) process->metrics.cpu);
		snprintf(membuf, sizeof(membuf), "%ldM",
			(unsigned long) (process->metrics.realMemory() / 1024));
		snprintf(buf, sizeof(buf),
				"  * PID: %-5lu   Sessions: %-2u      Processed: %-5u   Uptime: %s\n"
				"    CPU: %-5s   Memory  : %-5s   Last used: %s ago",
				(unsigned long) process->getPid(),
				process->sessions,
				process->processed,
				process->uptime().c_str(),
				cpubuf,
				membuf,
				distanceOfTimeInWords(process->lastUsed / 1000000).c_str());
		result << buf << endl;

		if (process->isBeingRollingRestarted()) {
			result << "    Rolling restarting..." << endl;
		}

		if (process->enabled == Process::DISABLING) {
			result << "    Disabling..." << endl;
		} else if (process->enabled == Process::DISABLED) {
			result << "    DISABLED" << endl;
		} else if (process->enabled == Process::DETACHED) {
			result << "    Shutting down..." << endl;
		}

		const Socket *socket;
		if (options.verbose && (socket = process->getSockets().findSocketWithName("http")) != NULL) {
			result << "    URL     : http://" << replaceString(socket->address, "tcp://", "") << endl;
			result << "    Password: " << group->getSecret() << endl;
		}
	}
}

static const char *maybeColorize(const InspectOptions &options, const char *color) {
	if (options.colorize) {
		return color;
	} else {
		return "";
	}
}


public:

string inspect(const InspectOptions &options = InspectOptions(), bool lock = true) const {
	DynamicScopedLock l(syncher, lock);
	stringstream result;
	const char *headerColor = maybeColorize(options, ANSI_COLOR_YELLOW ANSI_COLOR_BLUE_BG ANSI_COLOR_BOLD);
	const char *resetColor  = maybeColorize(options, ANSI_COLOR_RESET);

	result << headerColor << "----------- General information -----------" << resetColor << endl;
	result << "Max pool size : " << max << endl;
	result << "Processes     : " << getProcessCount(false) << endl;
	result << "Requests in top-level queue : " << getWaitlist.size() << endl;
	if (restarterThreadActive) {
		result << restarterThreadStatus << endl;
	}
	if (options.verbose) {
		unsigned int i = 0;
		foreach (const GetWaiter &waiter, getWaitlist) {
			result << "  " << i << ": " << waiter.options.getAppGroupName() << endl;
			i++;
		}
	}
	result << endl;

	result << headerColor << "----------- Application groups -----------" << resetColor << endl;
	GroupMap::ConstIterator g_it(groups);
	while (*g_it != NULL) {
		const GroupPtr &group = g_it.getValue();
		ProcessList::const_iterator p_it;

		result << group->getName() << ":" << endl;
		result << "  App root: " << group->options.appRoot << endl;
		if (group->restarting()) {
			result << "  (restarting...)" << endl;
		}
		if (group->spawning()) {
			if (group->processesBeingSpawned == 0) {
				result << "  (spawning...)" << endl;
			} else {
				result << "  (spawning " << group->processesBeingSpawned << " new " <<
					maybePluralize(group->processesBeingSpawned, "process", "processes") <<
					"...)" << endl;
			}
		}
		if (group->hasSpawnError) {
			result << "  Resisting deployment error!" << endl;
		}
		result << "  Requests in queue: " << group->getWaitlist.size() << endl;
		inspectProcessList(options, result, group.get(), group->enabledProcesses);
		inspectProcessList(options, result, group.get(), group->disablingProcesses);
		inspectProcessList(options, result, group.get(), group->disabledProcesses);
		inspectProcessList(options, result, group.get(), group->detachedProcesses);
		result << endl;

		g_it.next();
	}
	return result.str();
}

string toXml(bool includeSecrets = true, bool lock = true) const {
	DynamicScopedLock l(syncher, lock);
	stringstream result;
	GroupMap::ConstIterator g_it(groups);
	ProcessList::const_iterator p_it;

	result << "<?xml version=\"1.0\" encoding=\"iso8859-1\" ?>\n";
	result << "<info version=\"3\">";

	result << "<passenger_version>" << PASSENGER_VERSION << "</passenger_version>";
	result << "<process_count>" << getProcessCount(false) << "</process_count>";
	result << "<max>" << max << "</max>";
	result << "<capacity_used>" << capacityUsedUnlocked() << "</capacity_used>";
	result << "<get_wait_list_size>" << getWaitlist.size() << "</get_wait_list_size>";
	if (restarterThreadActive) {
		result << "<rolling_restart_status>" << escapeForXml(restarterThreadStatus) <<
			"</rolling_restart_status>";
	}

	if (includeSecrets) {
		vector<GetWaiter>::const_iterator w_it, w_end = getWaitlist.end();

		result << "<get_wait_list>";
		for (w_it = getWaitlist.begin(); w_it != w_end; w_it++) {
			const GetWaiter &waiter = *w_it;
			result << "<item>";
			result << "<app_group_name>" << escapeForXml(waiter.options.getAppGroupName()) << "</app_group_name>";
			result << "</item>";
		}
		result << "</get_wait_list>";
	}

	result << "<supergroups>";
	while (*g_it != NULL) {
		const GroupPtr &group = g_it.getValue();

		result << "<supergroup>";
		result << "<name>" << escapeForXml(group->getName()) << "</name>";
		result << "<state>READY</state>";
		result << "<get_wait_list_size>0</get_wait_list_size>";
		result << "<capacity_used>" << group->capacityUsed() << "</capacity_used>";
		if (includeSecrets) {
			result << "<secret>" << escapeForXml(group->getSecret()) << "</secret>";
		}

		result << "<group default=\"true\">";
		group->inspectXml(result, includeSecrets);
		result << "</group>";

		result << "</supergroup>";

		g_it.next();
	}
	result << "</supergroups>";

	result << "</info>";
	return result.str();
}
