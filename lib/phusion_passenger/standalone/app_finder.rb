#  Phusion Passenger - https://www.phusionpassenger.com/
#  Copyright (c) 2010-2013 Phusion
#
#  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
#
#  See LICENSE file for license information.
PhusionPassenger.require_passenger_lib 'utils/file_system_watcher'

module PhusionPassenger
module Standalone

class AppFinder
	attr_accessor :dirs
	attr_reader :apps

	def self.looks_like_app_directory?(dir)
		return File.exist?("#{dir}/config.ru") ||
			File.exist?("#{dir}/config/environment.rb") ||
			File.exist?("#{dir}/passenger_wsgi.py") ||
			File.exist?("#{dir}/app.js") ||
			File.exist?("#{dir}/.meteor")
	end

	def initialize(dirs, options = {})
		@dirs = dirs
		@options = options.dup
	end

	def global_options
		return @options
	end

	def scan
		apps = []
		watchlist = []

		if single_mode?
			app_root = find_app_root
			apps << {
				:server_names => ["_"],
				:root => app_root
			}
			watchlist << app_root
			watchlist << "#{app_root}/config" if File.exist?("#{app_root}/config")
			watchlist << "#{app_root}/passenger-standalone.json" if File.exist?("#{app_root}/passenger-standalone.json")

			config_filename = File.join(app_root, "passenger-standalone.json")
			if File.exist?(config_filename)
				global_options = load_config_file!(:global_config, config_filename)
				@options.merge!(global_options)
			end

			apps.map! do |app|
				@options.merge(app)
			end
		else
			dirs = @dirs.empty? ? ["."] : @dirs
			dirs.each do |dir|
				if looks_like_app_directory?(dir)
					app_root = File.expand_path(dir)
					server_names = filename_to_server_names(dir)
					apps << {
						:server_names => server_names,
						:root => app_root
					}
					watchlist << app_root
					watchlist << "#{app_root}/config" if File.exist?("#{app_root}/config")
					watchlist << "#{app_root}/passenger-standalone.json" if File.exist?("#{app_root}/passenger-standalone.json")
				else
					full_dir = File.expand_path(dir)
					watchlist << full_dir
					Dir["#{full_dir}/*"].each do |subdir|
						if looks_like_app_directory?(subdir)
							server_names = filename_to_server_names(subdir)
							apps << {
								:server_names => server_names,
								:root => subdir
							}
						end
						watchlist << subdir
						watchlist << "#{subdir}/config" if File.exist?("#{subdir}/config")
						watchlist << "#{subdir}/passenger-standalone.json" if File.exist?("#{subdir}/passenger-standalone.json")
					end
				end
			end

			apps.sort! do |a, b|
				a[:root] <=> b[:root]
			end
			apps.map! do |app|
				config_filename = File.join(app[:root], "passenger-standalone.json")
				if File.exist?(config_filename)
					local_options = load_config_file!(:local_config, config_filename)
					app = app.merge(local_options)
				end
				@options.merge(app)
			end
		end

		@apps = apps
		@watchlist = watchlist
		return apps
	end

	def monitor(termination_pipe)
		raise "You must call #scan first" if !@apps

		watcher = PhusionPassenger::Utils::FileSystemWatcher.new(@watchlist, termination_pipe)
		if wait_on_io(termination_pipe, 3)
			return
		end

		while true
			changed = watcher.wait_for_change
			watcher.close
			if changed
				old_apps = @apps
				# The change could be caused by a write to some passenger-standalone.json file.
				# Wait for a short period so that the write has a chance to finish.
				if wait_on_io(termination_pipe, 0.25)
					return
				end

				begin
					new_apps = scan
				rescue AppFinder::ConfigLoadError => e
					STDERR.puts " *** ERROR: #{e}"
					new_apps = old_apps
				end
				watcher = PhusionPassenger::Utils::FileSystemWatcher.new(@watchlist, termination_pipe)
				if old_apps != new_apps
					yield(new_apps)
				end

				# Don't process change events again for a short while,
				# but do detect changes while waiting.
				if wait_on_io(termination_pipe, 3)
					return
				end
			else
				return
			end
		end
	ensure
		watcher.close if watcher
	end

	def single_mode?
		return (@dirs.empty? && looks_like_app_directory?(".")) ||
			(@dirs.size == 1 && looks_like_app_directory?(@dirs[0]))
	end

	def multi_mode?
		return !single_mode?
	end

	##################
private
	class ConfigLoadError < StandardError
	end

	def find_app_root
		if @dirs.empty?
			return File.expand_path(".")
		else
			return File.expand_path(@dirs[0])
		end
	end

	def load_config_file!(context, filename)
		PhusionPassenger.require_passenger_lib 'utils/json' if !defined?(PhusionPassenger::Utils::JSON)
		begin
			data = File.open(filename, "r:utf-8") do |f|
				f.read
			end
		rescue SystemCallError => e
			raise ConfigLoadError, "cannot load config file #{filename} (#{e})"
		end

		begin
			config = PhusionPassenger::Utils::JSON.parse(data)
		rescue => e
			raise ConfigLoadError, "cannot parse config file #{filename} (#{e})"
		end
		if !config.is_a?(Hash)
			raise ConfigLoadError, "cannot parse config file #{filename} (it does not contain an object)"
		end

		result = {}
		config.each_pair do |key, val|
			result[key.to_sym] = val
		end
		return result
	end

	def load_config_file(context, filename)
		return load_config_file!(context, filename)
	rescue ConfigLoadError => e
		STDERR.puts "*** Warning: #{e.message}"
		return {}
	end

	def looks_like_app_directory?(dir)
		return AppFinder.looks_like_app_directory?(dir)
	end

	def filename_to_server_names(filename)
		basename = File.basename(filename)
		names = [basename]
		if basename !~ /^www\.$/i
			names << "www.#{basename}"
		end
		return names
	end

	# Wait until the given IO becomes readable, or until the timeout has
	# been reached. Returns true if the IO became readable, false if the
	# timeout has been reached.
	def wait_on_io(io, timeout)
		return !!select([io], nil, nil, timeout)
	end

	##################
end

end # module Standalone
end # module PhusionPassenger
