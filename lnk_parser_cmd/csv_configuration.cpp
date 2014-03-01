#include "globals.h"

char total_columns = 0;
char column_count = 0;

char col_date_created = 0;
char col_last_accessed = 1;
char col_last_modified = 2;
char col_file_size = 3;
char col_file_attributes = 4;
char col_icon_index = 5;
char col_showwindow_value = 6;
char col_hot_key_value = 7;
char col_link_flags = 8;
char col_link_target_id_list = 9;
char col_location_flags = 10;
char col_drive_type = 11;
char col_drive_serial_number = 12;
char col_volume_label_a = 13;
char col_volume_label_u = 14;
char col_local_path_a = 15;
char col_network_share_flags = 16;
char col_network_provider_type = 17;
char col_network_share_name_a = 18;
char col_device_name_a = 19;
char col_network_share_name_u = 20;
char col_device_name_u = 21;
char col_common_path_a = 22;
char col_local_path_u = 23;
char col_common_path_u = 24;
char col_comment_u = 25;
char col_comment_a = 26;
char col_relative_path_u = 27;
char col_relative_path_a = 28;
char col_working_directory_u = 29;
char col_working_directory_a = 30;
char col_arguments_u = 31;
char col_arguments_a = 32;
char col_icon_location_u = 33;
char col_icon_location_a = 34;
char col_color_flags = 35;
char col_screen_buffer_width = 36;
char col_screen_buffer_height = 37;
char col_window_width = 38;
char col_window_height = 39;
char col_window_x_coordinate = 40;
char col_window_y_coordinate = 41;
char col_font_size = 42;
char col_font_family_value = 43;
char col_font_weight = 44;
char col_font_face_name = 45;
char col_cursor_size = 46;
char col_fullscreen = 47;
char col_quickedit_mode = 48;
char col_insert_mode = 49;
char col_automatic_positioning = 50;
char col_history_buffer_size = 51;
char col_number_of_history_buffers = 52;
char col_duplicates_allowed_in_history = 53;
char col_color_table_values = 54;
char col_code_page = 55;
char col_application_identifier_a = 56;
char col_application_identifier_u = 57;
char col_environment_variables_location_a = 58;
char col_environment_variables_location_u = 59;
char col_icon_location2_a = 60;
char col_icon_location2_u = 61;
char col_known_folder_guid = 62;
char col_known_folder_first_child_segment_offset = 63;
char col_metadata_property_store = 64;
char col_shim_layer_u = 65;
char col_special_folder_identifier = 66;
char col_special_folder_first_child_segment_offset = 67;
char col_version = 68;
char col_netbios_name = 69;
char col_droid_volume_identifier = 70;
char col_droid_file_identifier = 71;
char col_birth_droid_volume_identifier = 72;
char col_birth_droid_file_identifier = 73;
char col_mac_address = 74;
char col_uuid_timestamp = 75;
char col_uuid_sequence_number = 76;
char col_distributed_link_tracker_notes = 77;
char col_vista_and_above_id_list = 78;
char col_output_notes = 79;

// Configuration values for the CSV columns.
bool cfg_date_created = true;
bool cfg_last_accessed = true;
bool cfg_last_modified = true;
bool cfg_file_size = true;
bool cfg_file_attributes = true;
bool cfg_icon_index = true;
bool cfg_showwindow_value = true;
bool cfg_hot_key_value = true;
bool cfg_link_flags = true;
bool cfg_link_target_id_list = true;
bool cfg_location_flags = true;
bool cfg_drive_type = true;
bool cfg_drive_serial_number = true;
bool cfg_volume_label_a = true;
bool cfg_volume_label_u = true;
bool cfg_local_path_a = true;
bool cfg_network_share_flags = true;
bool cfg_network_provider_type = true;
bool cfg_network_share_name_a = true;
bool cfg_device_name_a = true;
bool cfg_network_share_name_u = true;
bool cfg_device_name_u = true;
bool cfg_common_path_a = true;
bool cfg_local_path_u = true;
bool cfg_common_path_u = true;
bool cfg_comment_u = true;
bool cfg_comment_a = true;
bool cfg_relative_path_u = true;
bool cfg_relative_path_a = true;
bool cfg_working_directory_u = true;
bool cfg_working_directory_a = true;
bool cfg_arguments_u = true;
bool cfg_arguments_a = true;
bool cfg_icon_location_u = true;
bool cfg_icon_location_a = true;
bool cfg_color_flags = true;
bool cfg_screen_buffer_width = true;
bool cfg_screen_buffer_height = true;
bool cfg_window_width = true;
bool cfg_window_height = true;
bool cfg_window_x_coordinate = true;
bool cfg_window_y_coordinate = true;
bool cfg_font_size = true;
bool cfg_font_family_value = true;
bool cfg_font_weight = true;
bool cfg_font_face_name = true;
bool cfg_cursor_size = true;
bool cfg_fullscreen = true;
bool cfg_quickedit_mode = true;
bool cfg_insert_mode = true;
bool cfg_automatic_positioning = true;
bool cfg_history_buffer_size = true;
bool cfg_number_of_history_buffers = true;
bool cfg_duplicates_allowed_in_history = true;
bool cfg_color_table_values = true;
bool cfg_code_page = true;
bool cfg_application_identifier_a = true;
bool cfg_application_identifier_u = true;
bool cfg_environment_variables_location_a = true;
bool cfg_environment_variables_location_u = true;
bool cfg_icon_location2_a = true;
bool cfg_icon_location2_u = true;
bool cfg_known_folder_guid = true;
bool cfg_known_folder_first_child_segment_offset = true;
bool cfg_metadata_property_store = true;
bool cfg_shim_layer_u = true;
bool cfg_special_folder_identifier = true;
bool cfg_special_folder_first_child_segment_offset = true;
bool cfg_version = true;
bool cfg_netbios_name = true;
bool cfg_droid_volume_identifier = true;
bool cfg_droid_file_identifier = true;
bool cfg_birth_droid_volume_identifier = true;
bool cfg_birth_droid_file_identifier = true;
bool cfg_mac_address = true;
bool cfg_uuid_timestamp = true;
bool cfg_uuid_sequence_number = true;
bool cfg_distributed_link_tracker_notes = true;
bool cfg_vista_and_above_id_list = true;
bool cfg_output_notes = true;

void check_config()
{
	// Open our config file if it exists.
	HANDLE hFile_cfg = CreateFileA( "lpc.cfg", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile_cfg != INVALID_HANDLE_VALUE )
	{
		DWORD read = 0;
		DWORD fz = GetFileSize( hFile_cfg, NULL );

		// Our config file is going to be small. If it's something else, we're not going to read it.
		if ( fz > 0 && fz < 10240 )
		{
			char *cfg_buf = ( char * )malloc( sizeof( char ) * fz );

			// Get the header information for this link.
			ReadFile( hFile_cfg, cfg_buf, sizeof( char ) * fz, &read, NULL );

			// Read the config. It must be in the order specified below.
			if ( read == fz )
			{
				char next = 0;
				char *cfg_val = strchr( cfg_buf, ':' );
				bool set = false;
				while ( cfg_val != NULL && next < 80 && cfg_val + 2 != NULL )
				{
					set = ( cfg_val[ 1 ] == '1' ? true : false );
					switch ( next )
					{
						case 0: { cfg_date_created = set; col_date_created = total_columns; } break;
						case 1: { cfg_last_accessed = set; col_last_accessed = total_columns; } break;
						case 2: { cfg_last_modified = set; col_last_modified = total_columns; } break;
						case 3: { cfg_file_size = set; col_file_size = total_columns; } break;
						case 4: { cfg_file_attributes = set; col_file_attributes = total_columns; } break;
						case 5: { cfg_icon_index = set; col_icon_index = total_columns; } break;
						case 6: { cfg_showwindow_value = set; col_showwindow_value = total_columns; } break;
						case 7: { cfg_hot_key_value = set; col_hot_key_value = total_columns; } break;
						case 8: { cfg_link_flags = set; col_link_flags = total_columns; } break;
						case 9: { cfg_link_target_id_list = set; col_link_target_id_list = total_columns; } break;
						case 10: { cfg_location_flags = set; col_location_flags = total_columns; } break;
						case 11: { cfg_drive_type = set; col_drive_type = total_columns; } break;
						case 12: { cfg_drive_serial_number = set; col_drive_serial_number = total_columns; } break;
						case 13: { cfg_volume_label_a = set; col_volume_label_a = total_columns; } break;
						case 14: { cfg_volume_label_u = set; col_volume_label_u = total_columns; } break;
						case 15: { cfg_local_path_a = set; col_local_path_a = total_columns; } break;
						case 16: { cfg_network_share_flags = set; col_network_share_flags = total_columns; } break;
						case 17: { cfg_network_provider_type = set; col_network_provider_type = total_columns; } break;
						case 18: { cfg_network_share_name_a = set; col_network_share_name_a = total_columns; } break;
						case 19: { cfg_device_name_a = set; col_device_name_a = total_columns; } break;
						case 20: { cfg_network_share_name_u = set; col_network_share_name_u = total_columns; } break;
						case 21: { cfg_device_name_u = set; col_device_name_u = total_columns; } break;
						case 22: { cfg_common_path_a = set; col_common_path_a = total_columns; } break;
						case 23: { cfg_local_path_u = set; col_local_path_u = total_columns; } break;
						case 24: { cfg_common_path_u = set; col_common_path_u = total_columns; } break;
						case 25: { cfg_comment_u = set; col_comment_u = total_columns; } break;
						case 26: { cfg_comment_a = set; col_comment_a = total_columns; } break;
						case 27: { cfg_relative_path_u = set; col_relative_path_u = total_columns; } break;
						case 28: { cfg_relative_path_a = set; col_relative_path_a = total_columns; } break;
						case 29: { cfg_working_directory_u = set; col_working_directory_u = total_columns; } break;
						case 30: { cfg_working_directory_a = set; col_working_directory_a = total_columns; } break;
						case 31: { cfg_arguments_u = set; col_arguments_u = total_columns; } break;
						case 32: { cfg_arguments_a = set; col_arguments_a = total_columns; } break;
						case 33: { cfg_icon_location_u = set; col_icon_location_u = total_columns; } break;
						case 34: { cfg_icon_location_a = set; col_icon_location_a = total_columns; } break;
						case 35: { cfg_color_flags = set; col_color_flags = total_columns; } break;
						case 36: { cfg_screen_buffer_width = set; col_screen_buffer_width = total_columns; } break;
						case 37: { cfg_screen_buffer_height = set; col_screen_buffer_height = total_columns; } break;
						case 38: { cfg_window_width = set; col_window_width = total_columns; } break;
						case 39: { cfg_window_height = set; col_window_height = total_columns; } break;
						case 40: { cfg_window_x_coordinate = set; col_window_x_coordinate = total_columns; } break;
						case 41: { cfg_window_y_coordinate = set; col_window_y_coordinate = total_columns; } break;
						case 42: { cfg_font_size = set; col_font_size = total_columns; } break;
						case 43: { cfg_font_family_value = set; col_font_family_value = total_columns; } break;
						case 44: { cfg_font_weight = set; col_font_weight = total_columns; } break;
						case 45: { cfg_font_face_name = set; col_font_face_name = total_columns; } break;
						case 46: { cfg_cursor_size = set; col_cursor_size = total_columns; } break;
						case 47: { cfg_fullscreen = set; col_fullscreen = total_columns; } break;
						case 48: { cfg_quickedit_mode = set; col_quickedit_mode = total_columns; } break;
						case 49: { cfg_insert_mode = set; col_insert_mode = total_columns; } break;
						case 50: { cfg_automatic_positioning = set; col_automatic_positioning = total_columns; } break;
						case 51: { cfg_history_buffer_size = set; col_history_buffer_size = total_columns; } break;
						case 52: { cfg_number_of_history_buffers = set; col_number_of_history_buffers = total_columns; } break;
						case 53: { cfg_duplicates_allowed_in_history = set; col_duplicates_allowed_in_history = total_columns; } break;
						case 54: { cfg_color_table_values = set; col_color_table_values = total_columns; } break;
						case 55: { cfg_code_page = set; col_code_page = total_columns; } break;
						case 56: { cfg_application_identifier_a = set; col_application_identifier_a = total_columns; } break;
						case 57: { cfg_application_identifier_u = set; col_application_identifier_u = total_columns; } break;
						case 58: { cfg_environment_variables_location_a = set; col_environment_variables_location_a = total_columns; } break;
						case 59: { cfg_environment_variables_location_u = set; col_environment_variables_location_u = total_columns; } break;
						case 60: { cfg_icon_location2_a = set; col_icon_location2_a = total_columns; } break;
						case 61: { cfg_icon_location2_u = set; col_icon_location2_u = total_columns; } break;
						case 62: { cfg_known_folder_guid = set; col_known_folder_guid = total_columns; } break;
						case 63: { cfg_known_folder_first_child_segment_offset = set; col_known_folder_first_child_segment_offset = total_columns; } break;
						case 64: { cfg_metadata_property_store = set; col_metadata_property_store = total_columns; } break;
						case 65: { cfg_shim_layer_u = set; col_shim_layer_u = total_columns; } break;
						case 66: { cfg_special_folder_identifier = set; col_special_folder_identifier = total_columns; } break;
						case 67: { cfg_special_folder_first_child_segment_offset = set; col_special_folder_first_child_segment_offset = total_columns; } break;
						case 68: { cfg_version = set; col_version = total_columns; } break;
						case 69: { cfg_netbios_name = set; col_netbios_name = total_columns; } break;
						case 70: { cfg_droid_volume_identifier = set; col_droid_volume_identifier = total_columns; } break;
						case 71: { cfg_droid_file_identifier = set; col_droid_file_identifier = total_columns; } break;
						case 72: { cfg_birth_droid_volume_identifier = set; col_birth_droid_volume_identifier = total_columns; } break;
						case 73: { cfg_birth_droid_file_identifier = set; col_birth_droid_file_identifier = total_columns; } break;
						case 74: { cfg_mac_address = set; col_mac_address = total_columns; } break;
						case 75: { cfg_uuid_timestamp = set; col_uuid_timestamp = total_columns; } break;
						case 76: { cfg_uuid_sequence_number = set; col_uuid_sequence_number = total_columns; } break;
						case 77: { cfg_distributed_link_tracker_notes = set; col_distributed_link_tracker_notes = total_columns; } break;
						case 78: { cfg_vista_and_above_id_list = set; col_vista_and_above_id_list = total_columns; } break;
						case 79: { cfg_output_notes = set; col_output_notes = total_columns; } break;
					}
					if ( set == true ){ total_columns++; }
					++next;
					cfg_val = strchr( cfg_val + 2, ':' );
				}
			}
			else
			{
				printf( "Failed to load the CSV configuration file. All items will be displayed in the report.\n" );
			}

			free( cfg_buf );
		}
		else if ( fz == 0 )	// We could generate a default config for the user...
		{
			printf( "The CSV configuration file is empty. All items will be displayed in the report.\n" );
		}
		else // Config is too large/Not the right config.
		{
			printf( "The CSV configuration file is too large. All items will be displayed in the report.\n" );
		}

		CloseHandle( hFile_cfg );
	}
}
