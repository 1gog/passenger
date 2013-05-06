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
	
	begin
		website_config = YAML.load_file(File.expand_path("~/.passenger_website.yml"))
	rescue Errno::ENOENT
		abort "*** ERROR: Please put the Phusion Passenger website admin " +
			"password in ~/.passenger_website.yml:\n" +
			"admin_password: ..."
	end

	sh "git tag -s #{tag_prefix}-#{version} -u 0A212A8C -m 'Release #{version}'"

	puts "Proceed with pushing tag to remote Git repo and uploading the gem and signatures? [y/n]"
	if STDIN.readline == "y\n"
		sh "git push origin #{tag_prefix}-#{version}"
		
		if is_open_source
			sh "scp pkg/#{basename}.{gem.asc,tar.gz.asc} app@shell.phusion.nl:/u/apps/signatures/phusion-passenger/"
			sh "./dev/googlecode_upload.py -p phusion-passenger -s 'Phusion Passenger #{version}' pkg/passenger-#{version}.tar.gz"
			sh "gem push pkg/passenger-#{version}.gem"
			puts "--------------"
			puts "All done. Please update the version number in the Phusion Passenger website."
		else
			dir = "/u/apps/passenger_website/shared"
			subdir = string_option('NAME', version)
			sh "scp pkg/#{basename}.{gem,tar.gz,gem.asc,tar.gz.asc} app@shell.phusion.nl:#{dir}/"
			sh "ssh app@shell.phusion.nl 'mkdir -p \"#{dir}/assets/#{subdir}\" && mv #{dir}/#{basename}.{gem,tar.gz,gem.asc,tar.gz.asc} \"#{dir}/assets/#{subdir}/\"'"
		end

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
	else
		puts "Did not upload anything."
	end
end

task 'package:check' do
	require 'phusion_passenger'
	
	File.read("ext/common/Constants.h") =~ /PASSENGER_VERSION \"(.+)\"/
	if $1 != PhusionPassenger::VERSION_STRING
		abort "Version number in ext/common/Constants.h doesn't match."
	end
end

task 'package:gem' => Packaging::ASCII_DOCS + ['package:check'] do
	require 'phusion_passenger'
	sh "gem build #{PhusionPassenger::PACKAGE_NAME}.gemspec --sign --key 0x0A212A8C"
	sh "mkdir -p pkg"
	sh "mv #{PhusionPassenger::PACKAGE_NAME}-#{PhusionPassenger::VERSION_STRING}.gem pkg/"
end

task 'package:tarball' => Packaging::ASCII_DOCS + ['package:check'] do
	require 'phusion_passenger'
	require 'fileutils'

	basename = "#{PhusionPassenger::PACKAGE_NAME}-#{PhusionPassenger::VERSION_STRING}"
	sh "rm -rf pkg/#{basename}"
	sh "mkdir -p pkg/#{basename}"
	files = Dir[*PhusionPassenger::Packaging::GLOB] -
		Dir[*PhusionPassenger::Packaging::EXCLUDE_GLOB]
	files.each_with_index do |filename, i|
		dir = File.dirname(filename)
		if !File.exist?("pkg/#{basename}/#{dir}")
			FileUtils.mkdir_p("pkg/#{basename}/#{dir}")
		end
		if !File.directory?(filename)
			FileUtils.install(filename, "pkg/#{basename}/#{filename}")
		end
		printf "\r[%5d/%5d] [%3.0f%%] Copying files...", i, files.size, i * 100.0 / files.size
		STDOUT.flush
	end
	puts
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
task :fakeroot => [:apache2, :nginx] + Packaging::ASCII_DOCS do
	require 'rbconfig'
	require 'fileutils'
	include Config
	fakeroot = "pkg/fakeroot"
	
	# We don't use CONFIG['archdir'] and the like because we want
	# the files to be installed to /usr, and the Ruby interpreter
	# on the packaging machine might be in /usr/local.
	fake_libdir = "#{fakeroot}/usr/lib/ruby/#{CONFIG['ruby_version']}"
	fake_native_support_dir = "#{fakeroot}/usr/lib/ruby/#{CONFIG['ruby_version']}/#{CONFIG['arch']}"
	fake_agents_dir = "#{fakeroot}#{NATIVELY_PACKAGED_AGENTS_DIR}"
	fake_helper_scripts_dir = "#{fakeroot}#{NATIVELY_PACKAGED_HELPER_SCRIPTS_DIR}"
	fake_resources_dir = "#{fakeroot}/usr/share/phusion-passenger"
	fake_docdir = "#{fakeroot}#{NATIVELY_PACKAGED_DOC_DIR}"
	fake_bindir = "#{fakeroot}/usr/bin"
	fake_sbindir = "#{fakeroot}/usr/sbin"
	fake_apache2_module = "#{fakeroot}#{NATIVELY_PACKAGED_APACHE2_MODULE}"
	fake_apache2_module_dir = File.dirname(fake_apache2_module)
	
	sh "rm -rf #{fakeroot}"
	sh "mkdir -p #{fakeroot}"
	
	sh "mkdir -p #{fake_libdir}"
	sh "cp #{PhusionPassenger.ruby_libdir}/phusion_passenger.rb #{fake_libdir}/"
	sh "cp -R #{PhusionPassenger.ruby_libdir}/phusion_passenger #{fake_libdir}/"
	
	sh "mkdir -p #{fake_native_support_dir}"
	native_support_archdir = PlatformInfo.ruby_extension_binary_compatibility_id
	sh "mkdir -p #{fake_native_support_dir}"
	sh "cp -R libout/ruby/#{native_support_archdir}/*.#{LIBEXT} #{fake_native_support_dir}/"
	
	sh "mkdir -p #{fake_agents_dir}"
	sh "cp -R #{PhusionPassenger.agents_dir}/* #{fake_agents_dir}/"
	sh "rm -rf #{fake_agents_dir}/*.dSYM"
	sh "rm -rf #{fake_agents_dir}/*/*.dSYM"
	
	sh "mkdir -p #{fake_helper_scripts_dir}"
	sh "cp -R #{PhusionPassenger.helper_scripts_dir}/* #{fake_helper_scripts_dir}/"
	
	sh "mkdir -p #{fake_resources_dir}"
	sh "cp -R resources/* #{fake_resources_dir}/"
	
	sh "mkdir -p #{fake_docdir}"
	Packaging::ASCII_DOCS.each do |docfile|
		sh "cp", docfile, "#{fake_docdir}/"
	end
	sh "cp -R doc/images #{fake_docdir}/"
	
	sh "mkdir -p #{fake_bindir}"
	Packaging::USER_EXECUTABLES.each do |exe|
		sh "cp bin/#{exe} #{fake_bindir}/"
	end
	
	sh "mkdir -p #{fake_sbindir}"
	Packaging::SUPER_USER_EXECUTABLES.each do |exe|
		sh "cp bin/#{exe} #{fake_sbindir}/"
	end
	
	sh "mkdir -p #{fake_apache2_module_dir}"
	sh "cp #{APACHE2_MODULE} #{fake_apache2_module_dir}/"

	sh "find #{fakeroot} -name .DS_Store -print0 | xargs -0 rm -f"
end

desc "Create a Debian package"
task 'package:debian' => 'package:check' do
	checkbuilddeps = PlatformInfo.find_command("dpkg-checkbuilddeps")
	debuild = PlatformInfo.find_command("debuild")
	if !checkbuilddeps || !debuild
		# devscripts requires dpkg-dev which contains dpkg-checkbuilddeps.
		abort "Please run `apt-get install devscripts` first."
	end
	
	if !system(checkbuilddeps)
		STDERR.puts
		abort "Please install aforementioned build dependencies first."
	end
	
	sh "debuild"
end
