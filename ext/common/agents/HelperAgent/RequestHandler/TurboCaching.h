/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2014 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */
#ifndef _PASSENGER_TURBO_CACHING_H_
#define _PASSENGER_TURBO_CACHING_H_

#include <oxt/backtrace.hpp>
#include <ev++.h>
#include <ctime>
#include <cassert>
#include <MemoryKit/mbuf.h>
#include <ServerKit/Context.h>
#include <agents/HelperAgent/ResponseCache.h>
#include <Constants.h>
#include <Logging.h>
#include <Utils/StrIntUtils.h>

namespace Passenger {

using namespace std;


template<typename Request>
class TurboCaching {
public:
	/** The interval of the timer while we're in the DISABLED state. */
	static const unsigned int DISABLED_TIMEOUT = 1;
	/** The interval of the timer while we're in the ENABLED state. */
	static const unsigned int ENABLED_TIMEOUT = 2;
	/** The interval of the timer while we're in the EXTENDED_DISABLED state. */
	static const unsigned int EXTENDED_DISABLED_TIMEOUT = 10;

	OXT_FORCE_INLINE static double MIN_HIT_RATIO() { return 0.5; }

	/**
	 * Minimum number of event loop iterations per second necessary to
	 * trigger enabling turbocaching. 1000 implies that, on average, each
	 * event loop iteration may spend at most 1 ms.
	 */
	static const unsigned int THRESHOLD = 1000;

	enum State {
		/**
		 * Turbocaching is not enabled. It will be enabled upon
		 * detecting heavy load.
		 */
		DISABLED,
		/**
		 * Turbocaching is enabled. It will be disabled when the
		 * heavy load is over.
		 */
		ENABLED,
		/**
		 * In case turbocaching is enabled, and poor cache hit ratio
		 * is detected, this state will be entered. It will stay
		 * in this state for EXTENDED_DISABLED_TIMEOUT seconds before
		 * transitioning to DISABLED.
		 */
		EXTENDED_DISABLED,
		/**
		 * The user completely disabled turbocaching.
		 */
		USER_DISABLED
	};

	typedef ResponseCache<Request> ResponseCacheType;
	typedef typename ResponseCache<Request>::Entry ResponseCacheEntryType;

	State state;
	unsigned long long iterations;
	ev_tstamp lastTimeout, nextTimeout;
	ResponseCache<Request> responseCache;

	TurboCaching()
		: state(DISABLED),
		  iterations(0),
		  lastTimeout((ev_tstamp) std::time(NULL)),
		  nextTimeout((ev_tstamp) std::time(NULL) + DISABLED_TIMEOUT)
		{ }

	bool isEnabled() const {
		return state == ENABLED;
	}

	// Called when the event loop multiplexor returns.
	void onEventLoopCheck(ev_tstamp now) {
		if (OXT_UNLIKELY(state == USER_DISABLED)) {
			return;
		}

		iterations++;
		if (OXT_LIKELY(now < nextTimeout)) {
			return;
		}

		switch (state) {
		case DISABLED:
			if (iterations / (now - lastTimeout) >= (double) THRESHOLD) {
				P_INFO("Server is under heavy load. Turbocaching enabled");
				state = ENABLED;
				nextTimeout = now + ENABLED_TIMEOUT;
			} else {
				P_DEBUG("Server is not under enough load. Not enabling turbocaching");
				nextTimeout = now + DISABLED_TIMEOUT;
			}
			P_DEBUG("Activities per second: " << (iterations / (now - lastTimeout)));
			break;
		case ENABLED:
			if (responseCache.getFetches() > 1
			 && responseCache.getHitRatio() < MIN_HIT_RATIO())
			{
				P_INFO("Poor turbocaching hit ratio detected (" <<
					responseCache.getHits() << " hits, " <<
					responseCache.getFetches() << " fetches, " <<
					(int) (responseCache.getHitRatio() * 100) <<
					"%). Force disabling turbocaching "
					"for " << EXTENDED_DISABLED_TIMEOUT << " seconds");
				state = EXTENDED_DISABLED;
				nextTimeout = now + EXTENDED_DISABLED_TIMEOUT;
			} else {
				if (iterations / (now - lastTimeout) >= (double) THRESHOLD) {
					P_INFO("Clearing turbocache");
					nextTimeout = now + ENABLED_TIMEOUT;
				} else {
					P_INFO("Server is no longer under heavy load. Disabling turbocaching");
					state = DISABLED;
					nextTimeout = now + DISABLED_TIMEOUT;
				}
				P_INFO("Activities per second: " << (iterations / (now - lastTimeout)));
			}
			responseCache.resetStatistics();
			responseCache.clear();
			break;
		case EXTENDED_DISABLED:
			P_INFO("Stopping force disabling turbocaching");
			state = DISABLED;
			nextTimeout = now + DISABLED;
			break;
		default:
			P_BUG("Unknown state " << (int) state);
			break;
		}

		iterations = 0;
		lastTimeout = now;
	}


	struct ResponsePreparation {
		Request *req;
		const ResponseCacheEntryType *entry;

		time_t now;
		time_t age;
		unsigned int ageValueSize;
		bool showVersionInHeader;
	};

	template<typename Server>
	void prepareResponseHeader(ResponsePreparation &prep, Server *server,
		Request *req, const ResponseCacheEntryType &entry)
	{
		prep.req   = req;
		prep.entry = &entry;
		prep.now   = (time_t) ev_now(server->getLoop());

		if (prep.now >= entry.header->date) {
			prep.age = prep.now - entry.header->date;
		} else {
			prep.age = 0;
		}

		prep.ageValueSize = integerSizeInOtherBase<time_t, 10>(prep.age);
		prep.showVersionInHeader = server->showVersionInHeader;
	}

	template<typename Server>
	unsigned int buildResponseHeader(const ResponsePreparation &prep, Server *server,
		char *output, unsigned int outputSize)
	{
		#define PUSH_STATIC_STRING(str) \
			do { \
				result += sizeof(str) - 1; \
				if (output != NULL) { \
					pos = appendData(pos, end, str, sizeof(str) - 1); \
				} \
			} while (false)

		const ResponseCacheEntryType *entry = prep.entry;
		Request *req = prep.req;

		unsigned int httpVersion = req->httpMajor * 1000 + req->httpMinor * 10;
		unsigned int result = 0;
		char *pos = output;
		const char *end = output + outputSize;

		result += entry->body->httpHeaderSize;
		if (output != NULL) {
			pos = appendData(pos, end, entry->body->httpHeaderData,
				entry->body->httpHeaderSize);
		}

		PUSH_STATIC_STRING("Age: ");
		PUSH_STATIC_STRING("\r\n");
		result += prep.ageValueSize;
		if (output != NULL) {
			integerToOtherBase<time_t, 10>(prep.age, pos, end - pos);
			pos += prep.ageValueSize;
		}
		PUSH_STATIC_STRING("\r\n");

		if (prep.showVersionInHeader) {
			PUSH_STATIC_STRING("X-Powered-By: " PROGRAM_NAME " " PASSENGER_VERSION "\r\n");
		} else {
			PUSH_STATIC_STRING("X-Powered-By: " PROGRAM_NAME "\r\n");
		}

		if (server->canKeepAlive(req)) {
			if (httpVersion < 1010) {
				// HTTP < 1.1 defaults to "Connection: close"
				PUSH_STATIC_STRING("Connection: keep-alive\r\n\r\n");
			}
		} else {
			if (httpVersion >= 1010) {
				// HTTP 1.1 defaults to "Connection: keep-alive"
				PUSH_STATIC_STRING("Connection: close\r\n\r\n");
			}
		}

		#ifndef NDEBUG
			if (output != NULL) {
				assert(pos - output == result);
				assert(pos - output == outputSize);
			}
		#endif
		return result;

		#undef PUSH_STATIC_STRING
	}
};


} // namespace Passenger

#endif /* _PASSENGER_TURBO_CACHING_H_ */
