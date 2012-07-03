/*
 *  Phusion Passenger - http://www.modrails.com/
 *  Copyright (c) 2010, 2011, 2012 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  See LICENSE file for license information.
 */
#define _GNU_SOURCE

#include <oxt/system_calls.hpp>
#include <oxt/backtrace.hpp>
#include <sys/types.h>
#ifdef __linux__
	#include <sys/syscall.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#if defined(__APPLE__) || defined(__linux__)
	#define LIBC_HAS_BACKTRACE_FUNC
#endif
#ifdef LIBC_HAS_BACKTRACE_FUNC
	#include <execinfo.h>
#endif

#include <agents/Base.h>
#include <Constants.h>
#include <Exceptions.h>
#include <Logging.h>
#include <Utils.h>

namespace Passenger {

static bool _feedbackFdAvailable = false;
static const char digits[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
};
static const char hex_chars[] = "01234567890abcdef";

static bool shouldDumpWithCrashWatch = true;

// Pre-allocate an alternative stack for use in signal handlers in case
// the normal stack isn't usable.
static char *alternativeStack;
static unsigned int alternativeStackSize;

static void
ignoreSigpipe() {
	struct sigaction action;
	action.sa_handler = SIG_IGN;
	action.sa_flags   = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGPIPE, &action, NULL);
}

static bool
hasEnvOption(const char *name, bool defaultValue = false) {
	const char *value = getenv(name);
	if (value != NULL) {
		if (*value != '\0') {
			return strcmp(value, "yes") == 0
				|| strcmp(value, "y") == 0
				|| strcmp(value, "1") == 0
				|| strcmp(value, "on") == 0
				|| strcmp(value, "true") == 0;
		} else {
			return defaultValue;
		}
	} else {
		return defaultValue;
	}
}

// Async-signal safe way to fork().
// http://sourceware.org/bugzilla/show_bug.cgi?id=4737
static pid_t
asyncFork() {
	#if defined(__linux__)
		return (pid_t) syscall(SYS_fork);
	#else
		return fork();
	#endif
}

// No idea whether strlen() is async signal safe, but let's not risk it
// and write our own version instead that's guaranteed to be safe.
static size_t
safeStrlen(const char *str) {
	size_t size = 0;
	while (*str != '\0') {
		str++;
		size++;
	}
	return size;
}

// Async-signal safe way to print to stderr.
static void
safePrintErr(const char *message) {
	write(STDERR_FILENO, message, strlen(message));
}

// Must be async signal safe.
static char *
appendText(char *buf, const char *text) {
	size_t len = safeStrlen(text);
	strcpy(buf, text);
	return buf + len;
}

// Must be async signal safe.
static void
reverse(char *str, size_t len) {
	char *p1, *p2;
	if (*str == '\0') {
		return;
	}
	for (p1 = str, p2 = str + len - 1; p2 > p1; ++p1, --p2) {
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
}

// Must be async signal safe.
static char *
appendULL(char *buf, unsigned long long value) {
	unsigned long long remainder = value;
	unsigned int size = 0;

	do {
		buf[size] = digits[remainder % 10];
		remainder = remainder / 10;
		size++;
	} while (remainder != 0);
	
	reverse(buf, size);
	return buf + size;
}

// Must be async signal safe.
template<typename IntegerType>
static char *
appendIntegerAsHex(char *buf, IntegerType value) {
	IntegerType remainder = value;
	unsigned int size = 0;
	
	do {
		buf[size] = hex_chars[remainder % 16];
		remainder = remainder / 16;
		size++;
	} while (remainder != 0);
	
	reverse(buf, size);
	return buf + size;
}

// Must be async signal safe.
static char *
appendPointerAsString(char *buf, void *pointer) {
	// Use wierd union construction to avoid compiler warnings.
	if (sizeof(void *) == sizeof(unsigned int)) {
		union {
			void *pointer;
			unsigned int value;
		} u;
		u.pointer = pointer;
		return appendIntegerAsHex(appendText(buf, "0x"), u.value);
	} else if (sizeof(void *) == sizeof(unsigned long long)) {
		union {
			void *pointer;
			unsigned long long value;
		} u;
		u.pointer = pointer;
		return appendIntegerAsHex(appendText(buf, "0x"), u.value);
	} else {
		return appendText(buf, "(pointer size unsupported)");
	}
}

static char *
appendSignalName(char *buf, int signo) {
	switch (signo) {
	case SIGABRT:
		buf = appendText(buf, "SIGABRT");
		break;
	case SIGSEGV:
		buf = appendText(buf, "SIGSEGV");
		break;
	case SIGBUS:
		buf = appendText(buf, "SIGBUS");
		break;
	case SIGFPE:
		buf = appendText(buf, "SIGFPE");
		break;
	default:
		return appendULL(buf, (unsigned long long) signo);
	}
	buf = appendText(buf, "(");
	buf = appendULL(buf, (unsigned long long) signo);
	buf = appendText(buf, ")");
	return buf;
}

#define SI_CODE_HANDLER(name) \
	case name: \
		buf = appendText(buf, #name); \
		break

// Must be async signal safe.
static char *
appendSignalReason(char *buf, siginfo_t *info) {
	bool handled = true;
	
	switch (info->si_code) {
	SI_CODE_HANDLER(SI_USER);
	#ifdef SI_KERNEL
		SI_CODE_HANDLER(SI_KERNEL);
	#endif
	SI_CODE_HANDLER(SI_QUEUE);
	SI_CODE_HANDLER(SI_TIMER);
	#ifdef SI_ASYNCIO
		SI_CODE_HANDLER(SI_ASYNCIO);
	#endif
	#ifdef SI_MESGQ
		SI_CODE_HANDLER(SI_MESGQ);
	#endif
	#ifdef SI_SIGIO
		SI_CODE_HANDLER(SI_SIGIO);
	#endif
	#ifdef SI_TKILL
		SI_CODE_HANDLER(SI_TKILL);
	#endif
	default:
		switch (info->si_signo) {
		case SIGSEGV:
			switch (info->si_code) {
			#ifdef SEGV_MAPERR
				SI_CODE_HANDLER(SEGV_MAPERR);
			#endif
			#ifdef SEGV_ACCERR
				SI_CODE_HANDLER(SEGV_ACCERR);
			#endif
			default:
				handled = false;
				break;
			}
			break;
		case SIGBUS:
			switch (info->si_code) {
			#ifdef BUS_ADRALN
				SI_CODE_HANDLER(BUS_ADRALN);
			#endif
			#ifdef BUS_ADRERR
				SI_CODE_HANDLER(BUS_ADRERR);
			#endif
			#ifdef BUS_OBJERR
				SI_CODE_HANDLER(BUS_OBJERR);
			#endif
			default:
				handled = false;
				break;
			}
			break;
		default:
			handled = false;
			break;
		}
		if (!handled) {
			buf = appendText(buf, "#");
			buf = appendULL(buf, (unsigned long long) info->si_code);
		}
		break;
	}
	
	if (info->si_code <= 0) {
		buf = appendText(buf, ", signal sent by PID ");
		buf = appendULL(buf, (unsigned long long) info->si_pid);
		buf = appendText(buf, " with UID ");
		buf = appendULL(buf, (unsigned long long) info->si_uid);
	}

	buf = appendText(buf, ", si_addr=");
	buf = appendPointerAsString(buf, info->si_addr);
	
	return buf;
}

static void
dumpWithCrashWatch(char *messageBuf) {
	pid_t pid = getpid();
	const char *pidStr = messageBuf;
	char *end = messageBuf;
	end = appendULL(end, (unsigned long long) pid);
	*end = '\0';
	
	pid_t child = asyncFork();
	if (child == 0) {
		// Sleep for a short while to allow the parent process to raise SIGSTOP.
		// Strictly speaking usleep() is not async-signal safe so let's hope
		// it works.
		usleep(100000);

		resetSignalHandlersAndMask();

		child = asyncFork();
		if (child == 0) {
			execlp("crash-watch", "crash-watch", "--dump", pidStr, (char * const) 0);
			if (errno == ENOENT) {
				safePrintErr("Crash-watch is not installed. Please install it with 'gem install crash-watch' "
					"or download it from https://github.com/FooBarWidget/crash-watch.\n");
			} else {
				int e = errno;
				end = messageBuf;
				end = appendText(end, "crash-watch is installed, but it could not be executed! ");
				end = appendText(end, "(execlp() returned errno=");
				end = appendULL(end, e);
				end = appendText(end, ") Please check your file permissions or something.\n");
				write(STDERR_FILENO, messageBuf, end - messageBuf);
			}
			_exit(1);

		} else if (child == -1) {
			int e = errno;
			end = messageBuf;
			end = appendText(end, "Could not execute crash-watch: fork() failed with errno=");
			end = appendULL(end, e);
			end = appendText(end, "\n");
			write(STDERR_FILENO, messageBuf, end - messageBuf);
			_exit(1);

		} else {
			waitpid(child, NULL, 0);
			// Crash-watch may or may not resume the parent process.
			// We do it ourselves just to be sure.
			kill(pid, SIGCONT);
			_exit(0);
		}

	} else if (child == -1) {
		int e = errno;
		end = messageBuf;
		end = appendText(end, "Could not execute crash-watch: fork() failed with errno=");
		end = appendULL(end, e);
		end = appendText(end, "\n");
		write(STDERR_FILENO, messageBuf, end - messageBuf);

	} else {
		raise(SIGSTOP);
		// Will continue after the child process or crash-watch has done its job.
	}
}

static void
abortHandler(int signo, siginfo_t *info, void *ctx) {
	pid_t pid = getpid();
	char messageBuf[1024];
	#ifdef LIBC_HAS_BACKTRACE_FUNC
		void *backtraceStore[512];
		backtraceStore[0] = '\0'; // Don't let gdb print uninitialized contents.
	#endif
	
	char *end = messageBuf;
	end = appendText(end, "[ pid=");
	end = appendULL(end, (unsigned long long) pid);
	end = appendText(end, ", timestamp=");
	end = appendULL(end, (unsigned long long) time(NULL));
	end = appendText(end, " ] Process aborted! signo=");
	end = appendSignalName(end, signo);
	end = appendText(end, ", reason=");
	end = appendSignalReason(end, info);
	
	// It is important that writing the message and the backtrace are two
	// seperate operations because it's not entirely clear whether the
	// latter is async signal safe and thus can crash.
	#ifdef LIBC_HAS_BACKTRACE_FUNC
		end = appendText(end, ", backtrace available.\n");
	#else
		end = appendText(end, "\n");
	#endif
	write(STDERR_FILENO, messageBuf, end - messageBuf);
	
	#ifdef LIBC_HAS_BACKTRACE_FUNC
		int frames = backtrace(backtraceStore, sizeof(backtraceStore) / sizeof(void *));
		end = messageBuf;
		end = appendText(end, "--------------------------------------\n");
		end = appendText(end, "[ pid=");
		end = appendULL(end, (unsigned long long) pid);
		end = appendText(end, " ] Backtrace with ");
		end = appendULL(end, (unsigned long long) frames);
		end = appendText(end, " frames:\n");
		write(STDERR_FILENO, messageBuf, end - messageBuf);
		backtrace_symbols_fd(backtraceStore, frames, STDERR_FILENO);

		end = messageBuf;
		end = appendText(end, "--------------------------------------\n");
		end = appendText(end, "[ pid=");
		end = appendULL(end, (unsigned long long) pid);
		if (shouldDumpWithCrashWatch) {
			end = appendText(end, " ] Dumping a more detailed backtrace with crash-watch...\n");
		} else {
			end = appendText(end, " ]\n");
		}
		write(STDERR_FILENO, messageBuf, end - messageBuf);
	#else
		end = messageBuf;
		end = appendText(end, "--------------------------------------\n");
		end = appendText(end, "[ pid=");
		end = appendULL(end, (unsigned long long) pid);
		if (shouldDumpWithCrashWatch) {
			end = appendText(end, " ] Dumping a backtrace with crash-watch...\n");
		} else {
			end = appendText(end, " ]\n");
		}
		write(STDERR_FILENO, messageBuf, end - messageBuf);
	#endif

	if (shouldDumpWithCrashWatch) {
		dumpWithCrashWatch(messageBuf);
	}

	// Run default signal handler.
	raise(signo);
}

void
installAbortHandler() {
	alternativeStackSize = MINSIGSTKSZ + 64 * 1024;
	alternativeStack = (char *) malloc(alternativeStackSize);
	if (alternativeStack == NULL) {
		fprintf(stderr, "Cannot allocate an alternative with a size of %u bytes!\n",
			alternativeStackSize);
		fflush(stderr);
		abort();
	}
	
	stack_t stack;
	stack.ss_sp = alternativeStack;
	stack.ss_size = alternativeStackSize;
	stack.ss_flags = 0;
	if (sigaltstack(&stack, NULL) != 0) {
		int e = errno;
		fprintf(stderr, "Cannot install an alternative stack for use in signal handlers: %s (%d)\n",
			strerror(e), e);
		fflush(stderr);
		abort();
	}
	
	struct sigaction action;
	action.sa_sigaction = abortHandler;
	action.sa_flags = SA_RESETHAND | SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	sigaction(SIGABRT, &action, NULL);
	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGBUS, &action, NULL);
	sigaction(SIGFPE, &action, NULL);
}

bool
feedbackFdAvailable() {
	return _feedbackFdAvailable;
}

VariantMap
initializeAgent(int argc, char *argv[], const char *processName) {
	TRACE_POINT();
	VariantMap options;
	
	ignoreSigpipe();
	shouldDumpWithCrashWatch = hasEnvOption("PASSENGER_DUMP_WITH_CRASH_WATCH", true);
	if (hasEnvOption("PASSENGER_ABORT_HANDLER", true)) {
		installAbortHandler();
	}
	setup_syscall_interruption_support();
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	
	try {
		if (argc == 1) {
			int ret = fcntl(FEEDBACK_FD, F_GETFL);
			if (ret == -1) {
				if (errno == EBADF) {
					fprintf(stderr,
						"You're not supposed to start this program from the command line. "
						"It's used internally by Phusion Passenger.\n");
					exit(1);
				} else {
					int e = errno;
					fprintf(stderr,
						"Encountered an error in feedback file descriptor 3: %s (%d)\n",
							strerror(e), e);
					exit(1);
				}
			} else {
				_feedbackFdAvailable = true;
				options.readFrom(FEEDBACK_FD);
				if (options.getBool("fire_and_forget", false)) {
					_feedbackFdAvailable = false;
					close(FEEDBACK_FD);
				}
			}
		} else {
			options.readFrom((const char **) argv + 1, argc - 1);
		}
		
		setLogLevel(options.getInt("log_level", false, 0));
		if (!options.get("debug_log_file", false).empty()) {
			if (strcmp(processName, "PassengerWatchdog") == 0) {
				/* Have the watchdog set STDOUT and STDERR to the debug
				 * log file so that system abort() calls that stuff
				 * are properly logged.
				 */
				string filename = options.get("debug_log_file");
				options.erase("debug_log_file");
				
				int fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
				if (fd == -1) {
					int e = errno;
					throw FileSystemException("Cannot open debug log file " +
						filename, e, filename);
				}
				
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				close(fd);
			} else {
				setDebugFile(options.get("debug_log_file").c_str());
			}
		}
	} catch (const tracable_exception &e) {
		P_ERROR("*** ERROR: " << e.what() << "\n" << e.backtrace());
		exit(1);
	}
	
	// Change process title.
	strncpy(argv[0], processName, strlen(argv[0]));
	for (int i = 1; i < argc; i++) {
		memset(argv[i], '\0', strlen(argv[i]));
	}
	
	return options;
}

} // namespace Passenger
