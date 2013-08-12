# encoding: utf-8
#  Phusion Passenger - https://www.phusionpassenger.com/
#  Copyright (c) 2013 Phusion
#
#  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#  THE SOFTWARE.

require 'phusion_passenger/platform_info/binary_compatibility'
require 'phusion_passenger/utils/json'
require 'etc'

module PhusionPassenger
module Standalone

# Helper class for locating runtime files used by Passenger Standalone.
class RuntimeLocator
	attr_reader :runtime_dir

	def initialize(runtime_dir, nginx_version = PhusionPassenger::PREFERRED_NGINX_VERSION)
		@runtime_dir = runtime_dir || default_runtime_dir
		@nginx_version = nginx_version
	end

	def self.looks_like_support_dir?(dir)
		File.exist?("#{dir}/agents/PassengerWatchdog") &&
			File.exist?("#{dir}/common/libboost_oxt.a") &&
			File.exist?("#{dir}/common/libpassenger_common/ApplicationPool2/Implementation.o")
	end

	# Returns the directory in which Passenger Standalone
	# support binaries are stored, or nil if not installed.
	def find_support_dir
		return @support_dir if defined?(@support_dir)

		if PhusionPassenger.originally_packaged?
			if debugging?
				@support_dir = "#{PhusionPassenger.source_root}/buildout"
			else
				dir = "#{@runtime_dir}/#{version}/support-#{cxx_compat_id}"
				if self.class.looks_like_support_dir?(dir)
					@support_dir = dir
				else
					@support_dir = nil
				end
			end
		else
			@support_dir = PhusionPassenger.lib_dir
		end
		return @support_dir
	end

	# Returns the path to the Nginx binary that Passenger Standalone
	# may use, or nil if not installed.
	def find_nginx_binary
		return @nginx_binary if defined?(@nginx_binary)

		if File.exist?(config_filename)
			config = PhusionPassenger::Utils::JSON.parse(File.read(config_filename))
		else
			config = {}
		end
		if result = config["nginx_binary"]
			@nginx_binary = result
		elsif PhusionPassenger.natively_packaged? && @nginx_version == PhusionPassenger::PREFERRED_NGINX_VERSION
			@nginx_binary = "#{PhusionPassenger.lib_dir}/nginx"
		else
			filename = "#{@runtime_dir}/#{version}/nginx-#{@nginx_version}-#{cxx_compat_id}/nginx"
			if File.exist?(filename)
				@nginx_binary = filename
			else
				@nginx_binary = nil
			end
		end
		return @nginx_binary
	end

	def everything_installed?
		return find_support_dir && find_nginx_binary
	end

	# Returns the directory to which support binaries may be installed,
	# in case the RuntimeInstaller is to be invoked.
	def support_dir_install_target
		if PhusionPassenger.originally_packaged?
			return "#{@runtime_dir}/#{version}/support-#{cxx_compat_id}"
		else
			return nil
		end
	end

	# Returns the directory to which the Nginx binary may be installed,
	# in case the RuntimeInstaller is to be invoked.
	def nginx_binary_install_target
		return "#{@runtime_dir}/#{version}/nginx-#{@nginx_version}-#{cxx_compat_id}"
	end

private
	def default_runtime_dir
		home = Etc.getpwuid.dir
		return "#{home}/#{USER_NAMESPACE_DIRNAME}/standalone"
	end

	def debugging?
		return ['yes', 'y', 'true', '1'].include?(ENV['PASSENGER_DEBUG'].to_s.downcase)
	end

	def version
		return PhusionPassenger::VERSION_STRING
	end

	def cxx_compat_id
		return PlatformInfo.cxx_binary_compatibility_id
	end

	def config_filename
		return "#{@runtime_dir}/config.json"
	end
end

end # module Standalone
end # module PhusionPassenger
