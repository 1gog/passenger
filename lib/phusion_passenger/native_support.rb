#  Phusion Passenger - https://www.phusionpassenger.com/
#  Copyright (c) 2010-2013 Phusion
#
#  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
#
#  See LICENSE file for license information.

module PhusionPassenger

class NativeSupportLoader
	def supported?
		return !defined?(RUBY_ENGINE) || RUBY_ENGINE == "ruby" || RUBY_ENGINE == "rbx"
	end
	
	def start
		require 'phusion_passenger'
		load_from_native_support_output_dir ||
		load_from_source_root ||
		load_from_load_path ||
		load_from_home_dir ||
		compile_and_load
	end

private
	def archdir
		@archdir ||= begin
			require 'phusion_passenger/platform_info/binary_compatibility'
			PlatformInfo.ruby_extension_binary_compatibility_id
		end
	end
	
	def libext
		@libext ||= begin
			require 'phusion_passenger/platform_info/operating_system'
			PlatformInfo.library_extension
		end
	end
	
	def home
		@home ||= begin
			require 'etc' if !defined?(Etc)
			home = Etc.getpwuid(Process.uid).dir
		end
	end
	
	def library_name
		return "passenger_native_support.#{libext}"
	end
	
	def extconf_rb
		File.join(PhusionPassenger.ruby_extension_source_dir, "extconf.rb")
	end
	
	def native_support_dir_in_source_root
		if PhusionPassenger.originally_packaged?
			@native_support_dir_in_source_root ||=
				File.expand_path("#{PhusionPassenger.source_root}/buildout/ruby")
		else
			return nil
		end
	end

	def load_from_native_support_output_dir
		# Quick workaround for people suffering from
		# https://code.google.com/p/phusion-passenger/issues/detail?id=865
		output_dir = ENV['PASSENGER_NATIVE_SUPPORT_OUTPUT_DIR']
		if output_dir && !output_dir.empty?
			begin
				require "#{output_dir}/#{VERSION_STRING}/#{archdir}/#{library_name}"
				return true
			rescue LoadError
				return false
			end
		else
			return false
		end
	end
	
	def load_from_source_root
		if PhusionPassenger.originally_packaged?
			begin
				require "#{native_support_dir_in_source_root}/#{archdir}/#{library_name}"
				return true
			rescue LoadError
				return false
			end
		else
			return false
		end
	end
	
	def load_from_load_path
		require 'passenger_native_support'
		return true
	rescue LoadError
		return false
	end
	
	def load_from_home_dir
		begin
			require "#{home}/#{USER_NAMESPACE_DIRNAME}/native_support/#{VERSION_STRING}/#{archdir}/#{library_name}"
			return true
		rescue LoadError
			return false
		end
	end
	
	def compile_and_load
		STDERR.puts "*** Phusion Passenger: no #{library_name} found for " +
			"the current Ruby interpreter. Compiling one..."

		require 'fileutils'
		require 'phusion_passenger/platform_info/ruby'
		
		target_dirs = []
		if (output_dir = ENV['PASSENGER_NATIVE_SUPPORT_OUTPUT_DIR']) && !output_dir.empty?
			target_dirs << "#{output_dir}/#{VERSION_STRING}/#{archdir}"
		end
		if native_support_dir_in_source_root
			target_dirs << "#{native_support_dir_in_source_root}/#{archdir}"
		end
		target_dirs << "#{home}/#{USER_NAMESPACE_DIRNAME}/native_support/#{VERSION_STRING}/#{archdir}"
		
		target_dir = compile(target_dirs)
		if target_dir
			require "#{target_dir}/#{library_name}"
		else
			STDERR.puts "Ruby native_support extension not loaded. Continuing without native_support."
		end
	end
	
	def mkdir(dir)
		begin
			STDERR.puts "# mkdir -p #{dir}"
			FileUtils.mkdir_p(dir)
		rescue Errno::EEXIST
		end
	end
	
	def sh(*args)
		if !sh_nonfatal(*args)
			raise "Could not compile #{library_name} (\"#{command_string}\" failed)"
		end
	end

	def sh_nonfatal(*args)
		command_string = args.join(' ')
		STDERR.puts "# #{command_string}"
		return system(*args)
	end
	
	def compile(target_dirs)
		result = nil
		target_dirs.each_with_index do |target_dir, i|
			begin
				mkdir(target_dir)
				File.open("#{target_dir}/.permission_test", "w").close
				File.unlink("#{target_dir}/.permission_test")
				STDERR.puts "# cd #{target_dir}"
				Dir.chdir(target_dir) do
					result =
						sh_nonfatal("#{PlatformInfo.ruby_command} '#{extconf_rb}'") &&
						sh_nonfatal("make")
					if result
						STDERR.puts "Compilation succesful."
						STDERR.puts "-------------------------------"
						return target_dir
					else
						STDERR.puts "Compilation failed."
						STDERR.puts "-------------------------------"
						return nil
					end
				end
			rescue Errno::EACCES
				# If we encountered a permission error, then try
				# the next target directory. If we get a permission
				# error on the last one too then propagate the
				# exception.
				if i == target_dirs.size - 1
					STDERR.puts "Encountered permission error, " +
						"but no more directories to try. Giving up."
					STDERR.puts "-------------------------------"
					return nil
				else
					STDERR.puts "Encountered permission error, " +
						"trying a different directory..."
					STDERR.puts "-------------------------------"
				end
			rescue Errno::ENOTDIR
				# This can occur when PhusionPassenger.source_root
				# is a location configuration file, and natively_packaged
				# is set to false. For example, when we're running
				# in Phusion Passenger Standalone. In this case
				# just ignore this directory.
				if i == target_dirs.size - 1
					STDERR.puts "Encountered permission error, " +
						"but no more directories to try. Giving up."
					STDERR.puts "-------------------------------"
					return nil
				else
					STDERR.puts "Not a valid directory. Trying a different one..."
					STDERR.puts "-------------------------------"
				end
			end
		end
	end
end

end

loader = PhusionPassenger::NativeSupportLoader.new
loader.start if loader.supported?
