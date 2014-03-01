#include "globals.h"

unsigned int parse_system_kind( char *property_store_value, unsigned int property_size, unsigned int property_store_value_offset );	// 1e3ee840-bc2b-476c-8237-2acd1a839b22

wchar_t *get_value( char *property_store_value, unsigned int property_size, unsigned int &property_store_value_offset )
{
	// Make sure we have 4 bytes.
	if ( property_store_value_offset + sizeof( unsigned int ) > property_size ){ return NULL; }
	unsigned int value_length = 0;
	memcpy_s( &value_length, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );	// We'll exclude the added NULL chracter from this value.
	property_store_value_offset += sizeof( unsigned int );

	// The value won't be NULL terminated, but the length includes it...
	if ( value_length <= 0 )
	{
		return NULL;	// If the length is 0, then we can't continue.
	}
	--value_length;	// Ignore the included NULL character.

	// Make sure we have enough to get the string value.
	if ( ( property_store_value_offset + ( value_length * sizeof( wchar_t ) ) ) > property_size ){ return NULL; }
	wchar_t *value = ( wchar_t * )malloc( sizeof( wchar_t ) * ( value_length + 1 ) );
	wcsncpy_s( value, value_length + 1, ( wchar_t * )( property_store_value + property_store_value_offset ), value_length );
	property_store_value_offset += ( value_length * sizeof( wchar_t ) );

	wchar_t *tbuf = escape_html_unicode( value );
	if ( tbuf != NULL )
	{
		free( value );
		value = tbuf;
	}

	return value;
}

unsigned int parse_timestamp( char *property_store_value, unsigned int property_size, unsigned int property_store_value_offset )
{
	// Make sure we have 16 bytes to compare.
	if ( property_store_value_offset + 0x10 > property_size ){ return property_store_value_offset; }
	/*if ( memcmp( property_store_value + property_store_value_offset, "\x7b\x08\x54\x95\xb6\xce\xab\x45\x99\xff\x50\xe8\x42\x8e\x86\x0d", 16 ) == 0 )
	{
		int i = 0;
	}
	else
	{
		int i = 0;
	}*/

	property_store_value_offset += 0x10;	// Hash 1?

	// Make sure we have 16 bytes to compare.
	if ( property_store_value_offset + 0x10 > property_size ){ return property_store_value_offset; }
	/*if ( memcmp( property_store_value + property_store_value_offset, "\x66\x9b\x4b\xc6\x3d\xe5\x56\x4c\xb9\xae\xfe\xde\x4e\xe9\x5d\xb1", 16 ) == 0 )
	{
		int i = 0;
	}
	else
	{
		int i = 0;
	}*/

	property_store_value_offset += 0x10;	// Hash 2?

	// Make sure we have 4 bytes.
	if ( property_store_value_offset + sizeof( unsigned int ) > property_size ){ return property_store_value_offset; }
	unsigned int some_value = 0;
	memcpy_s( &some_value, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );	// Flag
	property_store_value_offset += sizeof( unsigned int );

	// Name and codepage included.
	if ( some_value == 0x00000000 )
	{
		wchar_t *name = get_value( property_store_value, property_size, property_store_value_offset );
		if ( name != NULL )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Value</div><div id=\"d\">%s</div></div>", name ); }
			if ( silent == false ){ wprintf( L"\n\t\t\t\t\tValue:\t\t\t%s", name ); }
			free( name );
		}

		name = get_value( property_store_value, property_size, property_store_value_offset );
		if ( name != NULL )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Language code</div><div id=\"d\">%s</div></div>", name ); }
			if ( silent == false ){ wprintf( L"\n\t\t\t\t\tLanguage code:\t\t%s", name ); }
			free( name );
		}

		if ( property_store_value_offset + 0x08 > property_size ){ return property_store_value_offset; }
		property_store_value_offset += 0x08;	// timestamp?
	}
	else
	{
		if ( property_store_value_offset + 0x10 > property_size ){ return property_store_value_offset; }
		property_store_value_offset += 0x08;	// timestamp?
		property_store_value_offset += 0x08;	// timestamp?
	}

	if ( property_store_value_offset + 0x08 > property_size ){ return property_store_value_offset; }
	FILETIME ft = { NULL };
	memcpy_s( &ft, sizeof( FILETIME ), property_store_value + property_store_value_offset, sizeof( FILETIME ) );
	SYSTEMTIME st;
	FileTimeToSystemTime( &ft, &st );
	if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Timestamp</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
	if ( silent == false ){ wprintf( L"\n\t\t\t\t\tTimestamp:\t\t%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }

	property_store_value_offset += 0x08;

	if ( some_value == 0x00000000 )
	{
		if ( property_store_value_offset + 0x04 > property_size ){ return property_store_value_offset; }
		property_store_value_offset += 0x04;	// some value

		property_store_value_offset = parse_system_kind( property_store_value, property_size, property_store_value_offset );
	}

	return property_store_value_offset;
}

unsigned int identify_property( char *property_store_value, unsigned int property_size, unsigned int property_store_value_offset )
{
	// Make sure we have 16 bytes for the hash.
	if ( property_store_value_offset + 0x10 > property_size ){ return 0; }
	char clsid[ 64 ] = { 0 };
	buffer_to_guid( ( unsigned char * )( property_store_value + property_store_value_offset ), clsid );

	char *property_name = get_clsid_type( clsid );

	// Make sure we have 4 bytes for the property_type.
	if ( property_store_value_offset + 0x10 + sizeof( unsigned int ) > property_size ){ return 0; }
	unsigned int property_type = 0;
	memcpy_s( &property_type, sizeof( unsigned int ), property_store_value + property_store_value_offset + 0x10, sizeof( unsigned int ) );

	if ( property_name == NULL )
	{
		property_name = get_prop_id_type( property_store_value + property_store_value_offset, property_type );

		if ( property_name == NULL )
		{
			// Sometimes the class name is a string rather than a hash. This check is not 100% guaranteed. TODO: Figure out why it occurs.
			unsigned int string_length = 0;
			memcpy_s( &string_length, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );
			if ( string_length > 0 && string_length <= property_size )
			{
				return 0;
			}
			/*else	// Unknown hash?
			{
				int i = 0;
			}*/
		}
	}

	if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">CLSID</div><div id=\"d\">%S</div></div>", clsid ); }
	if ( silent == false ){ printf( "\n\t\t\t\t\tCLSID:\t\t\t%s", clsid ); }

	if ( property_name == NULL )
	{
		if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">ID</div><div id=\"d\">%lu</div></div>", property_type ); }
		if ( silent == false ){ printf( "\n\t\t\t\t\tID:\t\t\t%lu", property_type ); }
	}
	else
	{
		if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">ID</div><div id=\"d\">%S</div></div>", property_name ); }
		if ( silent == false ){ printf( "\n\t\t\t\t\tID:\t\t\t%s", property_name ); }
	}

	property_store_value_offset += 0x10;	// Hash
	property_store_value_offset += 0x04;	// PropID

	return 0x14;//property_store_value_offset;
}

unsigned int parse_system_kind( char *property_store_value, unsigned int property_size, unsigned int property_store_value_offset )	// 1e3ee840-bc2b-476c-8237-2acd1a839b22
{
	// http://msdn.microsoft.com/en-us/library/cc251929(v=prot.10).aspx

	unsigned int new_offset = identify_property( property_store_value, property_size, property_store_value_offset );
	property_store_value_offset += new_offset;
	if ( new_offset == 0 )
	{
		wchar_t *name = get_value( property_store_value, property_size, property_store_value_offset );

		if ( name != NULL )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Class name</div><div id=\"d\">%s</div></div>", name ); }
			if ( silent == false ){ wprintf( L"\n\t\t\t\t\tClass name:\t\t%s", name ); }
			free( name );
		}

		if ( property_store_value_offset + 0x04 > property_size ){ return property_store_value_offset; }
		property_store_value_offset += 0x04;
	}

	if ( property_store_value_offset + 0x04 > property_size ){ return property_store_value_offset; }
	property_store_value_offset += 0x04;	// Some value

	// Make sure we have 2 bytes for the content_type.
	if ( property_store_value_offset + sizeof( unsigned short ) > property_size ){ return property_store_value_offset; }
	unsigned short content_type = 0;
	memcpy_s( &content_type, sizeof( unsigned short ), property_store_value + property_store_value_offset, sizeof( unsigned short ) );
	property_store_value_offset += sizeof( unsigned short );

	if ( content_type == 0x1F )
	{
		wchar_t *name = get_value( property_store_value, property_size, property_store_value_offset );
		if ( name != NULL )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Value</div><div id=\"d\">%s</div></div>", name ); }
			if ( silent == false ){ wprintf( L"\n\t\t\t\t\tValue:\t\t\t%s", name ); }
			free( name );
		}

		name = get_value( property_store_value, property_size, property_store_value_offset );
		if ( name != NULL )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Class name</div><div id=\"d\">%s</div></div>", name ); }
			if ( silent == false ){ wprintf( L"\n\t\t\t\t\tClass name:\t\t%s", name ); }
			free( name );
		}

		name = get_value( property_store_value, property_size, property_store_value_offset );
		if ( name != NULL )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Language code</div><div id=\"d\">%s</div></div>", name ); }
			if ( silent == false ){ wprintf( L"\n\t\t\t\t\tLanguage code:\t\t%s", name ); }
			free( name );
		}

		// Make sure we have 2 bytes for some_value1.
		if ( property_store_value_offset + sizeof( unsigned short ) > property_size ){ return property_store_value_offset; }
		unsigned short some_value1 = 0;
		memcpy_s( &some_value1, sizeof( unsigned short ), property_store_value + property_store_value_offset, sizeof( unsigned short ) );
		property_store_value_offset += sizeof( unsigned short );

		if ( some_value1 == 0x0200 )
		{
			if ( property_store_value_offset + 0x12 > property_size ){ return property_store_value_offset; }
			property_store_value_offset += 0x12;	// Some values;
		}
		else if ( some_value1 == 0x0102 )
		{
			if ( property_store_value_offset + 0x07 > property_size ){ return property_store_value_offset; }

			property_store_value_offset += 0x07;	// Some values;

			name = get_value( property_store_value, property_size, property_store_value_offset );
			if ( name != NULL )
			{
				if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Value</div><div id=\"d\">%s</div></div>", name ); }
				if ( silent == false ){ wprintf( L"\n\t\t\t\t\tValue:\t\t\t%s", name ); }
				free( name );
			}

			// Make sure we have 4 bytes for the offset.
			//if ( property_store_value_offset + sizeof( unsigned int ) > property_size ){ return property_store_value_offset; }
			//unsigned int _offset = 0;
			//memcpy_s( &_offset, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) ); // Offset?

			// Make sure we have 23 bytes to offset.
			if ( property_store_value_offset + 0x17 > property_size ){ return property_store_value_offset; }
			property_store_value_offset += 0x17;
		}

		// Make sure we have 1 byte for blah.
		if ( property_store_value_offset + sizeof( unsigned char ) > property_size ){ return property_store_value_offset; }
		//unsigned char blah = 0;
		//memcpy_s( &blah, sizeof( unsigned char ), property_store_value + property_store_value_offset, sizeof( unsigned char ) );
		property_store_value_offset += sizeof( unsigned char );

		// Make sure we have 4 bytes for the count.
		if ( property_store_value_offset + sizeof( unsigned int ) > property_size ){ return property_store_value_offset; }
		//unsigned int count = 0;
		//memcpy_s( &count, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) ); // Count
		property_store_value_offset += sizeof( unsigned int );

		// Make sure we have 4 bytes for some_value.
		if ( property_store_value_offset + sizeof( unsigned int ) > property_size ){ return property_store_value_offset; }
		//unsigned int some_value = 0;
		//memcpy_s( &some_value, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) ); // Flag
		property_store_value_offset += sizeof( unsigned int );

		name = get_value( property_store_value, property_size, property_store_value_offset );
		if ( name != NULL )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Value</div><div id=\"d\">%s</div></div>", name ); }
			if ( silent == false ){ wprintf( L"\n\t\t\t\t\tValue:\t\t\t%s", name ); }
			free( name );
		}
	}
	else if ( content_type == 0x14 )
	{
		// Make sure we have 27 bytes to offset.
		if ( property_store_value_offset + 0x1B > property_size ){ return property_store_value_offset; }
		property_store_value_offset += 0x1B;	// Some values

		wchar_t *name = get_value( property_store_value, property_size, property_store_value_offset );
		if ( name != NULL )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Value</div><div id=\"d\">%s</div></div>", name ); }
			if ( silent == false ){ wprintf( L"\n\t\t\t\t\tValue:\t\t\t%s", name ); }
			free( name );
		}
	}
	/*else if ( content_type != 0 )
	{
		int i = 0;
	}*/

	return property_store_value_offset;
}

unsigned int parse_compound_condition_class( char *property_store_value, unsigned int property_size, unsigned int property_store_value_offset )
{
	if ( property_store_value_offset + 0x10 > property_size ){ return property_store_value_offset; }
	property_store_value_offset += 0x10;

	// See if we have 4 bytes for some_value.
	if ( property_store_value_offset + sizeof( unsigned int ) > property_size ){ return property_store_value_offset; }
	unsigned int some_value = 0;
	memcpy_s( &some_value, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) ); // Flag?
	property_store_value_offset += sizeof( unsigned int );

	if ( some_value == 0x00000001 )
	{
		property_store_value_offset = parse_timestamp( property_store_value, property_size, property_store_value_offset );

		// See if we have 60 bytes for the values below.
		if ( property_store_value_offset + 0x3C > property_size ){ return property_store_value_offset; }
		property_store_value_offset += 0x04;	// some value
		property_store_value_offset += 0x10;	// Second CompoundCondition Class
		property_store_value_offset += 0x04;	// some value
		property_store_value_offset += 0x10;	// Hash 1?
		property_store_value_offset += 0x10;	// Hash 2?
		property_store_value_offset += 0x04;	// some value

		// The rest of this structure should be a wide name length - 1, wide name, wide language code - 1, wide language code, 8 bytes, timestamp, 8 bytes.

		wchar_t *name = get_value( property_store_value, property_size, property_store_value_offset );
		if ( name != NULL )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Value</div><div id=\"d\">%s</div></div>", name ); }
			if ( silent == false ){ wprintf( L"\n\t\t\t\t\tValue:\t\t\t%s", name ); }
			free( name );
		}

		name = get_value( property_store_value, property_size, property_store_value_offset );
		if ( name != NULL )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Language code</div><div id=\"d\">%s</div></div>", name ); }
			if ( silent == false ){ wprintf( L"\n\t\t\t\t\tLanguage code:\t\t%s", name ); }
			free( name );
		}

		// See if we have 8 bytes for the value below.
		if ( property_store_value_offset + 0x08 > property_size ){ return property_store_value_offset; }
		property_store_value_offset += 8;	// zeros timestamp?

		// See if we have 8 bytes for the timestamp.
		if ( property_store_value_offset + 0x08 > property_size ){ return property_store_value_offset; }
		FILETIME ft = { NULL };
		memcpy_s( &ft, sizeof( FILETIME ), property_store_value + property_store_value_offset, sizeof( FILETIME ) );
		SYSTEMTIME st;
		FileTimeToSystemTime( &ft, &st );
		if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Timestamp</div><div id=\"d\">%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]</div></div>", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
		if ( silent == false ){ wprintf( L"\n\t\t\t\t\tTimestamp:\t\t%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
		property_store_value_offset += 8;	// The above timestamp
	}
	/*else
	{
		int i = 0;
	}*/

	return property_store_value_offset;
}

unsigned int parse_leaf_condition_class( char *property_store_value, unsigned int property_size, unsigned int property_store_value_offset )
{
	if ( property_store_value_offset + 0x10 > property_size ){ return property_store_value_offset; }
	property_store_value_offset += 0x10;	// Hash

	// Make sure we have 4 bytes for some_value.
	if ( property_store_value_offset + sizeof( unsigned int ) > property_size ){ return property_store_value_offset; }
	unsigned int some_value = 0;
	memcpy_s( &some_value, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );	// Flag
	property_store_value_offset += sizeof( unsigned int );

	if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />" ); }
	if ( silent == false ){ printf( "\n" ); }

	// I'm guessing these are true or false? Or probably ID types.
	if ( some_value == 0x00000000 )
	{
		property_store_value_offset = parse_system_kind( property_store_value, property_size, property_store_value_offset );
	}
	else if ( some_value == 0x00000001 )
	{
		property_store_value_offset = parse_timestamp( property_store_value, property_size, property_store_value_offset );
	}
	/*else
	{
		int i = 0;
	}*/

	return property_store_value_offset;
}

void parse_metadata( char *metadata_property_store, unsigned int metadata_property_store_length )
{
	char *property_store_value = NULL;
	unsigned int property_store_value_offset = 0;
	unsigned int property_store_size = 0;
	unsigned int ps_offset = 0;
	unsigned int sps_offset = 0;
	while ( true )
	{
		if ( property_store_size + sizeof( unsigned int ) >= metadata_property_store_length ){ return; }
		ps_offset = 0;
		memcpy_s( &ps_offset, sizeof( unsigned int ), metadata_property_store + property_store_size, sizeof( unsigned int ) );

		// The last property store will terminate with an int equal to 0. At that point, property_store_size + 4 == metadata_property_store_length
		if ( ps_offset == 0 || ps_offset > metadata_property_store_length || property_store_size >= ( metadata_property_store_length - sizeof( unsigned int ) ) )
		{
			/*if ( property_store_size != ( metadata_property_store_length - sizeof( unsigned int ) ) )
			{
				int i = 0;
			}*/
			return;
		}

		if ( sps_offset > 0 )
		{
			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />" ); }
			if ( silent == false ){ printf( "\n" ); }
		}

		if ( property_store_size > 0 )
		{
			//if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br />b" ); }
			if ( silent == false ){ printf( "\n" ); }
		}

		if ( property_store_size + sizeof( SerializedPropertyStorage ) >= metadata_property_store_length ){ return; }
		SerializedPropertyStorage sps = { NULL };
		memcpy_s( &sps, sizeof( SerializedPropertyStorage ), metadata_property_store + property_store_size, sizeof( SerializedPropertyStorage ) );

		char guid1[ 64 ] = { 0 };
		buffer_to_guid( sps.FormatID, guid1 );
		if ( silent == false ){ printf( "Property set GUID:\t\t\t%s", guid1 ); }
		if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Property set GUID</div><div id=\"d\">%S</div></div>", guid1 ); }

		sps_offset = 0;	// Reset

		while ( sps_offset + sizeof( unsigned int ) < ( sps.StorageSize - sizeof( SerializedPropertyStorage ) ) )
		{
			// See if we can get a property store value from the property storage structure.
			if ( 0x09 > sps.StorageSize ){ break; }
			property_store_value = metadata_property_store + ( property_store_size + sps_offset + sizeof( SerializedPropertyStorage ) );
			SerializedPropertyValue spv = { NULL };
			memcpy_s( &spv, sizeof( SerializedPropertyValue ), property_store_value, sizeof( SerializedPropertyValue ) );
			property_store_value_offset = 0x09;

			if ( spv.ValueSize == 0x00000000 )
			{
				break;
			}

			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">ID</div><div id=\"d\">" ); }
			if ( silent == false ){ printf( "\nID:\t\t\t\t\t" ); }

			// Contains a name variable before the value.
			if ( memcmp( sps.FormatID, "\x05\xd5\xcd\xd5\x9c\x2e\x1b\x10\x93\x97\x08\x00\x2b\x2c\xf9\xae", 16 ) == 0 )
			{
				if ( property_store_value_offset + spv.NameIDSize > spv.ValueSize ){ break; }
				if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( ( wchar_t * )( property_store_value + property_store_value_offset ) ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"%s", ( tbuf != NULL ? tbuf : ( wchar_t * )( property_store_value + property_store_value_offset ) ) ); free( tbuf ); }
				if ( silent == false ){ wprintf( L"%s", property_store_value + property_store_value_offset ); }

				property_store_value_offset += spv.NameIDSize;
			}
			else
			{
				char *property_name = get_prop_id_type( ( char * )sps.FormatID, spv.NameIDSize );

				if ( property_name == NULL )
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"%lu", spv.NameIDSize ); }
					if ( silent == false ){ printf( "%lu", spv.NameIDSize ); }
				}
				else
				{
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"%S", property_name ); }
					if ( silent == false ){ printf( "%s", property_name ); }
				}
			}

			// http://msdn.microsoft.com/en-us/library/dd942532(v=prot.13).aspx
			// A TypedPropertyValue structure, as specified in [MS-OLEPS] section 2.15.
			if ( property_store_value_offset + ( sizeof( unsigned short ) * 2 ) > spv.ValueSize ){ break; }
			unsigned short property_type = 0;
			memcpy_s( &property_type, sizeof( unsigned short ), property_store_value + property_store_value_offset, sizeof( unsigned short ) );
			unsigned short padding = 0;
			memcpy_s( &padding, sizeof( unsigned short ), property_store_value + property_store_value_offset + sizeof( unsigned short ), sizeof( unsigned short ) );	// Should always be 0.

			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"</div></div><div id=\"r\"><div id=\"h\">Value</div><div id=\"m\">0x%.4x (%s)</div><div id=\"d\">", property_type, get_property_type( property_type ) ); }
			if ( silent == false ){ wprintf( L"\nValue:\t\t\t\t\t0x%.4x (%s)\t", property_type, get_property_type( property_type ) ); }

			/*if ( padding != 0 )
			{
				printf( "None NULL padding detected: 0x%.4x\n", padding );
			}*/

			property_store_value_offset += ( sizeof( unsigned short ) * 2 );

			bool use_vector = ( property_type & VT_VECTOR ? true : false );
			bool use_array = ( property_type & VT_ARRAY ? true : false );	// Haven't seen this yet.

			unsigned int vector_count = 0;
			if ( use_vector == true )
			{
				// VectorHeader = 4 byte unsigned int
				if ( property_store_value_offset + sizeof( unsigned int ) > spv.ValueSize ){ break; }
				vector_count = 0;
				memcpy_s( &vector_count, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );
				property_store_value_offset += ( sizeof( unsigned int ) );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 25 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"Vector count: %lu", vector_count );
				}
				if ( silent == false ){ wprintf( L"Vector count: %lu", vector_count ); }
			}

			if ( property_type == VT_EMPTY )
			{
				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 6 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					// MUST be zero bytes in length.
					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"Empty" );
				}
				if ( silent == false ){ wprintf( L"Empty" ); }
			}
			else if ( property_type == VT_NULL )
			{
				if ( output_html == true )
				{ 
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 5 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					// MUST be zero bytes in length.
					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"NULL" );
				}
				if ( silent == false ){ wprintf( L"NULL" ); }
			}
			else if ( property_type == VT_I2 || // MUST be a 16-bit signed integer, followed by zero padding to 4 bytes.
					  property_type == VT_I4 ||	// MUST be a 32-bit signed integer.
					  property_type == VT_I1 ||	// MUST be a 1-byte signed integer, followed by zero padding to 4 bytes.
					  property_type == VT_INT )	// MUST be a 4-byte signed integer.
			{
				if ( property_store_value_offset + sizeof( int ) > spv.ValueSize ){ break; }
				int i = 0;
				memcpy_s( &i, sizeof( int ), property_store_value + property_store_value_offset, sizeof( int ) );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 24 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"0x%.8x = %d", i, i );
				}
				if ( silent == false ){ wprintf( L"\t0x%.8x = %d", i, i ); }
			}
			else if ( property_type == VT_R4 )
			{
				if ( property_store_value_offset + sizeof( float ) > spv.ValueSize ){ break; }
				// MUST be a 4-byte (single-precision) IEEE floating-point number.
				float i = 0;
				memcpy_s( &i, sizeof( float ), property_store_value + property_store_value_offset, sizeof( float ) );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 24 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"0x%.8x = %f", i, i );
				}
				if ( silent == false ){ wprintf( L"\t0x%.8x = %f", i, i ); }
			}
			else if ( property_type == VT_R8 )
			{
				if ( property_store_value_offset + sizeof( double ) > spv.ValueSize ){ break; }
				// MUST be an 8-byte (double-precision) IEEE floating-point number.
				double i = 0;
				memcpy_s( &i, sizeof( double ), property_store_value + property_store_value_offset, sizeof( double ) );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 42 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"0x%.16llx = %f", i, i );
				}
				if ( silent == false ){ wprintf( L"\t0x%.16llx = %f", i, i ); }
			}
			else if ( property_type == VT_CY )
			{
				if ( property_store_value_offset + sizeof( CURRENCY ) > spv.ValueSize ){ break; }
				// MUST be a CURRENCY (Packet Version).
				CURRENCY i = { NULL };
				memcpy_s( &i, sizeof( CURRENCY ), property_store_value + property_store_value_offset, sizeof( CURRENCY ) );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 42 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"0x%.16llx = %lld", i, i );
				}
				if ( silent == false ){ wprintf( L"\t0x%.16llx = %lld", i, i ); }
			}
			else if ( property_type == VT_DATE )
			{
				if ( property_store_value_offset + sizeof( DATE ) > spv.ValueSize ){ break; }
				// MUST be a DATE (Packet Version).
				DATE i = 0;
				memcpy_s( &i, sizeof( DATE ), property_store_value + property_store_value_offset, sizeof( DATE ) );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 42 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"0x%.16llx = %f", i, i );
				}
				if ( silent == false ){ wprintf( L"0x%.16llx = %f", i, i ); }
			}
			else if ( property_type == VT_BSTR || property_type == VT_LPSTR )
			{
				// MUST be a CodePageString.
				// CodePage size

				// It should be ASCII, but it appears to be UNICODE.

				if ( property_store_value_offset + sizeof( unsigned int ) > spv.ValueSize ){ break; }
				unsigned int name_length = 0;
				memcpy_s( &name_length, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );	// Includes NULL character.
				property_store_value_offset += ( sizeof( unsigned int ) );

				if ( property_store_value_offset + name_length > spv.ValueSize ){ break; }

				if ( silent == false ){ wprintf( L"%s", ( wchar_t * )( property_store_value + property_store_value_offset ) ); }

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure. Accounts for all that could be written during each iteration.
					if ( ( write_offset2 + ( sizeof( wchar_t ) * name_length ) ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					// This is unlikely to occur.
					if ( ( sizeof( wchar_t ) * name_length ) >= BUFFER_CHUNK_SIZE )
					{
						unsigned int copy_amount = name_length / BUFFER_CHUNK_SIZE;
						for ( unsigned int i = 0; i < ( copy_amount * BUFFER_CHUNK_SIZE ); i += BUFFER_CHUNK_SIZE )
						{
							if ( write_offset2 >= BUFFER_CHUNK_SIZE )
							{
								write_html( hFile_html, write_buf2, write_offset2 );
							}

							wmemcpy_s( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, ( wchar_t * )( property_store_value + property_store_value_offset ), BUFFER_CHUNK_SIZE );
							write_offset2 += BUFFER_CHUNK_SIZE;
							property_store_value_offset += ( sizeof( wchar_t ) * BUFFER_CHUNK_SIZE );
						}

						unsigned int copy_remaining = name_length % BUFFER_CHUNK_SIZE;
						if ( copy_remaining > 0 )
						{
							if ( write_offset2 >= BUFFER_CHUNK_SIZE )
							{
								write_html( hFile_html, write_buf2, write_offset2 );
							}

							wmemcpy_s( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, ( wchar_t * )( property_store_value + property_store_value_offset ), copy_remaining );
							write_offset2 += copy_remaining;
							property_store_value_offset += ( sizeof( wchar_t ) * copy_remaining );
						}
						
						--write_offset2;	// Ignore the NULL character.
					}
					else	// The string will fit within our write_buf2 without the need to break it up into chunks.
					{
						// The string might be UTF8?
						wchar_t *tbuf = escape_html_unicode( ( wchar_t * )( property_store_value + property_store_value_offset ) );
						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"%s", ( tbuf != NULL ? tbuf : ( wchar_t * )( property_store_value + property_store_value_offset ) ) );
						free( tbuf );
						property_store_value_offset += ( sizeof( wchar_t ) * name_length );
					}
				}
			}
			else if ( property_type == VT_ERROR )
			{
				if ( property_store_value_offset + sizeof( HRESULT ) > spv.ValueSize ){ break; }
				// MUST be a 32-bit unsigned integer representing an HRESULT, as specified in [MS-DTYP], section 2.2.18.
				HRESULT i = 0;
				memcpy_s( &i, sizeof( HRESULT ), property_store_value + property_store_value_offset, sizeof( HRESULT ) );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 11 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"0x%.8x", i );
				}
				if ( silent == false ){ wprintf( L"0x%.8x", i ); }
			}
			else if ( property_type == VT_BOOL )
			{
				if ( property_store_value_offset + sizeof( VARIANT_BOOL ) > spv.ValueSize ){ break; }
				// MUST be a VARIANT_BOOL as specified in [MS-OAUT], section 2.2.27, followed by zero padding to 4 bytes.
				VARIANT_BOOL vb = 0;
				memcpy_s( &vb, sizeof( VARIANT_BOOL ), property_store_value + property_store_value_offset, sizeof( VARIANT_BOOL ) );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 27 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"0x%.4x = %s", vb, ( vb == VARIANT_TRUE ? L"VARIANT_TRUE" : L"VARIANT_FALSE" ) );
				}
				if ( silent == false ){ wprintf( L"0x%.4x = %s", vb, ( vb == VARIANT_TRUE ? L"VARIANT_TRUE" : L"VARIANT_FALSE" ) ); }
			}
			else if ( property_type == VT_DECIMAL )
			{
				if ( property_store_value_offset + sizeof( DECIMAL ) > spv.ValueSize ){ break; }
				// MUST be a DECIMAL (Packet Version).
				DECIMAL i = { NULL };
				memcpy_s( &i, sizeof( DECIMAL ), property_store_value + property_store_value_offset, sizeof( DECIMAL ) );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 137 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"scale: %d, sign: %d, signscale: %lu, Hi32: %lu, Lo32: %lu, Mid32: %lu, Lo64: %llu, wReserved: %lu", i.scale, i.sign, i.signscale, i.Hi32, i.Lo32, i.Mid32, i.Lo64, i.wReserved );
				}
				if ( silent == false ){ wprintf( L"scale: %d, sign: %d, signscale: %lu, Hi32: %lu, Lo32: %lu, Mid32: %lu, Lo64: %llu, wReserved: %lu", i.scale, i.sign, i.signscale, i.Hi32, i.Lo32, i.Mid32, i.Lo64, i.wReserved ); }
			}
			else if ( property_type == VT_UI1 ||	// MUST be a 1-byte unsigned integer, followed by zero padding to 4 bytes.
					  property_type == VT_UI2 ||	// MUST be a 2-byte unsigned integer, followed by zero padding to 4 bytes.
					  property_type == VT_UI4 ||	// MUST be a 4-byte unsigned integer.
					  property_type == VT_UINT )	// MUST be a 4-byte unsigned integer.
			{
				if ( property_store_value_offset + sizeof( unsigned int ) > spv.ValueSize ){ break; }
				unsigned int i = 0;
				memcpy_s( &i, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 24 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"0x%.8x = %u", i, i );
				}
				if ( silent == false ){ wprintf( L"\t0x%.8x = %u", i, i ); }
			}
			else if ( property_type == VT_I8 )
			{
				if ( property_store_value_offset + sizeof( __int64 ) > spv.ValueSize ){ break; }
				// MUST be an 8-byte signed integer.
				__int64 i = 0;
				memcpy_s( &i, sizeof( __int64 ), property_store_value + property_store_value_offset, sizeof( __int64 ) );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 42 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"0x%.16llx = %lld", i, i );
				}
				if ( silent == false ){ wprintf( L"\t0x%.16llx = %lld", i, i ); }
			}
			else if ( property_type == VT_UI8 )
			{
				if ( property_store_value_offset + sizeof( unsigned __int64 ) > spv.ValueSize ){ break; }
				// MUST be an 8-byte unsigned integer.
				unsigned __int64 i = 0;
				memcpy_s( &i, sizeof( unsigned __int64 ), property_store_value + property_store_value_offset, sizeof( unsigned __int64 ) );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 42 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"0x%.16llx = %llu", i, i );
				}
				if ( silent == false ){ wprintf( L"\t0x%.16llx = %llu", i, i ); }
			}
			else if ( property_type == VT_LPWSTR )
			{
				if ( property_store_value_offset + sizeof( unsigned int ) > spv.ValueSize ){ break; }
				// MUST be a UnicodeString.
				unsigned int name_length = 0;
				memcpy_s( &name_length, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );	// Includes NULL character.
				property_store_value_offset += sizeof( unsigned int );

				if ( property_store_value_offset + ( sizeof( wchar_t ) * name_length ) > spv.ValueSize ){ break; }

				if ( silent == false ){ wprintf( L"%s", ( wchar_t * )( property_store_value + property_store_value_offset ) ); }

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure. Accounts for all that could be written during each iteration.
					if ( ( write_offset2 + ( sizeof( wchar_t ) * name_length ) ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					// This is unlikely to occur.
					if ( ( sizeof( wchar_t ) * name_length ) >= BUFFER_CHUNK_SIZE )
					{
						unsigned int copy_amount = name_length / BUFFER_CHUNK_SIZE;
						for ( unsigned int i = 0; i < ( copy_amount * BUFFER_CHUNK_SIZE ); i += BUFFER_CHUNK_SIZE )
						{
							if ( write_offset2 >= BUFFER_CHUNK_SIZE )
							{
								write_html( hFile_html, write_buf2, write_offset2 );
							}

							wmemcpy_s( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, ( wchar_t * )( property_store_value + property_store_value_offset ), BUFFER_CHUNK_SIZE );
							write_offset2 += BUFFER_CHUNK_SIZE;
							property_store_value_offset += ( sizeof( wchar_t ) * BUFFER_CHUNK_SIZE );
						}

						unsigned int copy_remaining = name_length % BUFFER_CHUNK_SIZE;
						if ( copy_remaining > 0 )
						{
							if ( write_offset2 >= BUFFER_CHUNK_SIZE )
							{
								write_html( hFile_html, write_buf2, write_offset2 );
							}
							
							wmemcpy_s( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, ( wchar_t * )( property_store_value + property_store_value_offset ), copy_remaining );
							write_offset2 += copy_remaining;
							property_store_value_offset += ( sizeof( wchar_t ) * copy_remaining );
						}
						
						--write_offset2;	// Ignore the NULL character.
					}
					else	// The string will fit within our write_buf2 without the need to break it up into chunks.
					{
						// The string might be UTF8?
						wchar_t *tbuf = escape_html_unicode( ( wchar_t * )( property_store_value + property_store_value_offset ) ); 
						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"%s", ( tbuf != NULL ? tbuf : ( wchar_t * )( property_store_value + property_store_value_offset ) ) );
						free( tbuf );
						property_store_value_offset += ( sizeof( wchar_t ) * name_length );
					}
				}
			}
			else if ( property_type == VT_FILETIME )
			{
				if ( property_store_value_offset + sizeof( FILETIME ) > spv.ValueSize ){ break; }
				// MUST be a FILETIME (Packet Version).
				FILETIME ft = { NULL };
				memcpy_s( &ft, sizeof( FILETIME ), property_store_value + property_store_value_offset, sizeof( FILETIME ) );
				SYSTEMTIME st;
				FileTimeToSystemTime( &ft, &st );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 50 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );
				}
				if ( silent == false ){ wprintf( L"%02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
			}
			else if ( property_type == VT_BLOB || property_type == VT_BLOB_OBJECT )
			{
				// MUST be a BLOB.

				if ( output_html == true && dump_remnant == true && spv.ValueSize - property_store_value_offset > 0 )
				{
					write_html_dump( hFile_html, write_buf2, write_offset2, L"BLOB dump", ( unsigned char * )( property_store_value + property_store_value_offset ), spv.ValueSize - property_store_value_offset );
				}

				// The size in bytes of the Bytes field, not including padding (if any).
				//unsigned int size = 0;
				//memcpy_s( &size, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );

				//property_store_value_offset += ( sizeof( unsigned int ) );

				// MUST be an array of bytes, followed by zero padding to a multiple of 4 bytes.
			}
			else if ( property_type == VT_STREAM ||				// MUST be an IndirectPropertyName. The storage representing the (non-simple) property set MUST have a stream element with this name.
					  property_type == VT_STORAGE ||			// MUST be an IndirectPropertyName. The storage representing the (non-simple) property set MUST have a storage element with this name.
					  property_type == VT_STREAMED_OBJECT ||	// MUST be an IndirectPropertyName. The storage representing the (non-simple) property set MUST have a stream element with this name.
					  property_type == VT_STORED_OBJECT )		// MUST be an IndirectPropertyName. The storage representing the (non-simple) property set MUST have a storage element with this name.
			{
				if ( silent == false ){ printf( "\n" ); }

				// See if we have 4 bytes for the prop_name_length
				if ( property_store_value_offset + sizeof( unsigned int ) > spv.ValueSize ){ break; }
				unsigned int prop_name_length = 0;
				memcpy_s( &prop_name_length, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );
				property_store_value_offset += ( sizeof( unsigned int ) );

				// Make sure we have enough to get the string value.
				if ( ( property_store_value_offset + ( prop_name_length * sizeof( wchar_t ) ) ) > spv.ValueSize ){ break; }
				if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( ( wchar_t * )( property_store_value + property_store_value_offset ) ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"IndirectProperty: %s", ( tbuf != NULL ? tbuf : ( wchar_t * )( property_store_value + property_store_value_offset ) ) ); free( tbuf ); }
				if ( silent == false ){ wprintf( L"\t\t\t\t\tIndirectProperty:\t\"%s\"", property_store_value + property_store_value_offset ); }
				property_store_value_offset += prop_name_length;

				if ( memcmp( sps.FormatID, "\x40\xe8\x3e\x1e\x2b\xbc\x6c\x47\x82\x37\x2a\xcd\x1a\x83\x9b\x22", 16 ) == 0 )	// System.Kind : D5CDD505-2E9C-101B-9397-08002B2CF9AE/'PerceivedType' : 1E3EE840-BC2B-476C-8237-2ACD1A839B22/3
				{
					// See if we have 10 bytes for the value below
					if ( property_store_value_offset + 0x0A > spv.ValueSize ){ break; }
					property_store_value_offset += 0x0A;

					// See if we have 4 bytes for the stream_size
					if ( property_store_value_offset + sizeof( unsigned int ) > spv.ValueSize ){ break; }
					unsigned int stream_size = 0;
					memcpy_s( &stream_size, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );

					if ( property_store_value_offset + stream_size + 0x04 > spv.ValueSize ){ break; }
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<br /><u>Property Store</u><br />" ); }
					if ( silent == false ){ printf( "\n[Property Store]\n" ); }
					parse_metadata( property_store_value + property_store_value_offset, stream_size + 0x04 );	// Include the last property storage structure.
					if ( silent == false ){ printf( "\n[/Property Store]" ); }
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<u>End of Property Store</u><br />" ); }

					property_store_value_offset += ( stream_size + 0x04 );
				}
				else	// Stream.
				{
					// See if we have 2 bytes for the value below
					if ( property_store_value_offset + 0x02 > spv.ValueSize ){ break; }
					property_store_value_offset += 0x02;

					// See if we have 4 bytes for the stream_size
					if ( property_store_value_offset + sizeof( unsigned int ) > spv.ValueSize ){ break; }
					//unsigned int stream_size = 0;
					//memcpy_s( &stream_size, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );
					property_store_value_offset += sizeof( unsigned int );

					// See if we have 16 bytes for the hashes below
					if ( property_store_value_offset + 0x10 > spv.ValueSize ){ break; }
					char guid1[ 64 ] = { 0 };
					buffer_to_guid( ( unsigned char * )( property_store_value + property_store_value_offset ), guid1 );
					if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Stream GUID</div><div id=\"d\">%S</div></div>", guid1 ); }
					if ( silent == false ){ printf( "\n\t\t\t\t\tStream GUID:\t\t%s", guid1 ); }

					// See if we have 16 bytes for one of the hashes below.
					if ( property_store_value_offset + 0x10 > spv.ValueSize ){ break; }
					if ( memcmp( ( property_store_value + property_store_value_offset ), "\xae\xa5\x4e\x38\xe1\xad\x8a\x4e\x8a\x9b\x7b\xea\x78\xff\xf1\xe9", 16 ) == 0 )	// BinaryAutoList
					{
						property_store_value_offset += 0x10;	// Hash

						// Make sure we have 18 bytes for the values below.
						if ( property_store_value_offset + 0x12 > spv.ValueSize ){ break; }
						property_store_value_offset += 0x04;	// Some value 0x80000006
						property_store_value_offset += 0x04;	// Some value
						property_store_value_offset += 0x04;	// Some value
						property_store_value_offset += 0x04;	// Some value 0x80000002

						unsigned short num_pidls = 0;
						memcpy_s( &num_pidls, sizeof( unsigned short ), property_store_value + property_store_value_offset, sizeof( unsigned short ) );	// Flag?
						property_store_value_offset += 0x02;

						// I've seen the following values: 0x01, 0x03, 0x04, 0x05, 0x06, 0x08, 0x09, 0x0b, 0x0f, 0x10, 0x12, 0x13, 0x16
						if ( num_pidls > 0 )
						{
							bool _break = false;
							while ( true )
							{
								// Make sure we have 20 bytes for the values below.
								if ( property_store_value_offset + 0x14 > spv.ValueSize ){ _break = true; }
								property_store_value_offset += 0x02;	// Some value
								property_store_value_offset += 0x04;	// Some value
								property_store_value_offset += 0x04;	// Some value
								property_store_value_offset += 0x04;	// Some value
								property_store_value_offset += 0x04;	// Some value

								unsigned short pidl_size = 0;
								memcpy_s( &pidl_size, sizeof( unsigned short ), property_store_value + property_store_value_offset, sizeof( unsigned short ) );

								if ( pidl_size <= 0x00000000 )
								{
									property_store_value_offset -= 0x12;	// Reset our property offset.
									break;
								}

								parse_pidl( property_store_value + property_store_value_offset + 0x02, pidl_size, property_store_value_offset, true, false );
							}
							if ( _break == true ){ break; }
						}
						/*else	// Catch what we don't know.
						{
							if ( output_html == true && dump_remnant == true && spv.ValueSize - property_store_value_offset > 0 )
							{
								write_html_dump( hFile_html, write_buf2, write_offset2, L"Remaining Stream dump", ( unsigned char * )( property_store_value + property_store_value_offset ), spv.ValueSize - property_store_value_offset );
							}

							sps_offset += spv.ValueSize;

							if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"</div></div>" ); }

							continue;
						}*/

						// Make sure we have 38 bytes for the values below.
						if ( property_store_value_offset + 0x26 > spv.ValueSize ){ break; }
						property_store_value_offset += 0x16;	// Some values?
						property_store_value_offset += 0x04;	// Some value 0x00000001
						property_store_value_offset += 0x04;	// Some value 0x80000001
						property_store_value_offset += 0x04;	// Some value 0x00000001

						// Make sure we have 2 bytes for the values below.
						if ( property_store_value_offset + sizeof( unsigned short ) > spv.ValueSize ){ break; }
						unsigned short value_length = 0;
						memcpy_s( &value_length, sizeof( unsigned short ), property_store_value + property_store_value_offset, sizeof( unsigned short ) ); // Does not include NULL character.
						property_store_value_offset += sizeof( unsigned short );

						// Make sure we have enough to get the string value.
						if ( ( property_store_value_offset + ( value_length * sizeof( wchar_t ) ) ) > spv.ValueSize ){ break; }
						wchar_t *value = ( wchar_t * )malloc( sizeof( wchar_t ) * ( value_length + 1 ) );
						wcsncpy_s( value, value_length + 1, ( wchar_t * )( property_store_value + property_store_value_offset ), value_length );
						property_store_value_offset += ( value_length * sizeof( wchar_t ) );
						if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( value ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Value</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : value ) ); free( tbuf ); }
						if ( silent == false ){ wprintf( L"\n\t\t\t\t\tValue:\t\t\t%s", value ); }
						free( value );

						// Make sure we have 4 bytes for some_flag.
						if ( property_store_value_offset + sizeof( unsigned int ) > spv.ValueSize ){ break; }
						unsigned int some_flag = 0;
						memcpy_s( &some_flag, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );
						if ( some_flag == 0x00000000 )
						{
							// Make sure we have 40 bytes for the values below.
							if ( property_store_value_offset + 0x28 > spv.ValueSize ){ break; }
							property_store_value_offset += 0x04;	// Some value
							property_store_value_offset += 0x04;	// Some value
							property_store_value_offset += 0x04;	// Some value
							property_store_value_offset += 0x1c;	// Some values

							// Make sure we have 16 bytes for hash.
							if ( property_store_value_offset + 0x10 > spv.ValueSize ){ break; }
							char clsid[ 64 ] = { 0 };
							buffer_to_guid( ( unsigned char * )( property_store_value + property_store_value_offset ), clsid );
							char *p_type = get_clsid_type( ( property_store_value + property_store_value_offset ) );
							if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">CLSID</div><div id=\"d\">%S = %S</div></div>", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
							if ( silent == false ){ printf( "\n\t\t\t\t\tCLSID:\t\t\t%s = %s", clsid, ( p_type != NULL ? p_type : "Unknown" ) ); }
							property_store_value_offset += 0x10;

							// Make sure we have 20 bytes for the values below.
							if ( property_store_value_offset + 0x14 > spv.ValueSize ){ break; }
							property_store_value_offset += 0x04;	// Fs or 0s
							property_store_value_offset += 0x04;	// Fs or 0s
							property_store_value_offset += 0x04;	// Fs or 0s
							property_store_value_offset += 0x04;	// Fs or 0s
							property_store_value_offset += 0x04;	// 0x00000001
						}
						else if ( some_flag == 0x00000001 )
						{
							// Make sure we have 147 bytes for the value below.
							if ( property_store_value_offset + 0x93 > spv.ValueSize ){ break; }
							property_store_value_offset += 0x93;
						}
						/*else
						{
							int i = 0;
						}*/

						// Make sure we have 2 bytes for the values below.
						if ( property_store_value_offset + sizeof( unsigned short ) > spv.ValueSize ){ break; }
						value_length = 0;
						memcpy_s( &value_length, sizeof( unsigned short ), property_store_value + property_store_value_offset, sizeof( unsigned short ) ); // The actual length is twice this value.
						property_store_value_offset += sizeof( unsigned short );
						
						// Make sure we have enough to get the string value.
						if ( ( property_store_value_offset + ( value_length * sizeof( wchar_t ) ) ) > spv.ValueSize ){ break; }
						value = ( wchar_t * )malloc( sizeof( wchar_t ) * ( value_length + 1 ) );
						wcsncpy_s( value, value_length + 1, ( wchar_t * )( property_store_value + property_store_value_offset ), value_length );
						property_store_value_offset += ( value_length * sizeof( wchar_t ) );
						if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( value ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Search name string</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : value ) ); free( tbuf ); }
						if ( silent == false ){ wprintf( L"\n\t\t\t\t\tSearch name string:\t%s", value ); }
						free( value );

						// Make sure we have 4 bytes for some_value.
						if ( property_store_value_offset + sizeof( unsigned int ) > spv.ValueSize ){ break; }
						some_flag = 0;
						memcpy_s( &some_flag, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );	// Flag?

						if ( some_flag == 0x00000001 )
						{
							property_store_value_offset += 0x04;	// Flag?

							// Make sure we have 2 bytes for the values below.
							if ( property_store_value_offset + sizeof( unsigned short ) > spv.ValueSize ){ break; }
							value_length = 0;
							memcpy_s( &value_length, sizeof( unsigned short ), property_store_value + property_store_value_offset, sizeof( unsigned short ) ); // The actual length is twice this value.
							property_store_value_offset += sizeof( unsigned short );
							
							// Make sure we have enough to get the string value.
							if ( ( property_store_value_offset + ( value_length * sizeof( wchar_t ) ) ) > spv.ValueSize ){ break; }
							value = ( wchar_t * )malloc( sizeof( wchar_t ) * ( value_length + 1 ) );
							wcsncpy_s( value, value_length + 1, ( wchar_t * )( property_store_value + property_store_value_offset ), value_length );
							property_store_value_offset += ( value_length * sizeof( wchar_t ) );
							if ( output_html == true ){ wchar_t *tbuf = escape_html_unicode( value ); write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"<div id=\"r\"><div id=\"h\">Search name string 2</div><div id=\"d\">%s</div></div>", ( tbuf != NULL ? tbuf : value ) ); free( tbuf ); }
							if ( silent == false ){ wprintf( L"\n\t\t\t\t\tSearch name string 2:\t%s", value ); }
							free( value );
						}
					}
					else	// Other GUIDs
					{
						while ( true )
						{
							if ( memcmp( property_store_value + property_store_value_offset, "\x89\x5c\xf1\x52\x17\x5a\xe1\x48\xbb\xcd\x46\xa3\xf8\x9c\x7c\xc2", 16 ) == 0 )		// LeafCondition Class
							{
								property_store_value_offset = parse_leaf_condition_class( property_store_value, spv.ValueSize, property_store_value_offset );
							}
							else if ( memcmp( property_store_value + property_store_value_offset, "\x13\x8d\x6f\x11\x1e\x10\xa5\x4f\x84\xd4\xff\x82\x79\x38\x19\x35", 16 ) == 0 )	// CompoundCondition Class
							{
								property_store_value_offset = parse_compound_condition_class( property_store_value, spv.ValueSize, property_store_value_offset );
							}
							else if ( memcmp( property_store_value + property_store_value_offset, "\x40\xe8\x3e\x1e\x2b\xbc\x6c\x47\x82\x37\x2a\xcd\x1a\x83\x9b\x22", 16 ) == 0 )	// System.Kind
							{
								property_store_value_offset = parse_system_kind( property_store_value, spv.ValueSize, property_store_value_offset );
							}
							else if ( memcmp( property_store_value + property_store_value_offset, "\x2e\x37\xa3\x56\x9c\xce\xd2\x11\x9f\x0e\x00\x60\x97\xc6\x86\xf6", 16 ) == 0 )
							{
								property_store_value_offset = parse_system_kind( property_store_value, spv.ValueSize, property_store_value_offset );
							}
							else if ( memcmp( property_store_value + property_store_value_offset, "\x3c\x0a\xf1\xe4\xe6\x49\x5d\x40\x82\x88\xa2\x3b\xd4\xee\xaa\x6c", 16 ) == 0 )	// System.FileExtension
							{
								property_store_value_offset = parse_system_kind( property_store_value, spv.ValueSize, property_store_value_offset );
							}
							else if ( memcmp( property_store_value + property_store_value_offset, "\x30\xf1\x25\xb7\xef\x47\x1a\x10\xa5\xf1\x02\x60\x8c\x9e\xeb\xac", 16 ) == 0 )	// Property Storage
							{
								property_store_value_offset = parse_system_kind( property_store_value, spv.ValueSize, property_store_value_offset );
							}
							/*else
							{
								int i = 0;
							}*/

							// See if we have 4 bytes for some_value
							if ( property_store_value_offset + sizeof( unsigned int ) > spv.ValueSize ){ break; }
							unsigned int some_value = 0;
							memcpy_s( &some_value, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) ); // Flag?

							property_store_value_offset += sizeof( unsigned int );

							// See if we have 16 bytes for the hashes above
							if ( property_store_value_offset + 0x10 > spv.ValueSize ){ break; }
						}
					}
				}

				// Remnant values in search result streams are probably from shared system memory.

				if ( output_html == true && dump_remnant == true && spv.ValueSize - property_store_value_offset > sizeof ( unsigned int ) )
				{
					write_html_dump( hFile_html, write_buf2, write_offset2, L"Remaining Stream dump", ( unsigned char * )( property_store_value + property_store_value_offset ), spv.ValueSize - property_store_value_offset );
				}
			}
			else if ( property_type == VT_CF )
			{
				// MUST be a ClipboardData.

				if ( output_html == true && dump_remnant == true && spv.ValueSize - property_store_value_offset > 0 )
				{
					write_html_dump( hFile_html, write_buf2, write_offset2, L"Clipboard Data dump", ( unsigned char * )( property_store_value + property_store_value_offset ), spv.ValueSize - property_store_value_offset );
				}

				// The total size in bytes of the Format and Data fields, not including padding (if any).
				//unsigned int size = 0;
				//memcpy_s( &size, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );

				// An application-specific identifier for the format of the data in the Data field.
				//unsigned int format = 0;
				//memcpy_s( &format, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );

				//property_store_value_offset += ( 2 * sizeof( unsigned int ) );

				// MUST be an array of bytes, followed by zero padding to a multiple of 4 bytes.
			}
			else if ( property_type == VT_CLSID )
			{
				if ( property_store_value_offset + 0x10 > spv.ValueSize ){ break; }
				// MUST be a GUID (Packet Version).
				char guid1[ 64 ] = { 0 };
				buffer_to_guid( ( unsigned char * )( property_store_value + property_store_value_offset ), guid1 );

				if ( output_html == true )
				{
					// Prevent overflow if this is a large structure.
					if ( ( write_offset2 + 44 ) >= BUFFER_CHUNK_SIZE )
					{
						write_html( hFile_html, write_buf2, write_offset2 );
					}

					write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"CLSID: %S", guid1 );
				}
				if ( silent == false ){ printf( "CLSID: %s", guid1 ); }
			}
			else if ( property_type == VT_VERSIONED_STREAM )
			{
				// MUST be a VersionedStream. The storage representing the (non-simple) property set MUST have a stream element with the name in the StreamName field.

				if ( output_html == true && dump_remnant == true && spv.ValueSize - property_store_value_offset > 0 )
				{
					write_html_dump( hFile_html, write_buf2, write_offset2, L"Versioned Stream dump", ( unsigned char * )( property_store_value + property_store_value_offset ), spv.ValueSize - property_store_value_offset );
				}
				
				//char guid1[ 64 ] = { 0 };
				//buffer_to_guid( ( unsigned char * )( property_store_value + property_store_value_offset ), guid1 );
				//if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"VersionGUID: %S", guid1 ); }
				//printf( "VersionGUID: %s", guid1 );

				//property_store_value_offset += 16;
			}
			else if ( property_type == ( VT_VECTOR | VT_I2 ) ||	// MUST be a 16-bit signed integer, followed by zero padding to 4 bytes.
					  property_type == ( VT_VECTOR | VT_I4 ) ||	// MUST be a 32-bit signed integer.
					  property_type == ( VT_VECTOR | VT_I1 ) )	// MUST be a 1-byte signed integer, followed by zero padding to 4 bytes.
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( property_store_value_offset + sizeof( int ) > spv.ValueSize ){ break; }
					int i = 0;
					memcpy_s( &i, sizeof( int ), property_store_value + property_store_value_offset, sizeof( int ) );
					property_store_value_offset += ( sizeof( int ) );

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure.
						if ( ( write_offset2 + 27 ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : 0x%.8x = %d", i, i );
					}
					if ( silent == false ){ wprintf( L" : 0x%.8x = %d", i, i ); }
				}
			}
			else if ( property_type == ( VT_VECTOR | VT_R4 ) )
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( property_store_value_offset + sizeof( float ) > spv.ValueSize ){ break; }
					// MUST be a 4-byte (single-precision) IEEE floating-point number.
					float i = 0;
					memcpy_s( &i, sizeof( float ), property_store_value + property_store_value_offset, sizeof( float ) );
					property_store_value_offset += ( sizeof( float ) );

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure.
						if ( ( write_offset2 + 27 ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : 0x%.8x = %f", i, i );
					}
					if ( silent == false ){ wprintf( L" : 0x%.8x = %f", i, i ); }
				}
			}
			else if ( property_type == ( VT_VECTOR | VT_R8 ) )
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( property_store_value_offset + sizeof( double ) > spv.ValueSize ){ break; }
					// MUST be an 8-byte (double-precision) IEEE floating-point number.
					double i = 0;
					memcpy_s( &i, sizeof( double ), property_store_value + property_store_value_offset, sizeof( double ) );
					property_store_value_offset += ( sizeof( double ) );

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure.
						if ( ( write_offset2 + 45 ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : 0x%.16llx = %f", i, i );
					}
					if ( silent == false ){ wprintf( L" : 0x%.16llx = %f", i, i ); }
				}
			}
			else if ( property_type == ( VT_VECTOR | VT_CY ) )
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( property_store_value_offset + sizeof( CURRENCY ) > spv.ValueSize ){ break; }
					// MUST be a CURRENCY (Packet Version).
					CURRENCY i = { NULL };
					memcpy_s( &i, sizeof( CURRENCY ), property_store_value + property_store_value_offset, sizeof( CURRENCY ) );
					property_store_value_offset += ( sizeof( CURRENCY ) );

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure.
						if ( ( write_offset2 + 45 ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : 0x%.16llx = %lld", i, i );
					}
					if ( silent == false ){ wprintf( L" : 0x%.16llx = %lld", i, i ); }
				}
			}
			else if ( property_type == ( VT_VECTOR | VT_DATE ) )
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( property_store_value_offset + sizeof( DATE ) > spv.ValueSize ){ break; }
					// MUST be a DATE (Packet Version).
					DATE i = 0;
					memcpy_s( &i, sizeof( DATE ), property_store_value + property_store_value_offset, sizeof( DATE ) );
					property_store_value_offset += ( sizeof( DATE ) );

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure.
						if ( ( write_offset2 + 45 ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : 0x%.16llx = %f", i, i );
					}
					if ( silent == false ){ wprintf( L" : 0x%.16llx = %f", i, i ); }
				}
			}
			else if ( property_type == ( VT_VECTOR | VT_BSTR ) || property_type == ( VT_VECTOR | VT_LPSTR ) )
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					// MUST be a CodePageString.

					if ( property_store_value_offset + sizeof( unsigned int ) > spv.ValueSize ){ break; }
					// CodePage size
					unsigned int name_length = 0;
					memcpy_s( &name_length, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );
					property_store_value_offset += sizeof( unsigned int );

					// Bail if the name is too long.
					if ( ( sizeof( wchar_t ) * name_length ) > spv.ValueSize - property_store_value_offset )
					{
						if ( output_html == true && dump_remnant == true )
						{
							write_html_dump( hFile_html, write_buf2, write_offset2, L"Code Page String (vector) dump", ( unsigned char * )( property_store_value + property_store_value_offset ), spv.ValueSize - property_store_value_offset );
						}
						break;
					}

					if ( silent == false ){ wprintf( L" : %s", ( wchar_t * )( property_store_value + property_store_value_offset ) ); }

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure. Accounts for all that could be written during each iteration.
						if ( ( write_offset2 + ( sizeof( wchar_t ) * ( name_length + 4 ) ) ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						// This is unlikely to occur.
						if ( ( sizeof( wchar_t ) * ( name_length + 4 ) ) >= BUFFER_CHUNK_SIZE )
						{
							write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : " );
							write_html( hFile_html, write_buf2, write_offset2 );

							unsigned int copy_amount = name_length / BUFFER_CHUNK_SIZE;
							for ( unsigned int i = 0; i < ( copy_amount * BUFFER_CHUNK_SIZE ); i += BUFFER_CHUNK_SIZE )
							{
								if ( write_offset2 >= BUFFER_CHUNK_SIZE )
								{
									write_html( hFile_html, write_buf2, write_offset2 );
								}

								wmemcpy_s( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, ( wchar_t * )( property_store_value + property_store_value_offset ), BUFFER_CHUNK_SIZE );
								write_offset2 += BUFFER_CHUNK_SIZE;
								property_store_value_offset += ( sizeof( wchar_t ) * BUFFER_CHUNK_SIZE );
							}

							unsigned int copy_remaining = name_length % BUFFER_CHUNK_SIZE;
							if ( copy_remaining > 0 )
							{
								if ( write_offset2 >= BUFFER_CHUNK_SIZE )
								{
									write_html( hFile_html, write_buf2, write_offset2 );
								}

								wmemcpy_s( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, ( wchar_t * )( property_store_value + property_store_value_offset ), copy_remaining );
								write_offset2 += copy_remaining;
								property_store_value_offset += ( sizeof( wchar_t ) * copy_remaining );
							}
							
							--write_offset2;	// Ignore the NULL character.
						}
						else	// The string will fit within our write_buf2 without the need to break it up into chunks.
						{
							// The string might be UTF8?
							wchar_t *tbuf = escape_html_unicode( ( wchar_t * )( property_store_value + property_store_value_offset ) );
							write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : %s", ( tbuf != NULL ? tbuf : ( wchar_t * )( property_store_value + property_store_value_offset ) ) );
							free( tbuf );
							property_store_value_offset += ( sizeof( wchar_t ) * name_length );
						}
					}
					else	// We still need to update the offset.
					{
						property_store_value_offset += ( sizeof( wchar_t ) * name_length );
					}

					// The total length of the string must be a multiple of 4 byte and additional padding will be added. Offset if there is any padding.
					if ( ( ( sizeof( wchar_t ) * name_length ) % 4 ) != 0 )
					{
						if ( property_store_value_offset + sizeof( wchar_t ) > spv.ValueSize ){ break; }
						property_store_value_offset += sizeof( wchar_t );
					}
				}
			}
			else if ( property_type == ( VT_VECTOR | VT_ERROR ) )
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( property_store_value_offset + sizeof( HRESULT ) > spv.ValueSize ){ break; }
					// MUST be a 32-bit unsigned integer representing an HRESULT, as specified in [MS-DTYP], section 2.2.18.
					HRESULT i = 0;
					memcpy_s( &i, sizeof( HRESULT ), property_store_value + property_store_value_offset, sizeof( HRESULT ) );
					property_store_value_offset += ( sizeof( HRESULT ) );

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure.
						if ( ( write_offset2 + 14 ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : 0x%.8x", i );
					}
					if ( silent == false ){ wprintf( L" : 0x%.8x", i ); }
				}
			}
			else if ( property_type == ( VT_VECTOR | VT_BOOL ) )
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( property_store_value_offset + sizeof( VARIANT_BOOL ) > spv.ValueSize ){ break; }
					// MUST be a VARIANT_BOOL as specified in [MS-OAUT], section 2.2.27, followed by zero padding to 4 bytes.
					VARIANT_BOOL vb = 0;
					memcpy_s( &vb, sizeof( VARIANT_BOOL ), property_store_value + property_store_value_offset, sizeof( VARIANT_BOOL ) );
					property_store_value_offset += ( sizeof( VARIANT_BOOL ) );

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure.
						if ( ( write_offset2 + 30 ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : 0x%.4x = %s", vb, ( vb == VARIANT_TRUE ? L"VARIANT_TRUE" : L"VARIANT_FALSE" ) );
					}
					if ( silent == false ){ wprintf( L" : 0x%.4x = %s", vb, ( vb == VARIANT_TRUE ? L"VARIANT_TRUE" : L"VARIANT_FALSE" ) ); }
				}
			}
			else if ( property_type == ( VT_VECTOR | VT_VARIANT ) )
			{
				if ( output_html == true && dump_remnant == true && spv.ValueSize - property_store_value_offset > 0 )
				{
					write_html_dump( hFile_html, write_buf2, write_offset2, L"Variant (vector) dump", ( unsigned char * )( property_store_value + property_store_value_offset ), spv.ValueSize - property_store_value_offset );
				}
				//for ( unsigned int j = 0; j < vector_count; j++ )
				//{

				//}
			}
			else if ( property_type == ( VT_VECTOR | VT_UI1 ) ||	// MUST be a 1-byte unsigned integer, followed by zero padding to 4 bytes.
					  property_type == ( VT_VECTOR | VT_UI2 ) ||	// MUST be a 2-byte unsigned integer, followed by zero padding to 4 bytes.
					  property_type == ( VT_VECTOR | VT_UI4 ) )		// MUST be a 4-byte unsigned integer.
			{
				// This storage stucture seems to hold a pidl rather than a bunch of integers.
				parse_pidl( property_store_value + property_store_value_offset, spv.ValueSize - property_store_value_offset, property_store_value_offset, true, false );


				/*for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( j % 8 == 0 )
					{
						printf( "\n\t" );
					}
					unsigned int i = 0;
					memcpy_s( &i, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );

					property_store_value_offset += ( sizeof( unsigned int ) );

					wprintf( L" : 0x%.8x = %u", i, i );
				}*/
			}
			else if ( property_type == ( VT_VECTOR | VT_I8 ) )
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( property_store_value_offset + sizeof( __int64 ) > spv.ValueSize ){ break; }
					// MUST be an 8-byte signed integer.
					__int64 i = 0;
					memcpy_s( &i, sizeof( __int64 ), property_store_value + property_store_value_offset, sizeof( __int64 ) );
					property_store_value_offset += ( sizeof( __int64 ) );

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure.
						if ( ( write_offset2 + 45 ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : 0x%.16llx = %lld", i, i );
					}
					if ( silent == false ){ wprintf( L" : 0x%.16llx = %lld", i, i ); }
				}
			}
			else if ( property_type == ( VT_VECTOR | VT_UI8 ) )
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( property_store_value_offset + sizeof( unsigned __int64 ) > spv.ValueSize ){ break; }
					// MUST be an 8-byte unsigned integer.
					unsigned __int64 i = 0;
					memcpy_s( &i, sizeof( unsigned __int64 ), property_store_value + property_store_value_offset, sizeof( unsigned __int64 ) );
					property_store_value_offset += ( sizeof( unsigned __int64 ) );

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure.
						if ( ( write_offset2 + 45 ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : 0x%.16llx = %llu", i, i );
					}
					if ( silent == false ){ wprintf( L" : 0x%.16llx = %llu", i, i ); }
				}
			}
			else if ( property_type == ( VT_VECTOR | VT_LPWSTR ) )
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( property_store_value_offset + sizeof( unsigned int ) > spv.ValueSize ){ break; }
					unsigned int name_length = 0;
					memcpy_s( &name_length, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );	// Includes the NULL character.
					property_store_value_offset += ( sizeof( unsigned int ) );

					// Bail if the name is too long.
					if ( ( sizeof( wchar_t ) * name_length ) > spv.ValueSize - property_store_value_offset )
					{
						if ( output_html == true && dump_remnant == true )
						{
							write_html_dump( hFile_html, write_buf2, write_offset2, L"UNICODE String (vector) dump", ( unsigned char * )( property_store_value + property_store_value_offset ), spv.ValueSize - property_store_value_offset );
						}
						break;
					}

					if ( silent == false ){ wprintf( L" : %s", ( wchar_t * )( property_store_value + property_store_value_offset ) ); }

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure. Accounts for all that could be written during each iteration.
						if ( ( write_offset2 + ( sizeof( wchar_t ) * ( name_length + 4 ) ) ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						// This is unlikely to occur.
						if ( ( sizeof( wchar_t ) * ( name_length + 4 ) ) >= BUFFER_CHUNK_SIZE )
						{
							write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : " );
							write_html( hFile_html, write_buf2, write_offset2 );

							unsigned int copy_amount = name_length / BUFFER_CHUNK_SIZE;
							for ( unsigned int i = 0; i < ( copy_amount * BUFFER_CHUNK_SIZE ); i += BUFFER_CHUNK_SIZE )
							{
								if ( write_offset2 >= BUFFER_CHUNK_SIZE )
								{
									write_html( hFile_html, write_buf2, write_offset2 );
								}

								wmemcpy_s( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, ( wchar_t * )( property_store_value + property_store_value_offset ), BUFFER_CHUNK_SIZE );
								write_offset2 += BUFFER_CHUNK_SIZE;
								property_store_value_offset += ( sizeof( wchar_t ) * BUFFER_CHUNK_SIZE );
							}

							unsigned int copy_remaining = name_length % BUFFER_CHUNK_SIZE;
							if ( copy_remaining > 0 )
							{
								if ( write_offset2 >= BUFFER_CHUNK_SIZE )
								{
									write_html( hFile_html, write_buf2, write_offset2 );
								}

								wmemcpy_s( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, ( wchar_t * )( property_store_value + property_store_value_offset ), copy_remaining );
								write_offset2 += copy_remaining;
								property_store_value_offset += ( sizeof( wchar_t ) * copy_remaining );
							}
							
							--write_offset2;	// Ignore the NULL character.
						}
						else	// The string will fit within our write_buf2 without the need to break it up into chunks.
						{
							// The string might be UTF8?
							wchar_t *tbuf = escape_html_unicode( ( wchar_t * )( property_store_value + property_store_value_offset ) );
							write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : %s", ( tbuf != NULL ? tbuf : ( wchar_t * )( property_store_value + property_store_value_offset ) ) );
							free( tbuf );
							property_store_value_offset += ( sizeof( wchar_t ) * name_length );
						}
					}
					else	// We still need to update the offset.
					{
						property_store_value_offset += ( sizeof( wchar_t ) * name_length );
					}

					// The total length of the string must be a multiple of 4 byte and additional padding will be added. Offset if there is any padding.
					if ( ( ( sizeof( wchar_t ) * name_length ) % 4 ) != 0 )
					{
						if ( property_store_value_offset + sizeof( wchar_t ) > spv.ValueSize ){ break; }
						property_store_value_offset += sizeof( wchar_t );
					}
				}
			}
			else if ( property_type == ( VT_VECTOR | VT_FILETIME ) )
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( property_store_value_offset + sizeof( FILETIME ) > spv.ValueSize ){ break; }
					// MUST be a FILETIME (Packet Version).
					FILETIME ft = { NULL };
					memcpy_s( &ft, sizeof( FILETIME ), property_store_value + property_store_value_offset, sizeof( FILETIME ) );
					SYSTEMTIME st;
					FileTimeToSystemTime( &ft, &st );
					property_store_value_offset += ( sizeof( FILETIME ) );

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure. Accounts for all that could be written during each iteration.
						if ( ( write_offset2 + 53 ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}
						
						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );
					}
					if ( silent == false ){ wprintf( L" : %02d/%02d/%d (%02d:%02d:%02d.%d) [UTC]", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ); }
				}
			}
			else if ( property_type == ( VT_VECTOR | VT_CF ) )
			{
				if ( output_html == true && dump_remnant == true && spv.ValueSize - property_store_value_offset > 0 )
				{
					write_html_dump( hFile_html, write_buf2, write_offset2, L"Clipboard Data (vector) dump", ( unsigned char * )( property_store_value + property_store_value_offset ), spv.ValueSize - property_store_value_offset );
				}

				//for ( unsigned int j = 0; j < vector_count; j++ )
				//{
					// MUST be a ClipboardData.

					// The total size in bytes of the Format and Data fields, not including padding (if any).
					//unsigned int size = 0;
					//memcpy_s( &size, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );

					// An application-specific identifier for the format of the data in the Data field.
					//unsigned int format = 0;
					//memcpy_s( &format, sizeof( unsigned int ), property_store_value + property_store_value_offset, sizeof( unsigned int ) );

					//property_store_value_offset += ( 2 * sizeof( unsigned int ) );

					// MUST be an array of bytes, followed by zero padding to a multiple of 4 bytes.

					//property_store_value_offset += ( Size - sizeof( unsigned int ) );
				//}
			}
			else if ( property_type == ( VT_VECTOR | VT_CLSID ) )
			{
				for ( unsigned int j = 0; j < vector_count; j++ )
				{
					if ( property_store_value_offset + 0x10 > spv.ValueSize ){ break; }
					// MUST be a GUID (Packet Version).
					char guid1[ 64 ] = { 0 };
					buffer_to_guid( ( unsigned char * )( property_store_value + property_store_value_offset ), guid1 );
					property_store_value_offset += 16;

					if ( output_html == true )
					{
						// Prevent overflow if this is a large structure. Accounts for all that could be written during each iteration.
						if ( ( write_offset2 + 40 ) >= BUFFER_CHUNK_SIZE )
						{
							write_html( hFile_html, write_buf2, write_offset2 );
						}

						write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L" : %s", guid1 );
					}
					if ( silent == false ){ printf( " : %s", guid1 ); }
				}
			}

			sps_offset += spv.ValueSize;

			if ( output_html == true ){ write_offset2 += swprintf( write_buf2 + write_offset2, BUFFER_SIZE - write_offset2, L"</div></div>" ); }
		}

		property_store_size += ps_offset;	// Move to the next SerializedPropertyStorage struct.
	}
}
