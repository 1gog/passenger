#  Phusion Passenger - https://www.phusionpassenger.com/
#  Copyright (c) 2010-2013 Phusion
#
#  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
#
#  See LICENSE file for license information.

desc "List directories in the Phusion Passenger Enterprise customer area"
task 'package:list_enterprise' do
	dir = "/u/apps/passenger_website/shared"
	sh "ssh app@shell.phusion.nl ls \"#{dir}/assets\""
end

ORIG_TARBALL_FILES = lambda { PhusionPassenger::Packaging.files }

def recursive_copy_files(files, destination_dir, preprocess = false, variables = {})
	require 'fileutils' if !defined?(FileUtils)
	files.each_with_index do |filename, i|
		dir = File.dirname(filename)
		if !File.exist?("#{destination_dir}/#{dir}")
			FileUtils.mkdir_p("#{destination_dir}/#{dir}")
		end
		if !File.directory?(filename)
			if preprocess && filename =~ /\.template$/
				real_filename = filename.sub(/\.template$/, '')
				FileUtils.install(filename, "#{destination_dir}/#{real_filename}", :preserve => true)
				Preprocessor.new.start(filename, "#{destination_dir}/#{real_filename}",
					variables)
			else
				FileUtils.install(filename, "#{destination_dir}/#{filename}", :preserve => true)
			end
		end
		printf "\r[%5d/%5d] [%3.0f%%] Copying files...", i + 1, files.size, i * 100.0 / files.size
		STDOUT.flush
	end
	printf "\r[%5d/%5d] [%3.0f%%] Copying files...\n", files.size, files.size, 100
end


task :clobber => 'package:clean'

desc "Build, sign & upload gem & tarball"
task 'package:release' => ['package:gem', 'package:tarball', 'package:sign'] do
	require 'phusion_passenger'
	require 'yaml'
	require 'uri'
	require 'net/http'
	require 'net/https'
	basename   = "#{PhusionPassenger::PACKAGE_NAME}-#{PhusionPassenger::VERSION_STRING}"
	version    = PhusionPassenger::VERSION_STRING
	is_enterprise  = basename =~ /enterprise/
	is_open_source = !is_enterprise
	is_beta        = !!version.split('.')[3]
	tag_prefix     = is_open_source ? 'release' : 'enterprise'
	
	if !`git status --porcelain | grep -Ev '^\\?\\? '`.empty?
		STDERR.puts "-------------------"
		abort "*** ERROR: There are uncommitted files. See 'git status'"
	end
exit
	begin
		website_config = YAML.load_file(File.expand_path("~/.passenger_website.yml"))
	rescue Errno::ENOENT
		STDERR.puts "-------------------"
		abort "*** ERROR: Please put the Phusion Passenger website admin " +
			"password in ~/.passenger_website.yml:\n" +
			"admin_password: ..."
	end

	sh "git tag -s #{tag_prefix}-#{version} -u 0A212A8C -m 'Release #{version}'"

	puts "Proceed with pushing tag to remote Git repo and uploading the gem and signatures? [y/n]"
	if STDIN.readline == "y\n"
		sh "git push origin #{tag_prefix}-#{version}"
		
		if is_open_source
			sh "s3cmd -P put pkg/passenger-#{version}.{gem,tar.gz,gem.asc,tar.gz.asc} s3://phusion-passenger/releases/"
			sh "gem push pkg/passenger-#{version}.gem"
			puts "Updating version number on website..."
			if is_beta
				uri = URI.parse("https://www.phusionpassenger.com/latest_beta_version")
			else
				uri = URI.parse("https://www.phusionpassenger.com/latest_stable_version")
			end
			http = Net::HTTP.new(uri.host, uri.port)
			http.use_ssl = true
			http.verify_mode = OpenSSL::SSL::VERIFY_PEER
			request = Net::HTTP::Post.new(uri.request_uri)
			request.basic_auth("admin", website_config["admin_password"])
			request.set_form_data("version" => version)
			response = http.request(request)
			if response.code != 200 && response.body != "ok"
				abort "*** ERROR: Cannot update version number on www.phusionpassenger.com:\n" +
					"Status: #{response.code}\n\n" +
					response.body
			end
			puts "--------------"
			puts "All done."
		else
			dir = "/u/apps/passenger_website/shared"
			subdir = string_option('NAME', version)
			sh "scp pkg/#{basename}.{gem,tar.gz,gem.asc,tar.gz.asc} app@shell.phusion.nl:#{dir}/"
			sh "ssh app@shell.phusion.nl 'mkdir -p \"#{dir}/assets/#{subdir}\" && mv #{dir}/#{basename}.{gem,tar.gz,gem.asc,tar.gz.asc} \"#{dir}/assets/#{subdir}/\"'"
		end
	else
		puts "Did not upload anything."
	end
end

task 'package:gem' => Packaging::PREGENERATED_FILES do
	require 'phusion_passenger'
	sh "gem build #{PhusionPassenger::PACKAGE_NAME}.gemspec --sign --key 0x0A212A8C"
	sh "mkdir -p pkg"
	sh "mv #{PhusionPassenger::PACKAGE_NAME}-#{PhusionPassenger::VERSION_STRING}.gem pkg/"
end

task 'package:tarball' => Packaging::PREGENERATED_FILES do
	require 'phusion_passenger'
	require 'fileutils'

	basename = "#{PhusionPassenger::PACKAGE_NAME}-#{PhusionPassenger::VERSION_STRING}"
	sh "rm -rf pkg/#{basename}"
	sh "mkdir -p pkg/#{basename}"
	recursive_copy_files(ORIG_TARBALL_FILES.call, "pkg/#{basename}")
	sh "cd pkg && tar -c #{basename} | gzip --best > #{basename}.tar.gz"
	sh "rm -rf pkg/#{basename}"
end

task 'package:sign' do
	require 'phusion_passenger'

	if File.exist?(File.expand_path("~/.gnupg/gpg-agent.conf")) || ENV['GPG_AGENT_INFO']
		puts "It looks like you're using gpg-agent, so skipping automatically password caching."
	else
		begin
			require 'highline'
		rescue LoadError
			abort "Please run `gem install highline` first."
		end
		h = HighLine.new
		password = h.ask("Password for software-signing@phusion.nl GPG key: ") { |q| q.echo = false }
		passphrase_opt = "--passphrase-file .gpg-password"
	end
	
	begin
		if password
			File.open(".gpg-password", "w", 0600) do |f|
				f.write(password)
			end
		end
		version = PhusionPassenger::VERSION_STRING
		["passenger-#{version}.gem",
		 "passenger-#{version}.tar.gz",
		 "passenger-enterprise-server-#{version}.gem",
		 "passenger-enterprise-server-#{version}.tar.gz"].each do |name|
			if File.exist?("pkg/#{name}")
				sh "gpg --sign --detach-sign #{passphrase_opt} --local-user software-signing@phusion.nl --armor pkg/#{name}"
			end
		end
	ensure
		File.unlink('.gpg-password') if File.exist?('.gpg-password')
	end
end

desc "Remove gem, tarball and signatures"
task 'package:clean' do
	require 'phusion_passenger'
	basename = "#{PhusionPassenger::PACKAGE_NAME}-#{PhusionPassenger::VERSION_STRING}"
	sh "rm -f pkg/#{basename}.{gem,gem.asc,tar.gz,tar.gz.asc}"
end

desc "Create a fakeroot, useful for building native packages"
task :fakeroot => [:apache2, :nginx, :doc] do
	require 'rbconfig'
	require 'fileutils'
	include RbConfig
	fakeroot = "pkg/fakeroot"
	
	# We don't use CONFIG['archdir'] and the like because we want
	# the files to be installed to /usr, and the Ruby interpreter
	# on the packaging machine might be in /usr/local.
	fake_rubylibdir = "#{fakeroot}/usr/lib/ruby/vendor_ruby"
	fake_libdir = "#{fakeroot}/usr/lib/passenger"
	fake_native_support_dir = "#{fakeroot}/usr/lib/ruby/#{CONFIG['ruby_version']}/#{CONFIG['arch']}"
	fake_agents_dir = "#{fakeroot}/usr/lib/#{GLOBAL_NAMESPACE_DIRNAME}/agents"
	fake_helper_scripts_dir = "#{fakeroot}/usr/share/#{GLOBAL_NAMESPACE_DIRNAME}/helper-scripts"
	fake_resources_dir = "#{fakeroot}/usr/share/passenger"
	fake_include_dir = "#{fakeroot}/usr/share/passenger/include"
	fake_docdir = "#{fakeroot}/usr/share/doc/#{GLOBAL_NAMESPACE_DIRNAME}"
	fake_bindir = "#{fakeroot}/usr/bin"
	fake_sbindir = "#{fakeroot}/usr/sbin"
	fake_apache2_module_dir = "#{fakeroot}/usr/lib/apache2/modules"
	fake_apache2_module = "#{fake_apache2_module_dir}/mod_passenger.so"
	fake_ruby_extension_source_dir = "#{fakeroot}/usr/share/passenger/ruby_extension_source"
	
	sh "rm -rf #{fakeroot}"
	sh "mkdir -p #{fakeroot}"
	
	# Ruby sources
	sh "mkdir -p #{fake_rubylibdir}"
	sh "cp #{PhusionPassenger.ruby_libdir}/phusion_passenger.rb #{fake_rubylibdir}/"
	sh "cp -R #{PhusionPassenger.ruby_libdir}/phusion_passenger #{fake_rubylibdir}/"

	# Phusion Passenger common libraries
	sh "mkdir -p #{fake_libdir}"
	sh "cp -R #{PhusionPassenger.lib_dir}/common #{fake_libdir}/"
	sh "rm -rf #{fake_libdir}/common/libboost_oxt"
	
	# Ruby extension binaries
	sh "mkdir -p #{fake_native_support_dir}"
	native_support_archdir = PlatformInfo.ruby_extension_binary_compatibility_id
	sh "mkdir -p #{fake_native_support_dir}"
	sh "cp -R buildout/ruby/#{native_support_archdir}/*.#{LIBEXT} #{fake_native_support_dir}/"
	
	# Agents
	sh "mkdir -p #{fake_agents_dir}"
	sh "cp -R #{PhusionPassenger.agents_dir}/* #{fake_agents_dir}/"
	sh "rm -rf #{fake_agents_dir}/*.dSYM"
	sh "rm -rf #{fake_agents_dir}/*/*.dSYM"
	sh "rm -rf #{fake_agents_dir}/*.o"
	
	# Helper scripts
	sh "mkdir -p #{fake_helper_scripts_dir}"
	sh "cp -R #{PhusionPassenger.helper_scripts_dir}/* #{fake_helper_scripts_dir}/"
	
	# Resources
	sh "mkdir -p #{fake_resources_dir}"
	sh "cp -R resources/* #{fake_resources_dir}/"

	# Headers necessary for building the Nginx module
	sh "mkdir -p #{fake_include_dir}"
	# Infer headers that the Nginx module needs
	headers = []
	Dir["ext/nginx/*.[ch]"].each do |filename|
		File.read(filename).split("\n").grep(%r{#include "common/(.+)"}) do |match|
			headers << ["ext/common/#{$1}", $1]
		end
	end
	headers.each do |header|
		target = "#{fake_include_dir}/common/#{header[1]}"
		dir = File.dirname(target)
		if !File.directory?(dir)
			sh "mkdir -p #{dir}"
		end
		sh "cp #{header[0]} #{target}"
	end

	# Nginx module sources
	sh "mkdir -p #{fake_resources_dir}/ngx_http_passenger_module"
	sh "cp ext/nginx/* #{fake_resources_dir}/ngx_http_passenger_module/"
	
	# Documentation
	sh "mkdir -p #{fake_docdir}"
	sh "cp doc/*.html #{fake_docdir}/"
	sh "cp -R doc/images #{fake_docdir}/"
	
	# User binaries
	sh "mkdir -p #{fake_bindir}"
	Packaging::USER_EXECUTABLES.each do |exe|
		sh "cp bin/#{exe} #{fake_bindir}/"
	end
	
	# Superuser binaries
	sh "mkdir -p #{fake_sbindir}"
	Packaging::SUPER_USER_EXECUTABLES.each do |exe|
		sh "cp bin/#{exe} #{fake_sbindir}/"
	end
	
	# Apache 2 module
	sh "mkdir -p #{fake_apache2_module_dir}"
	sh "cp #{APACHE2_MODULE} #{fake_apache2_module_dir}/"

	# Ruby extension sources
	sh "mkdir -p #{fake_ruby_extension_source_dir}"
	sh "cp -R #{PhusionPassenger.ruby_extension_source_dir}/* #{fake_ruby_extension_source_dir}"

	puts "Creating #{fake_rubylibdir}/phusion_passenger/locations.ini"
	File.open("#{fake_rubylibdir}/phusion_passenger/locations.ini", "w") do |f|
		f.puts "[locations]"
		f.puts "natively_packaged=true"
		f.puts "bin=/usr/bin"
		f.puts "agents=/usr/lib/passenger/agents"
		f.puts "libdir=/usr/lib/passenger"
		f.puts "helper_scripts=/usr/share/passenger/helper-scripts"
		f.puts "resources=/usr/share/passenger"
		f.puts "includedir=/usr/share/passenger/include"
		f.puts "doc=/usr/share/doc/passenger"
		f.puts "rubylibdir=/usr/lib/ruby/vendor_ruby"
		f.puts "apache2_module=/usr/lib/apache2/modules/mod_passenger.so"
		f.puts "ruby_extension_source=/usr/share/passenger/ruby_extension_source"
	end

	sh "find #{fakeroot} -name .DS_Store -print0 | xargs -0 rm -f"
end
