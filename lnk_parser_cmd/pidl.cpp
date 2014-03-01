#include "globals.h"

void parse_pidl( char *pidl, unsigned int pidl_size, unsigned int &pidl_offset, bool tab, bool use_csv )
{
	shitemid_header sii = { 0 };
	unsigned int p_offset = 0;
	char *iid = NULL;
	unsigned int iid_offset = 0;
	while ( true )
	{
		p_offset += sii.size;

		// The last SHITEMID will have a size of 0 and be 2 bytes in length.
		if ( p_offset + sizeof( unsigned short ) >= pidl_size ){ break; }

		iid = pidl + p_offset;

		if ( p_offset + 0x03 > pidl_size ){ break; }
		//sii = *( ( shitemid_header * )( iid ) );
		memcpy_s( &sii, sizeof( shitemid_header ), iid, sizeof( shitemid_header ) );
		iid_offset = 3;

		// End of pidl
		if ( sii.size == 0x0000 )
		{
			break;
		}

		// Write in chunks.
		if ( output_html == true && write_offset2 + sii.size >= BUFFER_CHUNK_SIZE )
		{
			write_html( hFile_html, write_buf2, write_offset2 );
		}

		if ( output_csv == true && write_offset + sii.size >= BUFFER_CHUNK_SIZE )
		{
			write_csv( hFile_csv, write_buf, write_offset );
		}

		if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />" ); }
		if ( p_offset > 0 )
		{
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"\r\n" ); }
			if ( silent == false ){ printf( "\n" ); }
		}

		//printf( "%sSHITEMID size:\t%lu bytes\n", ( tab == true ? "\t\t\t\t\t" : "" ), sii.size );
		//printf( "Flag: 0x%.2x\n", sii.flag );

		if ( sii.flag == 0x00 || sii.flag == 0x1F || sii.flag == 0x70 || sii.flag == 0x71 )	// CLSID and Blob
		{	
			if ( iid_offset + sizeof( char ) > sii.size ){ continue; }else{ iid_offset += sizeof( char ); }

			// 20 - 4 = 16 byte guid?
			if ( sii.size == 0x14 )
			{
				if ( iid_offset + 0x10 > sii.size ){ continue; }
				char clsid[ 64 ] = { 0 };
				buffer_to_guid( ( unsigned char * )( iid + iid_offset ), clsid );
				char *p_type = get_clsid_type( iid + iid_offset );
				iid_offset += 0x10;

				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">CLSID</div><div id=\"d\">%S = %S</div></div>", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"CLSID: %S = %S\r\n", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				if ( silent == false ){ printf( "\n%sCLSID:%s%s = %s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t" : "\t\t\t\t\t" ), clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
			}
			else
			{
				// Sometimes there is no blob_size_2 or blob_content_offset. Find out why...
				if ( iid_offset + 0x0A > sii.size ){ continue; }
				unsigned short blob_size = 0; memcpy_s( &blob_size, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
				unsigned int blob_flag = 0; memcpy_s( &blob_flag, sizeof( unsigned int ), iid + iid_offset + 0x02, sizeof( unsigned int ) );
				unsigned short blob_size_2 = 0; memcpy_s( &blob_size_2, sizeof( unsigned short ), iid + iid_offset + 0x06, sizeof( unsigned short ) );
				unsigned short blob_content_offset = 0; memcpy_s( &blob_content_offset, sizeof( unsigned short ), iid + iid_offset + 0x08, sizeof( unsigned short ) );
				iid_offset += 0x0A;

				if ( blob_size_2 == 0 && ( blob_content_offset == 0 || blob_content_offset == 0x10 ) )
				{
					if ( iid_offset + 0x10 > sii.size ){ continue; }
					char clsid[ 64 ] = { 0 };
					buffer_to_guid( ( unsigned char * )( iid + iid_offset ), clsid );
					char *p_type = get_clsid_type( iid + iid_offset );
					iid_offset += 0x10;
					
					if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">CLSID</div><div id=\"d\">%S = %S</div></div>", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
					if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"CLSID: %S = %S\r\n", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
					if ( silent == false ){ printf( "\n%sCLSID:%s%s = %s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t" : "\t\t\t\t\t" ), clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				}

				if ( blob_size_2 != 0 && blob_size_2 < sii.size )
				{
					// If blob_size_2 and blob_conent_offset are missing, then we need to backtrack. Don't know what causes this...
					// This probably isn't perfect.
					if ( ( blob_size - ( blob_size_2 + blob_content_offset + 0x0A ) ) > 0 )	// 0x0A = the 4 blob values above.
					{
						iid_offset -= ( sizeof( unsigned short ) * 2 );
						blob_size_2 = blob_size - sizeof( unsigned int );
					}
					else	// Move to the content.
					{
						if ( iid_offset + blob_content_offset > sii.size ){ continue; }
						iid_offset += blob_content_offset;
					}

					// See if we can parse blob_size_2 bytes.
					if ( iid_offset + blob_size_2 > sii.size ){ continue; }

					if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"[Property Store]\r\n" ); }
					if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<u>Property Store</u><br />" ); }
					if ( silent == false ){ printf( "\n[Property Store]\n" ); }
					parse_metadata( iid + iid_offset, blob_size_2 );	// blob_size_2 includes the length of the terimanting property storage structure (4 bytes)
					if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<u>End of Property Store</u><br />" ); }
					if ( silent == false ){ printf( "\n[/Property Store]" ); }
				}
			}
		}
		else if ( sii.flag == 0x22 || sii.flag == 0x23 || sii.flag == 0x25 || sii.flag == 0x29 || sii.flag == 0x2a || sii.flag == 0x2F )	// Drive
		{
			if ( iid_offset >= sii.size ){ continue; }
			unsigned int s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
			if ( iid_offset + s_len > sii.size ){ continue; }
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Drive</div><div id=\"d\">%S</div></div>", iid + iid_offset ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Drive: %S\r\n", iid + iid_offset ); }
			if ( silent == false ){ printf( "\n%sDrive:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t" : "\t\t\t\t\t" ), iid + iid_offset ); }

			if ( sii.size == 0x29 ) // 0x03 (Header) 0x16 (Drive stuff) + 0x10 (GUID);
			{
				// Skip drive stuff
				if ( iid_offset + 0x16 > sii.size ){ continue; }
				iid_offset += 0x16;

				// Check if we can get a hash
				if ( iid_offset + 0x10 > sii.size ){ continue; }
				char clsid[ 64 ] = { 0 };
				buffer_to_guid( ( unsigned char * )( iid + iid_offset ), clsid );
				char *p_type = get_clsid_type( iid + iid_offset );
				iid_offset += 0x10;
				
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">CLSID</div><div id=\"d\">%S = %S</div></div>", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"CLSID: %S = %S\r\n", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				if ( silent == false ){ printf( "\n%sCLSID:%s%s = %s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t" : "\t\t\t\t\t" ), clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
			}
		}
		else if ( sii.flag == 0x31 || sii.flag == 0xB1 )	// Folder
		{
			// Adjust 5 bytes to the timestamp
			if ( iid_offset + 0x05 > sii.size ){ continue; }else{ iid_offset += 0x05; }

			if ( iid_offset + ( sizeof( unsigned short ) * 2 ) > sii.size ){ continue; }
			unsigned short fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
			unsigned short fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
			SYSTEMTIME st;
			FILETIME ft;
			if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
			{
				FileTimeToSystemTime( &ft, &st );
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last modified</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Last modified: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				if ( silent == false ){ printf( "\n%sLast modified:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
			}
			iid_offset += ( sizeof( unsigned short ) * 2 );

			if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }
			unsigned short attributes = 0; memcpy_s( &attributes, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Folder attributes</div><div id=\"m\">0x%.8x</div><div id=\"d\">%s</div></div>", attributes, get_file_attributes( attributes ) ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Folder attributes: 0x%.8x %s\r\n", attributes, get_file_attributes( attributes ) ); }
			if ( silent == false ){ wprintf( L"\n%sFolder attributes:%s0x%.8x\t(%s)", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t" : L"\t\t\t" ), attributes, get_file_attributes( attributes ) ); }
			iid_offset += sizeof( unsigned short );

			unsigned char ending_block = 0; memcpy_s( &ending_block, sizeof( unsigned char ), iid + sii.size - ( sizeof( unsigned char ) * 3 ), sizeof( unsigned char ) );
			if ( ending_block == 0 || sii.flag == 0xB1 )	// 0xB1 doesn't have a weird format (terminates with 0), but it does have extra data.
			{
				if ( iid_offset >= sii.size ){ continue; }
				unsigned int s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
				if ( iid_offset + s_len > sii.size ){ continue; }
				// Short directory ASCII
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Short directory name</div><div id=\"d\">%S</div></div>", iid + iid_offset ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Short directory name: %S\r\n", iid + iid_offset ); }
				if ( silent == false ){ printf( "\n%sShort directory name:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t" : "\t\t\t" ), iid + iid_offset ); }

				unsigned short unicode_block_offset = 0; memcpy_s( &unicode_block_offset, sizeof( unsigned short ), iid + sii.size - sizeof( unsigned short ), sizeof( unsigned short ) );

				if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }
				unsigned short unicode_block_length = 0;
				memcpy_s( &unicode_block_length, sizeof( unsigned short ), iid + unicode_block_offset, sizeof( unsigned short ) );

				if ( unicode_block_offset + unicode_block_length > sii.size || unicode_block_offset <= 0 || unicode_block_length <= 0 ){ continue; }
				iid_offset = unicode_block_offset;	// Reset

				// Skip 8 bytes to the timestamps
				if ( iid_offset + 0x08 > sii.size ){ continue; }else{ iid_offset += 0x08; }

				if ( iid_offset + ( sizeof( unsigned short ) * 2 ) > sii.size ){ continue; }
				fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
				fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
				if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
				{
					FileTimeToSystemTime( &ft, &st );
					if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Date created</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Date created: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( silent == false ){ printf( "\n%sDate created:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				}
				iid_offset += ( sizeof( unsigned short ) * 2 );

				if ( iid_offset + ( sizeof( unsigned short ) * 2 ) > sii.size ){ continue; }
				fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
				fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
				if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
				{
					FileTimeToSystemTime( &ft, &st );
					if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last accessed</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Last accessed: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( silent == false ){ printf( "\n%sLast accessed:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				}
				iid_offset += ( sizeof( unsigned short ) * 2 );

				if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }
				unsigned short unicode_name_offset = 0;
				memcpy_s( &unicode_name_offset, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) ); // Name Offset (from the end of the Short name)

				if ( unicode_name_offset >= sii.size || unicode_name_offset <= 0 ){ continue; }
				iid_offset = unicode_block_offset + unicode_name_offset;	// Reset
				
				if ( iid_offset >= sii.size ){ continue; }
				s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
				if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Long directory name</div><div id=\"d\">%s</div></div>", iid + iid_offset ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Long directory name: %s\r\n", iid + iid_offset ); }
				if ( silent == false ){ wprintf( L"\n%sLong directory name:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t" : L"\t\t\t" ), iid + iid_offset ); }
				iid_offset += ( sizeof( wchar_t ) * s_len );

				if ( iid_offset + sizeof( unsigned short ) < ( unsigned int )( unicode_block_offset + unicode_block_length ) )	// More UNICODE strings.
				{
					// Is the last argument UNICODE?
					unsigned short last_value = 0;
					memcpy_s( &last_value, sizeof( unsigned short ), iid + unicode_block_offset + unicode_block_length - sizeof( unsigned int ), sizeof( unsigned short ) );
					if ( last_value == 0x0000 )
					{
						if ( iid_offset >= sii.size ){ continue; }
						s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
						if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }
						if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Argument</div><div id=\"d\">%s</div></div>", iid + iid_offset ); }
						if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Argument: %s\r\n", iid + iid_offset ); }
						if ( silent == false ){ wprintf( L"\n%sArgument:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t" : L"\t\t\t\t" ), iid + iid_offset ); }
					}
					else	// Or ASCII
					{
						if ( iid_offset >= sii.size ){ continue; }
						s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
						if ( iid_offset + s_len > sii.size ){ continue; }
						if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Argument</div><div id=\"d\">%S</div></div>", ( wchar_t * )( iid + iid_offset ) ); }
						if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Argument: %S\r\n", ( wchar_t * )( iid + iid_offset ) ); }
						if ( silent == false ){ printf( "\n%sArgument:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + iid_offset ); }
					}
				}
				/*else if ( iid_offset + sizeof( unsigned short ) < sii.size )	// More info. Seems to contain either strings, or pidls.
				{
					int i = 0;
				}*/
			}
			else if ( ending_block != 0 )	// This is a weird case. Seems to be the opposite of above.
			{
				if ( iid_offset >= sii.size ){ continue; }
				unsigned int s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
				if ( iid_offset + s_len > sii.size ){ continue; }
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Long directory name</div><div id=\"d\">%S</div></div>", iid + iid_offset ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Long directory name: %S\r\n", iid + iid_offset ); }
				if ( silent == false ){ printf( "\n%sLong directory name:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t" : "\t\t\t" ), iid + iid_offset ); }
				iid_offset += s_len;
				
				if ( iid_offset >= sii.size ){ continue; }
				if ( iid_offset + ( strnlen( iid + iid_offset, sii.size - iid_offset ) + 1 ) > sii.size ){ continue; }
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Short directory name</div><div id=\"d\">%S</div></div>", iid + iid_offset ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Short directory name: %S\r\n", iid + iid_offset ); }
				if ( silent == false ){ printf( "\n%sShort directory name:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t" : "\t\t\t" ), iid + iid_offset ); }
			}
		}
		else if ( sii.flag == 0x30 )	// Filename/Folder
		{
			// Adjust 1 byte to the file size.
			if ( iid_offset + sizeof( char ) > sii.size ){ continue; }else{ iid_offset += sizeof( char ); }

			// See if we can get the 4 byte file size.
			if ( iid_offset + sizeof( unsigned int ) > sii.size ){ continue; }
			unsigned int file_size = 0; memcpy_s( &file_size, sizeof( unsigned int ), iid + iid_offset, sizeof( unsigned int ) );
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">File size</div><div id=\"d\">%u bytes</div></div>", file_size ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"File size: %u bytes\r\n", file_size ); }
			if ( silent == false ){ printf( "\n%sFile size:%s%u bytes", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), file_size ); }
			iid_offset += sizeof( unsigned int );

			if ( iid_offset + ( sizeof( unsigned short ) * 2 ) > sii.size ){ continue; }
			unsigned short fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
			unsigned short fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
			SYSTEMTIME st;
			FILETIME ft;
			if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
			{
				FileTimeToSystemTime( &ft, &st );
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last modified</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Last modified: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				if ( silent == false ){ printf( "\n%sLast modified:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
			}
			iid_offset += ( sizeof( unsigned short ) * 2 );

			if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }
			unsigned short attributes = 0; memcpy_s( &attributes, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Folder attributes</div><div id=\"m\">0x%.8x</div><div id=\"d\">%s</div></div>", attributes, get_file_attributes( attributes ) ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Folder attributes: 0x%.8x %s\r\n", attributes, get_file_attributes( attributes ) ); }
			if ( silent == false ){ wprintf( L"\n%sFile/Folder attributes:%s0x%.8x\t(%s)", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t" : L"\t\t\t" ), attributes, get_file_attributes( attributes ) ); }
			iid_offset += sizeof( unsigned short );

			if ( iid_offset >= sii.size ){ continue; }
			unsigned int s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
			if ( iid_offset + s_len > sii.size ){ continue; }
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Filename/Folder:</div><div id=\"d\">%S</div></div>", iid + iid_offset ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Filename/Folder: %S\r\n", iid + iid_offset ); }
			if ( silent == false ){ printf( "\n%sFilename/Folder:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t" : "\t\t\t" ), iid + iid_offset ); }
		}
		else if ( sii.flag == 0x32 || sii.flag == 0x35 )	// Filename
		{
			// Adjust 1 byte to the file size.
			if ( iid_offset + sizeof( char ) > sii.size ){ continue; }else{ iid_offset += sizeof( char ); }

			// See if we can get the 4 byte file size.
			if ( iid_offset + sizeof( unsigned int ) > sii.size ){ continue; }
			unsigned int file_size = 0; memcpy_s( &file_size, sizeof( unsigned int ), iid + iid_offset, sizeof( unsigned int ) );
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">File size</div><div id=\"d\">%u bytes</div></div>", file_size ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"File size: %u bytes\r\n", file_size ); }
			if ( silent == false ){ printf( "\n%sFile size:%s%u bytes", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), file_size ); }
			iid_offset += sizeof( unsigned int );

			if ( iid_offset + ( sizeof( unsigned short ) * 2 ) > sii.size ){ continue; }
			unsigned short fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
			unsigned short fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
			SYSTEMTIME st;
			FILETIME ft;
			if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
			{
				FileTimeToSystemTime( &ft, &st );
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last modified</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Last modified: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				if ( silent == false ){ printf( "\n%sLast modified:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
			}
			iid_offset += ( sizeof( unsigned short ) * 2 );

			if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }
			unsigned short attributes = 0; memcpy_s( &attributes, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Folder attributes</div><div id=\"m\">0x%.8x</div><div id=\"d\">%s</div></div>", attributes, get_file_attributes( attributes ) ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Folder attributes: 0x%.8x %s\r\n", attributes, get_file_attributes( attributes ) ); }
			if ( silent == false ){ wprintf( L"\n%sFile attributes:%s0x%.8x\t(%s)", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t" : L"\t\t\t" ), attributes, get_file_attributes( attributes ) ); }
			iid_offset += sizeof( unsigned short );

			unsigned char ending_block = 0; memcpy_s( &ending_block, sizeof( unsigned char ), iid + sii.size - ( sizeof( unsigned char ) * 3 ), sizeof( unsigned char ) );
			if ( ending_block == 0 )
			{
				if ( iid_offset >= sii.size ){ continue; }
				unsigned int s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
				if ( iid_offset + s_len > sii.size ){ continue; }
				// Short filename ASCII
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">8.3 filename</div><div id=\"d\">%S</div></div>", iid + iid_offset ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"8.3 filename: %S\r\n", iid + iid_offset ); }
				if ( silent == false ){ printf( "\n%s8.3 filename:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + iid_offset ); }

				unsigned short unicode_block_offset = 0; memcpy_s( &unicode_block_offset, sizeof( unsigned short ), iid + sii.size - sizeof( unsigned short ), sizeof( unsigned short ) );
				
				if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }
				unsigned short unicode_block_length = 0;
				memcpy_s( &unicode_block_length, sizeof( unsigned short ), iid + unicode_block_offset, sizeof( unsigned short ) );

				if ( unicode_block_offset + unicode_block_length > sii.size || unicode_block_offset <= 0 || unicode_block_length <= 0 ){ continue; }
				iid_offset = unicode_block_offset;	// Reset

				// Skip 8 bytes to the timestamps
				if ( iid_offset + 0x08 > sii.size ){ continue; }else{ iid_offset += 0x08; }

				if ( iid_offset + ( sizeof( unsigned short ) * 2 ) > sii.size ){ continue; }
				fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
				fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
				if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
				{
					FileTimeToSystemTime( &ft, &st );
					if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Date created</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Date created: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( silent == false ){ printf( "\n%sDate created:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				}
				iid_offset += ( sizeof( unsigned short ) * 2 );

				fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
				fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
				if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
				{
					FileTimeToSystemTime( &ft, &st );
					if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last accessed</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Last accessed: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( silent == false ){ printf( "\n%sLast accessed:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				}
				iid_offset += ( sizeof( unsigned short ) * 2 );

				if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }
				unsigned short unicode_name_offset = 0;
				memcpy_s( &unicode_name_offset, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) ); // Name Offset (from the end of the Short name)

				if ( unicode_name_offset >= sii.size || unicode_name_offset <= 0 ){ continue; }
				iid_offset = unicode_block_offset + unicode_name_offset;	// Reset
				
				if ( iid_offset >= sii.size ){ continue; }
				s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
				if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Long filename</div><div id=\"d\">%s</div></div>", iid + iid_offset ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Long filename: %s\r\n", iid + iid_offset ); }
				if ( silent == false ){ wprintf( L"\n%sLong filename:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t" : L"\t\t\t\t" ), iid + iid_offset ); }
				iid_offset += ( sizeof( wchar_t ) * s_len );

				if ( iid_offset + sizeof( unsigned short ) < ( unsigned int )( unicode_block_offset + unicode_block_length ) )	// More UNICODE strings.
				{
					// Is the last argument UNICODE?
					unsigned short last_block = 0;
					memcpy_s( &last_block, sizeof( unsigned short ), iid + unicode_block_offset + unicode_block_length - sizeof( unsigned int ), sizeof( unsigned short ) );
					if ( last_block == 0x0000 )
					{
						if ( iid_offset >= sii.size ){ continue; }
						s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
						if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }
						if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Argument</div><div id=\"d\">%s</div></div>", iid + iid_offset ); }
						if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Argument: %s\r\n", iid + iid_offset ); }
						if ( silent == false ){ wprintf( L"\n%sArgument:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t" : L"\t\t\t\t" ), iid + iid_offset ); }
					}
					else	// Or ASCII Haven't actually seen this for filenames.
					{
						if ( iid_offset >= sii.size ){ continue; }
						s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
						if ( iid_offset + s_len > sii.size ){ continue; }
						if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Argument</div><div id=\"d\">%S</div></div>", ( wchar_t * )( iid + iid_offset ) ); }
						if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Argument: %S\r\n", ( wchar_t * )( iid + iid_offset ) ); }
						if ( silent == false ){ printf( "\n%sArgument:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + iid_offset ); }
					}
				}
				/*else if ( iid_offset + sizeof( unsigned short ) < sii.size )	// More info.
				{
					int i = 0;
				}*/
			}
			else if ( ending_block != 0 )	// This is a weird case.
			{
				if ( iid_offset >= sii.size ){ continue; }
				unsigned int s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
				if ( iid_offset + s_len > sii.size ){ continue; }
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Long directory name</div><div id=\"d\">%S</div></div>", iid + iid_offset ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Long directory name: %S\r\n", iid + iid_offset ); }
				if ( silent == false ){ printf( "\n%sLong filename:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + iid_offset ); }
				iid_offset += s_len;
				
				if ( iid_offset >= sii.size ){ continue; }
				if ( iid_offset + ( strnlen( iid + iid_offset, sii.size - iid_offset ) + 1 ) > sii.size ){ continue; }
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Short directory name</div><div id=\"d\">%S</div></div>", iid + iid_offset ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Short directory name: %S\r\n", iid + iid_offset ); }
				if ( silent == false ){ printf( "\n%s8.3 filename:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + iid_offset ); }

				// There's some more stuff after this. I think it's another string.
			}
		}
		else if ( sii.flag == 0x2E || sii.flag == 0x74 || sii.flag == 0x77 )	// Filenames with GUIDs
		{
			if ( sii.size == 0x32 && sii.flag == 0x2E )	// Contains only 2 GUIDs
			{
				// Adjust 15 bytes to the first hash.
				if ( iid_offset + 0x0F > sii.size ){ continue; }else{ iid_offset += 0x0F; }

				// Check if we can get a hash
				if ( iid_offset + 0x10 > sii.size ){ continue; }
				char clsid[ 64 ] = { 0 };
				buffer_to_guid( ( unsigned char * )( iid + iid_offset ), clsid );
				char *p_type = get_clsid_type( iid + iid_offset );
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">CLSID</div><div id=\"d\">%S = %S</div></div>", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"CLSID: %S = %S\r\n", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				if ( silent == false ){ printf( "\n%sCLSID:%s%s = %s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t" : "\t\t\t\t\t" ), clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				iid_offset += 0x10;
				
				// Check if we can get a hash
				if ( iid_offset + 0x10 > sii.size ){ continue; }
				buffer_to_guid( ( unsigned char * )( iid + iid_offset ), clsid );
				p_type = get_clsid_type( iid + iid_offset );
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">CLSID</div><div id=\"d\">%S = %S</div></div>", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"CLSID: %S = %S\r\n", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				if ( silent == false ){ printf( "\n%sCLSID:%s%s = %s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t" : "\t\t\t\t\t" ), clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
			}
			else
			{
				// Adjust 11 bytes to the file size.
				if ( iid_offset + 0x0B > sii.size ){ continue; }else{ iid_offset += 0x0B; }

				// See if we can get the 4 byte file size.
				if ( iid_offset + sizeof( unsigned int ) > sii.size ){ continue; }
				unsigned int file_size = 0; memcpy_s( &file_size, sizeof( unsigned int ), iid + iid_offset, sizeof( unsigned int ) );
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">File size</div><div id=\"d\">%u bytes</div></div>", file_size ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"File size: %u bytes\r\n", file_size ); }
				if ( silent == false ){ printf( "\n%sFile size:%s%u bytes", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), file_size ); }
				iid_offset += sizeof( unsigned int );

				if ( iid_offset + ( sizeof( unsigned short ) * 2 ) > sii.size ){ continue; }
				unsigned short fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
				unsigned short fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
				SYSTEMTIME st;
				FILETIME ft;
				if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
				{
					FileTimeToSystemTime( &ft, &st );
					if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last modified</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Last modified: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( silent == false ){ printf( "\n%sLast modified:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				}
				iid_offset += ( sizeof( unsigned short ) * 2 );

				if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }
				unsigned short attributes = 0; memcpy_s( &attributes, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Folder attributes</div><div id=\"m\">0x%.8x</div><div id=\"d\">%s</div></div>", attributes, get_file_attributes( attributes ) ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Folder attributes: 0x%.8x %s\r\n", attributes, get_file_attributes( attributes ) ); }
				if ( silent == false ){ wprintf( L"\n%sFile attributes:%s0x%.8x\t(%s)", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t" : L"\t\t\t" ), attributes, get_file_attributes( attributes ) ); }
				iid_offset += sizeof( unsigned short );

				if ( iid_offset >= sii.size ){ continue; }
				unsigned int s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
				if ( iid_offset + s_len > sii.size ){ continue; }
				// Short filename ASCII
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">8.3 filename</div><div id=\"d\">%S</div></div>", iid + 0x0A + 0x0E ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"8.3 filename: %S\r\n", iid + 0x0A + 0x0E ); }
				if ( silent == false ){ printf( "\n%s8.3 filename:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + 0x0A + 0x0E ); }
				iid_offset += s_len;

				// Skip 2 bytes to the hash values.
				if ( iid_offset + 0x02 > sii.size ){ continue; }else{ iid_offset += 0x02; }

				// Check if we can get a hash
				if ( iid_offset + 0x10 > sii.size ){ continue; }
				char clsid[ 64 ] = { 0 };
				buffer_to_guid( ( unsigned char * )( iid + iid_offset ), clsid );
				char *p_type = get_clsid_type( iid + iid_offset );
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">CLSID</div><div id=\"d\">%S = %S</div></div>", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"CLSID: %S = %S\r\n", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				if ( silent == false ){ printf( "\n%sCLSID:%s%s = %s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t" : "\t\t\t\t\t" ), clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				iid_offset += 0x10;
				
				// Check if we can get a hash
				if ( iid_offset + 0x10 > sii.size ){ continue; }
				buffer_to_guid( ( unsigned char * )( iid + iid_offset ), clsid );
				p_type = get_clsid_type( iid + iid_offset );
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">CLSID</div><div id=\"d\">%S = %S</div></div>", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"CLSID: %S = %S\r\n", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				if ( silent == false ){ printf( "\n%sCLSID:%s%s = %s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t" : "\t\t\t\t\t" ), clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
				iid_offset += 0x10;

				unsigned short unicode_block_offset = 0; memcpy_s( &unicode_block_offset, sizeof( unsigned short ), iid + sii.size - sizeof( unsigned short ), sizeof( unsigned short ) );
				//unsigned short unicode_block_length = 0;
				//memcpy_s( &unicode_block_length, sizeof( unsigned short ), iid + unicode_block_offset, sizeof( unsigned short ) ); // Block LENGTH
				
				if ( unicode_block_offset >= sii.size || unicode_block_offset <= 0 ){ continue; }
				iid_offset = unicode_block_offset;	// Reset

				// Skip 8 bytes to the timestamps
				if ( iid_offset + 0x08 > sii.size ){ continue; }else{ iid_offset += 0x08; }

				if ( iid_offset + ( sizeof( unsigned short ) * 2 ) > sii.size ){ continue; }
				fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
				fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
				if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
				{
					FileTimeToSystemTime( &ft, &st );
					if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Date created</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Date created: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( silent == false ){ printf( "\n%sDate created:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				}
				iid_offset += ( sizeof( unsigned short ) * 2 );

				fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
				fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
				if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
				{
					FileTimeToSystemTime( &ft, &st );
					if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last accessed</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Last accessed: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( silent == false ){ printf( "\n%sLast accessed:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				}
				iid_offset += ( sizeof( unsigned short ) * 2 );

				if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }
				unsigned short unicode_name_offset = 0;
				memcpy_s( &unicode_name_offset, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) ); // Name Offset (from the end of the Short name)

				if ( unicode_name_offset >= sii.size || unicode_name_offset <= 0 ){ continue; }
				iid_offset = unicode_block_offset + unicode_name_offset;	// Reset
				
				if ( iid_offset >= sii.size ){ continue; }
				s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
				if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Long filename</div><div id=\"d\">%s</div></div>", iid + iid_offset ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Long filename: %s\r\n", iid + iid_offset ); }
				if ( silent == false ){ wprintf( L"\n%sLong filename:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t" : L"\t\t\t\t" ), iid + iid_offset ); }
				iid_offset += ( sizeof( wchar_t ) * s_len );
			}

			// There might be additional structures after this that contain GUID.
		}
		else if ( sii.flag == 0x36 )	// All strings are UNICODE
		{
			// Adjust 1 byte to the file size.
			if ( iid_offset + sizeof( char ) > sii.size ){ continue; }else{ iid_offset += sizeof( char ); }

			// See if we can get the 4 byte file size.
			if ( iid_offset + sizeof( unsigned int ) > sii.size ){ continue; }
			unsigned int file_size = 0;  memcpy_s( &file_size, sizeof( unsigned int ), iid + iid_offset, sizeof( unsigned int ) );
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">File size</div><div id=\"d\">%u bytes</div></div>", file_size ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"File size: %u bytes\r\n", file_size ); }
			if ( silent == false ){ printf( "\n%sFile size:%s%u bytes", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), file_size ); }
			iid_offset += sizeof( unsigned int );

			if ( iid_offset + ( sizeof( unsigned short ) * 2 ) > sii.size ){ continue; }
			unsigned short fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
			unsigned short fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
			SYSTEMTIME st;
			FILETIME ft;
			if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
			{
				FileTimeToSystemTime( &ft, &st );
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last modified</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Last modified: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				if ( silent == false ){ printf( "\n%sLast modified:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
			}
			iid_offset += ( sizeof( unsigned short ) * 2 );

			if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }
			unsigned short attributes = 0; memcpy_s( &attributes, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Folder attributes</div><div id=\"m\">0x%.8x</div><div id=\"d\">%s</div></div>", attributes, get_file_attributes( attributes ) ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Folder attributes: 0x%.8x %s\r\n", attributes, get_file_attributes( attributes ) ); }
			if ( silent == false ){ wprintf( L"\n%sFile attributes:%s0x%.8x\t(%s)", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t" : L"\t\t\t" ), attributes, get_file_attributes( attributes ) ); }
			iid_offset += sizeof( unsigned short );

			unsigned char ending_block = 0; memcpy_s( &ending_block, sizeof( unsigned char ), iid + sii.size - ( sizeof( unsigned char ) * 3 ), sizeof( unsigned char ) );
			if ( ending_block == 0 )
			{
				if ( iid_offset >= sii.size ){ continue; }
				unsigned int s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
				if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }
				// First long filename? UNICODE I found an instance in which this isn't terminated before the unicode block write_offset2.
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Long filename</div><div id=\"d\">%s</div></div>", iid + iid_offset ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Long filename: %s\r\n", iid + iid_offset ); }
				if ( silent == false ){ wprintf( L"\n%sLong filename:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t" : L"\t\t\t\t" ), iid + iid_offset ); }

				unsigned short unicode_block_offset = 0; memcpy_s( &unicode_block_offset, sizeof( unsigned short ), iid + sii.size - sizeof( unsigned short ), sizeof( unsigned short ) );
				
				if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }
				unsigned short unicode_block_length = 0;
				memcpy_s( &unicode_block_length, sizeof( unsigned short ), iid + unicode_block_offset, sizeof( unsigned short ) );

				if ( unicode_block_offset + unicode_block_length > sii.size || unicode_block_offset <= 0 || unicode_block_length <= 0 ){ continue; }
				iid_offset = unicode_block_offset;	// Reset

				// Skip 8 bytes to the timestamps
				if ( iid_offset + 0x08 > sii.size ){ continue; }else{ iid_offset += 0x08; }

				if ( iid_offset + ( sizeof( unsigned short ) * 2 ) > sii.size ){ continue; }
				fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
				fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
				if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
				{
					FileTimeToSystemTime( &ft, &st );
					if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Date created</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Date created: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( silent == false ){ printf( "\n%sDate created:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				}
				iid_offset += ( sizeof( unsigned short ) * 2 );

				fat_date = 0; memcpy_s( &fat_date, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) );
				fat_time = 0; memcpy_s( &fat_time, sizeof( unsigned short ), iid + iid_offset + 0x02, sizeof( unsigned short ) );
				if ( DosDateTimeToFileTime( fat_date, fat_time, &ft ) )
				{
					FileTimeToSystemTime( &ft, &st );
					if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Last accessed</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Last accessed: %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
					if ( silent == false ){ printf( "\n%sLast accessed:%s%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				}
				iid_offset += ( sizeof( unsigned short ) * 2 );

				if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }
				unsigned short unicode_name_offset = 0;
				memcpy_s( &unicode_name_offset, sizeof( unsigned short ), iid + iid_offset, sizeof( unsigned short ) ); // Name Offset (from the end of the Short name)

				if ( unicode_name_offset >= sii.size || unicode_name_offset <= 0 ){ continue; }
				iid_offset = unicode_block_offset + unicode_name_offset;	// Reset
				
				if ( iid_offset >= sii.size ){ continue; }
				s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
				if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }
				if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Long filename</div><div id=\"d\">%s</div></div>", iid + iid_offset ); }
				if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Long filename: %s\r\n", iid + iid_offset ); }
				if ( silent == false ){ wprintf( L"\n%sLong filename:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t" : L"\t\t\t\t" ), iid + iid_offset ); }
				iid_offset += ( sizeof( wchar_t ) * s_len );

				if ( iid_offset + sizeof( unsigned short ) < ( unsigned int )( unicode_block_offset + unicode_block_length ) )	// More UNICODE strings.
				{
					// Is the last argument UNICODE?
					unsigned short last_block = 0;
					memcpy_s( &last_block, sizeof( unsigned short ), iid + unicode_block_offset + unicode_block_length - sizeof( unsigned int ), sizeof( unsigned short ) );
					if ( last_block == 0x0000 )
					{
						if ( iid_offset >= sii.size ){ continue; }
						s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
						if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }
						if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Argument</div><div id=\"d\">%s</div></div>", iid + iid_offset ); }
						if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Argument: %s\r\n", iid + iid_offset ); }
						if ( silent == false ){ wprintf( L"\n%sArgument:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t" : L"\t\t\t\t" ), iid + iid_offset ); }
					}
					else	// Or ASCII Haven't actually seen this for filenames.
					{
						if ( iid_offset >= sii.size ){ continue; }
						s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
						if ( iid_offset + s_len > sii.size ){ continue; }
						if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Argument</div><div id=\"d\">%S</div></div>", ( wchar_t * )( iid + iid_offset ) ); }
						if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Argument: %S\r\n", ( wchar_t * )( iid + iid_offset ) ); }
						if ( silent == false ){ printf( "\n%sArgument:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + iid_offset ); }
					}
				}
				/*else if ( iid_offset + sizeof( unsigned short ) < sii.size )	// More info.
				{
					int i = 0;
				}*/
			}
			/*else if ( ending_block != 0 )
			{
				int i = 0;
			}*/
		}
		else if ( sii.flag == 0x44 )	// Single UNICODE string?
		{
			// Adjust 1 byte to the file size.
			if ( iid_offset + sizeof( char ) > sii.size ){ continue; }else{ iid_offset += sizeof( char ); }

			if ( iid_offset >= sii.size ){ continue; }
			unsigned int s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
			if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Long filename</div><div id=\"d\">%s</div></div>", iid + iid_offset ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Long filename: %s\r\n", iid + iid_offset ); }
			if ( silent == false ){ wprintf( L"\n%sLong filename:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t" : L"\t\t\t\t" ), iid + iid_offset ); }
		}
		else if ( sii.flag == 0x61 )	// URLs
		{
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"MSITStore<br />" ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"MSITStore\r\n" ); }
			if ( silent == false ){ printf( "\n%sMSITStore", ( tab == true ? "\t\t\t\t\t" : "" ) ); }
			
			if ( iid_offset + sizeof( unsigned char ) > sii.size ){ continue; }
			unsigned char type = 0; memcpy_s( &type, sizeof( unsigned char ), iid + iid_offset, sizeof( unsigned char ) );
			iid_offset += sizeof( unsigned char );

			if ( type == 0x80 )
			{
				// Skip 4 bytes to the string.
				if ( iid_offset + 0x04 > sii.size ){ continue; }else{ iid_offset += 0x04; }

				if ( iid_offset >= sii.size ){ continue; }
				unsigned int s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
				if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }

				if ( output_html == true ) { wchar_t *tbuf = escape_html_unicode( ( wchar_t * )( iid + iid_offset ) ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">URL</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : ( wchar_t * )( iid + iid_offset ) ) ); free( tbuf ); }
				if ( output_csv == true && use_csv == true ) { wchar_t *tbuf = escape_string_unicode( ( wchar_t * )( iid + iid_offset ) ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"URL: %s\r\n", ( tbuf != NULL ? tbuf : ( wchar_t * )( iid + iid_offset ) ) ); free( tbuf ); }
				if ( silent == false ){ wprintf( L"\n%sURL:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t\t" : L"\t\t\t\t\t" ), iid + iid_offset ); }
			}
			/*else if ( type == 0x03 )	// protocol, username, hostname
			{
				int i = 0;
			}
			else
			{
				int i = 0;
			}*/
		}
		else if ( sii.flag == 0x41 )	// Workgroup
		{
			// Adjust 2 bytes to the string name.
			if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }else{ iid_offset += sizeof( unsigned short ); }

			if ( iid_offset >= sii.size ){ continue; }
			unsigned int s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
			if ( iid_offset + s_len > sii.size ){ continue; }

			if ( output_html == true ) { char *tbuf = escape_html_ascii( iid + iid_offset ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"Workgroup<br /><div id=\"r\"><div id=\"h\">Network name</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( output_csv == true && use_csv == true ) { char *tbuf = escape_string_ascii( iid + iid_offset ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Workgroup\r\nNetwork name: %S\r\n", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( silent == false ){ printf( "\n%sWorkgroup\n%sNetwork name:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + iid_offset ); }
			iid_offset += s_len;

			if ( iid_offset >= sii.size ){ continue; }
			s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
			if ( iid_offset + s_len > sii.size ){ continue; }

			if ( output_html == true ) { char *tbuf = escape_html_ascii( iid + iid_offset ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Network protocol</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( output_csv == true && use_csv == true ) { char *tbuf = escape_string_ascii( iid + iid_offset ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Network protocol: %S\r\n", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( silent == false ){ printf( "\n%sNetwork protocol:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t" : "\t\t\t" ), iid + iid_offset ); }
		}
		else if ( sii.flag == 0x42 )	// Computer network
		{
			// Adjust 2 bytes to the string name.
			if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }else{ iid_offset += sizeof( unsigned short ); }

			if ( iid_offset >= sii.size ){ continue; }
			unsigned int s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
			if ( iid_offset + s_len > sii.size ){ continue; }

			if ( output_html == true ) { char *tbuf = escape_html_ascii( iid + iid_offset ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"Computer Network<br /><div id=\"r\"><div id=\"h\">Network name</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( output_csv == true && use_csv == true ) { char *tbuf = escape_string_ascii( iid + iid_offset ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Computer Network\r\nNetwork name: %S\r\n", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( silent == false ){ printf( "\n%sComputer Network\n%sNetwork name:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + iid_offset ); }
			iid_offset += s_len;

			if ( iid_offset >= sii.size ){ continue; }
			s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
			if ( iid_offset + s_len > sii.size ){ continue; }

			if ( output_html == true ) { char *tbuf = escape_html_ascii( iid + iid_offset ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Network protocol</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( output_csv == true && use_csv == true ) { char *tbuf = escape_string_ascii( iid + iid_offset ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Network protocol: %S\r\n", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( silent == false ){ printf( "\n%sNetwork protocol:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t" : "\t\t\t" ), iid + iid_offset ); }
		}
		else if ( sii.flag == 0x46 )	// Network provider
		{
			// Adjust 2 bytes to the string name.
			if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }else{ iid_offset += sizeof( unsigned short ); }

			if ( iid_offset >= sii.size ){ continue; }
			unsigned int s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
			if ( iid_offset + s_len > sii.size ){ continue; }

			if ( output_html == true ) { char *tbuf = escape_html_ascii( iid + iid_offset ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"Network Provider<br /><div id=\"r\"><div id=\"h\">Network name</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( output_csv == true && use_csv == true ) { char *tbuf = escape_string_ascii( iid + iid_offset ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Network Provider\r\nNetwork name: %S\r\n", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( silent == false ){ printf( "\n%sNetwork Provider\n%sNetwork name:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + iid_offset ); }
			iid_offset += s_len;

			if ( iid_offset >= sii.size ){ continue; }
			s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
			if ( iid_offset + s_len > sii.size ){ continue; }

			if ( output_html == true ) { char *tbuf = escape_html_ascii( iid + iid_offset ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Network protocol</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( output_csv == true && use_csv == true ) { char *tbuf = escape_string_ascii( iid + iid_offset ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Network protocol: %S\r\n", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( silent == false ){ printf( "\n%sNetwork protocol:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t" : "\t\t\t" ), iid + iid_offset ); }
			iid_offset += s_len;

			if ( iid_offset + sizeof( unsigned short ) < sii.size )	// More info?
			{
				if ( iid_offset >= sii.size ){ continue; }
				s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
				if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }

				if ( output_html == true ) { wchar_t *tbuf = escape_html_unicode( ( wchar_t * )( iid + iid_offset ) ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Network name (UNICODE)</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : ( wchar_t * )( iid + iid_offset ) ) ); free( tbuf ); }
				if ( output_csv == true && use_csv == true ) { wchar_t *tbuf = escape_string_unicode( ( wchar_t * )( iid + iid_offset ) ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Network name (UNICODE): %s\r\n", ( tbuf != NULL ? tbuf : ( wchar_t * )( iid + iid_offset ) ) ); free( tbuf ); }
				if ( silent == false ){ wprintf( L"\n%sNetwork name (UNICODE):%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t" : L"\t\t\t" ), iid + iid_offset ); }
				iid_offset += ( sizeof( wchar_t ) * s_len );

				if ( iid_offset >= sii.size ){ continue; }
				s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
				if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }

				if ( output_html == true ) { wchar_t *tbuf = escape_html_unicode( ( wchar_t * )( iid + iid_offset ) ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Network protocol (UNICODE)</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : ( wchar_t * )( iid + iid_offset ) ) ); free( tbuf ); }
				if ( output_csv == true && use_csv == true ) { wchar_t *tbuf = escape_string_unicode( ( wchar_t * )( iid + iid_offset ) ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Network protocol (UNICODE): %s\r\n", ( tbuf != NULL ? tbuf : ( wchar_t * )( iid + iid_offset ) ) ); free( tbuf ); }
				if ( silent == false ){ wprintf( L"\n%sNetwork protocol (UNICODE):%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t" : L"\t\t" ), iid + iid_offset ); }
			}
		}
		else if ( sii.flag == 0x47 )	// Whole network
		{
			// Adjust 2 bytes to the string name.
			if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }else{ iid_offset += sizeof( unsigned short ); }

			if ( iid_offset >= sii.size ){ continue; }
			unsigned int s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
			if ( iid_offset + s_len > sii.size ){ continue; }

			if ( output_html == true ) { char *tbuf = escape_html_ascii( iid + iid_offset ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"Whole Network<br /><div id=\"r\"><div id=\"h\">Network name</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( output_csv == true && use_csv == true ) { char *tbuf = escape_string_ascii( iid + iid_offset ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Whole Network\r\nNetwork name: %S\r\n", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( silent == false ){ printf( "\n%sWhole Network\n%sNetwork name:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + iid_offset ); }
		}
		else if ( sii.flag == 0xC3 )	// Network share
		{
			// Adjust 2 bytes to the string name.
			if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }else{ iid_offset += sizeof( unsigned short ); }

			if ( iid_offset >= sii.size ){ continue; }
			unsigned int s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
			if ( iid_offset + s_len > sii.size ){ continue; }

			if ( output_html == true ) { char *tbuf = escape_html_ascii( iid + iid_offset ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"Network Share<br /><div id=\"r\"><div id=\"h\">Network name</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( output_csv == true && use_csv == true ) { char *tbuf = escape_string_ascii( iid + iid_offset ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Network Share\r\nNetwork name: %S\r\n", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( silent == false ){ printf( "\n%sNetwork Share\n%sNetwork name:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + iid_offset ); }
			iid_offset += s_len;

			if ( iid_offset >= sii.size ){ continue; }
			s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
			if ( iid_offset + s_len > sii.size ){ continue; }

			if ( output_html == true ) { char *tbuf = escape_html_ascii( iid + iid_offset ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Network protocol</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( output_csv == true && use_csv == true ) { char *tbuf = escape_string_ascii( iid + iid_offset ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Network protocol: %S\r\n", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( silent == false ){ printf( "\n%sNetwork protocol:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t" : "\t\t\t" ), iid + iid_offset ); }
			iid_offset += s_len;

			if ( iid_offset >= sii.size ){ continue; }
			s_len = strnlen( iid + iid_offset, sii.size - iid_offset ) + 1;
			if ( iid_offset + s_len > sii.size ){ continue; }

			if ( output_html == true ) { char *tbuf = escape_html_ascii( iid + iid_offset ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Description</div><div id=\"d\">%S</div></div>", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( output_csv == true && use_csv == true ) { char *tbuf = escape_string_ascii( iid + iid_offset ); write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Description: %S\r\n", ( tbuf != NULL ? tbuf : iid + iid_offset ) ); free( tbuf ); }
			if ( silent == false ){ printf( "\n%sDescription:%s%s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t" : "\t\t\t\t" ), iid + iid_offset ); }
		}
		else if ( sii.flag == 0xFF ) // More network stuff. Remote Access Service
		{
			// Adjust 9 bytes to the first hash.
			if ( iid_offset + 0x09 > sii.size ){ continue; }else{ iid_offset += 0x09; }

			// Check if we can get a hash
			if ( iid_offset + 0x10 > sii.size ){ continue; }
			char clsid[ 64 ] = { 0 };
			buffer_to_guid( ( unsigned char * )( iid + iid_offset ), clsid );
			char *p_type = get_clsid_type( iid + iid_offset );
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">CLSID</div><div id=\"d\">%S = %S</div></div>", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"CLSID: %S = %S\r\n", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
			if ( silent == false ){ printf( "\n%sCLSID:%s%s = %s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t" : "\t\t\t\t\t" ), clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
			iid_offset += 0x10;
			
			// Check if we can get a hash
			if ( iid_offset + 0x10 > sii.size ){ continue; }
			buffer_to_guid( ( unsigned char * )( iid + iid_offset ), clsid );
			p_type = get_clsid_type( iid + iid_offset );
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">CLSID</div><div id=\"d\">%S = %S</div></div>", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"CLSID: %S = %S\r\n", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
			if ( silent == false ){ printf( "\n%sCLSID:%s%s = %s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t" : "\t\t\t\t\t" ), clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
			iid_offset += 0x10;

			// Adjust 68 bytes to the first string.
			if ( iid_offset + 0x44 > sii.size ){ continue; }else{ iid_offset += 0x44; }

			if ( iid_offset >= sii.size ){ continue; }
			unsigned int s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
			if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">String value</div><div id=\"d\">%s</div></div>", iid + iid_offset ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"String value: %s\r\n", iid + iid_offset ); }
			if ( silent == false ){ wprintf( L"\n%sString value:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t" : L"\t\t\t\t" ), iid + iid_offset ); }
			iid_offset += ( sizeof( wchar_t ) * s_len );

			if ( iid_offset >= sii.size ){ continue; }
			s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
			if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">String value</div><div id=\"d\">%s</div></div>", iid + iid_offset ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"String value: %s\r\n", iid + iid_offset ); }
			if ( silent == false ){ wprintf( L"\n%sString value:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t" : L"\t\t\t\t" ), iid + iid_offset ); }
			iid_offset += ( sizeof( wchar_t ) * s_len );

			// Adjust 2 bytes to the last string. Not sure what this value is.
			if ( iid_offset + sizeof( unsigned short ) > sii.size ){ continue; }else{ iid_offset += sizeof( unsigned short ); }

			if ( iid_offset >= sii.size ){ continue; }
			s_len = wcsnlen( ( wchar_t * )( iid + iid_offset ), ( sii.size - iid_offset ) / sizeof( wchar_t ) ) + 1;
			if ( iid_offset + ( sizeof( wchar_t ) * s_len ) > sii.size ){ continue; }
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">String value</div><div id=\"d\">%s</div></div>", iid + iid_offset ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"String value: %s\r\n", iid + iid_offset ); }
			if ( silent == false ){ wprintf( L"\n%sString value:%s%s", ( tab == true ? L"\t\t\t\t\t" : L"" ), ( tab == true ? L"\t\t" : L"\t\t\t\t" ), iid + iid_offset ); }
			iid_offset += ( sizeof( wchar_t ) * s_len );

			// Check if we can get a hash
			if ( iid_offset + 0x10 > sii.size ){ continue; }
			buffer_to_guid( ( unsigned char * )( iid + iid_offset ), clsid );
			p_type = get_clsid_type( iid + iid_offset );
			if ( output_html == true ) { write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">CLSID</div><div id=\"d\">%S = %S</div></div>", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"CLSID: %S = %S\r\n", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
			if ( silent == false ){ printf( "\n%sCLSID:%s%s = %s", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t\t\t" : "\t\t\t\t\t" ), clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
		}
		/*else if ( sii.flag == 0x01 ) // I have no idea what this is, but it exists. 0x0C in size.
		{
			
		}*/
		else
		{
			if ( output_html == true )
			{
				write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Unknown/Unsupported Entry</div><div id=\"d\">%.02x</div></div>", sii.flag );
			
				if ( dump_remnant == true && p_offset + sii.size <= pidl_size )
				{
					write_html_dump( hFile_html, write_buf2, write_offset2, L"Unknown/Unsupported entry dump", ( unsigned char * )iid, sii.size );
				}
			}
			if ( output_csv == true && use_csv == true ) { write_offset += swprintf( write_buf + write_offset, BUFFER_SIZE - write_offset, L"Unknown/Unsupported entry: %.02x\r\n", sii.flag ); }
			if ( silent == false ){ printf( "\n%sUnknown/Unsupported entry:%s%.02x", ( tab == true ? "\t\t\t\t\t" : "" ), ( tab == true ? "\t" : "\t\t" ), sii.flag ); }
		}
	}

	pidl_offset += pidl_size;	// Hopefully we won't go out of bounds. Update the offset with the size of the pidl.
}
