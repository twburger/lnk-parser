#include "globals.h"

wchar_t *shim_layer = NULL;
char *machine_id = NULL;
char *metadata_property_store = NULL;
char *va_pidl = NULL;

void cleanup()
{
	DWORD written = 0;
	if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); WriteFile( hFile_html, "<hr size=1 />", 13, &written, NULL ); }
	if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); WriteFile( hFile_csv, "\r\n", 2, &written, NULL ); }
	if ( silent == false ){ printf( "\n\n" ); }

	free( machine_id );
	free( shim_layer );
	free( metadata_property_store );
	free( va_pidl );

	machine_id = NULL;
	shim_layer = NULL;
	metadata_property_store = NULL;
	va_pidl = NULL;
}

void add_commas( unsigned char number_of_columns )	// This isn't safe. We just need to be reasonable.
{
	while ( number_of_columns > 0 )
	{
		--number_of_columns;
		write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"," );
		++column_count;
	}
}

bool scan_memory( HANDLE hFile, unsigned int &offset )
{
	// Allocate a 100 kilobyte chunk of memory to scan. This value is arbitrary.
	char *buf = ( char * )malloc( sizeof( char ) * 102400 );
	char *scan = NULL;
	DWORD read = 0;

	while ( true )
	{
		// Begin reading through the database.
		ReadFile( hFile, buf, sizeof( char ) * 102400, &read, NULL );
		if ( read <= 16 )	// If this is true, then there's nothing left for us to read.
		{
			free( buf );
			return false;
		}

		// Binary string search. Look for the magic identifier.
		scan = buf;
		while ( read-- > 16 && memcmp( scan++, "\x01\x14\x02\x00\x00\x00\x00\x00\xc0\x00\x00\x00\x00\x00\x00\x46\x9b", 16 ) != 0 )
		{
			++offset;
		}

		// If it's not found, then we'll keep scanning.
		if ( read < 16 )
		{
			// Adjust the offset back 16 characters (in case we truncated the magic identifier when reading).
			SetFilePointer( hFile, offset, NULL, FILE_BEGIN );
			// Keep scanning.
			continue;
		}

		break;
	}

	free( buf );
	return true;
}

void parse_shortcut( wchar_t *filepath )
{
	// Attempt to open our database file.
	HANDLE hFile = CreateFile( filepath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD read = 0;
		column_count = 0;

		if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\"><b>Filename</b></div><div id=\"d\">%s</div></div>", filepath ); }
		if ( output_csv == true ){ write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"\"%s\"", filepath ); }	// Mandatory
		if ( silent == false ){ wprintf( L"[Filename]:\t\t\t\t%s\n", filepath ); }

		// Get the header information for this link.
		ShellLinkHeader slh = { 0 };
		ReadFile( hFile, &slh, sizeof( ShellLinkHeader ), &read, NULL );
		if ( read != sizeof( ShellLinkHeader ) )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading the LNK header.<br />" ); }
			if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading the LNK header." ); }
			if ( silent == false ){ printf( "\nPremature end of file while reading the LNK header.\n" ); }
			CloseHandle( hFile );
			cleanup();
			return;
		}

		if ( memcmp( slh.LinkCLSID, "\x01\x14\x02\x00\x00\x00\x00\x00\xc0\x00\x00\x00\x00\x00\x00\x46\x9b", 16 ) != 0 )
		{
			// Let's scan for the CLSID before we bail.
			unsigned int current_position = SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
			// If we found the beginning of the LNK, attempt to read it again.
			if ( scan_memory( hFile, current_position ) == true )
			{
				// Offset back another 4 bytes for the header size int.
				SetFilePointer( hFile, current_position - sizeof( unsigned int ), NULL, FILE_BEGIN );

				// Get the header information for this link.
				ReadFile( hFile, &slh, sizeof( ShellLinkHeader ), &read, NULL );
				if ( read != sizeof( ShellLinkHeader ) )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading the LNK header.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading the LNK header." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading the LNK header.\n" ); }
					CloseHandle( hFile );
					cleanup();
					return;
				}
			}
			else	// No CLSID was found. The file is not a LNK, or it has become corrupted.
			{
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />LNK header CLSID not found<br />" ); }
				if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",LNK header CLSID not found." ); }
				if ( silent == false ){ printf( "\nLNK header CLSID not found.\n" ); }
				CloseHandle( hFile );
				cleanup();
				return;
			}
		}

		SYSTEMTIME st;
		FILETIME ft;

		//char guid1[ 64 ] = { 0 };
		//buffer_to_guid( slh.LinkCLSID, guid1 );
		//printf( "Link CLSID:\t\t\t\t%s\n", guid1 );

		if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Header</b>" ); }
		if ( silent == false ){ wprintf( L"\n[Header]\n" ); }

		if ( slh.CreationTimeLow == 0 && slh.CreationTimeHigh == 0 )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Date created</div><div id=\"d\">Unknown</div></div>" ); }
			if ( output_csv == true && cfg_date_created == true ){ write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Unknown" ); ++column_count; }
			if ( silent == false ){ printf( "Date created:\t\t\t\tUnknown\n" ); }
		}
		else
		{
			ft.dwLowDateTime = slh.CreationTimeLow;
			ft.dwHighDateTime = slh.CreationTimeHigh;
			FileTimeToSystemTime( &ft, &st );

			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Date created</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
			if ( output_csv == true && cfg_date_created == true ){ write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%02d/%02d/%d (%02d:%02d:%02d.%d)", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); ++column_count; }
			if ( silent == false ){ printf( "Date created:\t\t\t\t%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
		}

		if ( slh.AccessTimeLow == 0 && slh.AccessTimeHigh == 0 )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last accessed</div><div id=\"d\">Unknown</div></div>" ); }
			if ( output_csv == true && cfg_last_accessed == true ){ write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Unknown" ); ++column_count; }
			if ( silent == false ){ printf( "Last accessed:\t\t\t\tUnknown\n" ); }
		}
		else
		{
			ft.dwLowDateTime = slh.AccessTimeLow;
			ft.dwHighDateTime = slh.AccessTimeHigh;
			FileTimeToSystemTime( &ft, &st );

			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last accessed</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
			if ( output_csv == true && cfg_last_accessed == true ){ write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%02d/%02d/%d (%02d:%02d:%02d.%d)", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); ++column_count; }
			if ( silent == false ){ printf( "Last accessed:\t\t\t\t%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
		}
		
		if ( slh.WriteTimeLow == 0 && slh.WriteTimeHigh == 0 )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last modified</div><div id=\"d\">Unknown</div></div>" ); }
			if ( output_csv == true && cfg_last_modified == true ){ write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Unknown" ); ++column_count; }
			if ( silent == false ){ printf( "Last modified:\t\t\t\tUnknown\n" ); }
		}
		else
		{
			ft.dwLowDateTime = slh.WriteTimeLow;
			ft.dwHighDateTime = slh.WriteTimeHigh;
			FileTimeToSystemTime( &ft, &st );

			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last modified</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
			if ( output_csv == true && cfg_last_modified == true ){ write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%02d/%02d/%d (%02d:%02d:%02d.%d)", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); ++column_count; }
			if ( silent == false ){ printf( "Last modified:\t\t\t\t%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
		}

		if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">File size</div><div id=\"d\">%lu bytes</div></div><div id=\"r\"><div id=\"h\">File attributes</div><div id=\"m\">0x%.8x</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">Icon index</div><div id=\"d\">%lu</div></div><div id=\"r\"><div id=\"h\">ShowWindow value</div><div id=\"m\">%lu</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">Hot key value</div><div id=\"m\">0x%.4x</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">Link flags</div><div id=\"m\">0x%.8x</div><div id=\"d\">%s</div></div>", \
				slh.FileSize, slh.FileAttributes, get_file_attributes( slh.FileAttributes ), slh.IconIndex, slh.ShowCommand, get_showwindow_value( slh.ShowCommand ), slh.HotKey, get_hot_key_value( slh.HotKey ), slh.LinkFlags, get_data_flags( slh.LinkFlags ) ); }

		if ( output_csv == true )
		{ 
			if ( cfg_file_size == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", slh.FileSize ); ++column_count; }
			if ( cfg_file_attributes == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"0x%.8x %s\"", slh.FileAttributes, get_file_attributes( slh.FileAttributes ) ); ++column_count; }
			if ( cfg_icon_index == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", slh.IconIndex ); ++column_count; }
			if ( cfg_showwindow_value == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu %s", slh.ShowCommand, get_showwindow_value( slh.ShowCommand ) ); ++column_count; }
			if ( cfg_hot_key_value == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"0x%.4x %s\"", slh.HotKey, get_hot_key_value( slh.HotKey ) ); ++column_count; }
			if ( cfg_link_flags == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"0x%.8x %s\"", slh.LinkFlags, get_data_flags( slh.LinkFlags ) ); ++column_count; }
		}

		if ( silent == false ){ wprintf( L"File size:\t\t\t\t%lu bytes\nFile attributes:\t\t\t0x%.8x\t(%s)\nIcon index:\t\t\t\t%lu\nShowWindow value:\t\t\t%lu\t\t(%s)\nHot key value:\t\t\t\t0x%.4x\t\t(%s)\nLink flags:\t\t\t\t0x%.8x\t(%s)\n", \
				slh.FileSize, slh.FileAttributes, get_file_attributes( slh.FileAttributes ), slh.IconIndex, slh.ShowCommand, get_showwindow_value( slh.ShowCommand ), slh.HotKey, get_hot_key_value( slh.HotKey ), slh.LinkFlags, get_data_flags( slh.LinkFlags ) ); }

		if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
		if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }

		bool is_unicode = ( ( slh.LinkFlags & IsUnicode ) ? true : false );

		// Contains a PIDL
		if ( ( slh.LinkFlags & HasLinkTargetIDList ) )
		{
			// Get the link target ID list.
			unsigned short pidl_size = 0;
			ReadFile( hFile, &pidl_size, sizeof( short ), &read, NULL );

			if ( read != sizeof( unsigned short ) )
			{
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading the Link Target ID List.<br />" ); }
				if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading the Link Target ID List." ); }
				if ( silent == false ){ printf( "\nPremature end of file while reading the Link Target ID List.\n" ); }
				CloseHandle( hFile );
				cleanup();
				return;
			}
			
			if ( pidl_size > 0 )
			{
				char *pidl = ( char * )malloc( sizeof( char ) * pidl_size );
				ReadFile( hFile, pidl, pidl_size, &read, NULL );

				if ( read != pidl_size )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading the Link Target ID List.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading the Link Target ID List." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading the Link Target ID List.\n" ); }
					free( pidl );
					CloseHandle( hFile );
					cleanup();
					return;
				}

				// 10 columns up to this point.
				unsigned int offset = 0;
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Link Target ID List</b>" ); }
				if ( output_csv == true && cfg_link_target_id_list == true ){ write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"" ); }
				if ( silent == false ){ printf( "\n[Link Target ID List]" ); }
				parse_pidl( pidl, pidl_size, offset, false, cfg_link_target_id_list );
				if ( output_csv == true && cfg_link_target_id_list == true ){ write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"\"" ); ++column_count; }
				if ( silent == false ){ printf( "\n" ); }

				free( pidl );
			}
		}

		if ( ( slh.LinkFlags & HasLinkInfo ) )
		{
			// Get the header information for this link.
			LinkInfo li = { 0 };
			ReadFile( hFile, &li, sizeof( LinkInfo ), &read, NULL );
			if ( read != sizeof( LinkInfo ) )
			{
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Link Info header.<br />" ); }
				if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Link Info header." ); }
				if ( silent == false ){ printf( "\nPremature end of file while reading Link Info header.\n" ); }
				CloseHandle( hFile );
				cleanup();
				return;
			}

			unsigned int LocalBasePathOffsetUnicode = 0;
			unsigned int CommonPathSuffixOffsetUnicode = 0;
			if ( li.LinkInfoHeaderSize >= 0x00000024 )
			{
				ReadFile( hFile, &LocalBasePathOffsetUnicode, sizeof( unsigned int ), &read, NULL );
				if ( read != sizeof( unsigned int ) )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Local Base Path offset (UNICODE).<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Local Base Path offset (UNICODE)." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Local Base Path offset (UNICODE).\n" ); }
					CloseHandle( hFile );
					cleanup();
					return;
				}

				ReadFile( hFile, &CommonPathSuffixOffsetUnicode, sizeof( unsigned int ), &read, NULL );
				if ( read != sizeof( unsigned int ) )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Common Path Suffix offset (UNICODE).<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Common Path Suffix offset (UNICODE)." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Common Path Suffix offset (UNICODE).\n" ); }
					CloseHandle( hFile );
					cleanup();
					return;
				}
			}

			// 11 columns up to this point.
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Link Info</b><br /><div id=\"r\"><div id=\"h\">Location flags</div><div id=\"m\">0x%.8x</div><div id=\"d\">%s</div></div>", li.LinkInfoFlags, get_link_info_flags( li.LinkInfoFlags ) ); }
			if ( output_csv == true && cfg_location_flags == true ){ add_commas( col_location_flags - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"0x%.8x %s\"", li.LinkInfoFlags, get_link_info_flags( li.LinkInfoFlags ) ); ++column_count; }
			if ( silent == false ){ wprintf( L"\n[Link Info]\nLocation flags:\t\t\t\t0x%.8x\t(%s)\n", li.LinkInfoFlags, get_link_info_flags( li.LinkInfoFlags ) ); }

			// If false, then VolumeID, LocalBasePath, and LocalBasePathUnicode fields are not present, and the values of the VolumeIDOffset and LocalBasePathOffset fields are zero.
			// If the value of the LinkInfoHeaderSize field is greater than or equal to 0x00000024, the value of the LocalBasePathOffsetUnicode field is zero.
			if ( li.LinkInfoFlags & VolumeIDAndLocalBasePath )
			{
				VolumeID vi = { 0 };
				ReadFile( hFile, &vi, sizeof( VolumeID ), &read, NULL );
				if ( read != sizeof( VolumeID ) )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Volume ID header.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Volume ID header." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Volume ID header.\n" ); }
					CloseHandle( hFile );
					cleanup();
					return;
				}

				// 12 columns up to this point
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Drive type</div><div id=\"m\">%lu</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">Drive serial number</div><div id=\"d\">%.4x-%.4x</div></div>", vi.DriveType, get_drive_type( vi.DriveType ), vi.DriveSerialNumber >> 16, vi.DriveSerialNumber & 0xFFFF ); }
				if ( output_csv == true )
				{
					if ( cfg_drive_type == true ) { add_commas( col_drive_type - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu %s", vi.DriveType, get_drive_type( vi.DriveType ) ); ++column_count; }
					if ( cfg_drive_serial_number == true ) { add_commas( col_drive_serial_number - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%.4x-%.4x", vi.DriveSerialNumber >> 16, vi.DriveSerialNumber & 0xFFFF ); ++column_count; }
				}
				if ( silent == false ){ wprintf( L"Drive type:\t\t\t\t%lu\t\t(%s)\nDrive serial number:\t\t\t%.4x-%.4x\n", vi.DriveType, get_drive_type( vi.DriveType ), vi.DriveSerialNumber >> 16, vi.DriveSerialNumber & 0xFFFF ); }

				unsigned int VolumeLabelOffsetUnicode = 0;

				if ( vi.VolumeLabelOffset == 0x00000014 )	// Unicode label included.
				{
					ReadFile( hFile, &VolumeLabelOffsetUnicode, sizeof( unsigned int ), &read, NULL );
					if ( read != sizeof( unsigned int ) )
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Volume Label offset (UNICODE).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Volume Label offset (UNICODE)." ); }
						if ( silent == false ){ printf( "\nPremature end of file while reading Volume Label offset (UNICODE).\n" ); }
						CloseHandle( hFile );
						cleanup();
						return;
					}
				}

				unsigned int volume_label_length = 0;
				if ( vi.VolumeLabelOffset > 0 )
				{
					if ( VolumeLabelOffsetUnicode > 0 && VolumeLabelOffsetUnicode < vi.VolumeIDSize )
					{
						volume_label_length = VolumeLabelOffsetUnicode - vi.VolumeLabelOffset;
					}
					else if ( vi.VolumeLabelOffset < vi.VolumeIDSize )
					{
						volume_label_length = vi.VolumeIDSize - vi.VolumeLabelOffset;
					}
					else	// All offsets are incorrect. We can't continue.
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Incorrect Volume Label offset (ASCII).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Incorrect Volume Label offset (ASCII)." ); }
						if ( silent == false ){ printf( "\nIncorrect Volume Label offset (ASCII).\n" ); }
						CloseHandle( hFile );
						cleanup();
						return;
					}

					if ( volume_label_length > 0 )
					{
						char *volume_label = ( char * )malloc( sizeof( char ) * volume_label_length );
						ReadFile( hFile, volume_label, volume_label_length, &read, NULL );
						if ( read != volume_label_length )
						{
							if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Volume Label (ASCII).<br />" ); }
							if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Volume Label (ASCII)." ); }
							if ( silent == false ){ printf( "\nPremature end of file while reading Volume Label (ASCII).\n" ); }
							free( volume_label );
							CloseHandle( hFile );
							cleanup();
							return;
						}

						// 14 columns up to this point
						if ( output_html == true ){ char *tbuf = escape_html_ascii( volume_label ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Volume label (ASCII)</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : volume_label ) ); free( tbuf ); }
						if ( output_csv == true && cfg_volume_label_a == true ){ char *tbuf = escape_string_ascii( volume_label ); add_commas( col_volume_label_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", ( tbuf != NULL ? tbuf : volume_label ) ); ++column_count; free( tbuf ); }
						if ( silent == false ){ printf( "Volume label (ASCII):\t\t\t%s\n", volume_label ); }
						free( volume_label );
					}
				}

				// Includes NULL terminator
				if ( VolumeLabelOffsetUnicode > 0 )
				{
					if ( VolumeLabelOffsetUnicode < vi.VolumeIDSize )
					{
						volume_label_length = vi.VolumeIDSize - VolumeLabelOffsetUnicode;
					}
					else	// All offsets are incorrect. We can't continue.
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Incorrect Volume Label offset (UNICODE).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Incorrect Volume Label offset (UNICODE)." ); }
						if ( silent == false ){ printf( "\nIncorrect Volume Label offset (UNICODE).\n" ); }
						CloseHandle( hFile );
						cleanup();
						return;
					}

					if ( volume_label_length > 0 )
					{
						wchar_t *volume_label = ( wchar_t * )malloc( volume_label_length );
						ReadFile( hFile, volume_label, volume_label_length, &read, NULL );
						if ( read != volume_label_length )
						{
							if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Volume Label (UNICODE).<br />" ); }
							if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Volume Label (UNICODE)." ); }
							if ( silent == false ){ printf( "\nPremature end of file while reading Volume Label (UNICODE).\n" ); }
							free( volume_label );
							CloseHandle( hFile );
							cleanup();
							return;
						}

						// 15 columns up to this point
						if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( volume_label ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Volume label (UNICODE)</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : volume_label ) ); free( tbuf ); }
						if ( output_csv == true && cfg_volume_label_u == true ){ wchar_t *tbuf = escape_string_unicode( volume_label ); add_commas( col_volume_label_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", ( tbuf != NULL ? tbuf : volume_label ) ); ++column_count; free( tbuf ); }
						if ( silent == false ){ wprintf( L"Volume label (UNICODE):\t\t\t%s\n", volume_label ); }

						free( volume_label );
					}
				}

				if ( li.LocalBasePathOffset > 0 )
				{
					unsigned int local_path_length = 0;
					if ( li.CommonNetworkRelativeLinkOffset > 0 && li.CommonNetworkRelativeLinkOffset < li.LinkInfoSize )	// See if CommonNetworkRelativeLink follows LocalBasePath 
					{
						local_path_length = li.CommonNetworkRelativeLinkOffset - li.LocalBasePathOffset;
					}
					else if ( LocalBasePathOffsetUnicode > 0 && LocalBasePathOffsetUnicode < li.LinkInfoSize )	// If CommonNetworkRelativeLink doesn't exist, then neither will CommonPathSuffix. See if LocalBasePathUnicode exists instead.
					{
						local_path_length = LocalBasePathOffsetUnicode - li.LocalBasePathOffset;
					}
					else if ( li.LocalBasePathOffset < li.LinkInfoSize )	// If neither of the above exist, then use the size of the LinkInfo to calculate the length.
					{
						local_path_length = li.LinkInfoSize - li.LocalBasePathOffset;
					}
					else	// All offsets are incorrect. We can't continue.
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Incorrect Local Base Path offset (ASCII).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Incorrect Local Base Path offset (ASCII)." ); }
						if ( silent == false ){ printf( "\nIncorrect Local Base Path offset (ASCII).\n" ); }
						CloseHandle( hFile );
						cleanup();
						return;
					}

					if ( local_path_length > 0 )
					{
						char *local_path = ( char * )malloc( sizeof( char ) * local_path_length );
						ReadFile( hFile, local_path, local_path_length, &read, NULL );
						if ( read != local_path_length )
						{
							if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Local Base Path (ASCII).<br />" ); }
							if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Local Base Path (ASCII)." ); }
							if ( silent == false ){ printf( "\nPremature end of file while reading Local Base Path (ASCII).\n" ); }
							free( local_path );
							CloseHandle( hFile );
							cleanup();
							return;
						}

						// 16 columns up to this point.
						if ( output_html == true ){ char *tbuf = escape_html_ascii( local_path ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Local path (ASCII)</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : local_path ) ); free( tbuf ); }
						if ( output_csv == true && cfg_local_path_a == true ){ char *tbuf = escape_string_ascii( local_path ); add_commas( col_local_path_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", ( tbuf != NULL ? tbuf : local_path ) ); ++column_count; free( tbuf ); }
						if ( silent == false ){ printf( "Local path (ASCII):\t\t\t%s\n", local_path ); }
						free( local_path );
					}
				}
			}

			if ( li.LinkInfoFlags & CommonNetworkRelativeLinkAndPathSuffix )
			{
				CommonNetworkRelativeLink cnrl = { 0 };
				ReadFile( hFile, &cnrl, sizeof( CommonNetworkRelativeLink ), &read, NULL );
				if ( read != sizeof( CommonNetworkRelativeLink ) )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Common Network Relative Link.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Common Network Relative Link." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Common Network Relative Link.\n" ); }
					CloseHandle( hFile );
					cleanup();
					return;
				}

				unsigned int network_share_name_offset = 0;	// for UNICODE
				unsigned int device_name_offset = 0;		// for UNICODE
				// Both offsets MUST be present if NetNameOffset is greater than 20 bytes.
				if ( cnrl.NetNameOffset > 0x00000014 )
				{
					ReadFile( hFile, &network_share_name_offset, sizeof( unsigned int ), &read, NULL );
					if ( read != sizeof( unsigned int ) )
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Network Share Name offset (ASCII).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Network Share Name offset (ASCII)." ); }
						if ( silent == false ){ printf( "\nPremature end of file while reading Network Share Name offset (ASCII).\n" ); }
						CloseHandle( hFile );
						cleanup();
						return;
					}

					ReadFile( hFile, &device_name_offset, sizeof( unsigned int ), &read, NULL );
					if ( read != sizeof( unsigned int ) )
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Device Name offset (ASCII).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Device Name offset (ASCII)." ); }
						if ( silent == false ){ printf( "\nPremature end of file while reading Device Name offset (ASCII).\n" ); }
						CloseHandle( hFile );
						cleanup();
						return;
					}
				}

				// 17 columns up to this point.
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Network share flags</div><div id=\"m\">0x%.8x</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">Network provider type</div><div id=\"m\">0x%.8x</div><div id=\"d\">%s</div></div>", cnrl.CommonNetworkRelativeLinkFlags, get_common_network_relative_link_flags( cnrl.CommonNetworkRelativeLinkFlags ), cnrl.NetworkProviderType, get_network_provider_type( cnrl.NetworkProviderType ) ); }
				if ( output_csv == true )
				{
					if ( cfg_network_share_flags == true ) { add_commas( col_network_share_flags - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"0x%.8x %s\"", cnrl.CommonNetworkRelativeLinkFlags, get_common_network_relative_link_flags( cnrl.CommonNetworkRelativeLinkFlags ) ); ++column_count; }
					if ( cfg_network_provider_type == true ) { add_commas( col_network_provider_type - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",0x%.8x %s", cnrl.NetworkProviderType, get_network_provider_type( cnrl.NetworkProviderType ) ); ++column_count; }
				}
				if ( silent == false ){ wprintf( L"Network share flags:\t\t\t0x%.8x\t(%s)\nNetwork provider type:\t\t\t0x%.8x\t(%s)\n", cnrl.CommonNetworkRelativeLinkFlags, get_common_network_relative_link_flags( cnrl.CommonNetworkRelativeLinkFlags ), cnrl.NetworkProviderType, get_network_provider_type( cnrl.NetworkProviderType ) ); }

				// ANSI strings

				unsigned int network_share_name_length = 0;
				if ( cnrl.NetNameOffset > 0 )
				{
					if ( cnrl.DeviceNameOffset > 0 && cnrl.DeviceNameOffset < cnrl.CommonNetworkRelativeLinkSize )
					{
						network_share_name_length = cnrl.DeviceNameOffset - cnrl.NetNameOffset;
					}
					else if ( network_share_name_offset > 0 && network_share_name_offset < cnrl.CommonNetworkRelativeLinkSize )
					{
						network_share_name_length = network_share_name_offset - cnrl.NetNameOffset;
					}
					else if ( device_name_offset > 0 && device_name_offset < cnrl.CommonNetworkRelativeLinkSize )
					{
						network_share_name_length = device_name_offset - cnrl.NetNameOffset;
					}
					else if ( cnrl.NetNameOffset < cnrl.CommonNetworkRelativeLinkSize )
					{
						network_share_name_length = cnrl.CommonNetworkRelativeLinkSize - cnrl.NetNameOffset;
					}
					else	// All offsets are incorrect. We can't continue.
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Incorrect Network Share Name offset (ASCII).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Incorrect Network Share Name offset (ASCII)." ); }
						if ( silent == false ){ printf( "\nIncorrect Network Share Name offset (ASCII).\n" ); }
						CloseHandle( hFile );
						cleanup();
						return;
					}

					if ( network_share_name_length > 0 )
					{
						char *network_share_name = ( char * )malloc( sizeof( char ) * network_share_name_length );
						ReadFile( hFile, network_share_name, network_share_name_length, &read, NULL );
						if ( read != network_share_name_length )
						{
							if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Network Share Name (ASCII).<br />" ); }
							if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Network Share Name (ASCII)." ); }
							if ( silent == false ){ printf( "\nPremature end of file while reading Network Share Name (ASCII).\n" ); }
							free( network_share_name );
							CloseHandle( hFile );
							cleanup();
							return;
						}

						// 19 columns up to this point.
						if ( output_html == true ){ char *tbuf = escape_html_ascii( network_share_name ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Network share name (ASCII)</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : network_share_name ) ); free( tbuf ); }
						if ( output_csv == true && cfg_network_share_name_a == true ){ char *tbuf = escape_string_ascii( network_share_name ); add_commas( col_network_share_name_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", ( tbuf != NULL ? tbuf : network_share_name ) ); ++column_count; free( tbuf ); }
						if ( silent == false ){ printf( "Network share name (ASCII):\t\t%s\n", network_share_name ); }
						free( network_share_name );
					}
				}

				unsigned int device_name_length = 0;
				if ( cnrl.DeviceNameOffset > 0 )
				{
					if ( network_share_name_offset > 0 && network_share_name_offset < cnrl.CommonNetworkRelativeLinkSize )
					{
						device_name_length = network_share_name_offset - cnrl.DeviceNameOffset ;
					}
					else if ( device_name_offset > 0 && device_name_offset < cnrl.CommonNetworkRelativeLinkSize )
					{
						device_name_length = device_name_offset - cnrl.DeviceNameOffset;
					}
					else if ( cnrl.DeviceNameOffset < cnrl.CommonNetworkRelativeLinkSize )
					{
						device_name_length = cnrl.CommonNetworkRelativeLinkSize - cnrl.DeviceNameOffset;
					}
					else	// All offsets are incorrect. We can't continue.
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Incorrect Device Name offset (ASCII).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Incorrect Device Name offset (ASCII)." ); }
						if ( silent == false ){ printf( "\nIncorrect Device Name offset (ASCII).\n" ); }
						CloseHandle( hFile );
						cleanup();
						return;
					}

					if ( device_name_length > 0 )
					{
						char *device_name = ( char * )malloc( sizeof( char ) * device_name_length );
						ReadFile( hFile, device_name, device_name_length, &read, NULL );
						if ( read != device_name_length )
						{
							if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Device Name (ASCII).<br />" ); }
							if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Device Name (ASCII)." ); }
							if ( silent == false ){ printf( "\nPremature end of file while reading Device Name (ASCII).\n" ); }
							free( device_name );
							CloseHandle( hFile );
							cleanup();
							return;
						}

						// 20 columns up to this point.
						if ( output_html == true ){ char *tbuf = escape_html_ascii( device_name ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Device name (ASCII)</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : device_name ) ); free( tbuf ); }
						if ( output_csv == true && cfg_device_name_a == true ){ char *tbuf = escape_string_ascii( device_name ); add_commas( col_device_name_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", ( tbuf != NULL ? tbuf : device_name ) ); ++column_count; free( tbuf ); }
						if ( silent == false ){ printf( "Device name (ASCII):\t\t\t%s\n", device_name ); }
						free( device_name );
					}
				}

				// UNICODE strings.

				if ( network_share_name_offset > 0 )
				{
					network_share_name_length = 0;
					if ( device_name_offset > 0 && device_name_offset < cnrl.CommonNetworkRelativeLinkSize )
					{
						network_share_name_length = device_name_offset - network_share_name_offset;
					}
					else if ( network_share_name_offset < cnrl.CommonNetworkRelativeLinkSize )
					{
						network_share_name_length = cnrl.CommonNetworkRelativeLinkSize - network_share_name_offset;
					}
					else	// All offsets are incorrect. We can't continue.
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Incorrect Network Share Name offset (UNICODE).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Incorrect Network Share Name offset (UNICODE)." ); }
						if ( silent == false ){ printf( "\nIncorrect Network Share Name offset (UNICODE).\n" ); }
						CloseHandle( hFile );
						cleanup();
						return;
					}

					if ( network_share_name_length > 0 )
					{
						wchar_t *network_share_name = ( wchar_t * )malloc( network_share_name_length );
						ReadFile( hFile, network_share_name, network_share_name_length, &read, NULL );
						if ( read != network_share_name_length )
						{
							if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Network Share Name (UNICODE).<br />" ); }
							if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Network Share Name (UNICODE)." ); }
							if ( silent == false ){ printf( "\nPremature end of file while reading Network Share Name (UNICODE).\n" ); }
							free( network_share_name );
							CloseHandle( hFile );
							cleanup();
							return;
						}

						// 21 columns up to this point.
						if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( network_share_name ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Network share name (UNICODE)</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : network_share_name ) ); free( tbuf ); }
						if ( output_csv == true && cfg_network_share_name_u == true ){ wchar_t *tbuf = escape_string_unicode( network_share_name ); add_commas( col_network_share_name_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", ( tbuf != NULL ? tbuf : network_share_name ) ); ++column_count; free( tbuf ); }
						if ( silent == false ){ wprintf( L"Network share name (UNICODE):\t\t%s\n", network_share_name ); }

						free( network_share_name );
					}
				}

				if ( device_name_offset > 0 )
				{
					device_name_length = 0;
					if ( device_name_offset < cnrl.CommonNetworkRelativeLinkSize )
					{
						device_name_length = cnrl.CommonNetworkRelativeLinkSize - device_name_offset;
					}
					else	// All offsets are incorrect. We can't continue.
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Incorrect Device Name offset (UNICODE).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Incorrect Device Name offset (UNICODE)." ); }
						if ( silent == false ){ printf( "\nIncorrect Device Name offset (UNICODE).\n" ); }
						CloseHandle( hFile );
						cleanup();
						return;
					}

					if ( device_name_length > 0 )
					{
						wchar_t *device_name = ( wchar_t * )malloc( device_name_length );
						ReadFile( hFile, device_name, device_name_length, &read, NULL );
						if ( read != device_name_length )
						{
							if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Device Name (UNICODE).<br />" ); }
							if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Device Name (UNICODE)." ); }
							if ( silent == false ){ printf( "\nPremature end of file while reading Device Name (UNICODE).\n" ); }
							free( device_name );
							CloseHandle( hFile );
							cleanup();
							return;
						}

						// 22 columns up to this point.
						if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( device_name ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Device name (UNICODE)</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : device_name ) ); free( tbuf ); }
						if ( output_csv == true && cfg_device_name_u == true ){ wchar_t *tbuf = escape_string_unicode( device_name ); add_commas( col_device_name_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", ( tbuf != NULL ? tbuf : device_name ) ); ++column_count; free( tbuf ); }
						if ( silent == false ){ wprintf( L"Device name (UNICODE):\t\t\t%s\n", device_name ); }

						free( device_name );
					}
				}

				if ( li.CommonPathSuffixOffset > 0 )
				{
					unsigned int common_path_length = 0;
					if ( LocalBasePathOffsetUnicode > 0 && LocalBasePathOffsetUnicode < li.LinkInfoSize )			// See if LocalBasePathUnicode follows CommonPathSuffix 
					{
						common_path_length = LocalBasePathOffsetUnicode - li.CommonPathSuffixOffset;
					}
					else if ( CommonPathSuffixOffsetUnicode > 0 && CommonPathSuffixOffsetUnicode < li.LinkInfoSize )	// See if CommonPathSuffixUnicode follows CommonPathSuffix 
					{
						common_path_length = CommonPathSuffixOffsetUnicode - li.CommonPathSuffixOffset;
					}
					else if ( li.CommonPathSuffixOffset < li.LinkInfoSize )	// If neither of the above exist, then use the size of the LinkInfo to calculate the length.
					{
						common_path_length = li.LinkInfoSize - li.CommonPathSuffixOffset;
					}
					else	// All offsets are incorrect. We can't continue.
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Incorrect Common Path Suffix offset (ASCII).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Incorrect Common Path Suffix offset (ASCII)." ); }
						if ( silent == false ){ printf( "\nIncorrect Common Path Suffix offset (ASCII).\n" ); }
						CloseHandle( hFile );
						cleanup();
						return;
					}

					if ( common_path_length > 0 )
					{
						char *common_path = ( char * )malloc( sizeof( char ) * common_path_length );
						ReadFile( hFile, common_path, common_path_length, &read, NULL );
						if ( read != common_path_length )
						{
							if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Common Path Suffix (ASCII).<br />" ); }
							if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Common Path Suffix (ASCII)." ); }
							if ( silent == false ){ printf( "\nPremature end of file while reading Common Path Suffix (ASCII).\n" ); }
							free( common_path );
							CloseHandle( hFile );
							cleanup();
							return;
						}

						// 23 columns up to this point.
						// I think the formatting might be lost for this path.
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Common path (ASCII)</div><div id=\"d\">%S</div></div>", common_path ); }
						if ( output_csv == true && cfg_common_path_a == true ){ add_commas( col_common_path_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", common_path ); ++column_count; }
						if ( silent == false ){ printf( "Common path (ASCII):\t\t\t%s\n", common_path ); }
						free( common_path );
					}
				}
			}

			// UNICODE paths

			if ( LocalBasePathOffsetUnicode > 0 )
			{
				unsigned int local_path_length = 0;
				if ( CommonPathSuffixOffsetUnicode > 0 && CommonPathSuffixOffsetUnicode < li.LinkInfoSize )
				{
					local_path_length = CommonPathSuffixOffsetUnicode - LocalBasePathOffsetUnicode;
				}
				else if ( LocalBasePathOffsetUnicode < li.LinkInfoSize )
				{
					local_path_length = li.LinkInfoSize - LocalBasePathOffsetUnicode;
				}
				else	// All offsets are incorrect. We can't continue.
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Incorrect Local Base Path offset (UNICODE).<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Incorrect Local Base Path offset (UNICODE)." ); }
					if ( silent == false ){ printf( "\nIncorrect Local Base Path offset (UNICODE).\n" ); }
					CloseHandle( hFile );
					cleanup();
					return;
				}

				if ( local_path_length > 0 )
				{
					wchar_t *local_path = ( wchar_t * )malloc( local_path_length );
					ReadFile( hFile, local_path, local_path_length, &read, NULL );
					if ( read != local_path_length )
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Local Base Path (UNICODE).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Local Base Path (UNICODE)." ); }
						if ( silent == false ){ printf( "\nPremature end of file while reading Local Base Path (UNICODE).\n" ); }
						free( local_path );
						CloseHandle( hFile );
						cleanup();
						return;
					}

					// 24 columns up to this point.
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Local path (UNICODE)</div><div id=\"d\">%s</div></div>", local_path ); }
					if ( output_csv == true && cfg_local_path_u == true ){ add_commas( col_local_path_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", local_path ); ++column_count; }
					if ( silent == false ){ wprintf( L"Local path (UNICODE):\t\t\t%s\n", local_path ); }
					free( local_path );
				}
			}

			if ( CommonPathSuffixOffsetUnicode > 0 )
			{
				unsigned int common_path_length = 0;
				if ( CommonPathSuffixOffsetUnicode < li.LinkInfoSize )
				{
					common_path_length = li.LinkInfoSize - CommonPathSuffixOffsetUnicode;
				}
				else	// All offsets are incorrect. We can't continue.
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Incorrect Common Path Suffix offset (UNICODE).<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Incorrect Common Path Suffix offset (UNICODE)." ); }
					if ( silent == false ){ printf( "\nIncorrect Common Path Suffix offset (UNICODE).\n" ); }
					CloseHandle( hFile );
					cleanup();
					return;
				}

				if ( common_path_length > 0 )
				{
					wchar_t *common_path = ( wchar_t * )malloc( common_path_length );
					ReadFile( hFile, common_path, common_path_length, &read, NULL );
					if ( read != common_path_length )
					{
						if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Common Path Suffix (UNICODE).<br />" ); }
						if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Common Path Suffix (UNICODE)." ); }
						if ( silent == false ){ printf( "\nPremature end of file while reading Common Path Suffix (UNICODE).\n" ); }
						free( common_path );
						CloseHandle( hFile );
						cleanup();
						return;
					}

					// 25 columns up to this point.
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Common path (UNICODE)</div><div id=\"d\">%s</div></div>", common_path ); }
					if ( output_csv == true && cfg_common_path_u == true ){ add_commas( col_common_path_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", common_path ); ++column_count; }
					if ( silent == false ){ wprintf( L"Common path (UNICODE):\t\t\t%s\n", common_path ); }
					free( common_path );
				}
			}

			if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
			if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }
		}

		// STRING DATA - Strings are not NULL terminated.

		if ( ( slh.LinkFlags & HasName ) || ( slh.LinkFlags & HasRelativePath ) || ( slh.LinkFlags & HasWorkingDir ) || ( slh.LinkFlags & HasArguments ) || ( slh.LinkFlags & HasIconLocation ) )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>String Data</b><br />" ); }
			if ( silent == false ){ printf( "\n[String Data]\n" ); }
		}

		if ( ( slh.LinkFlags & HasName ) )
		{
			unsigned short comment_length = 0;
			ReadFile( hFile, &comment_length, sizeof( unsigned short ), &read, NULL );

			if ( read != sizeof ( unsigned short ) )
			{
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Comment.<br />" ); }
				if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Comment." ); }
				if ( silent == false ){ printf( "\nPremature end of file while reading Comment.\n" ); }
				CloseHandle( hFile );
				cleanup();
				return;
			}

			if ( is_unicode == true )
			{
				wchar_t *comment = ( wchar_t * )malloc( sizeof( wchar_t ) * ( comment_length + 1 ) );
				wmemset( comment, 0, comment_length + 1 );
				ReadFile( hFile, comment, sizeof( wchar_t ) * comment_length, &read, NULL );

				if ( read != sizeof( wchar_t ) * comment_length )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Comment.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Comment." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Comment.\n" ); }
					free( comment );
					CloseHandle( hFile );
					cleanup();
					return;
				}

				// 26 columns up to this point.
				if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( comment ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Comment (UNICODE)</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : comment ) ); free( tbuf ); }
				if ( output_csv == true && cfg_comment_u == true ){ wchar_t *tbuf = escape_string_unicode( comment ); add_commas( col_comment_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", ( tbuf != NULL ? tbuf : comment ) ); ++column_count; free( tbuf ); }
				if ( silent == false ){ wprintf( L"Comment (UNICODE):\t\t\t%s\n", comment ); }

				free( comment );
			}
			else
			{
				char *comment = ( char * )malloc( sizeof( char ) * ( comment_length + 1 ) );
				memset( comment, 0, comment_length + 1 );
				ReadFile( hFile, comment, comment_length, &read, NULL );

				if ( read != comment_length )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Comment.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Comment." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Comment.\n" ); }
					free( comment );
					CloseHandle( hFile );
					cleanup();
					return;
				}

				// 27 columns up to this point.
				if ( output_html == true ){ char *tbuf = escape_html_ascii( comment ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Comment (ASCII)</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : comment ) ); free( tbuf ); }
				if ( output_csv == true && cfg_comment_a == true ){ char *tbuf = escape_string_ascii( comment ); add_commas( col_comment_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", ( tbuf != NULL ? tbuf : comment ) ); ++column_count; free( tbuf ); }
				if ( silent == false ){ printf( "Comment (ASCII):\t\t\t%s\n", comment ); }

				free( comment );
			}
		}

		if ( ( slh.LinkFlags & HasRelativePath ) )
		{
			unsigned short relative_path_length = 0;
			ReadFile( hFile, &relative_path_length, sizeof( unsigned short ), &read, NULL );

			if ( read != sizeof ( unsigned short ) )
			{
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Relative Path.<br />" ); }
				if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Relative Path." ); }
				if ( silent == false ){ printf( "\nPremature end of file while reading Relative Path.\n" ); }
				CloseHandle( hFile );
				cleanup();
				return;
			}

			if ( is_unicode == true )
			{
				wchar_t *relative_path = ( wchar_t * )malloc( sizeof( wchar_t ) * ( relative_path_length + 1 ) );
				wmemset( relative_path, 0, relative_path_length + 1 );
				ReadFile( hFile, relative_path, sizeof( wchar_t ) * relative_path_length, &read, NULL );

				if ( read != sizeof( wchar_t ) * relative_path_length )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Relative Path.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Relative Path." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Relative Path.\n" ); }
					free( relative_path );
					CloseHandle( hFile );
					cleanup();
					return;
				}

				// 28 columns up to this point.
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Relative path (UNICODE)</div><div id=\"d\">%s</div></div>", relative_path ); }
				if ( output_csv == true && cfg_relative_path_u == true ){ add_commas( col_relative_path_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", relative_path ); ++column_count; }
				if ( silent == false ){ wprintf( L"Relative path (UNICODE):\t\t%s\n", relative_path ); }

				free( relative_path );
			}
			else
			{
				char *relative_path = ( char * )malloc( sizeof( char ) * ( relative_path_length + 1 ) );
				memset( relative_path, 0, relative_path_length + 1 );
				ReadFile( hFile, relative_path, relative_path_length, &read, NULL );

				if ( read != relative_path_length )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Relative Path.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Relative Path." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Relative Path.\n" ); }
					free( relative_path );
					CloseHandle( hFile );
					cleanup();
					return;
				}

				// 29 columns up to this point.
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Relative path (ASCII)</div><div id=\"d\">%S</div></div>", relative_path ); }
				if ( output_csv == true && cfg_relative_path_a == true ){ add_commas( col_relative_path_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", relative_path ); ++column_count; }
				if ( silent == false ){ printf( "Relative path (ASCII):\t\t\t%s\n", relative_path ); }

				free( relative_path );
			}
		}

		if ( ( slh.LinkFlags & HasWorkingDir ) )
		{
			unsigned short working_directory_length = 0;
			ReadFile( hFile, &working_directory_length, sizeof( unsigned short ), &read, NULL );

			if ( read != sizeof ( unsigned short ) )
			{
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Working Directory.<br />" ); }
				if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Working Directory." ); }
				if ( silent == false ){ printf( "\nPremature end of file while reading Working Directory.\n" ); }
				CloseHandle( hFile );
				cleanup();
				return;
			}

			if ( is_unicode == true )
			{
				wchar_t *working_directory = ( wchar_t * )malloc( sizeof( wchar_t ) * ( working_directory_length + 1 ) );
				wmemset( working_directory, 0, working_directory_length + 1 );
				ReadFile( hFile, working_directory, sizeof( wchar_t ) * working_directory_length, &read, NULL );

				if ( read != sizeof( wchar_t ) * working_directory_length )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Working Directory.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Working Directory." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Working Directory.\n" ); }
					free( working_directory );
					CloseHandle( hFile );
					cleanup();
					return;
				}

				// 30 columns up to this point.
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Working Directory (UNICODE)</div><div id=\"d\">%s</div></div>", working_directory ); }
				if ( output_csv == true && cfg_working_directory_u == true ){ add_commas( col_working_directory_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", working_directory ); ++column_count; }
				if ( silent == false ){ wprintf( L"Working Directory (UNICODE):\t\t%s\n", working_directory ); }

				free( working_directory );
			}
			else
			{
				char *working_directory = ( char * )malloc( sizeof( char ) * ( working_directory_length + 1 ) );
				memset( working_directory, 0, working_directory_length + 1 );
				ReadFile( hFile, working_directory, working_directory_length, &read, NULL );

				if ( read != working_directory_length )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Working Directory.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Working Directory." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Working Directory.\n" ); }
					free( working_directory );
					CloseHandle( hFile );
					cleanup();
					return;
				}

				// 31 columns up to this point.
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Working Directory (ASCII)</div><div id=\"d\">%S</div></div>", working_directory ); }
				if ( output_csv == true && cfg_working_directory_a == true ){ add_commas( col_working_directory_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", working_directory ); ++column_count; }
				if ( silent == false ){ printf( "Working Directory (ASCII):\t\t%s\n", working_directory ); }

				free( working_directory );
			}
		}

		if ( ( slh.LinkFlags & HasArguments ) )
		{
			unsigned short arguments_length = 0;
			ReadFile( hFile, &arguments_length, sizeof( unsigned short ), &read, NULL );

			if ( read != sizeof ( unsigned short ) )
			{
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Arguments.<br />" ); }
				if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Arguments." ); }
				if ( silent == false ){ printf( "\nPremature end of file while reading Arguments.\n" ); }
				CloseHandle( hFile );
				cleanup();
				return;
			}

			if ( is_unicode == true )
			{
				wchar_t *arguments = ( wchar_t * )malloc( sizeof( wchar_t ) * ( arguments_length + 1 ) );
				wmemset( arguments, 0, arguments_length + 1 );
				ReadFile( hFile, arguments, sizeof( wchar_t ) * arguments_length, &read, NULL );

				if ( read != sizeof( wchar_t ) * arguments_length )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Arguments.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Arguments." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Arguments.\n" ); }
					free( arguments );
					CloseHandle( hFile );
					cleanup();
					return;
				}

				// 32 columns up to this point.
				if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( arguments ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Arguments (UNICODE)</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : arguments ) ); free( tbuf ); }
				if ( output_csv == true && cfg_arguments_u == true ){ wchar_t *tbuf = escape_string_unicode( arguments ); add_commas( col_arguments_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", ( tbuf != NULL ? tbuf : arguments ) ); ++column_count; free( tbuf ); }
				if ( silent == false ){ wprintf( L"Arguments (UNICODE):\t\t\t%s\n", arguments ); }

				free( arguments );
			}
			else
			{
				char *arguments = ( char * )malloc( sizeof( char ) * ( arguments_length + 1 ) );
				memset( arguments, 0, arguments_length + 1 );
				ReadFile( hFile, arguments, arguments_length, &read, NULL );

				if ( read != arguments_length )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Arguments.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Arguments." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Arguments.\n" ); }
					free( arguments );
					CloseHandle( hFile );
					cleanup();
					return;
				}

				// 33 columns up to this point.
				if ( output_html == true ){ char *tbuf = escape_html_ascii( arguments ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Arguments (ASCII)</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : arguments ) ); free( tbuf ); }
				if ( output_csv == true && cfg_arguments_a == true ){ char *tbuf = escape_string_ascii( arguments ); add_commas( col_arguments_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", ( tbuf != NULL ? tbuf : arguments ) ); ++column_count; free( tbuf ); }
				if ( silent == false ){ printf( "Arguments (ASCII):\t\t\t%s\n", arguments ); }

				free( arguments );
			}
		}

		if ( ( slh.LinkFlags & HasIconLocation ) )
		{
			unsigned short icon_location_length = 0;
			ReadFile( hFile, &icon_location_length, sizeof( unsigned short ), &read, NULL );

			if ( read != sizeof ( unsigned short ) )
			{
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Icon Location.<br />" ); }
				if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Icon Location." ); }
				if ( silent == false ){ printf( "\nPremature end of file while reading Icon Location.\n" ); }
				CloseHandle( hFile );
				cleanup();
				return;
			}

			if ( is_unicode == true )
			{
				wchar_t *icon_location = ( wchar_t * )malloc( sizeof( wchar_t ) * ( icon_location_length + 1 ) );
				wmemset( icon_location, 0, icon_location_length + 1 );
				ReadFile( hFile, icon_location, sizeof( wchar_t ) * icon_location_length, &read, NULL );

				if ( read != sizeof( wchar_t ) * icon_location_length )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Icon Location.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Icon Location." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Icon Location.\n" ); }
					free( icon_location );
					CloseHandle( hFile );
					cleanup();
					return;
				}

				// 34 columns up to this point.
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Icon location (UNICODE)</div><div id=\"d\">%s</div></div>", icon_location ); }
				if ( output_csv == true && cfg_icon_location_u == true ){ add_commas( col_icon_location_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", icon_location ); ++column_count; }
				if ( silent == false ){ wprintf( L"Icon location (UNICODE):\t\t%s\n", icon_location ); }

				free( icon_location );
			}
			else
			{
				char *icon_location = ( char * )malloc( sizeof( char ) * ( icon_location_length + 1 ) );
				memset( icon_location, 0, icon_location_length + 1 );
				ReadFile( hFile, icon_location, icon_location_length, &read, NULL );

				if ( read != icon_location_length )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Icon Location.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Icon Location." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Icon Location.\n" ); }
					free( icon_location );
					CloseHandle( hFile );
					cleanup();
					return;
				}

				// 35 columns up to this point.
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Icon location (ASCII)</div><div id=\"d\">%S</div></div>", icon_location ); }
				if ( output_csv == true && cfg_icon_location_a == true ){ add_commas( col_icon_location_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", icon_location ); ++column_count; }
				if ( silent == false ){ printf( "Icon location (ASCII):\t\t\t%s\n", icon_location ); }

				free( icon_location );
			}
		}

		if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
		if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }

		// EXTRA DATA BLOCK

		// The pointers are freed in cleanup().
		EnvironmentVariableDataBlock evdb = { 0 };
		ConsoleDataBlock cdb = { 0 };
		TrackerDataBlock1 tdb1 = { 0 };
		machine_id = NULL;
		TrackerDataBlock2 tdb2 = { 0 };
		unsigned int console_codepage = 0;
		SpecialFolderDataBlock sfdb = { 0 };
		DarwinDataBlock ddb = { 0 };
		IconEnvironmentDataBlock iedb = { 0 };
		unsigned int shim_layer_length = 0;
		shim_layer = NULL;
		unsigned int metadata_property_store_length = 0;
		metadata_property_store = NULL;
		KnownFolderDataBlock kfdb = { 0 };
		unsigned int va_pidl_size = 0;
		va_pidl = NULL;

		bool has_environment_props = false;
		bool has_console_props = false;
		bool has_tracker_props = false;
		bool has_console_fe_props = false;
		bool has_special_folder_props = false;
		bool has_darwin_props = false;
		bool has_icon_environment_props = false;
		bool has_shim_props = false;
		bool has_property_store_props = false;
		bool has_known_folder_props = false;
		bool has_vista_and_above_idlist_props = false;

		unsigned char _break = 0;	// We should have a message after/if we break.
		unsigned int data_block_size = 0;
		unsigned int data_block_signature = 0;
		while ( true )
		{
			// If we reach the end of file, then exit the loop and process what we may have received.
			if ( _break > 0 )
			{
				break;
			}

			ReadFile( hFile, &data_block_size, sizeof( unsigned int ), &read, NULL );
			if ( read != sizeof( unsigned int ) )
			{
				_break = 1;
				break;
			}

			// If the size is 0, then it indicates the end of the data blocks.
			if ( data_block_size == 0 )
			{
				break;
			}
			
			ReadFile( hFile, &data_block_signature, sizeof( unsigned int ), &read, NULL );
			if ( read != sizeof( unsigned int ) )
			{
				_break = 1;
				break;
			}

			switch ( data_block_signature )
			{
				case ENVIRONMENT_PROPS:
				{
					// Get the header information for this link.
					ReadFile( hFile, &evdb, sizeof( EnvironmentVariableDataBlock ), &read, NULL );

					if ( read != sizeof( EnvironmentVariableDataBlock ) )
					{
						_break = 2;
						break;
					}

					has_environment_props = true;
				}
				break;

				case CONSOLE_PROPS:
				{
					// Get the header information for this link.
					ReadFile( hFile, &cdb, sizeof( ConsoleDataBlock ), &read, NULL );

					if ( read != sizeof( ConsoleDataBlock ) )
					{
						_break = 3;
						break;
					}

					has_console_props = true;
				}
				break;

				case TRACKER_PROPS:
				{
					ReadFile( hFile, &tdb1, sizeof( TrackerDataBlock1 ), &read, NULL );
					if ( read != sizeof( TrackerDataBlock1 ) )
					{
						_break = 4;
						break;
					}
					
					unsigned int machine_id_length = tdb1.Length - sizeof( TrackerDataBlock2 ) - ( 2 * sizeof( unsigned int ) );
					machine_id = ( char * )malloc( sizeof( char ) * machine_id_length );

					ReadFile( hFile, machine_id, machine_id_length, &read, NULL );
					if ( read != machine_id_length )
					{
						free( machine_id );
						machine_id = NULL;
						_break = 4;
						break;
					}

					ReadFile( hFile, &tdb2, sizeof( TrackerDataBlock2 ), &read, NULL );
					if ( read != sizeof( TrackerDataBlock2 ) )
					{
						free( machine_id );
						machine_id = NULL;
						_break = 4;
						break;
					}

					has_tracker_props = true;
				}
				break;

				case CONSOLE_FE_PROPS:
				{
					ReadFile( hFile, &console_codepage, sizeof( unsigned int ), &read, NULL );

					if ( read != sizeof( unsigned int ) )
					{
						_break = 5;
						break;
					}

					has_console_fe_props = true;
				}
				break;

				case SPECIAL_FOLDER_PROPS:
				{
					ReadFile( hFile, &sfdb, sizeof( SpecialFolderDataBlock ), &read, NULL );
					if ( read != sizeof( SpecialFolderDataBlock ) )
					{
						_break = 6;
						break;
					}

					has_special_folder_props = true;
				}
				break;

				case DARWIN_PROPS:
				{
					ReadFile( hFile, &ddb, sizeof( DarwinDataBlock ), &read, NULL );
					if ( read != sizeof( DarwinDataBlock ) )
					{
						_break = 7;
						break;
					}

					has_darwin_props = true;
				}
				break;

				case ICON_ENVIRONMENT_PROPS:
				{
					ReadFile( hFile, &iedb, sizeof( IconEnvironmentDataBlock ), &read, NULL );
					if ( read != sizeof( IconEnvironmentDataBlock ) )
					{
						_break = 8;
						break;
					}

					has_icon_environment_props = true;
				}
				break;

				case SHIM_PROPS:
				{
					shim_layer_length = data_block_size - ( 2 * sizeof( unsigned int ) );
					shim_layer = ( wchar_t * )malloc( shim_layer_length );

					ReadFile( hFile, shim_layer, shim_layer_length, &read, NULL );
					if ( read != shim_layer_length )
					{
						free( shim_layer );
						shim_layer = NULL;
						_break = 9;
						break;
					}

					has_shim_props = true;
				}
				break;

				case PROPERTY_STORE_PROPS:
				{
					metadata_property_store_length = data_block_size - ( sizeof( unsigned int ) * 2 );
					metadata_property_store = ( char * )malloc( sizeof( char ) * metadata_property_store_length );

					ReadFile( hFile, metadata_property_store, metadata_property_store_length, &read, NULL );
					if ( read != metadata_property_store_length )
					{
						free( metadata_property_store );
						metadata_property_store = NULL;
						_break = 10;
						break;
					}

					has_property_store_props = true;
				}
				break;

				case KNOWN_FOLDER_PROPS:
				{
					ReadFile( hFile, &kfdb, sizeof( KnownFolderDataBlock ), &read, NULL );
					if ( read != sizeof( KnownFolderDataBlock ) )
					{
						_break = 11;
						break;
					}

					has_known_folder_props = true;
				}
				break;

				case VISTA_AND_ABOVE_IDLIST_PROPS:
				{
					va_pidl_size = data_block_size - ( sizeof( unsigned int ) * 2 );
					va_pidl = ( char * )malloc( sizeof( char ) * va_pidl_size );

					ReadFile( hFile, va_pidl, va_pidl_size, &read, NULL );
					if ( read != va_pidl_size )
					{
						free( va_pidl );
						va_pidl = NULL;
						_break = 12;
						break;
					}

					has_vista_and_above_idlist_props = true;
				}
				break;
			}
		}

		if ( has_console_props == true )
		{
			if ( output_html == true )
			{
				write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Console Properties</b><br /><div id=\"r\"><div id=\"h\">Color flags</div><div id=\"m\">0x%.4x</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">Screen buffer width</div><div id=\"d\">%lu</div></div><div id=\"r\"><div id=\"h\">Screen buffer height</div><div id=\"d\">%lu</div></div><div id=\"r\"><div id=\"h\">Window width</div><div id=\"d\">%lu</div></div><div id=\"r\"><div id=\"h\">Window height</div><div id=\"d\">%lu</div></div><div id=\"r\"><div id=\"h\">Window X coordinate</div><div id=\"d\">%lu</div></div><div id=\"r\"><div id=\"h\">Window Y coordinate</div><div id=\"d\">%lu</div></div>", \
					cdb.FillAttributes, get_color_flags( cdb.FillAttributes ), cdb.ScreenBufferSizeX, cdb.ScreenBufferSizeY, cdb.WindowSizeX, cdb.WindowSizeY, cdb.WindowOriginX, cdb.WindowOriginY );
				wchar_t *tbuf = escape_html_unicode( cdb.FaceName );
				write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Font size</div><div id=\"d\">%lu x %lu</div></div><div id=\"r\"><div id=\"h\">Font family value</div><div id=\"m\">0x%.8x</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">Font weight</div><div id=\"m\">%lu</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">Font face name</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">Cursor size</div><div id=\"m\">%lu</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">Fullscreen</div><div id=\"m\">%lu</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">QuickEdit mode</div><div id=\"m\">%lu</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">Insert mode</div><div id=\"m\">%lu</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">Automatic positioning</div><div id=\"m\">%lu</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">History buffer size</div><div id=\"d\">%lu</div></div><div id=\"r\"><div id=\"h\">Number of history buffers</div><div id=\"d\">%lu</div></div><div id=\"r\"><div id=\"h\">Duplicates allowed in history</div><div id=\"m\">%lu</div><div id=\"d\">%s</div></div>", \
					cdb.FontSizeX, cdb.FontSizeY, cdb.FontFamily, get_font_family_value( cdb.FontFamily ), cdb.FontWeight, get_font_weight( cdb.FontWeight ), ( tbuf != NULL ? tbuf : cdb.FaceName ), cdb.CursorSize, ( cdb.CursorSize <= 25 ? L"Small" : ( cdb.CursorSize <= 50 ? L"Medium" : L"Large" ) ), cdb.FullScreen, ( cdb.FullScreen == 0 ? L"Disabled" : L"Enabled" ), cdb.QuickEdit, ( cdb.QuickEdit == 0 ? L"Disabled" : L"Enabled" ), cdb.InsertMode, ( cdb.InsertMode == 0 ? L"Disabled" : L"Enabled" ), cdb.AutoPosition, ( cdb.AutoPosition == 0 ? L"Disabled" : L"Enabled" ), cdb.HistoryBufferSize, cdb.NumberOfHistoryBuffers, cdb.HistoryNoDup, ( cdb.HistoryNoDup == 0 ? L"Disabled" : L"Enabled" ) );
				free( tbuf );
			}

			if ( output_csv == true )
			{
				if ( cfg_color_flags == true ) { add_commas( col_color_flags - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"0x%.4x %s\"", cdb.FillAttributes, get_color_flags( cdb.FillAttributes ) ); ++column_count; }
				if ( cfg_screen_buffer_width == true ) { add_commas( col_screen_buffer_width - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", cdb.ScreenBufferSizeX ); ++column_count; }
				if ( cfg_screen_buffer_height == true ) { add_commas( col_screen_buffer_height - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", cdb.ScreenBufferSizeY ); ++column_count; }
				if ( cfg_window_width == true ) { add_commas( col_window_width - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", cdb.WindowSizeX ); ++column_count; }
				if ( cfg_window_height == true ) { add_commas( col_window_height - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", cdb.WindowSizeY ); ++column_count; }
				if ( cfg_window_x_coordinate == true ) { add_commas( col_window_x_coordinate - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", cdb.WindowOriginX ); ++column_count; }
				if ( cfg_window_y_coordinate == true ) { add_commas( col_window_y_coordinate - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", cdb.WindowOriginY ); ++column_count; }
				if ( cfg_font_size == true ) { add_commas( col_font_size - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu x %lu", cdb.FontSizeX, cdb.FontSizeY ); ++column_count; }
				if ( cfg_font_family_value == true ) { add_commas( col_font_family_value - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"0x%.8x %s\"", cdb.FontFamily, get_font_family_value( cdb.FontFamily ) ); ++column_count; }
				if ( cfg_font_weight == true ) { add_commas( col_font_weight - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu %s", cdb.FontWeight, get_font_weight( cdb.FontWeight ) ); ++column_count; }
				if ( cfg_font_face_name == true ) { wchar_t *tbuf = escape_string_unicode( cdb.FaceName ); add_commas( col_font_face_name - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", ( tbuf != NULL ? tbuf : cdb.FaceName ) ); ++column_count; free( tbuf ); }
				if ( cfg_cursor_size == true ) { add_commas( col_cursor_size - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu %s", cdb.CursorSize, ( cdb.CursorSize <= 25 ? L"Small" : ( cdb.CursorSize <= 50 ? L"Medium" : L"Large" ) ) ); ++column_count; }
				if ( cfg_fullscreen == true ) { add_commas( col_fullscreen - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu %s", cdb.FullScreen, ( cdb.FullScreen == 0 ? L"Disabled" : L"Enabled" ) ); ++column_count; }
				if ( cfg_quickedit_mode == true ) { add_commas( col_quickedit_mode - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu %s", cdb.QuickEdit, ( cdb.QuickEdit == 0 ? L"Disabled" : L"Enabled" ) ); ++column_count; }
				if ( cfg_insert_mode == true ) { add_commas( col_insert_mode - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu %s", cdb.InsertMode, ( cdb.InsertMode == 0 ? L"Disabled" : L"Enabled" ) ); ++column_count; }
				if ( cfg_automatic_positioning == true ) { add_commas( col_automatic_positioning - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu %s", cdb.AutoPosition, ( cdb.AutoPosition == 0 ? L"Disabled" : L"Enabled" ) ); ++column_count; }
				if ( cfg_history_buffer_size == true ) { add_commas( col_history_buffer_size - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", cdb.HistoryBufferSize ); ++column_count; }
				if ( cfg_number_of_history_buffers == true ) { add_commas( col_number_of_history_buffers - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", cdb.NumberOfHistoryBuffers ); ++column_count; }
				if ( cfg_duplicates_allowed_in_history == true ) { add_commas( col_duplicates_allowed_in_history - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu %s", cdb.HistoryNoDup, ( cdb.HistoryNoDup == 0 ? L"Disabled" : L"Enabled" ) ); ++column_count; }
			}

			if ( silent == false )
			{
				wprintf( L"\n[Console Properties]\nColor flags:\t\t\t\t0x%.4x\t\t(%s)\nScreen buffer width:\t\t\t%lu\nScreen buffer height:\t\t\t%lu\nWindow width:\t\t\t\t%lu\nWindow height:\t\t\t\t%lu\nWindow X coordinate:\t\t\t%lu\nWindow Y coordinate:\t\t\t%lu\n", \
					cdb.FillAttributes, get_color_flags( cdb.FillAttributes ), cdb.ScreenBufferSizeX, cdb.ScreenBufferSizeY, cdb.WindowSizeX, cdb.WindowSizeY, cdb.WindowOriginX, cdb.WindowOriginY );
				wprintf( L"Font size:\t\t\t\t%lu x %lu\nFont family value:\t\t\t0x%.8x\t(%s)\nFont weight:\t\t\t\t%lu\t\t(%s)\nFont face name:\t\t\t\t%s\nCursor size:\t\t\t\t%lu\t\t(%s)\nFullscreen:\t\t\t\t%lu\t\t(%s)\nQuickEdit mode:\t\t\t\t%lu\t\t(%s)\nInsert mode:\t\t\t\t%lu\t\t(%s)\nAutomatic positioning:\t\t\t%lu\t\t(%s)\nHistory buffer size:\t\t\t%lu\nNumber of history buffers:\t\t%lu\nDuplicates allowed in history:\t\t%lu\t\t(%s)\n", \
					cdb.FontSizeX, cdb.FontSizeY, cdb.FontFamily, get_font_family_value( cdb.FontFamily ), cdb.FontWeight, get_font_weight( cdb.FontWeight ), cdb.FaceName, cdb.CursorSize, ( cdb.CursorSize <= 25 ? L"Small" : ( cdb.CursorSize <= 50 ? L"Medium" : L"Large" ) ), cdb.FullScreen, ( cdb.FullScreen == 0 ? L"Disabled" : L"Enabled" ), cdb.QuickEdit, ( cdb.QuickEdit == 0 ? L"Disabled" : L"Enabled" ), cdb.InsertMode, ( cdb.InsertMode == 0 ? L"Disabled" : L"Enabled" ), cdb.AutoPosition, ( cdb.AutoPosition == 0 ? L"Disabled" : L"Enabled" ), cdb.HistoryBufferSize, cdb.NumberOfHistoryBuffers, cdb.HistoryNoDup, ( cdb.HistoryNoDup == 0 ? L"Disabled" : L"Enabled" ) );
			}

			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Color table values</div><div>(" ); }
			if ( output_csv == true && cfg_color_table_values == true ){ add_commas( col_color_table_values - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"" ); ++column_count; }
			if ( silent == false ){ printf( "Color table values:\t\t\t(" ); }
			
			for ( int i = 0; i < 16; i++ )
			{
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, ( i < 15 ? L"0x%.8x, " : L"0x%.8x)</div></div>" ), cdb.ColorTable[ i ] ); }
				if ( output_csv == true && cfg_color_table_values == true ){ write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, ( i < 15 ? L"0x%.8x, " : L"0x%.8x\"" ), cdb.ColorTable[ i ] ); }
				if ( silent == false ){ printf( ( i < 15 ? "0x%.8x, " : "0x%.8x)\n" ), cdb.ColorTable[ i ] ); }
			}

			if ( dump_remnant == true )
			{
				unsigned int remnant_length = wcslen( cdb.FaceName );
				if ( remnant_length < ( 32 - 1 ) )
				{
					if ( output_html == true ){ write_html_dump( hFile_html, write_buf2, write_offset2, L"Remnant font face name", ( unsigned char * )&cdb.FaceName[ remnant_length + 1 ], sizeof( wchar_t ) * ( 32 - remnant_length - 1 ) ); }

					if ( silent == false )
					{
						printf( "Remnant font face name\n" );
						hex_dump( ( unsigned char * )&cdb.FaceName[ remnant_length + 1 ], sizeof( wchar_t ) * ( 32 - remnant_length - 1 ) );
					}
				}
			}

			if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
			if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }
		}

		if ( has_console_fe_props == true )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Console Code Page</b><br /><div id=\"r\"><div id=\"h\">Code page</div><div id=\"d\">%lu</div></div>", console_codepage ); }
			if ( output_csv == true && cfg_code_page == true ){ add_commas( col_code_page - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", console_codepage ); ++column_count; }
			if ( silent == false ){ printf( "\n[Console Code Page]\nCode page: %lu\n", console_codepage ); }

			if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
			if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }
		}

		if ( has_darwin_props == true )
		{
			if ( output_html == true ){ char *tbuf = escape_html_ascii( ddb.DarwinDataAnsi ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Darwin Properties</b><br /><div id=\"r\"><div id=\"h\">Application identifier (ASCII)</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : ddb.DarwinDataAnsi ) ); free( tbuf ); }
			if ( output_csv == true && cfg_application_identifier_a == true ){ char *tbuf = escape_string_ascii( ddb.DarwinDataAnsi ); add_commas( col_application_identifier_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", ( tbuf != NULL ? tbuf : ddb.DarwinDataAnsi ) ); ++column_count; free( tbuf ); }
			if ( silent == false ){ printf( "\n[Darwin Properties]\nApplication identifier (ASCII):\t\t%s\n", ddb.DarwinDataAnsi ); }

			if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( ddb.DarwinDataUnicode ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Application identifier (UNICODE)</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : ddb.DarwinDataUnicode ) ); free( tbuf ); }
			if ( output_csv == true && cfg_application_identifier_u == true ){ wchar_t *tbuf = escape_string_unicode( ddb.DarwinDataUnicode ); add_commas( col_application_identifier_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", ( tbuf != NULL ? tbuf : ddb.DarwinDataUnicode ) ); ++column_count; free( tbuf ); }
			if ( silent == false ){ wprintf( L"Application identifier (UNICODE):\t%s\n", ddb.DarwinDataUnicode ); }

			if ( dump_remnant == true )
			{
				unsigned int remnant_length = strlen( ddb.DarwinDataAnsi );
				if ( remnant_length < ( MAX_PATH - 1 ) )
				{
					if ( output_html == true ){ write_html_dump( hFile_html, write_buf2, write_offset2, L"Remnant application identifier (ASCII)", ( unsigned char * )&ddb.DarwinDataAnsi[ remnant_length + 1 ], MAX_PATH - remnant_length - 1 ); }

					if ( silent == false )
					{
						printf( "Remnant application identifier (ASCII)\n" );
						hex_dump( ( unsigned char * )&ddb.DarwinDataAnsi[ remnant_length + 1 ], MAX_PATH - remnant_length - 1 );
					}
				}

				remnant_length = wcslen( ddb.DarwinDataUnicode );
				if ( remnant_length < ( MAX_PATH - 1 ) )
				{
					if ( output_html == true ){ write_html_dump( hFile_html, write_buf2, write_offset2, L"Remnant application identifier (UNICODE)", ( unsigned char * )&ddb.DarwinDataUnicode[ remnant_length + 1 ], sizeof( wchar_t ) * ( MAX_PATH - remnant_length - 1 ) ); }

					if ( silent == false )
					{
						printf( "Remnant application identifier (UNICODE)\n" );
						hex_dump( ( unsigned char * )&ddb.DarwinDataUnicode[ remnant_length + 1 ], sizeof( wchar_t ) * ( MAX_PATH - remnant_length - 1 ) );
					}
				}
			}

			if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
			if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }
		}

		if ( has_environment_props == true )
		{
			if ( output_html == true ){ char *tbuf = escape_html_ascii( evdb.TargetAnsi ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Environment Variables Location</b><br /><div id=\"r\"><div id=\"h\">Environment variables location (ASCII)</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : evdb.TargetAnsi ) ); free( tbuf ); }
			if ( output_csv == true && cfg_environment_variables_location_a == true ){ char *tbuf = escape_string_ascii( evdb.TargetAnsi ); add_commas( col_environment_variables_location_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", ( tbuf != NULL ? tbuf : evdb.TargetAnsi ) ); ++column_count; free( tbuf ); }
			if ( silent == false ){ printf( "\n[Environment Variables Location]\nEnvironment variables location (ASCII)\t%s\n", evdb.TargetAnsi ); }

			if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( evdb.TargetUnicode ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Environment variables location (UNICODE)</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : evdb.TargetUnicode ) ); free( tbuf ); }
			if ( output_csv == true && cfg_environment_variables_location_u == true ){ wchar_t *tbuf = escape_string_unicode( evdb.TargetUnicode ); add_commas( col_environment_variables_location_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", ( tbuf != NULL ? tbuf : evdb.TargetUnicode ) ); ++column_count; free( tbuf ); }
			if ( silent == false ){ wprintf( L"Environment variables location (UNICODE):\t%s\n", evdb.TargetUnicode ); }

			if ( dump_remnant == true )
			{
				unsigned int remnant_length = strlen( evdb.TargetAnsi );
				if ( remnant_length < ( MAX_PATH - 1 ) )
				{
					if ( output_html == true ){ write_html_dump( hFile_html, write_buf2, write_offset2, L"Remnant environment variables location (ASCII)", ( unsigned char * )&evdb.TargetAnsi[ remnant_length + 1 ], MAX_PATH - remnant_length - 1 ); }

					if ( silent == false )
					{ 
						printf( "Remnant environment variables location (ASCII)\n" );
						hex_dump( ( unsigned char * )&evdb.TargetAnsi[ remnant_length + 1 ], MAX_PATH - remnant_length - 1 );
					}
				}
				
				remnant_length = wcslen( evdb.TargetUnicode );
				if ( remnant_length < ( MAX_PATH - 1 ) )
				{
					if ( output_html == true ){ write_html_dump( hFile_html, write_buf2, write_offset2, L"Remnant environment variables location (UNICODE)", ( unsigned char * )&evdb.TargetUnicode[ remnant_length + 1 ], sizeof( wchar_t ) * ( MAX_PATH - remnant_length - 1 ) ); }

					if ( silent == false )
					{
						printf( "Remnant environment variables location (UNICODE)\n" );
						hex_dump( ( unsigned char * )&evdb.TargetUnicode[ remnant_length + 1 ], sizeof( wchar_t ) * ( MAX_PATH - remnant_length - 1 ) );
					}
				}
			}

			if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
			if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }
		}

		if ( has_icon_environment_props == true )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Icon Location</b><br /><div id=\"r\"><div id=\"h\">Icon location (ASCII)</div><div id=\"d\">%S</div></div>", iedb.TargetAnsi ); }
			if ( output_csv == true && cfg_icon_location2_a == true ){ add_commas( col_icon_location2_a - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", iedb.TargetAnsi ); ++column_count; }
			if ( silent == false ){ printf( "\n[Icon Location]\nIcon location (ASCII):\t\t\t%s\n", iedb.TargetAnsi ); }

			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Icon location (UNICODE)</div><div id=\"d\">%s</div></div>", iedb.TargetUnicode ); }
			if ( output_csv == true && cfg_icon_location2_u == true ){ add_commas( col_icon_location2_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", iedb.TargetUnicode ); ++column_count; }
			if ( silent == false ){ wprintf( L"Icon location (UNICODE):\t\t%s\n", iedb.TargetUnicode ); }

			if ( dump_remnant == true )
			{
				unsigned int remnant_length = strlen( iedb.TargetAnsi );
				if ( remnant_length < ( MAX_PATH - 1 ) )
				{
					if ( output_html == true ){ write_html_dump( hFile_html, write_buf2, write_offset2, L"Remnant icon location (ASCII)", ( unsigned char * )&iedb.TargetAnsi[ remnant_length + 1 ], MAX_PATH - remnant_length - 1 ); }
				
					if ( silent == false )
					{
						printf( "Remnant icon location (ASCII)\n" );
						hex_dump( ( unsigned char * )&iedb.TargetAnsi[ remnant_length + 1 ], MAX_PATH - remnant_length - 1 );
					}
				}

				remnant_length = wcslen( iedb.TargetUnicode );
				if ( remnant_length < ( MAX_PATH - 1 ) )
				{
					if ( output_html == true ){ write_html_dump( hFile_html, write_buf2, write_offset2, L"Remnant icon location (UNICODE)", ( unsigned char * )&iedb.TargetUnicode[ remnant_length + 1 ], sizeof( wchar_t ) * ( MAX_PATH - remnant_length - 1 ) ); }

					if ( silent == false )
					{
						printf( "Remnant icon location (UNICODE)\n" );
						hex_dump( ( unsigned char * )&iedb.TargetUnicode[ remnant_length + 1 ], sizeof( wchar_t ) * ( MAX_PATH - remnant_length - 1 ) );
					}
				}
			}

			if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
			if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }
		}

		if ( has_known_folder_props == true )
		{
			char guid1[ 64 ] = { 0 };
			buffer_to_guid( kfdb.KnownFolderID, guid1 );

			char *p_type = get_clsid_type( ( char * )kfdb.KnownFolderID );
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Known Folder Location</b><br /><div id=\"r\"><div id=\"h\">Known folder GUID</div><div id=\"m\">%S</div><div id=\"d\">%S</div></div><div id=\"r\"><div id=\"h\">First child segment offset</div><div id=\"d\">%lu bytes</div></div>", guid1, ( p_type != NULL ? p_type : "Unknown" ), kfdb.Offset ); }
			if ( output_csv == true )
			{
				if ( cfg_known_folder_guid == true ) { add_commas( col_known_folder_guid - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%S %S", guid1, ( p_type != NULL ? p_type : "Unknown" ) ); ++column_count; }
				if ( cfg_known_folder_first_child_segment_offset == true ) { add_commas( col_known_folder_first_child_segment_offset - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", kfdb.Offset ); ++column_count; }
			}
			if ( silent == false ){ printf( "\n[Known Folder Location]\nKnown folder GUID:\t\t\t%s = %s\nFirst child segment offset:\t\t%lu bytes\n", guid1, ( p_type != NULL ? p_type : "Unknown" ), kfdb.Offset ); }

			if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
			if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }
		}

		if ( has_property_store_props == true )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Metadata Property Store</b>" ); }
			if ( output_csv == true && cfg_metadata_property_store == true ){ add_commas( col_metadata_property_store - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",[Property Store]" ); ++column_count; }
			if ( silent == false ){ printf( "\n[Metadata Property Store]\n" ); }
			parse_metadata( metadata_property_store, metadata_property_store_length );
			if ( silent == false ){ printf( "\n" ); }
			
			//free( metadata_property_store );

			if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
			if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }
		}

		if ( has_shim_props == true )
		{
			if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( shim_layer ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Shim Layer Properties</b><br /><div id=\"r\"><div id=\"h\">Shim layer (UNICODE)</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : shim_layer ) ); free( tbuf ); }
			if ( output_csv == true && cfg_shim_layer_u == true ){ wchar_t *tbuf = escape_string_unicode( shim_layer ); add_commas( col_shim_layer_u - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%s\"", ( tbuf != NULL ? tbuf : shim_layer ) ); ++column_count; free( tbuf ); }
			if ( silent == false ){ wprintf( L"\n[Shim Layer Properties]\nShim layer (UNICODE): %s\n", shim_layer ); }

			if ( dump_remnant == true )
			{
				unsigned int remnant_length = wcslen( shim_layer );
				if ( remnant_length < ( shim_layer_length - 1 ) )
				{
					if ( output_html == true ){ write_html_dump( hFile_html, write_buf2, write_offset2, L"Remnant shim layer (UNICODE)", ( unsigned char * )&shim_layer[ remnant_length + 1 ], sizeof( wchar_t ) * ( shim_layer_length - remnant_length - 1 ) ); }

					if ( silent == false )
					{
						printf( "Remnant shim layer (UNICODE)\n" );
						hex_dump( ( unsigned char * )&shim_layer[ remnant_length + 1 ], sizeof( wchar_t ) * ( shim_layer_length - remnant_length - 1 ) );
					}
				}
			}

			//free ( shim_layer );

			if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
			if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }
		}

		if ( has_special_folder_props == true )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Special Folder Location</b><br /><div id=\"r\"><div id=\"h\">Special folder identifier</div><div id=\"m\">%lu</div><div id=\"d\">%s</div></div><div id=\"r\"><div id=\"h\">First child segment offset</div><div id=\"d\">%lu bytes</div></div>", sfdb.SpecialFolderID, get_special_folder_type( sfdb.SpecialFolderID ), sfdb.Offset ); }
			if ( output_csv == true )
			{
				if ( cfg_special_folder_identifier == true ) { add_commas( col_special_folder_identifier - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu %s", sfdb.SpecialFolderID, get_special_folder_type( sfdb.SpecialFolderID ) ); ++column_count; }
				if ( cfg_special_folder_first_child_segment_offset == true ) { add_commas( col_special_folder_first_child_segment_offset - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", sfdb.Offset ); ++column_count; }
			}
			if ( silent == false ){ wprintf( L"\n[Special Folder Location]\nSpecial folder identifier:\t\t%lu\t\t(%s)\nFirst child segment offset:\t\t%lu bytes\n", sfdb.SpecialFolderID, get_special_folder_type( sfdb.SpecialFolderID ), sfdb.Offset ); }

			if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
			if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }
		}

		if ( has_tracker_props == true )
		{
			char guid1[ 64 ] = { 0 };
			char guid2[ 64 ] = { 0 };
			char guid3[ 64 ] = { 0 };
			char guid4[ 64 ] = { 0 };
			buffer_to_guid( tdb2.DroidVolumeID, guid1 );
			buffer_to_guid( tdb2.DroidFileID, guid2 );
			buffer_to_guid( tdb2.BirthDroidVolumeID, guid3 );
			buffer_to_guid( tdb2.BirthDroidFileID, guid4 );

			char mac[ 32 ] = { 0 };
			buffer_to_mac( guid4 + 24, mac );	// Use the birth droid file id.

			unsigned __int64 timestamp = 0;
			memcpy_s( &timestamp, sizeof( unsigned __int64 ), tdb2.BirthDroidFileID, sizeof( unsigned __int64 ) );
			timestamp -= 5748192000000000;		// Offset UUID timestamp (October 15, 1582) to match FILETIME (January 1, 1601).
			timestamp &= 0x0FFFFFFFFFFFFFFF;	// Mask out the version.
			memcpy_s( &ft, sizeof( FILETIME ), &timestamp, sizeof( FILETIME ) );
			FileTimeToSystemTime( &ft, &st );

			unsigned short seq_num = 0;
			memcpy_s( &seq_num, sizeof( unsigned short ), ( unsigned short * )&tdb2.BirthDroidFileID[ 8 ], sizeof( unsigned short ) );
			seq_num = ( seq_num >> 8 ) | ( seq_num << 8 );	// Swap to little endian.
			seq_num &= 0x3FFF;	// Mask out the two high order variant bits.

			char *tbuf1 = escape_html_ascii( machine_id );
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Distributed Link Tracker Properties</b><br /><div id=\"r\"><div id=\"h\">Version</div><div id=\"d\">%lu</div></div><div id=\"r\"><div id=\"h\">NetBIOS name</div><div id=\"d\">%S</div></div><div id=\"r\"><div id=\"h\">Droid volume identifier</div><div id=\"d\">%S</div></div><div id=\"r\"><div id=\"h\">Droid file identifier</div><div id=\"d\">%S</div></div><div id=\"r\"><div id=\"h\">Birth droid volume identifier</div><div id=\"d\">%S</div></div><div id=\"r\"><div id=\"h\">Birth droid file identifier</div><div id=\"d\">%S</div></div><div id=\"r\"><div id=\"h\">MAC address</div><div id=\"d\">%S</div></div><div id=\"r\"><div id=\"h\">UUID timestamp</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div><div id=\"r\"><div id=\"h\">UUID sequence number</div><div id=\"d\">%u</div></div>", \
				tdb1.Version, ( tbuf1 != NULL ? tbuf1 : machine_id ), guid1, guid2, guid3, guid4, mac, st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, seq_num ); }
			free( tbuf1 );
			if ( output_csv == true )
			{
				if ( cfg_version == true ) { add_commas( col_version - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%lu", tdb1.Version ); ++column_count; }
				if ( cfg_netbios_name == true ) { char *tbuf = escape_string_ascii( machine_id ); add_commas( col_netbios_name - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"%S\"", ( tbuf != NULL ? tbuf : machine_id ) ); ++column_count; free( tbuf ); }
				if ( cfg_droid_volume_identifier == true ) { add_commas( col_droid_volume_identifier - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%S", guid1 ); ++column_count; }
				if ( cfg_droid_file_identifier == true ) { add_commas( col_droid_file_identifier - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%S", guid2 ); ++column_count; }
				if ( cfg_birth_droid_volume_identifier == true ) { add_commas( col_birth_droid_volume_identifier - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%S", guid3 ); ++column_count; }
				if ( cfg_birth_droid_file_identifier == true ) { add_commas( col_birth_droid_file_identifier - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%S", guid4 ); ++column_count; }
				if ( cfg_mac_address == true ) { add_commas( col_mac_address - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%S", mac ); ++column_count; }
				if ( cfg_uuid_timestamp == true ){ add_commas( col_uuid_timestamp - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%02d/%02d/%d (%02d:%02d:%02d.%d)", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); ++column_count; }
				if ( cfg_uuid_sequence_number == true ){ add_commas( col_uuid_sequence_number - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",%u", seq_num ); ++column_count; }
			}
			if ( silent == false ){ printf( "\n[Distributed Link Tracker Properties]\nVersion:\t\t\t\t%lu\nNetBIOS name:\t\t\t\t%s\nDroid volume identifier:\t\t%s\nDroid file identifier:\t\t\t%s\nBirth droid volume identifier:\t\t%s\nBirth droid file identifier:\t\t%s\nMAC address:\t\t\t\t%s\nUUID timestamp:\t\t\t\t%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\nUUID sequence number:\t\t\t%u\n", \
				tdb1.Version, machine_id, guid1, guid2, guid3, guid4, mac, st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, seq_num ); }

			bool new_vol = false;
			bool new_com = false;

			// See if the file was moved to a new volume. The volume ID will have changed.
			if ( memcmp( tdb2.BirthDroidVolumeID, tdb2.DroidVolumeID, 16 ) != 0 )
			{
				new_vol = true;
			}

			// See if the file was moved to a different computer. The file ID will have changed.
			if ( memcmp( tdb2.BirthDroidFileID, tdb2.DroidFileID, 16 ) != 0 )
			{
				new_com = true;
			}

			// The file was moved to a new computer and/or volume. The volume should normally change if the computer does.
			if ( new_com == true )
			{
				buffer_to_mac( guid2 + 24, mac );	// Use the (new) droid file id.

				memcpy_s( &timestamp, sizeof( unsigned __int64 ), tdb2.DroidFileID, sizeof( unsigned __int64 ) );
				timestamp -= 5748192000000000;		// Offset UUID timestamp (October 15, 1582) to match FILETIME (January 1, 1601).
				timestamp &= 0x0FFFFFFFFFFFFFFF;	// Mask out the version.
				memcpy_s( &ft, sizeof( FILETIME ), &timestamp, sizeof( FILETIME ) );
				FileTimeToSystemTime( &ft, &st );

				memcpy_s( &seq_num, sizeof( unsigned short ), ( unsigned short * )&tdb2.DroidFileID[ 8 ], sizeof( unsigned short ) );
				seq_num = ( seq_num >> 8 ) | ( seq_num << 8 );	// Swap to little endian.
				seq_num &= 0x3FFF;	// Mask out the two high order variant bits.

				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"The file was moved to a new computer%s.<br /><div id=\"r\"><div id=\"h\">New MAC address</div><div id=\"d\">%S</div></div><div id=\"r\"><div id=\"h\">New UUID timestamp</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div><div id=\"r\"><div id=\"h\">New UUID sequence number</div><div id=\"d\">%u</div></div>", ( new_vol == true ? L" and volume" : L"" ), mac, st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, seq_num ); }
				if ( output_csv == true && cfg_distributed_link_tracker_notes == true ){ add_commas( col_distributed_link_tracker_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"The file was moved to a new computer%s.\r\nNew MAC address: %S\r\nNew UUID timestamp: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\nNew UUID sequence number: %u\"", ( new_vol == true ? L" and volume" : L"" ), mac, st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, seq_num ); }
				if ( silent == false ){ printf( "The file was moved to a new computer%s.\nNew MAC address:\t\t\t%s\nNew UUID timestamp:\t\t\t%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\nNew UUID sequence number:\t\t%u\n", ( new_vol == true ? " and volume" : "" ), mac, st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, seq_num ); }
			}
			else if ( new_vol == true )	// The file was moved to a new volume on the same computer. This isn't that uncommon. An example would be transferring a file across partitions.
			{
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"The file was moved to a new volume.<br />" ); }
				if ( output_csv == true && cfg_distributed_link_tracker_notes == true ){ add_commas( col_distributed_link_tracker_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",The file was moved to a new volume." ); }
				if ( silent == false ){ printf( "The file was moved to a new volume.\n" ); }
			}

			//free( machine_id );

			if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
			if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }
		}

		if ( has_vista_and_above_idlist_props == true )
		{
			unsigned int offset = 0;
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><b>Vista and Above ID List</b>" ); }
			if ( output_csv == true && cfg_vista_and_above_id_list == true ){ add_commas( col_vista_and_above_id_list - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",\"" ); ++column_count; }
			if ( silent == false ){ printf( "\n[Vista and Above ID List]" ); }
			parse_pidl( va_pidl, va_pidl_size, offset, false, cfg_vista_and_above_id_list );
			if ( output_csv == true && cfg_vista_and_above_id_list == true ){ write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"\"" ); }
			if ( silent == false ){ printf( "\n" ); }

			//free( va_pidl );

			if ( output_html == true ){ write_html( hFile_html, write_buf2, write_offset2 ); }
			if ( output_csv == true ){ write_csv( hFile_csv, write_buf, write_offset ); }
		}

		// If we reached the end of file, display an appropriate message.
		if ( _break > 0 )
		{
			switch ( _break )
			{
				case 1:
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Data Block.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Data Block." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Data Block.\n" ); }
				}
				break;

				case 2:
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Environment Properties.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Environment Properties." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Environment Properties.\n" ); }
				}
				break;

				case 3:
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Console Properties.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Console Properties." ); }	
					if ( silent == false ){ printf( "\nPremature end of file while reading Console Properties.\n" ); }
				}
				break;

				case 4:
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Tracker Properties.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Tracker Properties." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Tracker Properties.\n" ); }
				}
				break;

				case 5:
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Console FE Properties.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Console FE Properties." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Console FE Properties.\n" ); }
				}
				break;

				case 6:
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Special Folder Properties.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Special Folder Properties." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Special Folder Properties.\n" ); }
				}
				break;

				case 7:
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Darwin Properties.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Darwin Properties." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Darwin Properties.\n" ); }
				}
				break;

				case 8:
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Icon Environment Properties.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Icon Environment Properties." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Icon Environment Properties.\n" ); }
				}
				break;

				case 9:
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Shim Properties.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Shim Properties." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Shim Properties.\n" ); }
				}
				break;

				case 10:
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Property Store Properties.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Property Store Properties." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Property Store Properties.\n" ); }
				}
				break;

				case 11:
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Known Folder Properties.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Known Folder Properties." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Known Folder Properties.\n" ); }
				}
				break;

				case 12:
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Premature end of file while reading Vista and Above ID List Properties.<br />" ); }
					if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Premature end of file while reading Vista and Above ID List Properties." ); }
					if ( silent == false ){ printf( "\nPremature end of file while reading Vista and Above ID List Properties.\n" ); }
				}
				break;
			}
		}
		else	// See if there's anything at the end of our file.
		{
			unsigned int cur = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
			unsigned int end = SetFilePointer( hFile, 0, NULL, FILE_END );
			if ( end > cur )
			{
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />Unknown data at end of file.<br />" ); }
				if ( output_csv == true && cfg_output_notes == true ){ add_commas( col_output_notes - column_count ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L",Unknown data at end of file." ); }
				if ( silent == false ){ printf( "\nUnknown data at end of file.\n" ); }
				
				if ( dump_remnant == true )
				{
					// Reset the file pointer.
					SetFilePointer( hFile, cur, NULL, FILE_BEGIN );

					unsigned int remnant_length = end - cur;
					unsigned char *remnant = ( unsigned char * )malloc( sizeof( unsigned char ) * remnant_length );

					ReadFile( hFile, remnant, sizeof( unsigned char ) * remnant_length, &read, NULL );
					if ( read == ( sizeof( unsigned char ) * remnant_length ) )
					{
						if ( output_html == true )
						{ 
							write_html_dump( hFile_html, write_buf2, write_offset2, L"Unknown data", remnant, remnant_length );
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						if ( silent == false )
						{
							printf( "Unknown data.\n" );
							hex_dump( remnant, remnant_length );
						}
					}

					free( remnant );
				}
			}
		}

		cleanup();

		// Close the input file.
		CloseHandle( hFile );
	}
}
