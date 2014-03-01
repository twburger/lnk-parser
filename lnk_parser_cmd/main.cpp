#include "globals.h"

bool dump_remnant = false;

HANDLE hFile_html = INVALID_HANDLE_VALUE;
HANDLE hFile_csv = INVALID_HANDLE_VALUE;

bool output_html = false;
bool output_csv = false;
bool silent = false;

unsigned int write_offset2 = 0;
wchar_t write_buf2[ BUFFER_SIZE ];

unsigned int write_offset = 0;
wchar_t write_buf[ BUFFER_SIZE ];

int wmain( int argc, wchar_t *argv[] )
{
	// Ask user for input filename.
	unsigned int name_length = 0;
	wchar_t name[ MAX_PATH ] = { 0 };
	wchar_t output_path[ MAX_PATH ] = { 0 };
	if ( argc == 1 )
	{
		printf( "Please enter the name of the shortcut file, or a directory you wish to scan: " );
		fgetws( name, MAX_PATH, stdin );

		// Remove the newline character if it was appended.
		name_length = wcslen( name );
		if ( name[ name_length - 1 ] == L'\n' )
		{
			name[ --name_length ] = L'\0';
		}

		printf( "Select a report to output:\n 1\tHTML\n 2\tComma-separated values (CSV)\n 3\tHTML and CSV\n 0\tNo report\nSelect: " );
		wint_t choice = getwchar();		// Newline character will remain in buffer.
		if ( choice == L'1' )
		{
			output_html = true;
		}
		else if ( choice == L'2' )
		{
			output_csv = true;
		}
		else if ( choice == L'3' )
		{
			output_html = output_csv = true;
		}

		if ( output_html == true )
		{
			printf( "Do you want to dump remnant and unprocessed data? (Y/N) " );
			while ( getwchar() != L'\n' );	// Clear the input buffer.
			choice = getwchar();			// Newline character will remain in buffer.
			if ( choice == L'y' || choice == L'Y' )
			{
				dump_remnant = true;
			}
		}

		printf( "Do you want to output results to the console? (Y/N) " );
		while ( getwchar() != L'\n' );	// Clear the input buffer.
		choice = getwchar();			// Newline character will remain in buffer.
		if ( choice == L'n' || choice == L'N' )
		{
			silent = true;
		}

		while ( getwchar() != L'\n' );	// Clear the input buffer.

		if ( output_html == true || output_csv == true )
		{
			printf( "Please enter a path to output the report file(s) (Press Enter for the current directory): " );
			fgetws( output_path, MAX_PATH, stdin );

			// Remove the newline character if it was appended.
			int output_length = wcslen( output_path );
			if ( output_path[ output_length - 1 ] == L'\n' )
			{
				output_path[ output_length - 1 ] = L'\0';
			}
		}
	}
	else
	{
		// We're going to designate the last argument as the database path.
		name_length = wcslen( argv[ argc - 1 ] );
		wmemcpy_s( name, MAX_PATH, argv[ argc - 1 ], ( name_length > MAX_PATH ? MAX_PATH : name_length ) );

		// Go through each argument and set the appropriate switch.
		for ( int i = 1; i <= ( argc - 1 ); i++ )
		{
			if ( wcslen( argv[ i ] ) > 1 && ( argv[ i ][ 0 ] == '-' || argv[ i ][ 0 ] == '/' ) )
			{
				switch ( argv[ i ][ 1 ] )
				{
					case 'o':
					case 'O':
					{
						// Make sure our output switch is not the second to last argument.
						if ( i < ( argc - 2 ) )
						{
							int output_length = wcslen( argv[ ++i ] );
							wmemcpy_s( output_path, MAX_PATH, argv[ i ], ( output_length > MAX_PATH ? MAX_PATH : output_length ) );
						}
					}
					break;

					case 'w':
					case 'W':
					{
						output_html = true;
					}
					break;

					case 'c':
					case 'C':
					{
						output_csv = true;
					}
					break;

					case 'r':
					case 'R':
					{
						dump_remnant = true;
					}
					break;

					case 's':
					case 'S':
					{
						silent = true;
					}
					break;

					case '?':
					case 'h':
					case 'H':
					{
						printf( "\nlnk_parser_cmd [-o directory][-w][-c][-r][-s] directory[filename.lnk]\n" \
								" -o\tSet the output directory for reports.\n" \
								" -w\tGenerate an HTML report.\n" \
								" -c\tGenerate a comma-separated values (CSV) report.\n" \
								" -r\tDump remnant and unprocessed data.\n" \
								" -s\tSilent console output.\n" );
						return 0;
					}
					break;

					case 'a':
					case 'A':
					{
						printf( "\nLNK Parser is made free under the GPLv3 license.\nCopyright (c) 2012 Eric Kutcher\n" );
						return 0;
					}
					break;
				}
			}
		}
	}

	if ( output_csv == true )
	{
		check_config();
	}

	// Create and set the directory that we'll be outputting files to.
	if ( GetFileAttributes( output_path ) == INVALID_FILE_ATTRIBUTES )
	{
		CreateDirectory( output_path, NULL );
	}

	SetCurrentDirectory( output_path );				// Set the path (relative or full)

	DWORD written = 0;

	SYSTEMTIME st;
	char out_name[ 36 ];
	GetLocalTime( &st );

	if ( output_html == true )
	{
		sprintf_s( out_name, 36, "Report_%02d-%02d-%d_%d.html", st.wMonth, st.wDay, st.wYear, ( st.wHour * 3600 ) + ( st.wMinute * 60 ) + st.wSecond );

		hFile_html = /*INVALID_HANDLE_VALUE;//*/CreateFileA( out_name, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hFile_html == INVALID_HANDLE_VALUE )
		{
			printf( "HTML report could not be created.\n" );
			output_html = false;
		}
		else
		{
			// Add UTF-8 marker (BOM) if we're at the beginning of the file.
			if ( SetFilePointer( hFile_html, 0, NULL, FILE_END ) == 0 )
			{
				WriteFile( hFile_html, "\xEF\xBB\xBF", 3, &written, NULL );
			}

			// Write the header of our html file.
			write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head><title>Report</title><script type=\"text/javascript\">function t(s,i){var o=document.getElementById(i);if(o.style.display==\"none\"){o.style.display=\"block\";s.innerText=\"Hide\"}else{o.style.display=\"none\";s.innerText=\"Show\";}}</script><style type=\"text/css\">#r{display:table-row;}#r:hover{background-color:#DDDDDD;}#h{width:256px;float:left;}#m{width:320px;float:left;}#d{display:table-cell;float:left;}</style></head><body style=\"font-size:10pt;\">" );
		}
	}

	if ( output_csv == true )
	{
		sprintf_s( out_name, 36, "Report_%02d-%02d-%d_%d.csv", st.wMonth, st.wDay, st.wYear, ( st.wHour * 3600 ) + ( st.wMinute * 60 ) + st.wSecond );

		hFile_csv = /*INVALID_HANDLE_VALUE;//*/CreateFileA( out_name, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hFile_csv == INVALID_HANDLE_VALUE )
		{
			printf( "CVS report could not be created.\n" );
			output_csv = false;
		}
		else
		{
			// Add UTF-8 marker (BOM) if we're at the beginning of the file.
			if ( SetFilePointer( hFile_csv, 0, NULL, FILE_END ) == 0 )
			{
				WriteFile( hFile_csv, "\xEF\xBB\xBF", 3, &written, NULL );
			}

			write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Filename" );

			if ( cfg_date_created == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Date Created (UTC)" ); }
			if ( cfg_last_accessed == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Last Accessed (UTC)" ); }
			if ( cfg_last_modified == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Last Modified (UTC)" ); }
			if ( cfg_file_size == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",File Size (bytes)" ); }
			if ( cfg_file_attributes == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",File Attributes" ); }
			if ( cfg_icon_index == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Icon Index" ); }
			if ( cfg_showwindow_value == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",ShowWindow Value" ); }
			if ( cfg_hot_key_value == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Hot Key Value" ); }
			if ( cfg_link_flags == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Link Flags" ); }
			if ( cfg_link_target_id_list == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Link Target ID List" ); }
			if ( cfg_location_flags == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Location Flags" ); }
			if ( cfg_drive_type == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Drive Type" ); }
			if ( cfg_drive_serial_number == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Drive Serial Number" ); }
			if ( cfg_volume_label_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Volume Label (ASCII)" ); }
			if ( cfg_volume_label_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Volume Label (UNICODE)" ); }
			if ( cfg_local_path_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Local Path" ); }
			if ( cfg_network_share_flags == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Network Share Flags" ); }
			if ( cfg_network_provider_type == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Network Provider Type" ); }
			if ( cfg_network_share_name_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Network Share Name" ); }
			if ( cfg_device_name_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Device Name" ); }
			if ( cfg_network_share_name_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Network Share Name (UNICODE)" ); }
			if ( cfg_device_name_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Device Name (UNICODE)" ); }
			if ( cfg_common_path_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Common Path" ); }
			if ( cfg_local_path_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Local Path (UNICODE)" ); }
			if ( cfg_common_path_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Common Path (UNICODE)" ); }
			if ( cfg_comment_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Comment (UNICODE)" ); }
			if ( cfg_comment_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Comment (ASCII)" ); }
			if ( cfg_relative_path_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Relative Path (UNICODE)" ); }
			if ( cfg_relative_path_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Relative Path (ASCII)" ); }
			if ( cfg_working_directory_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Working Directory (UNICODE)" ); }
			if ( cfg_working_directory_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Working Directory (ASCII)" ); }
			if ( cfg_arguments_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Arguments (UNICODE)" ); }
			if ( cfg_arguments_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Arguments (ASCII)" ); }
			if ( cfg_icon_location_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Icon Location (UNICODE)" ); }
			if ( cfg_icon_location_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Icon Location (ASCII)" ); }
			if ( cfg_color_flags == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Color Flags" ); }
			if ( cfg_screen_buffer_width == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Screen Buffer Width" ); }
			if ( cfg_screen_buffer_height == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Screen Buffer Height" ); }
			if ( cfg_window_width == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Window Width" ); }
			if ( cfg_window_height == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Window Height" ); }
			if ( cfg_window_x_coordinate == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Window X Coordinate" ); }
			if ( cfg_window_y_coordinate == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Window Y Coordinate" ); }
			if ( cfg_font_size == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Font Size" ); }
			if ( cfg_font_family_value == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Font Family Value" ); }
			if ( cfg_font_weight == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Font Weight" ); }
			if ( cfg_font_face_name == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Font Face Name" ); }
			if ( cfg_cursor_size == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Cursor Size" ); }
			if ( cfg_fullscreen == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Fullscreen" ); }
			if ( cfg_quickedit_mode == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",QuickEdit Mode" ); }
			if ( cfg_insert_mode == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Insert Mode" ); }
			if ( cfg_automatic_positioning == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Automatic Positioning" ); }
			if ( cfg_history_buffer_size == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",History Buffer Size" ); }
			if ( cfg_number_of_history_buffers == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Number of History Buffers" ); }
			if ( cfg_duplicates_allowed_in_history == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Duplicates Allowed in History" ); }
			if ( cfg_color_table_values == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Color Table Values" ); }
			if ( cfg_code_page == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Code Page" ); }
			if ( cfg_application_identifier_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Application Identifier (ASCII)" ); }
			if ( cfg_application_identifier_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Application Identifier (UNICODE)" ); }
			if ( cfg_environment_variables_location_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Environment Variables Location (ASCII)" ); }
			if ( cfg_environment_variables_location_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Environment Variables Location (UNICODE)" ); }
			if ( cfg_icon_location2_a == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Icon Location (ASCII)" ); }
			if ( cfg_icon_location2_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Icon Location (UNICODE)" ); }
			if ( cfg_known_folder_guid == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Known Folder GUID" ); }
			if ( cfg_known_folder_first_child_segment_offset == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Known Folder - First Child Segment Offset (bytes)" ); }
			if ( cfg_metadata_property_store == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Metadata Property Store" ); }
			if ( cfg_shim_layer_u == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Shim Layer (UNICODE)" ); }
			if ( cfg_special_folder_identifier == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Special Folder Identifier" ); }
			if ( cfg_special_folder_first_child_segment_offset == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Special Folder - First Child Segment Offset (bytes)" ); }
			if ( cfg_version == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Version" ); }
			if ( cfg_netbios_name == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",NetBIOS Name" ); }
			if ( cfg_droid_volume_identifier == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Droid Volume Identifier" ); }
			if ( cfg_droid_file_identifier == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Droid File Identifier" ); }
			if ( cfg_birth_droid_volume_identifier == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Birth Droid Volume Identifier" ); }
			if ( cfg_birth_droid_file_identifier == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Birth Droid File Identifier" ); }
			if ( cfg_mac_address == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",MAC Address" ); }
			if ( cfg_uuid_timestamp == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",UUID Timestamp (UTC)" ); }
			if ( cfg_uuid_sequence_number == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",UUID Sequence Number" ); }
			if ( cfg_distributed_link_tracker_notes == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Distributed Link Tracker Notes" ); }
			if ( cfg_vista_and_above_id_list == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Vista and Above ID List" ); }
			if ( cfg_output_notes == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Output Notes" ); }

			write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"\r\n" );
		}
	}

	if ( GetFileAttributes( name ) & FILE_ATTRIBUTE_DIRECTORY )
	{
		// Remove any trailing \ from the path name.
		if ( name[ name_length - 1 ] == L'\\' )
		{
			name[ name_length - 1 ] = L'\0';
		}
		traverse_directory( name );
	}
	else
	{
		parse_shortcut( name );
	}

	//wchar_t current_directory[ MAX_PATH ] = { 0 };
	//GetCurrentDirectory( MAX_PATH, current_directory );
	//traverse_directory( current_directory );

	if ( output_html == true )
	{
		write_html( hFile_html, write_buf2, write_offset2 );
		WriteFile( hFile_html, "</body></html>", 14, &written, NULL );
		CloseHandle( hFile_html );
	}

	if ( output_csv == true )
	{
		CloseHandle( hFile_csv );
	}

	if ( argc == 1 )
	{
		printf( "Press any key to continue . . . " );
		getchar();
	}
	return 0;
}
