#!/bin/bash
set -e

export VERBOSE=1
export TRACE=1
export rvmsudo_secure_path=1

sudo sh -c 'cat >> /etc/hosts' <<EOF
127.0.0.1 passenger.test
127.0.0.1 mycook.passenger.test
127.0.0.1 zsfa.passenger.test
127.0.0.1 norails.passenger.test
127.0.0.1 1.passenger.test 2.passenger.test 3.passenger.test
127.0.0.1 4.passenger.test 5.passenger.test 6.passenger.test
127.0.0.1 7.passenger.test 8.passenger.test 9.passenger.test
EOF

function run()
{
	echo "$ $@"
	"$@"
}

function apt_get_update() {
	if [[ "$apt_get_updated" = "" ]]; then
		apt_get_updated=1
		if [[ "$TEST_DEBIAN_PACKAGING" = 1 ]]; then
			if ! [[ -e /usr/bin/add-apt-repository ]]; then
				run sudo apt-get update
				run sudo apt-get install python-software-properties
			fi
			run sudo add-apt-repository -y ppa:phusion.nl/misc
		fi
		run sudo apt-get update
	fi
}

function install_test_deps_without_rails_bundles()
{
	if [[ "$install_test_deps_without_rails_bundles" = "" ]]; then
		install_test_deps_without_rails_bundles=1
		run rake test:install_deps RAILS_BUNDLES=no
	fi
}

function install_test_deps_without_rails_bundles_without_doctools()
{
	if [[ "$install_test_deps_without_rails_bundles_without_doctools" = "" ]]; then
		install_test_deps_without_rails_bundles_without_doctools=1
		rake test:install_deps RAILS_BUNDLES=no DOCTOOLS=no
	fi
}

if [[ -f /persist/passenger-enterprise-license ]]; then
	run sudo cp /persist/passenger-enterprise-license /etc/
	echo "Using /persist/ccache as ccache directory"
	export USE_CCACHE=1
	export CCACHE_DIR=/persist/ccache
	export CCACHE_COMPRESS=1
	export CCACHE_COMPRESS_LEVEL=3
	run sudo apt-get install ccache
fi

run uname -a
run lsb_release -a
sudo tee /etc/dpkg/dpkg.cfg.d/02apt-speedup >/dev/null <<<"force-unsafe-io"
cp test/config.json.travis test/config.json

# Relax permissions on home directory so that the application root
# permission checks pass.
chmod g+x,o+x $HOME

if [[ "$TEST_RUBY_VERSION" != "" ]]; then
	echo "$ rvm use $TEST_RUBY_VERSION"
	if [[ -f ~/.rvm/scripts/rvm ]]; then
		source ~/.rvm/scripts/rvm
	else
		source /usr/local/rvm/scripts/rvm
	fi
	rvm use $TEST_RUBY_VERSION
	if [[ "$TEST_RUBYGEMS_VERSION" = "" ]]; then
		run gem --version
	fi
fi

if [[ "$TEST_RUBYGEMS_VERSION" != "" ]]; then
	run rvm install rubygems $TEST_RUBYGEMS_VERSION
	run gem --version
fi

if [[ "$TEST_CXX" = 1 ]]; then
	run rake test:install_deps RAILS_BUNDLES=no DOCTOOLS=no
	run rake test:cxx
	run rake test:oxt
fi

if [[ "$TEST_RUBY" = 1 ]]; then
	run rake test:install_deps DOCTOOLS=no
	run rake test:ruby
fi

if [[ "$TEST_NGINX" = 1 ]]; then
	run rake test:install_deps RAILS_BUNDLES=no DOCTOOLS=no
	run ./bin/passenger-install-nginx-module --auto --prefix=/tmp/nginx --auto-download
	run rake test:integration:nginx
fi

if [[ "$TEST_APACHE2" = 1 ]]; then
	apt_get_update
	run sudo apt-get install -y --no-install-recommends \
		apache2-mpm-worker apache2-threaded-dev
	install_test_deps_without_rails_bundles_without_doctools
	run ./bin/passenger-install-apache2-module --auto
	run rake test:integration:apache2
fi

if [[ "$TEST_STANDALONE" = 1 ]]; then
	apt_get_update
	install_test_deps_without_rails_bundles_without_doctools
	run rake test:integration:standalone
fi

if [[ "$TEST_DEBIAN_PACKAGING" = 1 ]]; then
	apt_get_update
	run sudo apt-get install -y --no-install-recommends \
		devscripts debhelper rake apache2-mpm-worker apache2-threaded-dev \
		ruby1.8 ruby1.8-dev ruby1.9.1 ruby1.9.1-dev rubygems libev-dev gdebi-core \
		source-highlight
	install_test_deps_without_rails_bundles
	run rake debian:dev debian:dev:reinstall
	run rake test:integration:native_packaging \
		LOCATIONS_INI=/usr/lib/ruby/vendor_ruby/phusion_passenger/locations.ini \
		SUDO=1
	run env PASSENGER_LOCATION_CONFIGURATION_FILE=/usr/lib/ruby/vendor_ruby/phusion_passenger/locations.ini \
		rake test:integration:apache2 SUDO=1
fi

if [[ "$TEST_SOURCE_PACKAGING" = 1 ]]; then
	apt_get_update
	run sudo apt-get install -y --no-install-recommends source-highlight
	install_test_deps_without_rails_bundles
	run rspec -f s -c test/integration_tests/source_packaging_test.rb
fi
