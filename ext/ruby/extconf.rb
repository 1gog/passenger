#  Phusion Passenger - https://www.phusionpassenger.com/
#  Copyright (c) 2010-2012 Phusion
#
#  See LICENSE file for license information.
require 'mkmf'
$LIBS = ""
$CFLAGS << " -g"
$LIBS << " -lpthread" if RUBY_PLATFORM =~ /openbsd/

if RUBY_PLATFORM =~ /solaris/
	have_library('xnet')
	$CFLAGS << " -D_XPG4_2"
	$CFLAGS << " -D__EXTENSIONS__"
	if RUBY_PLATFORM =~ /solaris2.9/
		$CFLAGS << " -D__SOLARIS9__"
	end
end

have_header('alloca.h')
have_header('ruby/io.h')

with_cflags($CFLAGS) do
	create_makefile('passenger_native_support')
end
