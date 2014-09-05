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
#ifndef _PASSENGER_SERVER_KIT_HTTP_REQUEST_H_
#define _PASSENGER_SERVER_KIT_HTTP_REQUEST_H_

#include <Utils/sysqueue.h>
#include <boost/cstdint.hpp>
#include <boost/atomic.hpp>
#include <ServerKit/http_parser.h>
#include <ServerKit/Hooks.h>
#include <ServerKit/Client.h>
#include <ServerKit/HeaderTable.h>
#include <ServerKit/FileBufferedChannel.h>
#include <ServerKit/HttpHeaderParserState.h>
#include <ServerKit/HttpChunkedBodyParserState.h>
#include <MemoryKit/palloc.h>
#include <DataStructures/LString.h>

namespace Passenger {
namespace ServerKit {


class BaseHttpRequest {
public:
	enum HttpState {
		/** The request headers are still being parsed. */
		PARSING_HEADERS,
		/** Internal state used by the parser. Users should never see this state. */
		PARSED_HEADERS,
		/** The request headers have been parsed, and there is no body. */
		COMPLETE,
		/** The request headers have been parsed, and we are now receiving/parsing the body,
		 * which does not have the chunked transfer-encoding. */
		PARSING_BODY,
		/** The request headers have been parsed, and we are now receiving/parsing the body,
		 * which has the chunked transfer-encoding. */
		PARSING_CHUNKED_BODY,
		/** The request headers have been parsed, and the connection has been upgraded. */
		UPGRADED,

		// The following states are recognized as 'ended'.

		/** An error occurred. */
		ERROR,
		/**
		 * The request has been ended. We've deinitialized the request object, and we're now
		 * waiting for output to be flushed before transitioning to WAITING_FOR_REFERENCES.
		 * In this state, the client object's `currentRequest` field still points to this
		 * request.
		 */
		FLUSHING_OUTPUT,
		/**
		 * The request has ended. We've deinitialized the request object, and we're now
		 * waiting until all references to this request object are gone. In this state,
		 * the client object's `currentRequest` field no longer points to this request.
		 */
		WAITING_FOR_REFERENCES,
		/** This request object is in the freelist. */
		IN_FREELIST
	};

	// Enum values are deliberately chosen so that hasBody() can be branchless.
	enum BodyType {
		/** The request has no body. */
		RBT_NO_BODY = 0,
		/** The connection has been upgraded. */
		RBT_UPGRADE = 1,
		/** The request body's size is determined by the Content-Length header. */
		RBT_CONTENT_LENGTH = 2,
		/** The request body's size is determined by the chunked Transfer-Encoding. */
		RBT_CHUNKED = 4
	};

	boost::uint8_t httpMajor;
	boost::uint8_t httpMinor;
	HttpState httpState: 5;
	BodyType bodyType: 3;

	http_method method: 5;
	bool wantKeepAlive: 1;
	bool responseBegun: 1;

	boost::atomic<int> refcount;

	BaseClient *client;
	union {
		HttpHeaderParserState *headerParser;
		HttpChunkedBodyParserState chunkedBodyParser;
	} parserState;
	psg_pool_t *pool;
	Hooks hooks;
	LString path;
	HeaderTable headers;
	// We separate headers and secure headers because the number of normal
	// headers is variable, but the number of secure headers is more or less
	// constant.
	HeaderTable secureHeaders;
	FileBufferedChannel bodyChannel;

	/** If a request parsing error occurred, the error message is stored here. */
	const char *parseError;

	/** Length of the message body. Only has meaning when state is PARSING_BODY. */
	union {
		// If bodyType == RBT_CONTENT_LENGTH
		boost::uint64_t contentLength;
		// If bodyType == RBT_CHUNKED
		bool endChunkReached;
	} bodyInfo;
	boost::uint64_t bodyAlreadyRead;


	BaseHttpRequest()
		: refcount(1),
		  client(NULL),
		  pool(NULL),
		  headers(16),
		  secureHeaders(32),
		  parseError(NULL),
		  bodyAlreadyRead(0)
	{
		psg_lstr_init(&path);
		bodyInfo.contentLength = 0; // Also sets endChunkReached to false
	}

	const char *getHttpStateString() const {
		switch (httpState) {
		case PARSING_HEADERS:
			return "PARSING_HEADERS";
		case PARSED_HEADERS:
			return "PARSED_HEADERS";
		case COMPLETE:
			return "COMPLETE";
		case PARSING_BODY:
			return "PARSING_BODY";
		case PARSING_CHUNKED_BODY:
			return "PARSING_CHUNKED_BODY";
		case UPGRADED:
			return "UPGRADED";
		case ERROR:
			return "ERROR";
		case FLUSHING_OUTPUT:
			return "FLUSHING_OUTPUT";
		case WAITING_FOR_REFERENCES:
			return "WAITING_FOR_REFERENCES";
		case IN_FREELIST:
			return "IN_FREELIST";
		}
	}

	const char *getBodyTypeString() const {
		switch (bodyType) {
		case RBT_NO_BODY:
			return "NO_BODY";
		case RBT_UPGRADE:
			return "UPGRADE";
		case RBT_CONTENT_LENGTH:
			return "CONTENT_LENGTH";
		case RBT_CHUNKED:
			return "CHUNKED";
		}
	}

	bool bodyFullyRead() const {
		switch (bodyType) {
		case RBT_NO_BODY:
			return true;
		case RBT_CONTENT_LENGTH:
			return bodyAlreadyRead >= bodyInfo.contentLength;
		case RBT_CHUNKED:
			return bodyInfo.endChunkReached;
		case RBT_UPGRADE:
			return false;
		}
	}

	bool hasBody() const {
		// Branchless way to check whether RBT_CONTENT_LENGTH or RBT_CHUNKED is set.
		return bodyType & 0x6;
	}

	bool canKeepAlive() const {
		return wantKeepAlive && bodyFullyRead();
	}

	// Not mutually exclusive with ended(). If a request has begun() and is ended(),
	// then it just means that it hasn't been reinitialized for the next request yet.
	bool begun() const {
		return (int) httpState >= COMPLETE;
	}

	bool ended() const {
		// Branchless OR.
		return ((int) httpState >= (int) ERROR) | !client->connected();
	}
};


#define DEFINE_SERVER_KIT_BASE_HTTP_REQUEST_FOOTER(RequestType) \
	public: \
	union { \
		STAILQ_ENTRY(RequestType) freeRequest; \
		LIST_ENTRY(RequestType) endedRequest; \
	} nextRequest


class HttpRequest: public BaseHttpRequest {
public:
	DEFINE_SERVER_KIT_BASE_HTTP_REQUEST_FOOTER(HttpRequest);
};


} // namespace ServerKit
} // namespace Passenger

#endif /* _PASSENGER_SERVER_KIT_HTTP_REQUEST_H_ */
