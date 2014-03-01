#include "globals.h"

wchar_t *get_extension_from_filename( wchar_t *filename )
{
	if ( filename == NULL )
	{
		return NULL;
	}

	unsigned long length = wcslen( filename );
	while ( length != 0 && filename[ --length ] != L'.' );

	return filename + length;
}

void traverse_directory( wchar_t *path )
{
	//int total_directories = 0;
	//int total_files = 0;

	wchar_t filepath[ MAX_PATH ];
	swprintf_s( filepath, MAX_PATH, L"%s\\*", path );

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFileEx( ( LPCWSTR )filepath, FindExInfoStandard, &FindFileData, FindExSearchNameMatch, NULL, 0 );
	if ( hFind != INVALID_HANDLE_VALUE ) 
	{
		do
		{
			// See if the file is a directory.
			if ( ( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
			{
				// Add all directories except "." and ".." (current and parent)
				if ( ( wcscmp( FindFileData.cFileName, L"." ) != 0 ) && ( wcscmp( FindFileData.cFileName, L".." ) != 0 ) )
				{
					// Create new tree node.
					wchar_t new_path[ MAX_PATH ];
					swprintf_s( new_path, MAX_PATH, L"%s\\%s", path, FindFileData.cFileName );
					//wprintf( L"%s\n", new_path );

					traverse_directory( new_path );

					//total_directories++;
				}
			}
			else
			{
				// REMOVE THE .BIN WHEN FINISHED
				if ( /*_wcsicmp( get_extension_from_filename( FindFileData.cFileName ), L".bin" ) != 0 &&/**/_wcsicmp( get_extension_from_filename( FindFileData.cFileName ), L".lnk" ) != 0  /*&& FindFileData.nFileSizeLow > 18240*/ )
				{
					continue;
				}

				wchar_t full_path[ MAX_PATH ];
				swprintf_s( full_path, MAX_PATH, L"%s\\%s", path, FindFileData.cFileName );
				//wprintf( L"%s\n", full_path );

				parse_shortcut( full_path );

				//total_files++;
			}

		} while ( FindNextFile( hFind, &FindFileData ) != 0 );

		FindClose( hFind );
	}
}

wchar_t *get_showwindow_value( unsigned int sw_value )
{
	switch ( sw_value )
	{
		case SW_HIDE:				{ return L"SW_HIDE"; } break;
		//case SW_SHOWNORMAL:
		case SW_NORMAL:				{ return L"SW_SHOWNORMAL / SW_NORMAL"; } break;
		case SW_SHOWMINIMIZED:		{ return L"SW_SHOWMINIMIZED"; } break;
		//case SW_SHOWMAXIMIZED:
		case SW_MAXIMIZE:			{ return L"SW_SHOWMAXIMIZED / SW_MAXIMIZE"; } break;
		case SW_SHOWNOACTIVATE:		{ return L"SW_SHOWNOACTIVATE"; } break;
		case SW_SHOW:				{ return L"SW_SHOW"; } break;
		case SW_MINIMIZE:			{ return L"SW_MINIMIZE"; } break;
		case SW_SHOWMINNOACTIVE:	{ return L"SW_SHOWMINNOACTIVE"; } break;
		case SW_SHOWNA:				{ return L"SW_SHOWNA"; } break;
		case SW_RESTORE:			{ return L"SW_RESTORE"; } break;
		case SW_SHOWDEFAULT:		{ return L"SW_SHOWDEFAULT"; } break;
		//case SW_FORCEMINIMIZE:
		case SW_MAX:				{ return L"SW_FORCEMINIMIZE / SW_MAX"; } break;
		default:
			return L"Unknown";
			break;
	}

	return NULL;
}

wchar_t *get_hot_key_value( unsigned short hk_value )
{
	unsigned char low = LOBYTE( hk_value );
	unsigned char high = HIBYTE( hk_value );

	static wchar_t buf[ 128 ];

	// See if the low value is between 0 and 9, or A and Z.
	if ( ( low >= 0x30 && low <= 0x39 ) || ( low >= 0x41 && low <= 0x5A ) )
	{
		swprintf_s( buf, 128, L"%s%s%s%c", ( ( high & HOTKEYF_CONTROL ) != FALSE ? L"Ctrl + " : L"" ), ( ( high & HOTKEYF_SHIFT ) != FALSE ? L"Shift + " : L"" ), ( ( high & HOTKEYF_ALT ) != FALSE ? L"Alt + " : L"" ), low );
	}
	else
	{
		wchar_t *vk_key = NULL;
		switch ( low )
		{
			// Common/acceptable key codes.
			case 0:							{ vk_key = L"None"; } break;
			case VK_CAPITAL:				{ vk_key = L"Caps Lock"; } break;
			case VK_PRIOR:					{ vk_key = L"Page Up"; } break;
			case VK_NEXT:					{ vk_key = L"Page Down"; } break;
			case VK_END:					{ vk_key = L"End"; } break;
			case VK_HOME:					{ vk_key = L"Home"; } break;
			case VK_LEFT:					{ vk_key = L"Left"; } break;
			case VK_UP:						{ vk_key = L"Up"; } break;
			case VK_RIGHT:					{ vk_key = L"Right"; } break;
			case VK_DOWN:					{ vk_key = L"Down"; } break;
			case VK_INSERT:					{ vk_key = L"Insert"; } break;
			case VK_NUMPAD0:				{ vk_key = L"Num 0"; } break;
			case VK_NUMPAD1:				{ vk_key = L"Num 1"; } break;
			case VK_NUMPAD2:				{ vk_key = L"Num 2"; } break;
			case VK_NUMPAD3:				{ vk_key = L"Num 3"; } break;
			case VK_NUMPAD4:				{ vk_key = L"Num 4"; } break;
			case VK_NUMPAD5:				{ vk_key = L"Num 5"; } break;
			case VK_NUMPAD6:				{ vk_key = L"Num 6"; } break;
			case VK_NUMPAD7:				{ vk_key = L"Num 7"; } break;
			case VK_NUMPAD8:				{ vk_key = L"Num 8"; } break;
			case VK_NUMPAD9:				{ vk_key = L"Num 9"; } break;
			case VK_MULTIPLY:				{ vk_key = L"Num *"; } break;
			case VK_ADD:					{ vk_key = L"Num +"; } break;
			case VK_SUBTRACT:				{ vk_key = L"Num -"; } break;
			case VK_DIVIDE:					{ vk_key = L"Num /"; } break;
			case VK_F1:						{ vk_key = L"F1"; } break;
			case VK_F2:						{ vk_key = L"F2"; } break;
			case VK_F3:						{ vk_key = L"F3"; } break;
			case VK_F4:						{ vk_key = L"F4"; } break;
			case VK_F5:						{ vk_key = L"F5"; } break;
			case VK_F6:						{ vk_key = L"F6"; } break;
			case VK_F7:						{ vk_key = L"F7"; } break;
			case VK_F8:						{ vk_key = L"F8"; } break;
			case VK_F9:						{ vk_key = L"F9"; } break;
			case VK_F10:					{ vk_key = L"F10"; } break;
			case VK_F11:					{ vk_key = L"F11"; } break;
			case VK_F12:					{ vk_key = L"F12"; } break;
			case VK_F13:					{ vk_key = L"F13"; } break;
			case VK_F14:					{ vk_key = L"F14"; } break;
			case VK_F15:					{ vk_key = L"F15"; } break;
			case VK_F16:					{ vk_key = L"F16"; } break;
			case VK_F17:					{ vk_key = L"F17"; } break;
			case VK_F18:					{ vk_key = L"F18"; } break;
			case VK_F19:					{ vk_key = L"F19"; } break;
			case VK_F20:					{ vk_key = L"F20"; } break;
			case VK_F21:					{ vk_key = L"F21"; } break;
			case VK_F22:					{ vk_key = L"F22"; } break;
			case VK_F23:					{ vk_key = L"F23"; } break;
			case VK_F24:					{ vk_key = L"F24"; } break;
			case VK_NUMLOCK:				{ vk_key = L"Num Lock"; } break;
			case VK_SCROLL:					{ vk_key = L"Scroll Lock"; } break;
			case VK_OEM_1:					{ vk_key = L";"; } break;
			case VK_OEM_PLUS:				{ vk_key = L"="; } break;
			case VK_OEM_COMMA:				{ vk_key = L","; } break;
			case VK_OEM_MINUS:				{ vk_key = L"-"; } break;
			case VK_OEM_PERIOD:				{ vk_key = L"."; } break;
			case VK_OEM_2:					{ vk_key = L"/"; } break;
			case VK_OEM_3:					{ vk_key = L"`"; } break;
			case VK_OEM_4:					{ vk_key = L"["; } break;
			case VK_OEM_5:					{ vk_key = L"\\"; } break;
			case VK_OEM_6:					{ vk_key = L"]"; } break;
			case VK_OEM_7:					{ vk_key = L"\'"; } break;

			// Uncommon/unacceptable key codes.
			case VK_LBUTTON:				{ vk_key = L"VK_LBUTTON"; } break;
			case VK_RBUTTON:				{ vk_key = L"VK_RBUTTON"; } break;
			case VK_CANCEL:					{ vk_key = L"VK_CANCEL"; } break;
			case VK_MBUTTON:				{ vk_key = L"VK_MBUTTON"; } break;
			case VK_XBUTTON1:				{ vk_key = L"VK_XBUTTON1"; } break;
			case VK_XBUTTON2:				{ vk_key = L"VK_XBUTTON2"; } break;
			case VK_BACK:					{ vk_key = L"VK_BACK"; } break;
			case VK_TAB:					{ vk_key = L"VK_TAB"; } break;
			case VK_CLEAR:					{ vk_key = L"VK_CLEAR"; } break;
			case VK_RETURN:					{ vk_key = L"VK_RETURN"; } break;
			case VK_SHIFT:					{ vk_key = L"VK_SHIFT"; } break;
			case VK_CONTROL:				{ vk_key = L"VK_CONTROL"; } break;
			case VK_MENU:					{ vk_key = L"VK_MENU"; } break;
			case VK_PAUSE:					{ vk_key = L"VK_PAUSE"; } break;
			//case VK_KANA:
			//case VK_HANGEUL:
			case VK_HANGUL:					{ vk_key = L"VK_KANA / VK_HANGEUL / VK_HANGUL"; } break;
			case VK_JUNJA:					{ vk_key = L"VK_JUNJA"; } break;
			case VK_FINAL:					{ vk_key = L"VK_FINAL"; } break;
			//case VK_HANJA:
			case VK_KANJI:					{ vk_key = L"VK_HANJA / VK_KANJI"; } break;
			case VK_ESCAPE:					{ vk_key = L"VK_ESCAPE"; } break;
			case VK_CONVERT:				{ vk_key = L"VK_CONVERT"; } break;
			case VK_NONCONVERT:				{ vk_key = L"VK_NONCONVERT"; } break;
			case VK_ACCEPT:					{ vk_key = L"VK_ACCEPT"; } break;
			case VK_MODECHANGE:				{ vk_key = L"VK_MODECHANGE"; } break;
			case VK_SPACE:					{ vk_key = L"VK_SPACE"; } break;
			case VK_SELECT:					{ vk_key = L"VK_SELECT"; } break;
			case VK_PRINT:					{ vk_key = L"VK_PRINT"; } break;
			case VK_EXECUTE:				{ vk_key = L"VK_EXECUTE"; } break;
			case VK_SNAPSHOT:				{ vk_key = L"VK_SNAPSHOT"; } break;
			case VK_DELETE:					{ vk_key = L"VK_DELETE"; } break;
			case VK_HELP:					{ vk_key = L"VK_HELP"; } break;
			case VK_LWIN:					{ vk_key = L"VK_LWIN"; } break;
			case VK_RWIN:					{ vk_key = L"VK_RWIN"; } break;
			case VK_APPS:					{ vk_key = L"VK_APPS"; } break;
			case VK_SLEEP:					{ vk_key = L"VK_SLEEP"; } break;
			case VK_SEPARATOR:				{ vk_key = L"VK_SEPARATOR"; } break;
			case VK_DECIMAL:				{ vk_key = L"VK_DECIMAL"; } break;
			//case VK_OEM_NEC_EQUAL:
			case VK_OEM_FJ_JISHO:			{ vk_key = L"VK_OEM_NEC_EQUAL / VK_OEM_FJ_JISHO"; } break;
			case VK_OEM_FJ_MASSHOU:			{ vk_key = L"VK_OEM_FJ_MASSHOU"; } break;
			case VK_OEM_FJ_TOUROKU:			{ vk_key = L"VK_OEM_FJ_TOUROKU"; } break;
			case VK_OEM_FJ_LOYA:			{ vk_key = L"VK_OEM_FJ_LOYA"; } break;
			case VK_OEM_FJ_ROYA:			{ vk_key = L"VK_OEM_FJ_ROYA"; } break;
			case VK_LSHIFT:					{ vk_key = L"VK_LSHIFT"; } break;
			case VK_RSHIFT:					{ vk_key = L"VK_RSHIFT"; } break;
			case VK_LCONTROL:				{ vk_key = L"VK_LCONTROL"; } break;
			case VK_RCONTROL:				{ vk_key = L"VK_RCONTROL"; } break;
			case VK_LMENU:					{ vk_key = L"VK_LMENU"; } break;
			case VK_RMENU:					{ vk_key = L"VK_RMENU"; } break;
			case VK_BROWSER_BACK:			{ vk_key = L"VK_BROWSER_BACK"; } break;
			case VK_BROWSER_FORWARD:		{ vk_key = L"VK_BROWSER_FORWARD"; } break;
			case VK_BROWSER_REFRESH:		{ vk_key = L"VK_BROWSER_REFRESH"; } break;
			case VK_BROWSER_STOP:			{ vk_key = L"VK_BROWSER_STOP"; } break;
			case VK_BROWSER_SEARCH:			{ vk_key = L"VK_BROWSER_SEARCH"; } break;
			case VK_BROWSER_FAVORITES:		{ vk_key = L"VK_BROWSER_FAVORITES"; } break;
			case VK_BROWSER_HOME:			{ vk_key = L"VK_BROWSER_HOME"; } break;
			case VK_VOLUME_MUTE:			{ vk_key = L"VK_VOLUME_MUTE"; } break;
			case VK_VOLUME_DOWN:			{ vk_key = L"VK_VOLUME_DOWN"; } break;
			case VK_VOLUME_UP:				{ vk_key = L"VK_VOLUME_UP"; } break;
			case VK_MEDIA_NEXT_TRACK:		{ vk_key = L"VK_MEDIA_NEXT_TRACK"; } break;
			case VK_MEDIA_PREV_TRACK:		{ vk_key = L"VK_MEDIA_PREV_TRACK"; } break;
			case VK_MEDIA_STOP:				{ vk_key = L"VK_MEDIA_STOP"; } break;
			case VK_MEDIA_PLAY_PAUSE:		{ vk_key = L"VK_MEDIA_PLAY_PAUSE"; } break;
			case VK_LAUNCH_MAIL:			{ vk_key = L"VK_LAUNCH_MAIL"; } break;
			case VK_LAUNCH_MEDIA_SELECT:	{ vk_key = L"VK_LAUNCH_MEDIA_SELECT"; } break;
			case VK_LAUNCH_APP1:			{ vk_key = L"VK_LAUNCH_APP1"; } break;
			case VK_LAUNCH_APP2:			{ vk_key = L"VK_LAUNCH_APP2"; } break;
			case VK_OEM_8:					{ vk_key = L"VK_OEM_8"; } break;
			case VK_OEM_AX:					{ vk_key = L"VK_OEM_AX"; } break;
			case VK_OEM_102:				{ vk_key = L"VK_OEM_102"; } break;
			case VK_ICO_HELP:				{ vk_key = L"VK_ICO_HELP"; } break;
			case VK_ICO_00:					{ vk_key = L"VK_ICO_00"; } break;
			case VK_PROCESSKEY:				{ vk_key = L"VK_PROCESSKEY"; } break;
			case VK_ICO_CLEAR:				{ vk_key = L"VK_ICO_CLEAR"; } break;
			case VK_PACKET:					{ vk_key = L"VK_PACKET"; } break;
			case VK_OEM_RESET:				{ vk_key = L"VK_OEM_RESET"; } break;
			case VK_OEM_JUMP:				{ vk_key = L"VK_OEM_JUMP"; } break;
			case VK_OEM_PA1:				{ vk_key = L"VK_OEM_PA1"; } break;
			case VK_OEM_PA2:				{ vk_key = L"VK_OEM_PA2"; } break;
			case VK_OEM_PA3:				{ vk_key = L"VK_OEM_PA3"; } break;
			case VK_OEM_WSCTRL:				{ vk_key = L"VK_OEM_WSCTRL"; } break;
			case VK_OEM_CUSEL:				{ vk_key = L"VK_OEM_CUSEL"; } break;
			case VK_OEM_ATTN:				{ vk_key = L"VK_OEM_ATTN"; } break;
			case VK_OEM_FINISH:				{ vk_key = L"VK_OEM_FINISH"; } break;
			case VK_OEM_COPY:				{ vk_key = L"VK_OEM_COPY"; } break;
			case VK_OEM_AUTO:				{ vk_key = L"VK_OEM_AUTO"; } break;
			case VK_OEM_ENLW:				{ vk_key = L"VK_OEM_ENLW"; } break;
			case VK_OEM_BACKTAB:			{ vk_key = L"VK_OEM_BACKTAB"; } break;
			case VK_ATTN:					{ vk_key = L"VK_ATTN"; } break;
			case VK_CRSEL:					{ vk_key = L"VK_CRSEL"; } break;
			case VK_EXSEL:					{ vk_key = L"VK_EXSEL"; } break;
			case VK_EREOF:					{ vk_key = L"VK_EREOF"; } break;
			case VK_PLAY:					{ vk_key = L"VK_PLAY"; } break;
			case VK_ZOOM:					{ vk_key = L"VK_ZOOM"; } break;
			case VK_NONAME:					{ vk_key = L"VK_NONAME"; } break;
			case VK_PA1:					{ vk_key = L"VK_PA1"; } break;
			case VK_OEM_CLEAR:				{ vk_key = L"VK_OEM_CLEAR"; } break;

			// Unused
			case 0x07:
			case 0x40:
			case 0x88:
			case 0x89:
			case 0x8A:
			case 0x8B:
			case 0x8C:
			case 0x8D:
			case 0x8E:
			case 0x8F:
			case 0x97:
			case 0x98:
			case 0x99:
			case 0x9A:
			case 0x9B:
			case 0x9C:
			case 0x9D:
			case 0x9E:
			case 0x9F:
			case 0xD8:
			case 0xD9:
			case 0xDA:
			case 0xE8:
				vk_key = L"Unassigned";
				break;
			case 0x0A:
			case 0x0B:
			case 0x5E:
			case 0xB8:
			case 0xB9:
			case 0xC1:
			case 0xC2:
			case 0xC3:
			case 0xC4:
			case 0xC5:
			case 0xC6:
			case 0xC7:
			case 0xC8:
			case 0xC9:
			case 0xCA:
			case 0xCB:
			case 0xCC:
			case 0xCD:
			case 0xCE:
			case 0xCF:
			case 0xD0:
			case 0xD1:
			case 0xD2:
			case 0xD3:
			case 0xD4:
			case 0xD5:
			case 0xD6:
			case 0xD7:
			case 0xE0:
				vk_key = L"Reserved";
				break;
			default:
				vk_key = L"Unknown";
				break;
		}

		swprintf_s( buf, 128, L"%s%s%s%s", ( ( high & HOTKEYF_CONTROL ) != FALSE ? L"Ctrl + " : L"" ), ( ( high & HOTKEYF_SHIFT ) != FALSE ? L"Shift + " : L"" ), ( ( high & HOTKEYF_ALT ) != FALSE ? L"Alt + " : L"" ), vk_key );
	}

	return buf;
}

wchar_t *get_file_attributes( unsigned int fa_flags )
{
	if ( fa_flags == 0 )
	{
		return L"None";
	}

	static wchar_t buf[ 512 ];
	int size = swprintf_s( buf, 512, L"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
						( ( fa_flags & FILE_ATTRIBUTE_READONLY ) ? L"FILE_ATTRIBUTE_READONLY, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_HIDDEN ) ? L"FILE_ATTRIBUTE_HIDDEN, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_SYSTEM ) ? L"FILE_ATTRIBUTE_SYSTEM, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_DIRECTORY ) ? L"FILE_ATTRIBUTE_DIRECTORY, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_ARCHIVE ) ? L"FILE_ATTRIBUTE_ARCHIVE, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_DEVICE ) ? L"FILE_ATTRIBUTE_DEVICE, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_NORMAL ) ? L"FILE_ATTRIBUTE_NORMAL, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_TEMPORARY ) ? L"FILE_ATTRIBUTE_TEMPORARY, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_SPARSE_FILE ) ? L"FILE_ATTRIBUTE_SPARSE_FILE, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_REPARSE_POINT ) ? L"FILE_ATTRIBUTE_REPARSE_POINT, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_COMPRESSED ) ? L"FILE_ATTRIBUTE_COMPRESSED, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_OFFLINE ) ? L"FILE_ATTRIBUTE_OFFLINE, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED ) ? L"FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_ENCRYPTED ) ? L"FILE_ATTRIBUTE_ENCRYPTED, " : L"" ),
						( ( fa_flags & FILE_ATTRIBUTE_VIRTUAL ) ? L"FILE_ATTRIBUTE_VIRTUAL" : L"" ) );

	if ( size > 1 && buf[ size - 1 ] == L' ' )
	{
		buf[ size - 2 ] = L'\0';
	}
	return buf;
}

wchar_t *get_data_flags( unsigned int d_flags )
{
	if ( d_flags == 0 )
	{
		return L"None";
	}

	static wchar_t buf[ 1024 ];
	int size = swprintf_s( buf, 1024, L"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
						( ( d_flags & HasLinkTargetIDList ) ? L"HasLinkTargetIDList, " : L"" ),
						( ( d_flags & HasLinkInfo ) ? L"HasLinkInfo, " : L"" ),
						( ( d_flags & HasName ) ? L"HasName, " : L"" ),
						( ( d_flags & HasRelativePath ) ? L"HasRelativePath, " : L"" ),
						( ( d_flags & HasWorkingDir ) ? L"HasWorkingDir, " : L"" ),
						( ( d_flags & HasArguments ) ? L"HasArguments, " : L"" ),
						( ( d_flags & HasIconLocation ) ? L"HasIconLocation, " : L"" ),
						( ( d_flags & IsUnicode ) ? L"IsUnicode, " : L"" ),
						( ( d_flags & ForceNoLinkInfo ) ? L"ForceNoLinkInfo, " : L"" ),
						( ( d_flags & HasExpString ) ? L"HasExpString, " : L"" ),
						( ( d_flags & RunInSeparateProcess ) ? L"RunInSeparateProcess, " : L"" ),
						( ( d_flags & Unused1 ) ? L"Unused1, " : L"" ),
						( ( d_flags & HasDarwinID ) ? L"HasDarwinID, " : L"" ),
						( ( d_flags & RunAsUser ) ? L"RunAsUser, " : L"" ),
						( ( d_flags & HasExpIcon ) ? L"HasExpIcon, " : L"" ),
						( ( d_flags & NoPidlAlias ) ? L"NoPidlAlias, " : L"" ),
						( ( d_flags & Unused2 ) ? L"Unused2, " : L"" ),
						( ( d_flags & RunWithShimLayer ) ? L"RunWithShimLayer, " : L"" ),
						( ( d_flags & ForceNoLinkTrack ) ? L"ForceNoLinkTrack, " : L"" ),
						( ( d_flags & EnableTargetMetadata ) ? L"EnableTargetMetadata, " : L"" ),
						( ( d_flags & DisableLinkPathTracking ) ? L"DisableLinkPathTracking, " : L"" ),
						( ( d_flags & DisableKnownFolderTracking ) ? L"DisableKnownFolderTracking, " : L"" ),
						( ( d_flags & DisableKnownFolderAlias ) ? L"DisableKnownFolderAlias, " : L"" ),
						( ( d_flags & AllowLinkToLink ) ? L"AllowLinkToLink, " : L"" ),
						( ( d_flags & UnaliasOnSave ) ? L"UnaliasOnSave, " : L"" ),
						( ( d_flags & PreferEnvironmentPath ) ? L"PreferEnvironmentPath, " : L"" ),
						( ( d_flags & KeepLocalIDListForUNCTarget ) ? L"KeepLocalIDListForUNCTarget" : L"" ) );

	if ( size > 1 && buf[ size - 1 ] == L' ' )
	{
		buf[ size - 2 ] = L'\0';
	}
	return buf;
}

wchar_t *get_link_info_flags( unsigned int li_flags )
{
	if ( li_flags == 0 )
	{
		return L"None";
	}

	static wchar_t buf[ 128 ];
	int size = swprintf_s( buf, 128, L"%s%s",
						( ( li_flags & VolumeIDAndLocalBasePath ) ? L"VolumeIDAndLocalBasePath, " : L"" ),
						( ( li_flags & CommonNetworkRelativeLinkAndPathSuffix ) ? L"CommonNetworkRelativeLinkAndPathSuffix" : L"" ) );

	if ( size > 1 && buf[ size - 1 ] == L' ' )
	{
		buf[ size - 2 ] = L'\0';
	}
	return buf;
}

wchar_t *get_common_network_relative_link_flags( unsigned int cnrl_flags )
{
	if ( cnrl_flags == 0 )
	{
		return L"None";
	}

	static wchar_t buf[ 128 ];
	int size = swprintf_s( buf, 128, L"%s%s",
						( ( cnrl_flags & ValidDevice ) ? L"ValidDevice, " : L"" ),
						( ( cnrl_flags & ValidNetType ) ? L"ValidNetType" : L"" ) );

	if ( size > 1 && buf[ size - 1 ] == L' ' )
	{
		buf[ size - 2 ] = L'\0';
	}
	return buf;
}

wchar_t *get_drive_type( unsigned int d_type )
{
	switch ( d_type )
	{
		case DRIVE_UNKNOWN:		{ return L"DRIVE_UNKNOWN"; } break;
		case DRIVE_NO_ROOT_DIR:	{ return L"DRIVE_NO_ROOT_DIR"; } break;
		case DRIVE_REMOVABLE:	{ return L"DRIVE_REMOVABLE"; } break;
		case DRIVE_FIXED:		{ return L"DRIVE_FIXED"; } break;
		case DRIVE_REMOTE:		{ return L"DRIVE_REMOTE"; } break;
		case DRIVE_CDROM:		{ return L"DRIVE_CDROM"; } break;
		case DRIVE_RAMDISK:		{ return L"DRIVE_RAMDISK"; } break;
		default:
			return L"Unknown";
			break;
	}
}

wchar_t *get_network_provider_type( unsigned int np_type )
{
	switch ( np_type )
	{
		case WNNC_NET_MSNET: { return L"WNNC_NET_MSNET"; } break;
		case WNNC_NET_LANMAN: { return L"WNNC_NET_LANMAN"; } break;
		case WNNC_NET_NETWARE: { return L"WNNC_NET_NETWARE"; } break;
		case WNNC_NET_VINES: { return L"WNNC_NET_VINES"; } break;
		case WNNC_NET_10NET: { return L"WNNC_NET_10NET"; } break;
		case WNNC_NET_LOCUS: { return L"WNNC_NET_LOCUS"; } break;
		case WNNC_NET_SUN_PC_NFS: { return L"WNNC_NET_SUN_PC_NFS"; } break;
		case WNNC_NET_LANSTEP: { return L"WNNC_NET_LANSTEP"; } break;
		case WNNC_NET_9TILES: { return L"WNNC_NET_9TILES"; } break;
		case WNNC_NET_LANTASTIC: { return L"WNNC_NET_LANTASTIC"; } break;
		case WNNC_NET_AS400: { return L"WNNC_NET_AS400"; } break;
		case WNNC_NET_FTP_NFS: { return L"WNNC_NET_FTP_NFS"; } break;
		case WNNC_NET_PATHWORKS: { return L"WNNC_NET_PATHWORKS"; } break;
		case WNNC_NET_LIFENET: { return L"WNNC_NET_LIFENET"; } break;
		case WNNC_NET_POWERLAN: { return L"WNNC_NET_POWERLAN"; } break;
		case WNNC_NET_BWNFS: { return L"WNNC_NET_BWNFS"; } break;
		case WNNC_NET_COGENT: { return L"WNNC_NET_COGENT"; } break;
		case WNNC_NET_FARALLON: { return L"WNNC_NET_FARALLON"; } break;
		case WNNC_NET_APPLETALK: { return L"WNNC_NET_APPLETALK"; } break;
		case WNNC_NET_INTERGRAPH: { return L"WNNC_NET_INTERGRAPH"; } break;
		case WNNC_NET_SYMFONET: { return L"WNNC_NET_SYMFONET"; } break;
		case WNNC_NET_CLEARCASE: { return L"WNNC_NET_CLEARCASE"; } break;
		case WNNC_NET_FRONTIER: { return L"WNNC_NET_FRONTIER"; } break;
		case WNNC_NET_BMC: { return L"WNNC_NET_BMC"; } break;
		case WNNC_NET_DCE: { return L"WNNC_NET_DCE"; } break;
		case WNNC_NET_AVID: { return L"WNNC_NET_AVID"; } break;
		case WNNC_NET_DOCUSPACE: { return L"WNNC_NET_DOCUSPACE"; } break;
		case WNNC_NET_MANGOSOFT: { return L"WNNC_NET_MANGOSOFT"; } break;
		case WNNC_NET_SERNET: { return L"WNNC_NET_SERNET"; } break;
		case WNNC_NET_RIVERFRONT1: { return L"WNNC_NET_RIVERFRONT1"; } break;
		case WNNC_NET_RIVERFRONT2: { return L"WNNC_NET_RIVERFRONT2"; } break;
		case WNNC_NET_DECORB: { return L"WNNC_NET_DECORB"; } break;
		case WNNC_NET_PROTSTOR: { return L"WNNC_NET_PROTSTOR"; } break;
		case WNNC_NET_FJ_REDIR: { return L"WNNC_NET_FJ_REDIR"; } break;
		case WNNC_NET_DISTINCT: { return L"WNNC_NET_DISTINCT"; } break;
		case WNNC_NET_TWINS: { return L"WNNC_NET_TWINS"; } break;
		case WNNC_NET_RDR2SAMPLE: { return L"WNNC_NET_RDR2SAMPLE"; } break;
		case WNNC_NET_CSC: { return L"WNNC_NET_CSC"; } break;
		case WNNC_NET_3IN1: { return L"WNNC_NET_3IN1"; } break;
		case WNNC_NET_EXTENDNET: { return L"WNNC_NET_EXTENDNET"; } break;
		case WNNC_NET_STAC: { return L"WNNC_NET_STAC"; } break;
		case WNNC_NET_FOXBAT: { return L"WNNC_NET_FOXBAT"; } break;
		case WNNC_NET_YAHOO: { return L"WNNC_NET_YAHOO"; } break;
		case WNNC_NET_EXIFS: { return L"WNNC_NET_EXIFS"; } break;
		case WNNC_NET_DAV: { return L"WNNC_NET_DAV"; } break;
		case WNNC_NET_KNOWARE: { return L"WNNC_NET_KNOWARE"; } break;
		case WNNC_NET_OBJECT_DIRE: { return L"WNNC_NET_OBJECT_DIRE"; } break;
		case WNNC_NET_MASFAX: { return L"WNNC_NET_MASFAX"; } break;
		case WNNC_NET_HOB_NFS: { return L"WNNC_NET_HOB_NFS"; } break;
		case WNNC_NET_SHIVA: { return L"WNNC_NET_SHIVA"; } break;
		case WNNC_NET_IBMAL: { return L"WNNC_NET_IBMAL"; } break;
		case WNNC_NET_LOCK: { return L"WNNC_NET_LOCK"; } break;
		case WNNC_NET_TERMSRV: { return L"WNNC_NET_TERMSRV"; } break;
		case WNNC_NET_SRT: { return L"WNNC_NET_SRT"; } break;
		case WNNC_NET_QUINCY: { return L"WNNC_NET_QUINCY"; } break;
		case WNNC_NET_OPENAFS: { return L"WNNC_NET_OPENAFS"; } break;
		case WNNC_NET_AVID1: { return L"WNNC_NET_AVID1"; } break;
		case WNNC_NET_DFS: { return L"WNNC_NET_DFS"; } break;
		case WNNC_NET_KWNP: { return L"WNNC_NET_KWNP"; } break;
		case WNNC_NET_ZENWORKS: { return L"WNNC_NET_ZENWORKS"; } break;
		case WNNC_NET_DRIVEONWEB: { return L"WNNC_NET_DRIVEONWEB"; } break;
		case WNNC_NET_VMWARE: { return L"WNNC_NET_VMWARE"; } break;
		case WNNC_NET_RSFX: { return L"WNNC_NET_RSFX"; } break;
		case WNNC_NET_MFILES: { return L"WNNC_NET_MFILES"; } break;
		case WNNC_NET_MS_NFS: { return L"WNNC_NET_MS_NFS"; } break;
		case WNNC_NET_GOOGLE: { return L"WNNC_NET_GOOGLE"; } break;
		case WNNC_CRED_MANAGER: { return L"WNNC_CRED_MANAGER"; } break;
		default:
			return L"Unknown";
			break;
	}
}

wchar_t *get_font_family_value( unsigned short ff_value )
{
	if ( ff_value == 0 )
	{
		return L"None";
	}

	static wchar_t buf[ 128 ];
	int size = swprintf_s( buf, 512, L"%s%s%s%s%s%s",
						( ( ff_value & FF_DONTCARE ) ? L"FF_DONTCARE, " : L"" ),
						( ( ff_value & FF_ROMAN ) ? L"FF_ROMAN, " : L"" ),
						( ( ff_value & FF_SWISS ) ? L"FF_SWISS, " : L"" ),
						( ( ff_value & FF_MODERN ) ? L"FF_MODERN, " : L"" ),
						( ( ff_value & FF_SCRIPT ) ? L"FF_SCRIPT, " : L"" ),
						( ( ff_value & FF_DECORATIVE ) ? L"FF_DECORATIVE, " : L"" ) );

	if ( size > 1 && buf[ size - 1 ] == L' ' )
	{
		buf[ size - 2 ] = L'\0';
	}
	return buf;
}

wchar_t *get_font_weight( unsigned int fw )
{
	switch ( fw )
	{
		case FW_DONTCARE:		{ return L"FW_DONTCARE"; } break;
		case FW_THIN:	{ return L"FW_THIN"; } break;
		//case FW_ULTRALIGHT:
		case FW_EXTRALIGHT:	{ return L"FW_EXTRALIGHT / FW_ULTRALIGHT"; } break;
		case FW_LIGHT:		{ return L"FW_LIGHT"; } break;
		//case FW_REGULAR:
		case FW_NORMAL:		{ return L"FW_NORMAL / FW_REGULAR"; } break;
		case FW_MEDIUM:		{ return L"FW_MEDIUM"; } break;
		//case FW_DEMIBOLD:
		case FW_SEMIBOLD:		{ return L"FW_SEMIBOLD / FW_DEMIBOLD"; } break;
		case FW_BOLD:		{ return L"FW_BOLD"; } break;
		//case FW_ULTRABOLD:
		case FW_EXTRABOLD:		{ return L"FW_EXTRABOLD / FW_ULTRABOLD"; } break;
		//case FW_BLACK:
		case FW_HEAVY:		{ return L"FW_HEAVY / FW_BLACK"; } break;
		default:
			return L"Unknown";
			break;
	}
}

wchar_t *get_color_flags( unsigned short c_flags )
{
	if ( c_flags == 0 )
	{
		return L"None";
	}

	static wchar_t buf[ 128 ];
	int size = swprintf_s( buf, 512, L"%s%s%s%s%s%s%s%s",
						( ( c_flags & FOREGROUND_BLUE ) ? L"FOREGROUND_BLUE, " : L"" ),
						( ( c_flags & FOREGROUND_GREEN ) ? L"FOREGROUND_GREEN, " : L"" ),
						( ( c_flags & FOREGROUND_RED ) ? L"FOREGROUND_RED, " : L"" ),
						( ( c_flags & FOREGROUND_INTENSITY ) ? L"FOREGROUND_INTENSITY, " : L"" ),
						( ( c_flags & BACKGROUND_BLUE ) ? L"BACKGROUND_BLUE, " : L"" ),
						( ( c_flags & BACKGROUND_GREEN ) ? L"BACKGROUND_GREEN, " : L"" ),
						( ( c_flags & BACKGROUND_RED ) ? L"BACKGROUND_RED, " : L"" ),
						( ( c_flags & BACKGROUND_INTENSITY ) ? L"BACKGROUND_INTENSITY, " : L"" ) );

	if ( size > 1 && buf[ size - 1 ] == L' ' )
	{
		buf[ size - 2 ] = L'\0';
	}
	return buf;
}

wchar_t *get_special_folder_type( unsigned int sf_type )
{
	switch ( sf_type )
	{
		case SF_Desktop:				{ return L"Desktop"; } break;
		case SF_Internet:				{ return L"Internet"; } break;
		case SF_Programs:				{ return L"Programs"; } break;
		case SF_Controls:				{ return L"Controls"; } break;
		case SF_Printers:				{ return L"Printers"; } break;
		case SF_Personal:				{ return L"Personal"; } break;
		case SF_Favorites:				{ return L"Favorites"; } break;
		case SF_Startup:				{ return L"Startup"; } break;
		case SF_Recent:					{ return L"Recent"; } break;
		case SF_SendTo:					{ return L"SendTo"; } break;
		case SF_BitBucket:				{ return L"BitBucket"; } break;
		case SF_StartMenu:				{ return L"StartMenu"; } break;
		case SF_MyDocuments:			{ return L"MyDocuments"; } break;
		case SF_MyMusic:				{ return L"MyMusic"; } break;
		case SF_MyVideo:				{ return L"MyVideo"; } break;
		case SF_DesktopDirectory:		{ return L"DesktopDirectory"; } break;
		case SF_Drives:					{ return L"Drives"; } break;
		case SF_Network:				{ return L"Network"; } break;
		case SF_Nethood:				{ return L"Nethood"; } break;
		case SF_Fonts:					{ return L"Fonts"; } break;
		case SF_Templates:				{ return L"Templates"; } break;
		case SF_CommonStartMenu:		{ return L"CommonStartMenu"; } break;
		case SF_CommonPrograms:			{ return L"CommonPrograms"; } break;
		case SF_CommonStartup:			{ return L"CommonStartup"; } break;
		case SF_CommonDesktopDirectory:	{ return L"CommonDesktopDirectory"; } break;
		case SF_AppData:				{ return L"AppData"; } break;
		case SF_PrintHood:				{ return L"PrintHood"; } break;
		case SF_LocalAppData:			{ return L"LocalAppData"; } break;
		case SF_AltStartup:				{ return L"AltStartup"; } break;
		case SF_CommonAltStartup:		{ return L"CommonAltStartup"; } break;
		case SF_CommonFavorites:		{ return L"CommonFavorites"; } break;
		case SF_InternetCache:			{ return L"InternetCache"; } break;
		case SF_Cookies:				{ return L"Cookies"; } break;
		case SF_History:				{ return L"History"; } break;
		case SF_CommonAppData:			{ return L"CommonAppData"; } break;
		case SF_Windows:				{ return L"Windows"; } break;
		case SF_System:					{ return L"System"; } break;
		case SF_ProgramFiles:			{ return L"ProgramFiles"; } break;
		case SF_MyPictures:				{ return L"MyPictures"; } break;
		case SF_Profile:				{ return L"Profile"; } break;
		case SF_SystemX86:				{ return L"SystemX86"; } break;
		case SF_ProgramFilesX86:		{ return L"ProgramFilesX86"; } break;
		case SF_ProgramFilesCommon:		{ return L"ProgramFilesCommon"; } break;
		case SF_ProgramFilesCommonX86:	{ return L"ProgramFilesCommonX86"; } break;
		case SF_CommonTemplates:		{ return L"CommonTemplates"; } break;
		case SF_CommonDocuments:		{ return L"CommonDocuments"; } break;
		case SF_CommonAdminTools:		{ return L"CommonAdminTools"; } break;
		case SF_AdminTools:				{ return L"AdminTools"; } break;
		case SF_Connections:			{ return L"Connections"; } break;
		case SF_CommonMusic:			{ return L"CommonMusic"; } break;
		case SF_CommonPictures:			{ return L"CommonPictures"; } break;
		case SF_CommonVideo:			{ return L"CommonVideo"; } break;
		case SF_Resources:				{ return L"Resources"; } break;
		case SF_ResourcesLocalized:		{ return L"ResourcesLocalized"; } break;
		case SF_CommonOEMLinks:			{ return L"CommonOEMLinks"; } break;
		case SF_CDBurnArea:				{ return L"CDBurnArea"; } break;
		case SF_ComputersNearMe:		{ return L"ComputersNearMe"; } break;
		case SF_FlagCreate:				{ return L"FlagCreate"; } break;
		case SF_FlagDontVerify:			{ return L"FlagDontVerify"; } break;
		case SF_FlagNoAlias:			{ return L"FlagNoAlias"; } break;
		case SF_FlagPerUserInit:		{ return L"FlagPerUserInit"; } break;
		case SF_FlagMask:				{ return L"FlagMask"; } break;
		default:
			return L"Unknown";
			break;
	}

	return NULL;
}

wchar_t *get_property_type( unsigned int p_type )
{
	switch ( p_type )
	{
		case VT_EMPTY:				{ return L"VT_EMPTY"; } break;
		case VT_NULL:				{ return L"VT_NULL"; } break;
		case VT_I2:					{ return L"VT_I2"; } break;
		case VT_I4:					{ return L"VT_I4"; } break;
		case VT_R4:					{ return L"VT_R4"; } break;
		case VT_R8:					{ return L"VT_R8"; } break;
		case VT_CY:					{ return L"VT_CY"; } break;
		case VT_DATE:				{ return L"VT_DATE"; } break;
		case VT_BSTR:				{ return L"VT_BSTR"; } break;
		case VT_DISPATCH:			{ return L"VT_DISPATCH"; } break;		// Should not be used
		case VT_ERROR:				{ return L"VT_ERROR"; } break;
		case VT_BOOL:				{ return L"VT_BOOL"; } break;
		case VT_VARIANT:			{ return L"VT_VARIANT"; } break;		// Should not be used alone
		case VT_UNKNOWN:			{ return L"VT_UNKNOWN"; } break;		// Should not be used
		case VT_DECIMAL:			{ return L"VT_DECIMAL"; } break;
		case VT_I1:					{ return L"VT_I1"; } break;
		case VT_UI1:				{ return L"VT_UI1"; } break;
		case VT_UI2:				{ return L"VT_UI2"; } break;
		case VT_UI4:				{ return L"VT_UI4"; } break;
		case VT_I8:					{ return L"VT_I8"; } break;
		case VT_UI8:				{ return L"VT_UI8"; } break;
		case VT_INT:				{ return L"VT_INT"; } break;
		case VT_UINT:				{ return L"VT_UINT"; } break;
		case VT_VOID:				{ return L"VT_VOID"; } break;			// Should not be used
		case VT_HRESULT:			{ return L"VT_HRESULT"; } break;		// Should not be used
		case VT_PTR:				{ return L"VT_PTR"; } break;			// Should not be used
		case VT_SAFEARRAY:			{ return L"VT_SAFEARRAY"; } break;		// Should not be used
		case VT_CARRAY:				{ return L"VT_CARRAY"; } break;			// Should not be used
		case VT_USERDEFINED:		{ return L"VT_USERDEFINED"; } break;	// Should not be used
		case VT_LPSTR:				{ return L"VT_LPSTR"; } break;
		case VT_LPWSTR:				{ return L"VT_LPWSTR"; } break;
		case VT_RECORD:				{ return L"VT_RECORD"; } break;			// Should not be used
		case VT_INT_PTR:			{ return L"VT_INT_PTR"; } break;		// Should not be used
		case VT_UINT_PTR:			{ return L"VT_UINT_PTR"; } break;		// Should not be used
		case VT_FILETIME:			{ return L"VT_FILETIME"; } break;
		case VT_BLOB:				{ return L"VT_BLOB"; } break;
		case VT_STREAM:				{ return L"VT_STREAM"; } break;
		case VT_STORAGE:			{ return L"VT_STORAGE"; } break;
		case VT_STREAMED_OBJECT:	{ return L"VT_STREAMED_OBJECT"; } break;
		case VT_STORED_OBJECT:		{ return L"VT_STORED_OBJECT"; } break;
		case VT_BLOB_OBJECT:		{ return L"VT_BLOB_OBJECT"; } break;
		case VT_CF:					{ return L"VT_CF"; } break;
		case VT_CLSID:				{ return L"VT_CLSID"; } break;
		case VT_VERSIONED_STREAM:	{ return L"VT_VERSIONED_STREAM"; } break;
		//case VT_BSTR_BLOB:												// Should not be used
		case VT_VECTOR:				{ return L"VT_VECTOR"; } break;			// Should not be used alone
		case VT_VECTOR | VT_I2:			{ return L"VT_VECTOR | VT_I2"; } break;
		case VT_VECTOR | VT_I4:			{ return L"VT_VECTOR | VT_I4"; } break;
		case VT_VECTOR | VT_R4:			{ return L"VT_VECTOR | VT_R4"; } break;
		case VT_VECTOR | VT_R8:			{ return L"VT_VECTOR | VT_R8"; } break;
		case VT_VECTOR | VT_CY:			{ return L"VT_VECTOR | VT_CY"; } break;
		case VT_VECTOR | VT_DATE :		{ return L"VT_VECTOR | VT_DATE "; } break;
		case VT_VECTOR | VT_BSTR :		{ return L"VT_VECTOR | VT_BSTR "; } break;
		case VT_VECTOR | VT_ERROR :		{ return L"VT_VECTOR | VT_ERROR "; } break;
		case VT_VECTOR | VT_BOOL :		{ return L"VT_VECTOR | VT_BOOL "; } break;
		case VT_VECTOR | VT_VARIANT:	{ return L"VT_VECTOR | VT_VARIANT"; } break;
		case VT_VECTOR | VT_I1:			{ return L"VT_VECTOR | VT_I1"; } break;
		case VT_VECTOR | VT_UI1:		{ return L"VT_VECTOR | VT_UI1"; } break;
		case VT_VECTOR | VT_UI2:		{ return L"VT_VECTOR | VT_UI2"; } break;
		case VT_VECTOR | VT_UI4:		{ return L"VT_VECTOR | VT_UI4"; } break;
		case VT_VECTOR | VT_I8:			{ return L"VT_VECTOR | VT_I8"; } break;
		case VT_VECTOR | VT_UI8:		{ return L"VT_VECTOR | VT_UI8"; } break;
		case VT_VECTOR | VT_LPSTR:		{ return L"VT_VECTOR | VT_LPSTR"; } break;
		case VT_VECTOR | VT_LPWSTR:		{ return L"VT_VECTOR | VT_LPWSTR"; } break;
		case VT_VECTOR | VT_FILETIME:	{ return L"VT_VECTOR | VT_FILETIME"; } break;
		case VT_VECTOR | VT_CF:			{ return L"VT_VECTOR | VT_CF"; } break;
		case VT_VECTOR | VT_CLSID:		{ return L"VT_VECTOR | VT_CLSID"; } break;
		case VT_ARRAY:				{ return L"VT_ARRAY"; } break;			// Should not be used alone
		case VT_ARRAY | VT_I2:			{ return L"VT_ARRAY | VT_I2"; } break;
		case VT_ARRAY | VT_I4:			{ return L"VT_ARRAY | VT_I4"; } break;
		case VT_ARRAY | VT_R4:			{ return L"VT_ARRAY | VT_R4"; } break;
		case VT_ARRAY | VT_R8:			{ return L"VT_ARRAY | VT_R8"; } break;
		case VT_ARRAY | VT_CY:			{ return L"VT_ARRAY | VT_CY"; } break;
		case VT_ARRAY | VT_DATE:		{ return L"VT_ARRAY | VT_DATE"; } break;
		case VT_ARRAY | VT_BSTR:		{ return L"VT_ARRAY | VT_BSTR"; } break;
		case VT_ARRAY | VT_ERROR:		{ return L"VT_ARRAY | VT_ERROR"; } break;
		case VT_ARRAY | VT_BOOL:		{ return L"VT_ARRAY | VT_BOOL"; } break;
		case VT_ARRAY | VT_VARIANT:		{ return L"VT_ARRAY | VT_VARIANT"; } break;
		case VT_ARRAY | VT_DECIMAL:		{ return L"VT_ARRAY | VT_DECIMAL"; } break;
		case VT_ARRAY | VT_I1:			{ return L"VT_ARRAY | VT_I1"; } break;
		case VT_ARRAY | VT_UI1:			{ return L"VT_ARRAY | VT_UI1"; } break;
		case VT_ARRAY | VT_UI2:			{ return L"VT_ARRAY | VT_UI2"; } break;
		case VT_ARRAY | VT_UI4:			{ return L"VT_ARRAY | VT_UI4"; } break;
		case VT_ARRAY | VT_INT:			{ return L"VT_ARRAY | VT_INT"; } break;
		case VT_ARRAY | VT_UINT:		{ return L"VT_ARRAY | VT_UINT"; } break;
		case VT_BYREF:				{ return L"VT_BYREF"; } break;			// Should not be used
		case VT_RESERVED:			{ return L"VT_RESERVED"; } break;		// Should not be used
		case VT_ILLEGAL:			{ return L"VT_ILLEGAL"; } break;		// Should not be used
		//case VT_ILLEGALMASKED:											// Should not be used
		case VT_TYPEMASK:			{ return L"VT_BSTR_BLOB / VT_ILLEGALMASKED / VT_TYPEMASK"; } break;		// Should not be used
		default:
			return L"Unknown";
			break;
	}

	return NULL;
}

void buffer_to_guid( unsigned char *buffer, char *guid )
{
	int offset = sprintf_s( guid, 64, "%.8x-%.4x-%.4x-", *( ( unsigned int * )buffer ), *( ( unsigned short * )buffer + 2 ), *( ( unsigned short * )buffer + 3 ) );
	for ( unsigned char i = 8; i < 16; ++i )
	{
		if ( offset == 23 )
		{
			guid[ offset ] = '-';
			++offset;
		}
		offset += sprintf_s( guid + offset, 64 - offset, "%.2x", buffer[ i ] );
	}
}

void buffer_to_mac( char *buffer, char *mac )
{
	unsigned char offset = 0;
	for ( unsigned char i = 0; i < 12; ++i )
	{
		if ( offset == 2 || offset == 5 || offset == 8 || offset == 11 || offset == 14 )
		{
			mac[ offset++ ] = ':';
		}

		mac[ offset++ ] = buffer[ i ];
	}
}

void hex_dump( unsigned char *buffer, unsigned int buf_length )
{
	unsigned int i = 0;
	int hex_offset = 0;
	int text_offset = 0;
	char hex_buf[ 50 ] = { 0 };
	char text_buf[ 17 ] = { 0 };
	for ( i = 0; i < buf_length; ++i )
	{
		if ( i > 0 && i % 8 == 0 )
		{
			if ( i % 16 == 0 )
			{
				printf( "%s\t%s\n", hex_buf, text_buf );
				hex_offset = text_offset = 0;
			}
			else
			{
				hex_buf[ hex_offset++ ] = ' ';
			}
		}

		hex_offset += sprintf_s( hex_buf + hex_offset, 50 - hex_offset, "%.02x ", buffer[ i ] );
		text_offset += sprintf_s( text_buf + text_offset, 17 - text_offset, "%c", ( buffer[ i ] < 0x21 || ( buffer[ i ] >= 0x7f && buffer[ i ] <= 0x9f ) ) ? '.' : buffer[ i ] );
	}

	if ( i > 0 && ( ( i % 8 != 0 ) || ( i % 16 == 8 ) ) ) // 15 or less characters filled.
	{
		printf( hex_buf );
		unsigned char r = ( i % 16 >= 8 ? 8 : 16 ) - ( i % 8 );
		while ( r-- )
		{
			printf( "   " );
		}
		printf( "\t%s\n", text_buf );
	}
	else	// 16 characters filled.
	{
		printf( "%s\t%s\n", hex_buf, text_buf );
	}
}
 
const clsid_type clsid_list[] = { 
								{ GUID_AddNewPrograms, "AddNewPrograms" },
								{ GUID_AdminTools, "AdminTools" },
								{ GUID_AppDataLow, "AppDataLow" },
								{ GUID_ApplicationShortcuts, "ApplicationShortcuts" },
								{ GUID_AppsFolder, "AppsFolder" },
								{ GUID_AppUpdates, "AppUpdates" },
								{ GUID_CDBurning, "CDBurning" },
								{ GUID_ChangeRemovePrograms, "ChangeRemovePrograms" },
								{ GUID_CommonAdminTools, "CommonAdminTools" },
								{ GUID_CommonOEMLinks, "CommonOEMLinks" },
								{ GUID_CommonPrograms, "CommonPrograms" },
								{ GUID_CommonStartMenu, "CommonStartMenu" },
								{ GUID_CommonStartup, "CommonStartup" },
								{ GUID_CommonTemplates, "CommonTemplates" },
								{ GUID_ComputerFolder, "ComputerFolder" },
								{ GUID_ConflictFolder, "ConflictFolder" },
								{ GUID_ConnectionsFolder, "ConnectionsFolder" },
								{ GUID_Contacts, "Contacts" },
								{ GUID_ControlPanelFolder, "ControlPanelFolder" },
								{ GUID_Cookies, "Cookies" },
								{ GUID_Desktop, "Desktop" },
								{ GUID_DeviceMetadataStore, "DeviceMetadataStore" },
								{ GUID_Documents, "Documents" },
								{ GUID_DocumentsLibrary, "DocumentsLibrary" },
								{ GUID_Downloads, "Downloads" },
								{ GUID_Favorites, "Favorites" },
								{ GUID_Fonts, "Fonts" },
								{ GUID_Games, "Games" },
								{ GUID_GameTasks, "GameTasks" },
								{ GUID_History, "History" },
								{ GUID_HomeGroup, "HomeGroup" },
								{ GUID_HomeGroupCurrentUser, "HomeGroupCurrentUser" },
								{ GUID_ImplicitAppShortcuts, "ImplicitAppShortcuts" },
								{ GUID_InternetCache, "InternetCache" },
								{ GUID_InternetFolder, "InternetFolder" },
								{ GUID_Libraries, "Libraries" },
								{ GUID_Links, "Links" },
								{ GUID_LocalAppData, "LocalAppData" },
								{ GUID_LocalAppDataLow, "LocalAppDataLow" },
								{ GUID_LocalizedResourcesDir, "LocalizedResourcesDir" },
								{ GUID_Music, "Music" },
								{ GUID_MusicLibrary, "MusicLibrary" },
								{ GUID_NetHood, "NetHood" },
								{ GUID_NetworkFolder, "NetworkFolder" },
								{ GUID_OriginalImages, "OriginalImages" },
								{ GUID_PhotoAlbums, "PhotoAlbums" },
								{ GUID_Pictures, "Pictures" },
								{ GUID_PicturesLibrary, "PicturesLibrary" },
								{ GUID_Playlists, "Playlists" },
								{ GUID_PrintersFolder, "PrintersFolder" },
								{ GUID_PrintHood, "PrintHood" },
								{ GUID_Profile, "Profile" },
								{ GUID_ProgramData, "ProgramData" },
								{ GUID_ProgramFiles, "ProgramFiles" },
								{ GUID_ProgramFilesCommon, "ProgramFilesCommon" },
								{ GUID_ProgramFilesCommonX64, "ProgramFilesCommonX64" },
								{ GUID_ProgramFilesCommonX86, "ProgramFilesCommonX86" },
								{ GUID_ProgramFilesX64, "ProgramFilesX64" },
								{ GUID_ProgramFilesX86, "ProgramFilesX86" },
								{ GUID_Programs, "Programs" },
								{ GUID_Public, "Public" },
								{ GUID_PublicDesktop, "PublicDesktop" },
								{ GUID_PublicDocuments, "PublicDocuments" },
								{ GUID_PublicDownloads, "PublicDownloads" },
								{ GUID_PublicGameTasks, "PublicGameTasks" },
								{ GUID_PublicLibraries, "PublicLibraries" },
								{ GUID_PublicMusic, "PublicMusic" },
								{ GUID_PublicPictures, "PublicPictures" },
								{ GUID_PublicRingtones, "PublicRingtones" },
								{ GUID_PublicUserTiles, "PublicUserTiles" },
								{ GUID_PublicVideos, "PublicVideos" },
								{ GUID_QuickLaunch, "QuickLaunch" },
								{ GUID_Recent, "Recent" },
								{ GUID_RecordedTV, "RecordedTV" },
								{ GUID_RecordedTVLibrary, "RecordedTVLibrary" },
								{ GUID_RecycleBin, "RecycleBin" },
								{ GUID_ResourceDir, "ResourceDir" },
								{ GUID_Ringtones, "Ringtones" },
								{ GUID_RoamingAppData, "RoamingAppData" },
								{ GUID_RoamingTiles, "RoamingTiles" },
								{ GUID_SampleMusic, "SampleMusic" },
								{ GUID_SamplePictures, "SamplePictures" },
								{ GUID_SamplePlaylists, "SamplePlaylists" },
								{ GUID_SampleVideos, "SampleVideos" },
								{ GUID_SavedGames, "SavedGames" },
								{ GUID_SavedSearches, "SavedSearches" },
								{ GUID_SEARCH_CSC, "SEARCH_CSC" },
								{ GUID_SEARCH_MAPI, "SEARCH_MAPI" },
								{ GUID_SearchHome, "SearchHome" },
								{ GUID_SendTo, "SendTo" },
								{ GUID_SidebarDefaultParts, "SidebarDefaultParts" },
								{ GUID_SidebarParts, "SidebarParts" },
								{ GUID_StartMenu, "StartMenu" },
								{ GUID_Startup, "Startup" },
								{ GUID_SyncManagerFolder, "SyncManagerFolder" },
								{ GUID_SyncResults, "SyncResults" },
								{ GUID_SyncSetupFolder, "SyncSetupFolder" },
								{ GUID_System, "System" },
								{ GUID_SystemX86, "SystemX86" },
								{ GUID_Templates, "Templates" },
								{ GUID_TreeProperties, "TreeProperties" },
								{ GUID_UserPinned, "UserPinned" },
								{ GUID_UserProfiles, "UserProfiles" },
								{ GUID_UserProgramFiles, "UserProgramFiles" },
								{ GUID_UserProgramFilesCommon, "UserProgramFilesCommon" },
								{ GUID_UsersFiles, "UsersFiles" },
								{ GUID_UsersLibraries, "UsersLibraries" },
								{ GUID_UsersLibrariesFolder, "UsersLibrariesFolder" },
								{ GUID_UserTiles, "UserTiles" },
								{ GUID_Videos, "Videos" },
								{ GUID_VideosLibrary, "VideosLibrary" },
								{ GUID_Windows, "Windows" },

								{ GUID_My_Computer, "My Computer" },
								{ GUID_My_Documents, "My Documents" },
								{ GUID_Control_Panel, "Control Panel" },
								{ GUID_Control_Panel2, "Control Panel" },
								{ GUID_Internet_Explorer, "Internet Explorer" },
								{ GUID_My_Games, "My Games" },
								{ GUID_My_Network_Places, "My Network Places" },
								{ GUID_Network_Connections, "Network Connections" },
								{ GUID_Printers_and_Faxes, "Printers and Faxes" },
								{ GUID_Dial_up_Connection, "Dial-up Connection" },
								{ GUID_Show_Desktop, "Show Desktop" },
								{ GUID_Users, "Users" },
								{ GUID_Window_Switcher, "Window Switcher" },
								{ GUID_CD_Burner, "CD Burner" },
								{ GUID_CSC_Folder, "CSC Folder" },
								{ GUID_Search, "Search" },
								{ GUID_Help_and_Support, "Help and Support" },
								{ GUID_Windows_Security, "Windows Security" },
								{ GUID_Run, "Run..." },
								{ GUID_Email, "E-mail" },
								{ GUID_Set_Program_Access, "Set Program Access and Defaults" },
								{ GUID_Start_Menu_Provider, "StartMenuProviderFolder" },
								{ GUID_Start_Menu, "Start Menu" },
								{ GUID_Search_Results, "Search Results" },
								{ NULL, NULL } };

char *get_clsid_type( char *clsid )
{
	for ( int i = 0; clsid_list[ i ].clsid != NULL; i++ )
	{
		if ( memcmp( clsid, clsid_list[ i ].clsid, 16 ) == 0 )
		{
			return clsid_list[ i ].name;
		}
	}

	return NULL;
}

char *get_prop_id_type( char *guid, unsigned int prop_id )
{
	if ( memcmp( guid, "\xe0\x85\x9f\xF2\xF9\x4f\x68\x10\xAB\x91\x08\x00\x2B\x27\xB3\xD9", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 2: { return "System.Title"; } break;
			case 3: { return "System.Subject"; } break;
			case 4: { return "System.Author"; } break;
			case 5: { return "System.Keywords"; } break;
			case 6: { return "System.Comment"; } break;
			case 8: { return "System.Document.LastAuthor"; } break;
			case 9: { return "System.Document.RevisionNumber"; } break;
			case 10: { return "System.Document.TotalEditingTime"; } break;
			case 11: { return "System.Document.DatePrinted"; } break;
			case 12: { return "System.Document.DateCreated"; } break;
			case 13: { return "System.Document.DateSaved"; } break;
			case 14: { return "System.Document.PageCount"; } break;
			case 15: { return "System.Document.WordCount"; } break;
			case 16: { return "System.Document.CharacterCount"; } break;
			case 18: { return "System.ApplicationName"; } break;
		}
	}
	else if ( memcmp( guid, "\x2e\x37\xa3\x56\x9c\xce\xd2\x11\x9f\x0e\x00\x60\x97\xc6\x86\xf6", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 2: { return "System.Music.Artist"; } break;
			case 4: { return "System.Music.AlbumTitle"; } break;
			case 5: { return "System.Media.Year"; } break;
			case 7: { return "System.Music.TrackNumber"; } break;
			case 8: { return "AudioTimeLength"; } break;
			case 11: { return "System.Music.Genre"; } break;
			case 12: { return "System.Music.Lyrics"; } break;
			case 13: { return "System.Music.AlbumArtist"; } break;
			case 33: { return "System.Music.ContentGroupDescription"; } break;
			case 34: { return "System.Music.InitialKey"; } break;
			case 35: { return "System.Music.BeatsPerMinute"; } break;
			case 36: { return "System.Music.Conductor"; } break;
			case 37: { return "System.Music.PartOfSet"; } break;
			case 38: { return "System.Media.SubTitle"; } break;
			case 39: { return "System.Music.Mood"; } break;
			case 100: { return "System.Music.AlbumID"; } break;
		}
	}
	else if ( memcmp( guid, "\x30\xf1\x25\xB7\xEF\x47\x1A\x10\xA5\xF1\x02\x60\x8C\x9E\xEB\xAC", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 2: { return "System.ItemFolderNameDisplay"; } break;
			case 4: { return "System.ItemTypeText"; } break;
			case 10: { return "System.ItemTypeText"; } break;
			case 12: { return "System.Size"; } break;
			case 13: { return "System.FileAttributes"; } break;
			case 14: { return "System.DateModified"; } break;
			case 15: { return "System.DateCreated"; } break;
			case 16: { return "System.DateAccessed"; } break;
			case 19: { return "System.Search.Contents"; } break;
			case 21: { return "System.FileFRN"; } break;
		}
	}
	else if ( memcmp( guid, "\xa6\x6a\x63\x28\x3D\x95\xD2\x11\xB5\xD6\x00\xC0\x4F\xD9\x18\xD0", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 0: { return "System.FindData"; } break;
			case 2: { return "System.DescriptionID"; } break;
			case 5: { return "System.ComputerName"; } break;
			case 6: { return "System.NamespaceCLSID"; } break;
			case 8: { return "System.ItemPathDisplayNarrow"; } break;
			case 9: { return "System.PerceivedType"; } break;
			case 11: { return "System.ItemType"; } break;
			case 12: { return "System.FileCount"; } break;
			case 14: { return "System.TotalFileSize"; } break;
			case 24: { return "System.ParsingName"; } break;
			case 25: { return "System.SFGAOFlags"; } break;
			case 29: { return "System.ContainedItems"; } break;
			case 30: { return "System.ParsingPath"; } break;
			case 33: { return "System.IsSendToTarget"; } break;
		}
	}
	else if ( memcmp( guid, "\x02\xd5\xcd\xd5\x9c\x2e\x1b\x10\x93\x97\x08\x00\x2b\x2c\xf9\xae", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 2: { return "System.Category"; } break;
			case 3: { return "System.Document.PresentationFormat"; } break;
			case 4: { return "System.Document.ByteCount"; } break;
			case 5: { return "System.Document.LineCount"; } break;
			case 6: { return "System.Document.ParagraphCount"; } break;
			case 7: { return "System.Document.SlideCount"; } break;
			case 8: { return "System.Document.NoteCount"; } break;
			case 9: { return "System.Document.HiddenSlideCount"; } break;
			case 10: { return "System.Document.MultimediaClipCount"; } break;
			case 14: { return "System.Document.Manager"; } break;
			case 15: { return "System.Company"; } break;
			case 26: { return "System.ContentType"; } break;
			case 27: { return "System.ContentStatus"; } break;
			case 28: { return "System.Language"; } break;
			case 29: { return "System.Document.Version"; } break;
		}
	}
	else if ( memcmp( guid, "\x7f\xb6\x76\x5d\x3d\x9b\xbb\x44\xb6\xae\x25\xda\x4f\x63\x8a\x67", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 2: { return "System.IsPinnedToNameSpaceTree"; } break;
			case 3: { return "System.IsDefaultSaveLocation"; } break;
			case 5: { return "System.IsDefaultNonOwnerSaveLocation"; } break;
			case 6: { return "System.OwnerSID"; } break;
			case 8: { return "System.IsLocationSupported"; } break;
		}
	}
	else if ( memcmp( guid, "\x53\x7d\xef\x0C\x64\xfa\xD1\x11\xA2\x03\x00\x00\xF8\x1F\xED\xEE", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 3: { return "System.FileDescription"; } break;
			case 4: { return "System.FileVersion"; } break;
			case 5: { return "System.InternalName"; } break;
			case 6: { return "System.OriginalFileName"; } break;
			case 7: { return "System.Software.ProductName"; } break;
			case 8: { return "System.Software.ProductVersion"; } break;
			case 9: { return "System.Trademarks"; } break;
		}
	}		
	else if ( memcmp( guid, "\x92\x04\x44\x64\x8B\x4c\xD1\x11\x70\x8b\x08\x00\x36\xB1\x1A\x03", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 9: { return "System.Rating"; } break;
			case 11: { return "System.Copyright"; } break;
			case 12: { return "System.ShareUserRating"; } break;
			case 13: { return "System.Media.ClassPrimaryID"; } break;
			case 14: { return "System.Media.ClassSecondaryID"; } break;
			case 15: { return "System.Media.DVDID"; } break;
			case 16: { return "System.Media.MCDI"; } break;
			case 17: { return "System.Media.MetadataContentProvider"; } break;
			case 18: { return "System.Media.ContentDistributor"; } break;
			case 19: { return "System.Music.Composer"; } break;
			case 20: { return "System.Video.Director"; } break;
			case 21: { return "System.ParentalRating"; } break;
			case 22: { return "System.Media.Producer"; } break;
			case 23: { return "System.Media.Writer"; } break;
			case 24: { return "System.Media.CollectionGroupID"; } break;
			case 25: { return "System.Media.CollectionID"; } break;
			case 26: { return "System.Media.ContentID"; } break;
			case 27: { return "System.Media.CreatorApplication"; } break;
			case 28: { return "System.Media.CreatorApplicationVersion"; } break;
			case 30: { return "System.Media.Publisher"; } break;
			case 31: { return "System.Music.Period"; } break;
			case 32: { return "System.Media.AuthorUrl"; } break;
			case 33: { return "System.Media.PromotionUrl"; } break;
			case 34: { return "System.Media.UserWebUrl"; } break;
			case 35: { return "System.Media.UniqueFileIdentifier"; } break;
			case 36: { return "System.Media.EncodedBy"; } break;
			case 38: { return "System.Media.ProtectionType"; } break;
			case 39: { return "System.Media.ProviderRating"; } break;
			case 40: { return "System.Media.ProviderStyle"; } break;
			case 41: { return "System.Media.UserNoAutoInfo"; } break;
		}
	}
	else if ( memcmp( guid, "\x21\x4a\x94\xc9\x06\xa4\xfe\x48\x82\x25\xae\xc7\xe2\x4c\x21\x1b", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 2: { return "System.PropList.FullDetails"; } break;
			case 3: { return "System.PropList.TileInfo"; } break;
			case 4: { return "System.PropList.InfoTip"; } break;
			case 5: { return "System.PropList.QuickTip"; } break;
			case 6: { return "System.PropList.PreviewTitle"; } break;
			case 8: { return "System.PropList.PreviewDetails"; } break;
			case 9: { return "System.PropList.ExtendedTileInfo"; } break;
			case 10: { return "System.PropList.FileOperationPrompt"; } break;
			case 11: { return "System.PropList.ConflictPrompt"; } break;
			case 13: { return "System.PropList.ContentViewModeForBrowse"; } break;
			case 14: { return "System.PropList.ContentViewModeForSearch"; } break;
			case 17: { return "System.InfoTipText"; } break;
			case 500: { return "System.LayoutPattern.ContentViewModeForBrowse"; } break;
			case 501: { return "System.LayoutPattern.ContentViewModeForSearch"; } break;
		}
	}
	else if ( memcmp( guid, "\x90\x1c\x69\x49\x17\x7e\x1A\x10\xA9\x1C\x08\x00\x2B\x2E\xCD\xA9", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 3: { return "System.Search.Rank"; } break;
			case 4: { return "System.Search.HitCount"; } break;
			case 5: { return "System.Search.EntryID"; } break;
			case 8: { return "System.Search.ReverseFileName"; } break;
			case 9: { return "System.ItemUrl"; } break;
			case 10: { return "System.ContentUrl"; } break;
		}
	}
	else if ( memcmp( guid, "\x40\xe8\x3e\x1e\x2b\xbc\x6c\x47\x82\x37\x2a\xcd\x1a\x83\x9b\x22", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 3: { return "System.Kind"; } break;
			case 6: { return "System.FullText"; } break;
		}
	}
	else if ( memcmp( guid, "\xb1\x16\x6d\x44\xAD\x8d\x70\x48\xA7\x48\x40\x2E\xA4\x3D\x78\x8C", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 100: { return "System.ThumbnailCacheId"; } break;
		}
	}
	else if ( memcmp( guid, "\x90\x1c\x69\x49\x17\x7e\x1A\x10\xA9\x1C\x08\x00\x2B\x2E\xCD\xA9", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 3: { return "System.Search.Rank"; } break;
			case 4: { return "System.Search.HitCount"; } break;
			case 5: { return "System.Search.EntryID"; } break;
			case 8: { return "System.Search.ReverseFileName"; } break;
			case 9: { return "System.ItemUrl"; } break;
			case 10: { return "System.ContentUrl"; } break;
		}
	}
	else if ( memcmp(guid, "\xed\x30\xbd\xDA\x43\x00\x89\x47\xA7\xF8\xD0\x13\xA4\x73\x66\x22", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 100: { return "System.ItemFolderPathDisplayNarrow"; } break;
		}
	}
	else if ( memcmp( guid, "\x8f\x04\x44\x64\x8B\x4c\xD1\x11\x8B\x70\x08\x00\x36\xB1\x1A\x03", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 3: { return "System.Image.HorizontalSize"; } break;
			case 4: { return "System.Image.VerticalSize"; } break;
			case 5: { return "System.Image.HorizontalResolution"; } break;
			case 6: { return "System.Image.VerticalResolution"; } break;
			case 7: { return "System.Image.BitDepth"; } break;
			case 12: { return "System.Media.FrameCount"; } break;
			case 13: { return "System.Image.Dimensions"; } break;
		}
	}
	else if ( memcmp( guid, "\x90\x04\x44\x64\x8B\x4c\xD1\x11\x8B\x70\x08\x00\x36\xB1\x1A\x03", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 2: { return "System.Audio.Format"; } break;
			case 3: { return "System.Media.Duration"; } break;
			case 4: { return "System.Audio.EncodingBitrate"; } break;
			case 5: { return "System.Audio.SampleRate"; } break;
			case 6: { return "System.Audio.SampleSize"; } break;
			case 7: { return "System.Audio.ChannelCount"; } break;
			case 8: { return "System.Audio.StreamNumber"; } break;
			case 9: { return "System.Audio.StreamName"; } break;
			case 10: { return "System.Audio.Compression"; } break;
		}
	}
	else if ( memcmp( guid, "\x91\x04\x44\x64\x8B\x4c\xD1\x11\x8B\x70\x08\x00\x36\xB1\x1A\x03", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 2: { return "System.Video.StreamName"; } break;
			case 3: { return "System.Video.FrameWidth"; } break;
			case 4: { return "System.Video.FrameHeight"; } break;
			case 6: { return "System.Video.FrameRate"; } break;
			case 8: { return "System.Video.EncodingBitrate"; } break;
			case 9: { return "System.Video.SampleSize"; } break;
			case 10: { return "System.Video.Compression"; } break;
			case 11: { return "System.Video.StreamNumber"; } break;
			case 42: { return "System.Video.HorizontalAspectRatio"; } break;
			case 43: { return "System.Video.TotalBitrate"; } break;
			case 44: { return "System.Video.FourCC"; } break;
			case 45: { return "System.Video.VerticalAspectRatio"; } break;
			case 46: { return "System.Video.TranscodedForSync"; } break;
		}
	}
	else if ( memcmp( guid, "\x91\x04\x44\x64\x8B\x4c\xD1\x11\x8B\x70\x08\x00\x36\xB1\x1A\x03", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 9: { return "System.Rating"; } break;
			case 11: { return "System.Copyright"; } break;
			case 12: { return "System.ShareUserRating"; } break;
			case 13: { return "System.Media.ClassPrimaryID"; } break;
			case 14: { return "System.Media.ClassSecondaryID"; } break;
			case 15: { return "System.Media.DVDID"; } break;
			case 16: { return "System.Media.MCDI"; } break;
			case 17: { return "System.Media.MetadataContentProvider"; } break;
			case 18: { return "System.Media.ContentDistributor"; } break;
			case 19: { return "System.Music.Composer"; } break;
			case 20: { return "System.Video.Director"; } break;
			case 21: { return "System.ParentalRating"; } break;
			case 22: { return "System.Media.Producer"; } break;
			case 23: { return "System.Media.Writer"; } break;
			case 24: { return "System.Media.CollectionGroupID"; } break;
			case 25: { return "System.Media.CollectionID"; } break;
			case 26: { return "System.Media.ContentID"; } break;
			case 27: { return "System.Media.CreatorApplication"; } break;
			case 28: { return "System.Media.CreatorApplicationVersion"; } break;
			case 30: { return "System.Media.Publisher"; } break;
			case 31: { return "System.Music.Period"; } break;
			case 32: { return "System.Media.AuthorUrl"; } break;
			case 33: { return "System.Media.PromotionUrl"; } break;
			case 34: { return "System.Media.UserWebUrl"; } break;
			case 35: { return "System.Media.UniqueFileIdentifier"; } break;
			case 36: { return "System.Media.EncodedBy"; } break;
			case 38: { return "System.Media.ProtectionType"; } break;
			case 39: { return "System.Media.ProviderRating"; } break;
			case 40: { return "System.Media.ProviderStyle"; } break;
			case 41: { return "System.Media.UserNoAutoInfo"; } break;
		}
	}
	else if ( memcmp( guid, "\xa1\x1d\xb8\x14\x35\x01\x31\x4d\x96\xD9\x6C\xBF\xC9\x67\x1A\x99", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 259: { return "System.Image.Compression"; } break;
			case 271: { return "System.Photo.CameraManufacturer"; } break;
			case 272: { return "System.Photo.CameraModel"; } break;
			case 273: { return "System.Photo.CameraSerialNumber"; } break;
			case 274: { return "System.Photo.Orientation"; } break;
			case 305: { return "System.SoftwareUsed"; } break;
			case 18248: { return "System.Photo.Event"; } break;
			case 18258: { return "System.DateImported"; } break;
			case 33434: { return "System.Photo.ExposureTime"; } break;
			case 33437: { return "System.Photo.FNumber"; } break;
			case 34850: { return "System.Photo.ExposureProgram"; } break;
			case 34855: { return "System.Photo.ISOSpeed"; } break;
			case 36867: { return "System.Photo.DateTaken"; } break;
			case 37377: { return "System.Photo.ShutterSpeed"; } break;
			case 37378: { return "System.Photo.Aperture"; } break;
			case 37380: { return "System.Photo.ExposureBias"; } break;
			case 37382: { return "System.Photo.SubjectDistance"; } break;
			case 37383: { return "System.Photo.MeteringMode"; } break;
			case 37384: { return "System.Photo.LightSource"; } break;
			case 37385: { return "System.Photo.Flash"; } break;
			case 37386: { return "System.Photo.FocalLength"; } break;
			case 40961: { return "System.Image.ColorSpace"; } break;
			case 41483: { return "System.Photo.FlashEnergy"; } break;
		}
	}
	else if ( memcmp( guid, "\x3c\x0a\xf1\xE4\xE6\x49\x5D\x40\x82\x88\xA2\x3B\xD4\xEE\xAA\x6C", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 100: { return "System.FileExtension"; } break;
		}
	}
	else if ( memcmp( guid, "\xb4\x74\xdb\xF7\x87\x42\x03\x41\xAF\xBA\xF1\xB1\x3D\xCD\x75\xCF", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 100: { return "System.ItemDate"; } break;
		}
	}	
	else if ( memcmp( guid, "\xe4\x19\xac\xAE\xAE\x89\x08\x45\xB9\xB7\xBB\x86\x7A\xBE\xE2\xED", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 2: { return "System.DRM.IsProtected"; } break;
		}
	}
	else if ( memcmp( guid, "\x4c\x58\xe0\xE3\x88\xb7\x5A\x4a\xBB\x20\x7F\x5A\x44\xC9\xAC\xDD", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 2: { return "System.Message.BccAddress"; } break;
			case 3: { return "System.Message.BccName"; } break;
			case 4: { return "System.Message.CcAddress"; } break;
			case 5: { return "System.Message.CcName"; } break;
			case 6: { return "System.ItemFolderPathDisplay"; } break;
			case 7: { return "System.ItemPathDisplay"; } break;
			case 9: { return "System.Communication.AccountName"; } break;
			case 10: { return "System.IsRead"; } break;
			case 11: { return "System.Importance"; } break;
			case 12: { return "System.FlagStatus"; } break;
			case 13: { return "System.Message.FromAddress"; } break;
			case 14: { return "System.Message.FromName"; } break;
			case 15: { return "System.Message.Store"; } break;
			case 16: { return "System.Message.ToAddress"; } break;
			case 17: { return "System.Message.ToName"; } break;
			case 18: { return "System.Contact.WebPage"; } break;
			case 19: { return "System.Message.DateSent"; } break;
			case 20: { return "System.Message.DateReceived"; } break;
			case 21: { return "System.Message.AttachmentNames"; } break;
		}
	}
	else if ( memcmp( guid, "\x53\x29\x12\xfd\x93\xfa\xf7\x4e\x92\xc3\x04\xc9\x46\xb2\xf7\xc8", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 100: { return "System.Music.DisplayArtist"; } break;
		}
	}
	else if ( memcmp( guid, "\x55\x28\x4c\x9F\x79\x9f\x39\x4b\xA8\xD0\xE1\xD4\x2D\xE1\xD5\xF3", 16 ) == 0 )
	{
		switch ( prop_id )
		{
			case 2: { return "System.AppUserModel.RelaunchCommand"; } break;
			case 3: { return "System.AppUserModel.RelaunchIconResource"; } break;
			case 4: { return "System.AppUserModel.RelaunchDisplayNameResource"; } break;
			case 5: { return "System.AppUserModel.ID"; } break;
			case 6: { return "System.AppUserModel.IsDestListSeparator"; } break;
			case 8: { return "System.AppUserModel.ExcludeFromShowInNewInstall"; } break;
			case 9: { return "System.AppUserModel.PreventPinning"; } break;
		}
	}
	
	return NULL;
}

void write_html( HANDLE hFile, wchar_t *buffer, unsigned int &offset )
{
	if ( offset > 0 )
	{
		DWORD written = 0;
		int write_length = WideCharToMultiByte( CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL );
		char *utf8_buf = ( char * )malloc( sizeof( char ) * write_length );	// Includes NULL character.
		WideCharToMultiByte( CP_UTF8, 0, buffer, -1, utf8_buf, write_length, NULL, NULL );
		WriteFile( hFile, utf8_buf, write_length - 1, &written, NULL );
		free( utf8_buf );
		offset = 0;
	}
}

void write_csv( HANDLE hFile, wchar_t *buffer, unsigned int &offset )
{
	if ( offset > 0 )
	{
		DWORD written = 0;
		int write_length = WideCharToMultiByte( CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL );
		char *utf8_buf = ( char * )malloc( sizeof( char ) * write_length );	// Includes NULL character.
		WideCharToMultiByte( CP_UTF8, 0, buffer, -1, utf8_buf, write_length, NULL, NULL );
		WriteFile( hFile, utf8_buf, write_length - 1, &written, NULL );
		free( utf8_buf );
		offset = 0;
	}
}

char *random_string()
{
	static const char rs[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	static char buf[ 11 ];
	static unsigned int ran_count = 0;

	srand( GetTickCount() + ran_count );
	ran_count = ++ran_count % 65535;

    for ( int i = 0; i < 10; ++i )
	{
        buf[ i ] = rs[ rand() % ( sizeof( rs ) - 1 ) ];
    }

    buf[ 10 ] = 0;

	return buf;
}

void write_html_dump( HANDLE hFile, wchar_t *buffer, unsigned int &offset, wchar_t *text, unsigned char *dump_buffer, unsigned int dump_length )
{
	char *rs = random_string();
	offset += swprintf( buffer + offset, BUFFER_SIZE - offset, L"%s <a href=\"\" onclick=\"t(this,\'%S\');return false;\">Show</a><div id=\"%S\" style=\"display:none\"><pre>", text, rs, rs );

	write_html( hFile, buffer, offset );

	unsigned int i = 0;
	int out_offset = 0;
	int hex_offset = 0;
	int text_offset = 0;
	wchar_t out_buf[ 8193 ];
	char hex_buf[ 50 ] = { 0 };
	char text_buf[ 65 ] = { 0 };	// We're going to escape < and > with &lt; and &gt; So the maximum size will be 4 times 16, if all characters are <.
	int write_length;
	char *utf8_buf;
	DWORD written = 0;

	for ( i = 0; i < dump_length; ++i )
	{
		if ( i > 0 && i % 8 == 0 )
		{
			if ( i % 16 == 0 )
			{
				out_offset += swprintf_s( out_buf + out_offset, 8193 - out_offset, ( i > 16 ? L"\n%S\t%S" : L"%S\t%S" ), hex_buf, text_buf );
				if ( out_offset >= 4096 )
				{
					write_length = WideCharToMultiByte( CP_UTF8, 0, out_buf, -1, NULL, 0, NULL, NULL );
					utf8_buf = ( char * )malloc( sizeof( char ) * write_length );	// Includes NULL character.
					WideCharToMultiByte( CP_UTF8, 0, out_buf, -1, utf8_buf, write_length, NULL, NULL );
					WriteFile( hFile, utf8_buf, write_length - 1, &written, NULL );
					free( utf8_buf );
					out_offset = 0;
				}

				hex_offset = text_offset = 0;
			}
			else
			{
				hex_buf[ hex_offset++ ] = ' ';
			}
		}

		hex_offset += sprintf_s( hex_buf + hex_offset, 50 - hex_offset, "%.02x ", dump_buffer[ i ] );
		if ( dump_buffer[ i ] == '<' )
		{
			text_offset += sprintf_s( text_buf + text_offset, 65 - text_offset, "&lt;" );
		}
		else if ( dump_buffer[ i ] == '>' )
		{
			text_offset += sprintf_s( text_buf + text_offset, 65 - text_offset, "&gt;" );
		}
		else if ( dump_buffer[ i ] == '&' )
		{
			text_offset += sprintf_s( text_buf + text_offset, 65 - text_offset, "&amp;" );
		}
		else
		{
			text_offset += sprintf_s( text_buf + text_offset, 65 - text_offset, "%c", ( dump_buffer[ i ] < 0x21 || ( dump_buffer[ i ] >= 0x7f && dump_buffer[ i ] <= 0x9f ) ) ? '.' : dump_buffer[ i ] );
		}
	}

	// Dump whatever we didn't write to out_buf.
	if ( out_offset > 0 )
	{
		write_length = WideCharToMultiByte( CP_UTF8, 0, out_buf, -1, NULL, 0, NULL, NULL );
		utf8_buf = ( char * )malloc( sizeof( char ) * write_length );	// Includes NULL character.
		WideCharToMultiByte( CP_UTF8, 0, out_buf, -1, utf8_buf, write_length, NULL, NULL );
		WriteFile( hFile, utf8_buf, write_length - 1, &written, NULL );
		free( utf8_buf );
		out_offset = 0;
	}

	// Write to out_buf what we have left in hex_buf and text_buf.
	if ( i > 0 && ( ( i % 8 != 0 ) || ( i % 16 == 8 ) ) ) // 15 or less characters filled.
	{
		out_offset += swprintf_s( out_buf + out_offset, 8193 - out_offset, L"\n%S", hex_buf );
		unsigned char r = ( i % 16 >= 8 ? 8 : 16 ) - ( i % 8 );
		while ( r-- )
		{
			out_offset += swprintf_s( out_buf + out_offset, 8193 - out_offset, L"   " );
		}
		out_offset += swprintf_s( out_buf + out_offset, 8193 - out_offset, L"\t%S", text_buf );
	}
	else	// 16 characters filled.
	{
		out_offset += swprintf_s( out_buf + out_offset, 8193 - out_offset, L"\n%S\t%S", hex_buf, text_buf );
	}

	write_length = WideCharToMultiByte( CP_UTF8, 0, out_buf, -1, NULL, 0, NULL, NULL );
	utf8_buf = ( char * )malloc( sizeof( char ) * write_length );	// Includes NULL character.
	WideCharToMultiByte( CP_UTF8, 0, out_buf, -1, utf8_buf, write_length, NULL, NULL );
	WriteFile( hFile, utf8_buf, write_length - 1, &written, NULL );
	free( utf8_buf );

	offset += swprintf( buffer + offset, BUFFER_SIZE - offset, L"</pre></div><br />" );
}

wchar_t *escape_html_unicode( wchar_t *buf )
{
	// If a string consists of all & then each will be escaped as such: &amp; In total, we have a string that's 5 times the original length plus the NULL character.
	// Let's see if there's actually a character before we allocate memory.
	if ( wcschr( buf, L'<' ) != NULL || wcschr( buf, L'>' ) != NULL || wcschr( buf, L'&' ) != NULL )
	{
		unsigned int string_length = ( 5 * wcslen( buf ) ) + 1;
		wchar_t *out = ( wchar_t * )malloc( sizeof( wchar_t ) * string_length );
		unsigned int out_offset = 0;

		while ( *buf != NULL )
		{
			if ( *buf == L'<' )
			{
				wmemcpy_s( out + out_offset, string_length - out_offset, L"&lt;", 4 );
				out_offset += 4;
			}
			else if ( *buf == L'>' )
			{
				wmemcpy_s( out + out_offset, string_length - out_offset, L"&gt;", 4 );
				out_offset += 4;
			}
			else if ( *buf == L'&' )
			{
				wmemcpy_s( out + out_offset, string_length - out_offset, L"&amp;", 5 );
				out_offset += 5;
			}
			else
			{
				out[ out_offset++ ] = *buf;
			}

			++buf;	
		}

		out[ out_offset ] = L'\0';

		return out;
	}

	return NULL;
}

char *escape_html_ascii( char *buf )
{
	// If a string consists of all & then each will be escaped as such: &amp; In total, we have a string that's 5 times the original length plus the NULL character.
	// Let's see if there's actually a character before we allocate memory.
	if ( strchr( buf, L'<' ) != NULL || strchr( buf, L'>' ) != NULL || strchr( buf, L'&' ) != NULL )
	{
		unsigned int string_length = ( 5 * strlen( buf ) ) + 1;
		char *out = ( char * )malloc( sizeof( char ) * string_length );
		unsigned int out_offset = 0;

		while ( *buf != NULL )
		{
			if ( *buf == '<' )
			{
				memcpy_s( out + out_offset, string_length - out_offset, "&lt;", 4 );
				out_offset += 4;
			}
			else if ( *buf == '>' )
			{
				memcpy_s( out + out_offset, string_length - out_offset, "&gt;", 4 );
				out_offset += 4;
			}
			else if ( *buf == '&' )
			{
				memcpy_s( out + out_offset, string_length - out_offset, "&amp;", 5 );
				out_offset += 5;
			}
			else
			{
				out[ out_offset++ ] = *buf;
			}

			++buf;	
		}

		out[ out_offset ] = '\0';

		return out;
	}

	return NULL;
}

wchar_t *escape_string_unicode( wchar_t *buf )
{
	// If a string consists of all " then each will be escaped as such: "" In total, we have a string that's 2 times the original length plus the NULL character.
	unsigned int string_length = 0;
	wchar_t *out = NULL;
	unsigned int out_offset = 0;

	wchar_t *last_pos = buf;
	wchar_t *cur_pos = wcschr( buf, L'\"' );

	// Let's see if there's actually a character before we allocate memory.
	if ( cur_pos != NULL )
	{
		string_length = ( 2 * wcslen( buf ) ) + 1;
		out = ( wchar_t * )malloc( sizeof( wchar_t ) * string_length );

		while ( cur_pos != NULL )
		{
			unsigned int run_length = cur_pos - last_pos;
			wmemcpy_s( out + out_offset, string_length - out_offset, last_pos, run_length );
			out_offset += run_length;
			wmemcpy_s( out + out_offset, string_length - out_offset, L"\"\"", 2 );
			out_offset += 2;

			last_pos = cur_pos + 1;
			cur_pos = wcschr( last_pos, '\"' );
		}

		swprintf_s( out + out_offset, string_length - out_offset, L"%s", last_pos );
	}

	return out;
}

char *escape_string_ascii( char *buf )
{
	// If a string consists of all " then each will be escaped as such: "" In total, we have a string that's 2 times the original length plus the NULL character.
	unsigned int string_length = 0;
	char *out = NULL;
	unsigned int out_offset = 0;

	char *last_pos = buf;
	char *cur_pos = strchr( buf, '\"' );

	// Let's see if there's actually a character before we allocate memory.
	if ( cur_pos != NULL )
	{
		string_length = ( 2 * strlen( buf ) ) + 1;
		out = ( char * )malloc( sizeof( char ) * string_length );

		while ( cur_pos != NULL )
		{
			unsigned int run_length = cur_pos - last_pos;
			memcpy_s( out + out_offset, string_length - out_offset, last_pos, run_length );
			out_offset += run_length;
			memcpy_s( out + out_offset, string_length - out_offset, "\"\"", 2 );
			out_offset += 2;

			last_pos = cur_pos + 1;
			cur_pos = strchr( last_pos, '\"' );
		}

		sprintf_s( out + out_offset, string_length - out_offset, "%s", last_pos );
	}

	return out;
}
