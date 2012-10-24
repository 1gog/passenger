# encoding: binary
#  Phusion Passenger - http://www.modrails.com/
#  Copyright (c) 2010-2012 Phusion
#
#  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
#
#  See LICENSE file for license information.

require 'phusion_passenger/utils/tee_input'

module PhusionPassenger
module Rack

module ThreadHandlerExtension
	# Constants which exist to relieve Ruby's garbage collector.
	RACK_VERSION       = "rack.version"        # :nodoc:
	RACK_VERSION_VALUE = [1, 0]                # :nodoc:
	RACK_INPUT         = "rack.input"          # :nodoc:
	RACK_ERRORS        = "rack.errors"         # :nodoc:
	RACK_MULTITHREAD   = "rack.multithread"    # :nodoc:
	RACK_MULTIPROCESS  = "rack.multiprocess"   # :nodoc:
	RACK_RUN_ONCE      = "rack.run_once"       # :nodoc:
	RACK_URL_SCHEME	   = "rack.url_scheme"     # :nodoc:
	SCRIPT_NAME        = "SCRIPT_NAME"         # :nodoc:
	HTTPS          = "HTTPS"  # :nodoc:
	HTTPS_DOWNCASE = "https"  # :nodoc:
	HTTP           = "http"   # :nodoc:
	YES            = "yes"    # :nodoc:
	ON             = "on"     # :nodoc:
	ONE            = "1"      # :nodoc:
	CRLF           = "\r\n"   # :nodoc:
	NEWLINE        = "\n"     # :nodoc:
	STATUS         = "Status: "       # :nodoc:
	NAME_VALUE_SEPARATOR = ": "       # :nodoc:

	def process_request(env, connection, full_http_response)
		rewindable_input = PhusionPassenger::Utils::TeeInput.new(connection, env)
		begin
			env[RACK_VERSION]      = RACK_VERSION_VALUE
			env[RACK_INPUT]        = rewindable_input
			env[RACK_ERRORS]       = STDERR
			env[RACK_MULTITHREAD]  = @request_handler.concurrency > 1
			env[RACK_MULTIPROCESS] = true
			env[RACK_RUN_ONCE]     = false
			if env[HTTPS] == YES || env[HTTPS] == ON || env[HTTPS] == ONE
				env[RACK_URL_SCHEME] = HTTPS_DOWNCASE
			else
				env[RACK_URL_SCHEME] = HTTP
			end
			
			status, headers, body = @app.call(env)
			begin
				if full_http_response
					connection.write("HTTP/1.1 #{status.to_i.to_s} Whatever#{CRLF}")
					connection.write("Connection: close#{CRLF}")
				end
				headers_output = [
					STATUS, status.to_i.to_s, CRLF
				]
				headers.each do |key, values|
					if values.is_a?(String)
						values = values.split(NEWLINE)
					end
					values.each do |value|
						headers_output << key
						headers_output << NAME_VALUE_SEPARATOR
						headers_output << value
						headers_output << CRLF
					end
				end
				headers_output << CRLF
				
				if body.is_a?(Array)
					# The body may be an ActionController::StringCoercion::UglyBody
					# object instead of a real Array, even when #is_a? claims so.
					# Call #to_a just to be sure.
					connection.writev2(headers_output, body.to_a)
				elsif body.is_a?(String)
					headers_output << body
					connection.writev(headers_output)
				else
					connection.writev(headers_output)
					if body
						body.each do |s|
							connection.write(s)
						end
					end
				end
			ensure
				body.close if body.respond_to?(:close)
			end
		ensure
			rewindable_input.close
		end
	end
end

end # module Rack
end # module PhusionPassenger
