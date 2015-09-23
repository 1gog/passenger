#  Phusion Passenger - https://www.phusionpassenger.com/
#  Copyright (c) 2010-2015 Phusion
#
#  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
#
#  See LICENSE file for license information.

module PhusionPassenger

  module Packaging
    # A list of HTML files that are generated with Asciidoc.
    ASCII_DOCS = [
      'doc/Users guide.html',
      'doc/Users guide Apache.html',
      'doc/Users guide Nginx.html',
      'doc/Users guide Standalone.html',
      'doc/Security of user switching support.html',
      'doc/Design and Architecture.html'
    ]

    # Files that must be generated before packaging.
    PREGENERATED_FILES = [
      'src/cxx_supportlib/Constants.h',
      'doc/Packaging.html',
      'doc/CloudLicensingConfiguration.html',
      'doc/ServerOptimizationGuide.html'
    ] + ASCII_DOCS

    USER_EXECUTABLES = [
      'passenger',
      'passenger-install-apache2-module',
      'passenger-install-nginx-module',
      'passenger-config',
      'flying-passenger'
    ]

    SUPER_USER_EXECUTABLES = [
      'passenger-status',
      'passenger-memory-stats',
      'passenger-irb'
    ]

    # Used during native packaging. Specifies executables for
    # which the shebang should NOT be set to #!/usr/bin/ruby,
    # so that these executables can be run with any Ruby interpreter
    # the user desires.
    EXECUTABLES_WITH_FREE_RUBY = [
      'passenger',
      'passenger-config',
      'passenger-install-apache2-module',
      'passenger-install-nginx-module',
      'passenger-irb',
      'flying-passenger'
    ]

    # A list of globs which match all files that should be packaged
    # in the Phusion Passenger gem or tarball.
    GLOB = [
      '.editorconfig',
      'configure',
      'Rakefile',
      'README.md',
      'CONTRIBUTORS',
      'CONTRIBUTING.md',
      'LICENSE',
      'CHANGELOG',
      'INSTALL.md',
      'NEWS',
      'package.json',
      'npm-shrinkwrap.json',
      'passenger-enterprise-server.gemspec',
      'build/*',
      'bin/*',
      'doc/**/*',
      'man/*',
      'dev/**/*',
      'src/**/*',
      'resources/**/*'
    ]

    # Files that should be excluded from the gem or tarball. Overrides GLOB.
    EXCLUDE_GLOB = [
      '**/.DS_Store',
      '.gitignore',
      '.gitmodules',
      '.travis.yml',
      '.settings/*',
      '.externalToolBuilders/*',
      '.cproject',
      '.project',
      'Gemfile',
      'Gemfile.lock',
      'Vagrantfile',
      'Passenger.sublime-project',
      'Passenger.xcodeproj/**/*',
      'src/ruby_supportlib/phusion_passenger/vendor/*/.*',
      'src/ruby_supportlib/phusion_passenger/vendor/*/hacking/**/*',
      'src/ruby_supportlib/phusion_passenger/vendor/*/spec/**/*',
      'src/ruby_supportlib/phusion_passenger/vendor/union_station_hooks_rails/rails_test_apps/**/*',
      'packaging/**/*',
      'test/**/*'
    ]

    # Files and directories that should be excluded from the Homebrew installation.
    HOMEBREW_EXCLUDE = [
      "package.json", "npm-shrinkwrap.json"
    ]

    def self.files
      result = Dir[*GLOB] - Dir[*EXCLUDE_GLOB]
      result.reject! { |path| path =~ %r{/\.\.?$} }
      result
    end
  end

end # module PhusionPassenger
