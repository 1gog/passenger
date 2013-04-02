#  Phusion Passenger - https://www.phusionpassenger.com/
#  Copyright (c) 2010-2013 Phusion
#
#  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
#
#  See LICENSE file for license information.

# Rake functions for compiling/linking C++ stuff.

def run_compiler(*command)
	require 'phusion_passenger/utils/ansi_colors' if !defined?(PhusionPassenger::Utils::AnsiColors)
	show_command = command.join(' ')
	puts show_command
	if !system(*command)
		if $? && $?.exitstatus == 4
			# This probably means the compiler ran out of memory.
			msg = "<b>" +
			      "-----------------------------------------------\n" +
			      "Your compiler failed with the exit status 4. This " +
			      "probably means that it ran out of memory. To solve " +
			      "this problem, try increasing your swap space: " +
			      "https://www.digitalocean.com/community/articles/how-to-add-swap-on-ubuntu-12-04" +
			      "</b>"
			fail(PhusionPassenger::Utils::AnsiColors.ansi_colorize(msg))
		else
			fail "Command failed with status (#{$? ? $?.exitstatus : 1}): [#{show_command}]"
		end
	end
end

def compile_c(source, flags = "#{EXTRA_PRE_CFLAGS} #{PlatformInfo.portability_cflags} #{EXTRA_CXXFLAGS}")
	run_compiler "#{CC} #{flags} -c #{source}"
end

def compile_cxx(source, flags = "#{EXTRA_PRE_CXXFLAGS} #{PlatformInfo.portability_cflags} #{EXTRA_CXXFLAGS}")
	run_compiler "#{CXX} #{flags} -c #{source}"
end

def create_static_library(target, sources)
	# On OS X, 'ar cru' will sometimes fail with an obscure error:
	#
	#   ar: foo.a is a fat file (use libtool(1) or lipo(1) and ar(1) on it)
	#   ar: foo.a: Inappropriate file type or format
	#
	# So here we delete the ar file before creating it, which bypasses this problem.
	sh "rm -rf #{target}"
	sh "ar cru #{target} #{sources}"
	sh "ranlib #{target}"
end

def create_executable(target, sources, linkflags = "#{EXTRA_PRE_CXXFLAGS} #{EXTRA_PRE_LDFLAGS} #{PlatformInfo.portability_cflags} #{EXTRA_CXXFLAGS} #{PlatformInfo.portability_ldflags} #{EXTRA_LDFLAGS}")
	run_compiler "#{CXX} #{sources} -o #{target} #{linkflags}"
end

def create_c_executable(target, sources, linkflags = "#{EXTRA_PRE_CFLAGS} #{EXTRA_PRE_LDFLAGS} #{PlatformInfo.portability_cflags} #{EXTRA_CXXFLAGS} #{PlatformInfo.portability_ldflags} #{EXTRA_LDFLAGS}")
	run_compiler "#{CC} #{sources} -o #{target} #{linkflags}"
end

def create_shared_library(target, sources, flags = "#{EXTRA_PRE_CXXFLAGS} #{EXTRA_PRE_LDFLAGS} #{PlatformInfo.portability_cflags} #{EXTRA_CXXFLAGS} #{PlatformInfo.portability_ldflags} #{EXTRA_LDFLAGS}")
	if RUBY_PLATFORM =~ /darwin/
		shlib_flag = "-flat_namespace -bundle -undefined dynamic_lookup"
	else
		shlib_flag = "-shared"
	end
	run_compiler "#{CXX} #{shlib_flag} #{sources} -fPIC -o #{target} #{flags}"
end
