/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2011-2015 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  See LICENSE file for license information.
 */
#include <UnionStationFilterSupport.h>
#include <cstring>
#include <cstdlib>

using namespace Passenger;

extern "C" {

PassengerFilter *
passenger_filter_create(const char *source, int size, char **error) {
	if (size == -1) {
		size = strlen(source);
	}
	try {
		return (PassengerFilter *) new FilterSupport::Filter(StaticString(source, size));
	} catch (const SyntaxError &e) {
		if (error != NULL) {
			*error = strdup(e.what());
		}
		return NULL;
	}
}

void
passenger_filter_free(PassengerFilter *filter) {
	delete (FilterSupport::Filter *) filter;
}

char *
passenger_filter_validate(const char *source, int size) {
	if (size == -1) {
		size = strlen(source);
	}
	try {
		(void) FilterSupport::Filter(StaticString(source, size));
		return NULL;
	} catch (const SyntaxError &e) {
		return strdup(e.what());
	}
}

} // extern "C"
