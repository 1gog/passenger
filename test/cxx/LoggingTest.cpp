#include "TestSupport.h"
#include "Logging.h"
#include "MessageClient.h"
#include "LoggingAgent/LoggingServer.h"

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <oxt/thread.hpp>

using namespace Passenger;
using namespace std;
using namespace oxt;

namespace tut {
	struct LoggingTest {
		static const unsigned long long YESTERDAY = 1263299017000000ull;  // January 12, 2009, 12:23:37 UTC
		static const unsigned long long TODAY     = 1263385422000000ull;  // January 13, 2009, 12:23:42 UTC
		static const unsigned long long TOMORROW  = 1263471822000000ull;  // January 14, 2009, 12:23:42 UTC
		#define FOOBAR_MD5 "3858f62230ac3c915f300c664312c63f"
		#define LOCALHOST_MD5 "421aa90e079fa326b6494f812ad13e79"
		#define FOOBAR_LOCALHOST_PREFIX FOOBAR_MD5 "/" LOCALHOST_MD5
		
		ServerInstanceDirPtr serverInstanceDir;
		ServerInstanceDir::GenerationPtr generation;
		string socketFilename;
		string loggingDir;
		AccountsDatabasePtr accountsDatabase;
		ev::dynamic_loop eventLoop;
		LoggingServerPtr server;
		shared_ptr<oxt::thread> serverThread;
		AnalyticsLoggerPtr logger, logger2, logger3, logger4;
		
		LoggingTest() {
			createServerInstanceDirAndGeneration(serverInstanceDir, generation);
			socketFilename = generation->getPath() + "/logging.socket";
			loggingDir = generation->getPath() + "/logs";
			accountsDatabase = ptr(new AccountsDatabase());
			accountsDatabase->add("test", "1234", false);
			
			server = ptr(new LoggingServer(eventLoop,
				createUnixServer(socketFilename.c_str()),
				accountsDatabase, loggingDir));
			serverThread = ptr(new oxt::thread(
				boost::bind(&LoggingTest::runLoop, this)
			));
			
			logger = ptr(new AnalyticsLogger(socketFilename, "test", "1234",
				"localhost"));
			logger2 = ptr(new AnalyticsLogger(socketFilename, "test", "1234",
				"localhost"));
			logger3 = ptr(new AnalyticsLogger(socketFilename, "test", "1234",
				"localhost"));
			logger4 = ptr(new AnalyticsLogger(socketFilename, "test", "1234",
				"localhost"));
		}
		
		~LoggingTest() {
			MessageClient client;
			client.connect(socketFilename, "test", "1234");
			client.write("exit", NULL);
			serverThread->join();
			SystemTime::releaseAll();
		}
		
		void runLoop() {
			eventLoop.loop();
		}
		
		string timestampString(unsigned long long timestamp) {
			char str[2 * sizeof(unsigned long long) + 1];
			integerToHex<unsigned long long>(timestamp, str);
			return str;
		}
	};
	
	DEFINE_TEST_GROUP(LoggingTest);
	
	TEST_METHOD(1) {
		// Test logging of new transaction.
		SystemTime::forceAll(YESTERDAY);
		
		AnalyticsLogPtr log = logger->newTransaction("foobar");
		log->message("hello");
		log->message("world");
		log->flushToDiskAfterClose(true);
		
		ensure(!logger->isNull());
		ensure(!log->isNull());
		
		log.reset();
		
		string data = readAll(loggingDir + "/1/" FOOBAR_LOCALHOST_PREFIX "/requests/2010/01/12/12/log.txt");
		ensure(data.find("hello\n") != string::npos);
		ensure(data.find("world\n") != string::npos);
	}
	
	TEST_METHOD(2) {
		// Test logging of existing transaction.
		SystemTime::forceAll(YESTERDAY);
		
		AnalyticsLogPtr log = logger->newTransaction("foobar");
		log->message("message 1");
		log->flushToDiskAfterClose(true);
		
		AnalyticsLogPtr log2 = logger2->continueTransaction(log->getTxnId(),
			log->getGroupName(), log->getCategory());
		log2->message("message 2");
		log2->flushToDiskAfterClose(true);

		log.reset();
		log2.reset();
		
		string data = readAll(loggingDir + "/1/" FOOBAR_LOCALHOST_PREFIX "/requests/2010/01/12/12/log.txt");
		ensure("(1)", data.find("message 1\n") != string::npos);
		ensure("(2)", data.find("message 2\n") != string::npos);
	}
	
	TEST_METHOD(3) {
		// Test logging with different points in time.
		SystemTime::forceAll(YESTERDAY);
		AnalyticsLogPtr log = logger->newTransaction("foobar");
		log->message("message 1");
		SystemTime::forceAll(TODAY);
		log->message("message 2");
		log->flushToDiskAfterClose(true);
		
		SystemTime::forceAll(TOMORROW);
		AnalyticsLogPtr log2 = logger2->continueTransaction(log->getTxnId(),
			log->getGroupName(), log->getCategory());
		log2->message("message 3");
		log2->flushToDiskAfterClose(true);
		
		AnalyticsLogPtr log3 = logger3->newTransaction("foobar");
		log3->message("message 4");
		log3->flushToDiskAfterClose(true);
		
		log.reset();
		log2.reset();
		log3.reset();
		
		string yesterdayData = readAll(loggingDir + "/1/" FOOBAR_LOCALHOST_PREFIX "/requests/2010/01/12/12/log.txt");
		string tomorrowData = readAll(loggingDir + "/1/" FOOBAR_LOCALHOST_PREFIX "/requests/2010/01/14/12/log.txt");
		ensure("(1)", yesterdayData.find(timestampString(YESTERDAY) + " 1 message 1\n") != string::npos);
		ensure("(2)", yesterdayData.find(timestampString(TODAY) + " 2 message 2\n") != string::npos);
		ensure("(3)", yesterdayData.find(timestampString(TOMORROW) + " 4 message 3\n") != string::npos);
		ensure("(4)", tomorrowData.find(timestampString(TOMORROW) + " 1 message 4\n") != string::npos);
	}
	
	TEST_METHOD(4) {
		// newTransaction() and continueTransaction() write an ATTACH message
		// to the log file, while AnalyticsLogPtr writes a DETACH message upon
		// destruction.
		SystemTime::forceAll(YESTERDAY);
		AnalyticsLogPtr log = logger->newTransaction("foobar");
		
		SystemTime::forceAll(TODAY);
		AnalyticsLogPtr log2 = logger2->continueTransaction(log->getTxnId(),
			log->getGroupName(), log->getCategory());
		log2->flushToDiskAfterClose(true);
		log2.reset();
		
		SystemTime::forceAll(TOMORROW);
		log->flushToDiskAfterClose(true);
		log.reset();
		
		string data = readAll(loggingDir + "/1/" FOOBAR_LOCALHOST_PREFIX "/requests/2010/01/12/12/log.txt");
		ensure("(1)", data.find(timestampString(YESTERDAY) + " 0 ATTACH\n") != string::npos);
		ensure("(2)", data.find(timestampString(TODAY) + " 1 ATTACH\n") != string::npos);
		ensure("(3)", data.find(timestampString(TODAY) + " 2 DETACH\n") != string::npos);
		ensure("(4)", data.find(timestampString(TOMORROW) + " 3 DETACH\n") != string::npos);
	}
	
	TEST_METHOD(5) {
		// newTransaction() generates a new ID, while continueTransaction()
		// reuses the ID.
		AnalyticsLogPtr log = logger->newTransaction("foobar");
		AnalyticsLogPtr log2 = logger2->newTransaction("foobar");
		AnalyticsLogPtr log3 = logger3->continueTransaction(log->getTxnId(),
			log->getGroupName(), log->getCategory());
		AnalyticsLogPtr log4 = logger4->continueTransaction(log2->getTxnId(),
			log2->getGroupName(), log2->getCategory());
		
		ensure_equals(log->getTxnId(), log3->getTxnId());
		ensure_equals(log2->getTxnId(), log4->getTxnId());
		ensure(log->getTxnId() != log2->getTxnId());
	}
	
	TEST_METHOD(6) {
		// An empty AnalyticsLog doesn't do anything.
		AnalyticsLog log;
		ensure(log.isNull());
		log.message("hello world");
		ensure_equals(getFileType(loggingDir), FT_NONEXISTANT);
	}
	
	TEST_METHOD(7) {
		// An empty AnalyticsLogger doesn't do anything.
		AnalyticsLogger logger;
		ensure(logger.isNull());
		
		AnalyticsLogPtr log = logger.newTransaction("foo");
		ensure(log->isNull());
		log->message("hello world");
		ensure_equals(getFileType(loggingDir), FT_NONEXISTANT);
	}
	
	TEST_METHOD(8) {
		// It creates a file group_name.txt under the group directory.
		AnalyticsLogPtr log = logger->newTransaction("foobar");
		log->flushToDiskAfterClose(true);
		log.reset();
		string data = readAll(loggingDir + "/1/" FOOBAR_MD5 "/group_name.txt");
		ensure_equals(data, "foobar");
	}
	
	TEST_METHOD(9) {
		// It creates a file node_name.txt under the node directory.
		AnalyticsLogPtr log = logger->newTransaction("foobar");
		log->flushToDiskAfterClose(true);
		log.reset();
		string data = readAll(loggingDir + "/1/" FOOBAR_LOCALHOST_PREFIX "/node_name.txt");
		ensure_equals(data, "localhost");
	}
}
