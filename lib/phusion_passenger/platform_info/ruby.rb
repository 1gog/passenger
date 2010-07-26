#  Phusion Passenger - http://www.modrails.com/
#  Copyright (c) 2010 Phusion
#
#  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
#
#  See LICENSE file for license information.

require 'rbconfig'
require 'phusion_passenger/platform_info'

module PhusionPassenger

module PlatformInfo
	# The absolute path to the current Ruby interpreter.
	RUBY = Config::CONFIG['bindir'] + '/' + Config::CONFIG['RUBY_INSTALL_NAME'] + Config::CONFIG['EXEEXT']
	
	if defined?(::RUBY_ENGINE)
		RUBY_ENGINE = ::RUBY_ENGINE
	else
		RUBY_ENGINE = "ruby"
	end
	
	# Returns whether the Ruby interpreter supports process forking.
	def self.fork_supported?
		# MRI >= 1.9.2's respond_to? returns false for methods
		# that are not implemented.
		return Process.respond_to?(:fork) &&
			RUBY_ENGINE != "jruby" &&
			RUBY_ENGINE != "macruby" &&
			Config::CONFIG['target_os'] !~ /mswin|windows|mingw/
	end
	
	# The correct 'gem' command for this Ruby interpreter.
	def self.gem_command
		return locate_ruby_executable('gem')
	end
	memoize :gem_command
	
	# Returns the absolute path to the Rake executable that
	# belongs to the current Ruby interpreter. Returns nil if it
	# doesn't exist.
	def self.rake
		return locate_ruby_executable('rake')
	end
	memoize :rake
	
	# Returns the absolute path to the RSpec runner program that
	# belongs to the current Ruby interpreter. Returns nil if it
	# doesn't exist.
	def self.rspec
		return locate_ruby_executable('spec')
	end
	memoize :rspec
	
	def self.locate_ruby_executable(name)
		if RUBY_PLATFORM =~ /darwin/ &&
		   RUBY =~ %r(\A/System/Library/Frameworks/Ruby.framework/Versions/.*?/usr/bin/ruby\Z)
			# On OS X we must look for Ruby binaries in /usr/bin.
			# RubyGems puts executables (e.g. 'rake') in there, not in
			# /System/Libraries/(...)/bin.
			filename = "/usr/bin/#{name}"
		else
			filename = File.dirname(RUBY) + "/#{name}"
		end
		if File.file?(filename) && File.executable?(filename)
			return filename
		else
			# RubyGems might put binaries in a directory other
			# than Ruby's bindir. Debian packaged RubyGems and
			# DebGem packaged RubyGems are the prime examples.
			begin
				require 'rubygems' unless defined?(Gem)
				filename = Gem.bindir + "/#{name}"
				if File.file?(filename) && File.executable?(filename)
					return filename
				else
					return nil
				end
			rescue LoadError
				return nil
			end
		end
	end
end

end # module PhusionPassenger
