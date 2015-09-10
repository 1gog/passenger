/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2010-2015 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  See LICENSE file for license information.
 */
#ifndef _PASSENGER_UST_ROUTER_TRANSACTION_H_
#define _PASSENGER_UST_ROUTER_TRANSACTION_H_

#include <string>
#include <ev++.h>
#include <jsoncpp/json.h>
#include <UstRouter/LogSink.h>
#include <UstRouter/DataStoreId.h>
#include <UnionStationFilterSupport.h>
#include <Utils/StrIntUtils.h>
#include <Utils/JsonUtils.h>

namespace Passenger {
namespace UstRouter {

using namespace std;
using namespace boost;


class Controller;
inline void Controller_closeLogSink(Controller *controller, const LogSinkPtr &logSink);
inline FilterSupport::Filter &Controller_compileFilter(Controller *controller, const StaticString &source);


struct Transaction {
	Controller *controller;
	ev_tstamp createdAt;
	LogSinkPtr logSink;
	string txnId;
	DataStoreId dataStoreId;
	unsigned int writeCount;
	int refcount;
	bool crashProtect, discarded;
	string data;
	string filters;

	Transaction(Controller *_controller, ev_tstamp _createdAt)
		: controller(_controller),
		  createdAt(_createdAt),
		  writeCount(0),
		  refcount(0),
		  crashProtect(false),
		  discarded(false)
	{
		data.reserve(8 * 1024);
	}

	~Transaction() {
		if (logSink != NULL) {
			if (!discarded && passesFilter()) {
				logSink->append(dataStoreId, data);
			}
			Controller_closeLogSink(controller, logSink);
		}
	}

	StaticString getGroupName() const {
		return dataStoreId.getGroupName();
	}

	StaticString getNodeName() const {
		return dataStoreId.getNodeName();
	}

	StaticString getCategory() const {
		return dataStoreId.getCategory();
	}

	void discard() {
		data.clear();
		discarded = true;
	}

	Json::Value inspectStateAsJson() const {
		Json::Value doc;
		doc["txn_id"] = txnId;
		doc["created_at"] = timeToJson(createdAt * 1000000.0);
		doc["group"] = getGroupName().toString();
		doc["node"] = getNodeName().toString();
		doc["category"] = getGroupName().toString();
		doc["refcount"] = refcount;
		return doc;
	}

private:
	bool passesFilter() {
		if (filters.empty()) {
			return true;
		}

		const char *current = filters.data();
		const char *end     = filters.data() + filters.size();
		bool result         = true;
		FilterSupport::ContextFromLog ctx(data);

		// 'filters' may contain multiple filter sources, separated
		// by '\1' characters. Process each.
		while (current < end && result) {
			StaticString tmp(current, end - current);
			size_t pos = tmp.find('\1');
			if (pos == string::npos) {
				pos = tmp.size();
			}

			StaticString source(current, pos);
			FilterSupport::Filter &filter = Controller_compileFilter(controller, source);
			result = filter.run(ctx);

			current = tmp.data() + pos + 1;
		}
		return result;
	}
};

typedef boost::shared_ptr<Transaction> TransactionPtr;


} // namespace UstRouter
} // namespace Passenger

#endif /* _PASSENGER_UST_ROUTER_TRANSACTION_H_ */
