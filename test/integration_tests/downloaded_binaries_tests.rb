# These tests are run by passenger_autobuilder, right after it has built binaries.
# passenger_autobuilder populates the download_cache directory and runs this test script.

source_root = File.expand_path("../..", File.dirname(__FILE__))
$LOAD_PATH.unshift("#{source_root}/lib")
require 'phusion_passenger'
PhusionPassenger.locate_directories
require 'tmpdir'
require 'fileutils'
require 'webrick'
require 'open-uri'

ENV['PATH'] = "#{PhusionPassenger.bin_dir}:#{ENV['PATH']}"
# This environment variable changes Passenger Standalone's behavior,
# so ensure that it's not set.
ENV.delete('PASSENGER_DEBUG')

describe "Downloaded Phusion Passenger binaries" do
	before :each do
		@temp_dir = Dir.mktmpdir
		File.open("#{PhusionPassenger.resources_dir}/release.txt", "w").close
	end

	after :each do
		FileUtils.remove_entry_secure(@temp_dir)
		File.unlink("#{PhusionPassenger.resources_dir}/release.txt")
	end

	let(:version) { PhusionPassenger::VERSION_STRING }
	let(:nginx_version) { PhusionPassenger::PREFERRED_NGINX_VERSION }
	let(:compat_id) { PhusionPassenger::PlatformInfo.cxx_binary_compatibility_id }

	def sh(*command)
		if !system(*command)
			abort "Command failed: #{command.join(' ')}"
		end
	end

	def start_server(document_root)
		server = WEBrick::HTTPServer.new(:BindAddress => '127.0.0.1',
			:Port => 0,
			:DocumentRoot => document_root,
			:Logger => WEBrick::Log.new("/dev/null"),
			:AccessLog => [])
		Thread.new do
			Thread.current.abort_on_exception = true
			server.start
		end
		[server, "http://127.0.0.1:#{server.config[:Port]}"]
	end

	specify "Passenger Standalone is able to use the binaries" do
		Dir.mkdir("#{@temp_dir}/#{version}")
		Dir.chdir("#{@temp_dir}/#{version}") do
			tarballs = Dir["#{PhusionPassenger.download_cache_dir}/*.tar.gz"]
			tarballs.should_not be_empty
			
			File.open("config.ru", "w") do |f|
				f.write(%Q{
					app = lambda do |env|
						[200, { "Content-Type" => "text/plain" }, ["ok"]]
					end
					run app
				})
			end
			Dir.mkdir("public")
			Dir.mkdir("tmp")
			Dir.mkdir("log")

			begin
				sh("passenger start " +
					"-p 4000 " +
					"-d " +
					"--no-compile-runtime " +
					"--binaries-url-root http://127.0.0.1:4001 " +
					"--runtime-dir '#{@temp_dir}' >log/start.log")
			rescue Exception
				system("cat log/start.log")
				raise
			end
			begin
				open("http://127.0.0.1:4000/") do |f|
					f.read.should == "ok"
				end
			rescue
				system("cat log/passenger.4000.log")
				raise
			ensure
				sh "passenger stop -p 4000"
			end
		end
	end

	specify "helper-scripts/download_binaries/extconf.rb succeeds in downloading all necessary binaries" do
		FileUtils.mkdir_p("server_root")
		server, url_root = start_server("server_root")
		File.rename("download_cache", "download_cache.old")
		begin
			FileUtils.cp_r("download_cache.old", "server_root/#{VERSION_STRING}")
			sh "cd #{PhusionPassenger.source_root} && " +
				"env BINARIES_URL_ROOT=#{url_root} " +
				"ruby helper-scripts/download_binaries/extconf.rb --abort-on-error"
			Dir["download_cache/*"].should_not be_empty
		ensure
			File.unlink("Makefile") rescue nil
			FileUtils.rm_rf("download_cache")
			FileUtils.rm_rf("server_root")
			File.rename("download_cache.old", "download_cache")
			server.stop
		end
	end
end