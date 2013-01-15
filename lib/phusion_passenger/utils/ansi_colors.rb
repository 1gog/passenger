#  Phusion Passenger - https://www.phusionpassenger.com/
#  Copyright (c) 2010 Phusion
#
#  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
#
#  See LICENSE file for license information.

module PhusionPassenger
module Utils

module AnsiColors
	RESET    = "\e[0m".freeze
	BOLD     = "\e[1m".freeze
	RED      = "\e[31m".freeze
	GREEN    = "\e[32m".freeze
	YELLOW   = "\e[33m".freeze
	WHITE    = "\e[37m".freeze
	BLACK_BG = "\e[40m".freeze
	BLUE_BG  = "\e[44m".freeze
	DEFAULT_TERMINAL_COLOR = "#{RESET}#{WHITE}#{BLACK_BG}".freeze
	
	extend self  # Make methods available as class methods.
	
	def self.included(klass)
		# When included into another class, make sure that Utils
		# methods are made private.
		public_instance_methods(false).each do |method_name|
			klass.send(:private, method_name)
		end
	end
	
	def ansi_colorize(text)
		text = text.gsub(%r{<b>(.*?)</b>}m, "#{BOLD}\\1#{DEFAULT_TERMINAL_COLOR}")
		text.gsub!(%r{<red>(.*?)</red>}m, "#{BOLD}#{RED}\\1#{DEFAULT_TERMINAL_COLOR}")
		text.gsub!(%r{<green>(.*?)</green>}m, "#{BOLD}#{GREEN}\\1#{DEFAULT_TERMINAL_COLOR}")
		text.gsub!(%r{<yellow>(.*?)</yellow>}m, "#{BOLD}#{YELLOW}\\1#{DEFAULT_TERMINAL_COLOR}")
		text.gsub!(%r{<banner>(.*?)</banner>}m, "#{BOLD}#{BLUE_BG}#{YELLOW}\\1#{DEFAULT_TERMINAL_COLOR}")
		return text
	end
end

end # module Utils
end # module PhusionPassenger
