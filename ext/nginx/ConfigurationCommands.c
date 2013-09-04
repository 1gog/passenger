/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2010-2013 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  See LICENSE file for license information.
 */

/*
 * ConfigurationCommands.c is automatically generated from ConfigurationCommands.c.erb,
 * using definitions from lib/phusion_passenger/nginx/config_options.rb.
 * Edits to ConfigurationCommands.c will be lost.
 *
 * To update ConfigurationCommands.c:
 *   rake nginx
 *
 * To force regeneration of ConfigurationCommands.c:
 *   rm -f ext/nginx/ConfigurationCommands.c
 *   rake ext/nginx/ConfigurationCommands.c
 */




{
	
	ngx_string("passenger_enabled"),
	NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_FLAG,
	passenger_enabled,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, enabled),
	NULL
},

{
	
	ngx_string("passenger_ruby"),
	NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, ruby),
	NULL
},

{
	
	ngx_string("passenger_python"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, python),
	NULL
},

{
	
	ngx_string("passenger_app_env"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, environment),
	NULL
},

{
	
	ngx_string("passenger_friendly_error_pages"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_FLAG,
	ngx_conf_set_flag_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, friendly_error_pages),
	NULL
},

{
	
	ngx_string("passenger_min_instances"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_num_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, min_instances),
	NULL
},

{
	
	ngx_string("passenger_max_requests"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_num_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, max_requests),
	NULL
},

{
	
	ngx_string("passenger_start_timeout"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_num_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, start_timeout),
	NULL
},

{
	
	ngx_string("passenger_base_uri"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_array_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, base_uris),
	NULL
},

{
	
	ngx_string("passenger_user"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, user),
	NULL
},

{
	
	ngx_string("passenger_group"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, group),
	NULL
},

{
	
	ngx_string("passenger_app_group_name"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, app_group_name),
	NULL
},

{
	
	ngx_string("passenger_app_root"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, app_root),
	NULL
},

{
	
	ngx_string("passenger_app_rights"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, app_rights),
	NULL
},

{
	
	ngx_string("union_station_support"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_FLAG,
	ngx_conf_set_flag_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, union_station_support),
	NULL
},

{
	
	ngx_string("union_station_filter"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	union_station_filter,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, union_station_filters),
	NULL
},

{
	
	ngx_string("passenger_debugger"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_FLAG,
	ngx_conf_set_flag_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, debugger),
	NULL
},

{
	
	ngx_string("passenger_show_version_in_header"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_FLAG,
	ngx_conf_set_flag_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, show_version_in_header),
	NULL
},

{
	
	ngx_string("passenger_max_preloader_idle_time"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_num_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, max_preloader_idle_time),
	NULL
},

{
	
	ngx_string("passenger_ignore_headers"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_1MORE,
	ngx_conf_set_bitmask_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, upstream_config.ignore_headers),
	NULL
},

{
	
	ngx_string("passenger_set_cgi_param"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE2,
	set_null_terminated_keyval_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, vars_source),
	NULL
},

{
	
	ngx_string("passenger_pass_header"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_array_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, upstream_config.pass_headers),
	NULL
},

{
	
	ngx_string("passenger_ignore_client_abort"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_FLAG,
	ngx_conf_set_flag_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, upstream_config.ignore_client_abort),
	NULL
},

{
	
	ngx_string("passenger_buffer_response"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_FLAG,
	ngx_conf_set_flag_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, upstream_config.buffering),
	NULL
},

{
	
	ngx_string("passenger_buffer_size"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_size_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, upstream_config.buffer_size),
	NULL
},

{
	
	ngx_string("passenger_buffers"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE2,
	ngx_conf_set_bufs_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, upstream_config.bufs),
	NULL
},

{
	
	ngx_string("passenger_busy_buffers_size"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_size_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, upstream_config.busy_buffers_size_conf),
	NULL
},

{
	
	ngx_string("passenger_intercept_errors"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_FLAG,
	ngx_conf_set_flag_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, upstream_config.intercept_errors),
	NULL
},

{
	
	ngx_string("passenger_spawn_method"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, spawn_method),
	NULL
},

{
	
	ngx_string("union_station_key"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, union_station_key),
	NULL
},

{
	
	ngx_string("passenger_max_request_queue_size"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_num_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, max_request_queue_size),
	NULL
},

{
	
	ngx_string("passenger_request_queue_overflow_status_code"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_num_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, request_queue_overflow_status_code),
	NULL
},

{
	
	ngx_string("passenger_fly_with"),
	NGX_HTTP_MAIN_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_MAIN_CONF_OFFSET,
	offsetof(passenger_main_conf_t, fly_with),
	NULL
},

{
	
	ngx_string("passenger_max_instances"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_num_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, max_instances),
	NULL
},

{
	
	ngx_string("passenger_max_request_time"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_num_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, max_request_time),
	NULL
},

{
	
	ngx_string("passenger_memory_limit"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_num_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, memory_limit),
	NULL
},

{
	
	ngx_string("passenger_concurrency_model"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, concurrency_model),
	NULL
},

{
	
	ngx_string("passenger_thread_count"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_num_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, thread_count),
	NULL
},

{
	
	ngx_string("passenger_rolling_restarts"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_FLAG,
	ngx_conf_set_flag_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, rolling_restarts),
	NULL
},

{
	
	ngx_string("passenger_resist_deployment_errors"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_FLAG,
	ngx_conf_set_flag_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, resist_deployment_errors),
	NULL
},

{
	
	ngx_string("rails_spawn_method"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, spawn_method),
	NULL
},

{
	
	ngx_string("rails_env"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, environment),
	NULL
},

{
	
	ngx_string("rack_env"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_str_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, environment),
	NULL
},

{
	
	ngx_string("rails_app_spawner_idle_time"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	ngx_conf_set_num_slot,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(passenger_loc_conf_t, max_preloader_idle_time),
	NULL
},

{
	
	ngx_string("rails_framework_spawner_idle_time"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_TAKE1,
	rails_framework_spawner_idle_time,
	NGX_HTTP_LOC_CONF_OFFSET,
	0,
	NULL
},

{
	
	ngx_string("passenger_use_global_queue"),
	NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_CONF_FLAG,
	passenger_use_global_queue,
	NGX_HTTP_LOC_CONF_OFFSET,
	0,
	NULL
},

