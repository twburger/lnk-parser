#ifndef _GLOBALS_H
#define _GLOBALS_H

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
//#include <shellapi.h>
#include <shlobj.h>

#pragma region DEFINES

#define BUFFER_SIZE			32768	// Total size of our output buffer.
#define BUFFER_CHUNK_SIZE	10240	// The limit at which we'll write the buffer to a file.

// (ShellLinkHeader) LinkFlags
#define HasLinkTargetIDList			0x00000001	// The shell link is saved with an item ID list (IDList).
#define HasLinkInfo					0x00000002	// The shell link is saved with link information.
#define HasName						0x00000004	// The shell link is saved with a name string.
#define HasRelativePath				0x00000008	// The shell link is saved with a relative path string.
#define HasWorkingDir				0x00000010	// The shell link is saved with a working directory string.
#define HasArguments				0x00000020	// The shell link is saved with command line arguments.
#define HasIconLocation				0x00000040	// The shell link is saved with an icon location string.
#define IsUnicode					0x00000080	// The shell link contains Unicode encoded strings. This bit SHOULD be set.
#define ForceNoLinkInfo				0x00000100	// The LinkInfo structure is ignored.
#define HasExpString				0x00000200	// The shell link is saved with an EnvironmentVariableDataBlock.
#define RunInSeparateProcess		0x00000400	// The target is run in a separate virtual machine when launching a link target that is a 16-bit application.
#define Unused1						0x00000800
#define HasDarwinID					0x00001000	// The shell link is saved with a DarwinDataBlock.
#define RunAsUser					0x00002000	// The application is run as a different user when the target of the shell link is activated.
#define HasExpIcon					0x00004000	// The shell link is saved with an IconEnvironmentDataBlock.
#define NoPidlAlias					0x00008000	// The file system location is represented in the shell namespace when the path to an item is parsed into an IDList.
#define Unused2						0x00010000
#define RunWithShimLayer			0x00020000	// The shell link is saved with a ShimDataBlock.
#define ForceNoLinkTrack			0x00040000	// The TrackerDataBlock is ignored.
#define EnableTargetMetadata		0x00080000	// The shell link attempts to collect target properties and store them in the PropertyStoreDataBlock when the link target is set.
#define DisableLinkPathTracking		0x00100000	// The EnvironmentVariableDataBlock is ignored.
#define DisableKnownFolderTracking	0x00200000	// The SpecialFolderDataBlock and the KnownFolderDataBlock are ignored when loading the shell link.
#define DisableKnownFolderAlias		0x00400000	// If the link has a KnownFolderDataBlock (section 2.5.6), the unaliased form of the known folder IDList SHOULD be used when translating the target IDList at the time that the link is loaded.
#define AllowLinkToLink				0x00800000	// Creating a link that references another link is enabled. Otherwise, specifying a link as the target IDList SHOULD NOT be allowed.
#define UnaliasOnSave				0x01000000	// When saving a link for which the target IDList is under a known folder, either the unaliased form of that known folder or the target IDList SHOULD be used.
#define PreferEnvironmentPath		0x02000000	// The target IDList SHOULD NOT be stored; instead, the path specified in the EnvironmentVariableDataBlock SHOULD be used to refer to the target.
#define KeepLocalIDListForUNCTarget	0x04000000	// When the target is a UNC name that refers to a location on a local machine, the local path IDList in the PropertyStoreDataBlock SHOULD be stored, so it can be used when the link is loaded on the local machine.

// (LinkInfo) LinkInfoFlags
#define VolumeIDAndLocalBasePath				0x00000001	// If set, the VolumeID and LocalBasePath fields are present, and their locations are specified by the values of the VolumeIDOffset and LocalBasePathOffset fields, respectively.
															// If the value of the LinkInfoHeaderSize field is greater than or equal to 0x00000024, the LocalBasePathUnicode field is present, and its location is specified by the value of the LocalBasePathOffsetUnicode field.
															// If not set, the VolumeID, LocalBasePath, and LocalBasePathUnicode fields are not present, and the values of the VolumeIDOffset and LocalBasePathOffset fields are zero.
															// If the value of the LinkInfoHeaderSize field is greater than or equal to 0x00000024, the value of the LocalBasePathOffsetUnicode field is zero.
#define CommonNetworkRelativeLinkAndPathSuffix	0x00000002	// If set, the CommonNetworkRelativeLink field is present, and its location is specified by the value of the CommonNetworkRelativeLinkOffset field. 
															// If not set, the CommonNetworkRelativeLink field is not present, and the value of the CommonNetworkRelativeLinkOffset field is zero.

// (CommonNetworkRelativeLink) CommonNetworkRelativeLinkFlags
#define ValidDevice		0x00000001	// If set, the DeviceNameOffset field contains an offset to the device name.
#define ValidNetType	0x00000002	// If set, the NetProviderType field contains the network provider type.

// Hot key modifier flags.
#define HOTKEYF_SHIFT	0x01
#define HOTKEYF_CONTROL	0x02
#define HOTKEYF_ALT		0x04
#define HOTKEYF_EXT		0x08

// EXTRA_DATA signatures
#define ENVIRONMENT_PROPS				0xA0000001
#define CONSOLE_PROPS					0xA0000002
#define TRACKER_PROPS					0xA0000003
#define CONSOLE_FE_PROPS				0xA0000004
#define SPECIAL_FOLDER_PROPS			0xA0000005
#define DARWIN_PROPS					0xA0000006
#define ICON_ENVIRONMENT_PROPS			0xA0000007
#define SHIM_PROPS						0xA0000008
#define PROPERTY_STORE_PROPS			0xA0000009
#define KNOWN_FOLDER_PROPS				0xA000000B
#define VISTA_AND_ABOVE_IDLIST_PROPS	0xA000000C

// Special Folder types
#define SF_Desktop					0x0000	// Desktop
#define SF_Internet					0x0001	// Internet Explorer (icon on desktop)
#define SF_Programs					0x0002	// Start Menu\Programs
#define SF_Controls					0x0003	// My Computer\Control Panel
#define SF_Printers					0x0004	// My Computer\Printers
#define SF_Personal					0x0005	// My Documents
#define SF_Favorites				0x0006	// user name\Favorites
#define SF_Startup					0x0007	// Start Menu\Programs\Startup
#define SF_Recent					0x0008	// user name\Recent
#define SF_SendTo					0x0009	// user name\SendTo
#define SF_BitBucket				0x000a	// desktop\Recycle Bin
#define SF_StartMenu				0x000b	// user name\Start Menu
#define SF_MyDocuments				0x000c	// logical "My Documents" desktop icon
#define SF_MyMusic					0x000d	// "My Music" folder
#define SF_MyVideo					0x000e	// "My Videos" folder
#define SF_DesktopDirectory			0x0010	// user name\Desktop
#define SF_Drives					0x0011	// My Computer
#define SF_Network					0x0012	// Network Neighborhood (My Network Places)
#define SF_Nethood					0x0013	// user name\nethood
#define SF_Fonts					0x0014	// windows\fonts
#define SF_Templates				0x0015	
#define SF_CommonStartMenu			0x0016	// All Users\Start Menu
#define SF_CommonPrograms			0x0017	// All Users\Start Menu\Programs
#define SF_CommonStartup			0x0018	// All Users\Startup
#define SF_CommonDesktopDirectory	0x0019	// All Users\Desktop
#define SF_AppData					0x001a	// user name\Application Data
#define SF_PrintHood				0x001b	// user name\PrintHood
#define SF_LocalAppData				0x001c	// user name\Local Settings\Applicaiton Data (non roaming)
#define SF_AltStartup				0x001d	// non localized startup
#define SF_CommonAltStartup			0x001e	// non localized common startup
#define SF_CommonFavorites			0x001f	
#define SF_InternetCache			0x0020	
#define SF_Cookies					0x0021	
#define SF_History					0x0022	
#define SF_CommonAppData			0x0023	// All Users\Application Data
#define SF_Windows					0x0024	// GetWindowsDirectory()
#define SF_System					0x0025	// GetSystemDirectory()
#define SF_ProgramFiles				0x0026	// C:\Program Files
#define SF_MyPictures				0x0027	// C:\Program Files\My Pictures
#define SF_Profile					0x0028	// USERPROFILE
#define SF_SystemX86				0x0029	// x86 system directory on RISC
#define SF_ProgramFilesX86			0x002a	// x86 C:\Program Files on RISC
#define SF_ProgramFilesCommon		0x002b	// C:\Program Files\Common
#define SF_ProgramFilesCommonX86	0x002c	// x86 Program Files\Common on RISC
#define SF_CommonTemplates			0x002d	// All Users\Templates
#define SF_CommonDocuments			0x002e	// All Users\Documents
#define SF_CommonAdminTools			0x002f	// All Users\Start Menu\Programs\Administrative Tools
#define SF_AdminTools				0x0030	// user name\Start Menu\Programs\Administrative Tools
#define SF_Connections				0x0031	// Network and Dial-up Connections
#define SF_CommonMusic				0x0035	// All Users\My Music
#define SF_CommonPictures			0x0036	// All Users\My Pictures
#define SF_CommonVideo				0x0037	// All Users\My Video
#define SF_Resources				0x0038	// Resource Direcotry
#define SF_ResourcesLocalized		0x0039	// Localized Resource Direcotry
#define SF_CommonOEMLinks			0x003a	// Links to All Users OEM specific apps
#define SF_CDBurnArea				0x003b	// USERPROFILE\Local Settings\Application Data\Microsoft\CD Burning
#define SF_ComputersNearMe			0x003d	// Computers Near Me (computered from Workgroup membership)
#define SF_FlagCreate				0x8000	// combine with CSIDL_ value to force folder creation in SHGetFolderPath()
#define SF_FlagDontVerify			0x4000	// combine with CSIDL_ value to return an unverified folder path
#define SF_FlagNoAlias				0x1000	// combine with CSIDL_ value to insure non-alias versions of the pidl
#define SF_FlagPerUserInit			0x0800	// combine with CSIDL_ value to indicate per-user init (eg. upgrade)
#define SF_FlagMask					0xFF00	// mask for all possible flag values


// KNOWNFOLDERID
#define GUID_AddNewPrograms			"\x71\xd9\x61\xde\xbc\x5e\x02\x4f\xa3\xa9\x6c\x82\x89\x5e\x5c\x04"
#define GUID_AdminTools				"\x70\xf1\x4e\x72\x2d\xa4\xef\x4f\x9f\x26\xb6\x0e\x84\x6f\xba\x4f"
#define GUID_AppDataLow				"\xa4\xa1\x20\xa5\x80\x17\xf6\x4f\xbd\x18\x16\x73\x43\xc5\xaf\x16"
#define GUID_ApplicationShortcuts	"\x81\x87\x91\xA3\xF2\xe5\x90\x48\xB3\xD9\xA7\xE5\x43\x32\x32\x8C"
#define GUID_AppsFolder				"\x8d\x50\x87\x1e\xc2\x89\xf0\x42\x8a\x7e\x64\x5a\x0f\x50\xca\x58"
#define GUID_AppUpdates				"\x99\xce\x05\xa3\x27\xf5\x2b\x49\x8b\x1a\x7e\x76\xfa\x98\xd6\xe4"
#define GUID_CDBurning				"\x10\xab\x52\x9e\x0d\xf8\xdf\x49\xac\xb8\x43\x30\xf5\x68\x78\x55"
#define GUID_ChangeRemovePrograms	"\xac\x66\x72\xdf\x74\x92\x67\x48\x8d\x55\x3b\xd6\x61\xde\x87\x2d"
#define GUID_CommonAdminTools		"\x7d\x4e\x38\xd0\xc3\xba\x97\x47\x8f\x14\xcb\xa2\x29\xb3\x92\xb5"
#define GUID_CommonOEMLinks			"\xd0\xe2\xba\xc1\xdf\x10\x34\x43\xbe\xdd\x7a\xa2\x0b\x22\x7a\x9d"
#define GUID_CommonPrograms			"\x4e\xd4\x39\x01\xfe\x6a\xf2\x49\x86\x90\x3d\xaf\xca\xe6\xff\xb8"
#define GUID_CommonStartMenu		"\x19\x57\x11\xa4\x2e\xd6\x1d\x49\xaa\x7c\xe7\x4b\x8b\xe3\xb0\x67"
#define GUID_CommonStartup			"\x35\xea\xa5\x82\xcd\xd9\xc5\x47\x96\x29\xe1\x5d\x2f\x71\x4e\x6e"
#define GUID_CommonTemplates		"\xe7\x37\x42\xb9\xac\x57\x47\x43\x91\x51\xb0\x8c\x6c\x32\xd1\xf7"
#define GUID_ComputerFolder			"\x7c\x83\xc0\x0a\xf8\xbb\x2a\x45\x85\x0d\x79\xd0\x8e\x66\x7c\xa7"
#define GUID_ConflictFolder			"\x45\xfb\xfe\x4b\x7d\x34\x06\x40\xa5\xbe\xac\x0c\xb0\x56\x71\x92"
#define GUID_ConnectionsFolder		"\x2b\xd9\x0c\x6f\x97\x2e\xd1\x45\x88\xff\xb0\xd1\x86\xb8\xde\xdd"
#define GUID_Contacts				"\x54\x48\x78\x56\xcb\xc6\x2b\x46\x81\x69\x88\xe3\x50\xac\xb8\x82"
#define GUID_ControlPanelFolder		"\xeb\x4a\xa7\x82\xb4\xae\x5c\x46\xa0\x14\xd0\x97\xee\x34\x6d\x63"
#define GUID_Cookies				"\x5d\x76\x0f\x2b\xe9\xc0\x71\x41\x90\x8e\x08\xa6\x11\xb8\x4f\xf6"
#define GUID_Desktop				"\x3a\xcc\xbf\xb4\x2c\xdb\x4c\x42\xb0\x29\x7f\xe9\x9a\x87\xc6\x41"
#define GUID_DeviceMetadataStore	"\xe9\xa5\xe4\x5C\xEB\xe4\x9D\x47\xB8\x9F\x13\x0C\x02\x88\x61\x55"
#define GUID_Documents				"\xd0\x9a\xd3\xfd\x8f\x23\xaf\x46\xad\xb4\x6c\x85\x48\x03\x69\xc7"
#define GUID_DocumentsLibrary		"\x7d\xb1\x0d\x7B\xD2\x9c\x93\x4a\x97\x33\x46\xCC\x89\x02\x2E\x7C"
#define GUID_Downloads				"\x90\xe2\x4d\x37\x3f\x12\x65\x45\x91\x64\x39\xc4\x92\x5e\x46\x7b"
#define GUID_Favorites				"\x61\xf7\x77\x17\xad\x68\x8a\x4d\x87\xbd\x30\xb7\x59\xfa\x33\xdd"
#define GUID_Fonts					"\xb7\x8c\x22\xfd\x11\xae\xe3\x4a\x86\x4c\x16\xf3\x91\x0a\xb8\xfe"
#define GUID_Games					"\x1a\x2c\xc5\xca\x3d\xb5\xdc\x4e\x92\xd7\x6b\x2e\x8a\xc1\x94\x34"
#define GUID_GameTasks				"\x61\xae\x4f\x05\xd8\x4d\x87\x47\x80\xb6\x09\x02\x20\xc4\xb7\x00"
#define GUID_History				"\x3b\x8a\xdc\xd9\x84\xb7\x2e\x43\xa7\x81\x5a\x11\x30\xa7\x59\x63"
#define GUID_HomeGroup				"\x6b\x8a\x52\x52\xE3\xb9\xDD\x4a\xB6\x0D\x58\x8C\x2D\xBA\x84\x2D"
#define GUID_HomeGroupCurrentUser	"\xa3\xb6\x74\x9B\xFD\x0d\x11\x4f\x9E\x78\x5F\x78\x00\xF2\xE7\x72"
#define GUID_ImplicitAppShortcuts	"\x6f\x25\xb5\xBC\xF6\x79\xEE\x4c\xB7\x25\xDC\x34\xE4\x02\xFD\x46"
#define GUID_InternetCache			"\xe8\x81\x24\x35\xbe\x33\x51\x42\xba\x85\x60\x07\xca\xed\xcf\x9d"
#define GUID_InternetFolder			"\x74\x78\x9f\x4d\x0c\x4e\x04\x49\x96\x7b\x40\xb0\xd2\x0c\x3e\x4b"
#define GUID_Libraries				"\xdc\xa5\x3e\x1B\x87\xb5\x86\x47\xB4\xEF\xBD\x1D\xC3\x32\xAE\xAE"
#define GUID_Links					"\xe0\xd5\xb9\xbf\xa9\xc6\x4c\x40\xb2\xb2\xae\x6d\xb6\xaf\x49\x68"
#define GUID_LocalAppData			"\x85\x27\xb3\xf1\xba\x6f\xcf\x4f\x9d\x55\x7b\x8e\x7f\x15\x70\x91"
#define GUID_LocalAppDataLow		"\xa4\xa1\x20\xA5\x80\x17\xF6\x4f\xBD\x18\x16\x73\x43\xC5\xAF\x16"
#define GUID_LocalizedResourcesDir	"\x5e\x37\x00\x2a\x4c\x22\xde\x49\xb8\xd1\x44\x0d\xf7\xef\x3d\xdc"
#define GUID_Music					"\x71\xd5\xd8\x4b\x19\x6d\xd3\x48\xbe\x97\x42\x22\x20\x08\x0e\x43"
#define GUID_MusicLibrary			"\x0a\xab\x12\x21\x6A\xc8\xFE\x4f\xA3\x68\x0D\xE9\x6E\x47\x01\x2E"
#define GUID_NetHood				"\x53\xbf\xab\xc5\x7f\xe1\x21\x41\x89\x00\x86\x62\x6f\xc2\xc9\x73"
#define GUID_NetworkFolder			"\xc4\xee\x0b\xd2\xa8\x5c\x05\x49\xae\x3b\xbf\x25\x1e\xa0\x9b\x53"
#define GUID_OriginalImages			"\xaa\xc0\x36\x2c\x12\x58\x87\x4b\xbf\xd0\x4c\xd0\xdf\xb1\x9b\x39"
#define GUID_PhotoAlbums			"\x90\xcf\xd2\x69\x33\xfc\xb7\x4f\x9a\x0c\xeb\xb0\xf0\xfc\xb4\x3c"
#define GUID_Pictures				"\x30\x81\xe2\x33\x1e\x4e\x76\x46\x83\x5a\x98\x39\x5c\x3b\xc3\xbb"
#define GUID_PicturesLibrary		"\x9f\xae\x90\xA9\x3B\xa0\x80\x4e\x94\xBC\x99\x12\xD7\x50\x41\x04"
#define GUID_Playlists				"\xc7\xc1\x92\xde\x7f\x83\x69\x4f\xa3\xbb\x86\xe6\x31\x20\x4a\x23"
#define GUID_PrintersFolder			"\x2d\x4e\xfc\x76\xad\xd6\x19\x45\xa6\x63\x37\xbd\x56\x06\x81\x85"
#define GUID_PrintHood				"\x8d\xbd\x74\x92\xd1\xcf\xc3\x41\xb3\x5e\xb1\x3f\x55\xa7\x58\xf4"
#define GUID_Profile				"\x8f\x85\x6c\x5e\x22\x0e\x60\x47\x9a\xfe\xea\x33\x17\xb6\x71\x73"
#define GUID_ProgramData			"\x82\x5d\xab\x62\xc1\xfd\xc3\x4d\xa9\xdd\x07\x0d\x1d\x49\x5d\x97"
#define GUID_ProgramFiles			"\xb6\x63\x5e\x90\xbf\xc1\x4e\x49\xb2\x9c\x65\xb7\x32\xd3\xd2\x1a"
#define GUID_ProgramFilesCommon		"\x05\xed\xf1\xf7\x6d\x9f\xa2\x47\xaa\xae\x29\xd3\x17\xc6\xf0\x66"
#define GUID_ProgramFilesCommonX64	"\xa7\xd5\x65\x63\x0d\x0f\xe5\x45\x87\xf6\x0d\xa5\x6b\x6a\x4f\x7d"
#define GUID_ProgramFilesCommonX86	"\x24\x4d\x97\xde\xc6\xd9\x3e\x4d\xbf\x91\xf4\x45\x51\x20\xb9\x17"
#define GUID_ProgramFilesX64		"\x77\x93\x80\x6d\xf0\x6a\x4b\x44\x89\x57\xa3\x77\x3f\x02\x20\x0e"
#define GUID_ProgramFilesX86		"\xef\x40\x5a\x7c\xfb\xa0\xfc\x4b\x87\x4a\xc0\xf2\xe0\xb9\xfa\x8e"
#define GUID_Programs				"\x77\x5d\x7f\xa7\x2b\x2e\xc3\x44\xa6\xa2\xab\xa6\x01\x05\x4a\x51"
#define GUID_Public					"\xa2\x76\xdf\xdf\x2a\xc8\x63\x4d\x90\x6a\x56\x44\xac\x45\x73\x85"
#define GUID_PublicDesktop			"\x0d\x34\xaa\xc4\x0f\xf2\x63\x48\xaf\xef\xf8\x7e\xf2\xe6\xba\x25"
#define GUID_PublicDocuments		"\xaf\x24\x48\xed\xe4\xdc\xa8\x45\x81\xe2\xfc\x79\x65\x08\x36\x34"
#define GUID_PublicDownloads		"\x9b\x4c\x64\x3d\xb8\x1f\x30\x4f\x9b\x45\xf6\x70\x23\x5f\x79\xc0"
#define GUID_PublicGameTasks		"\x36\x25\xbf\xde\xa8\xe1\x59\x4c\xb6\xa2\x41\x45\x86\x47\x6a\xea"
#define GUID_PublicLibraries		"\x0b\xf8\xda\x48\xCF\xe6\x4E\x4f\xB8\x00\x0E\x69\xD8\x4E\xE3\x84"
#define GUID_PublicMusic			"\xb5\xfa\x14\x32\x57\x97\x98\x42\xbb\x61\x92\xa9\xde\xaa\x44\xff"
#define GUID_PublicPictures			"\x86\xfb\xeb\xb6\x07\x69\x3c\x41\x9a\xf7\x4f\xc2\xab\xf0\x7c\xc5"
#define GUID_PublicRingtones		"\x60\xab\x55\xE5\x3B\x15\x17\x4d\x9F\x04\xA5\xFE\x99\xFC\x15\xEC"
#define GUID_PublicUserTiles		"\x6c\xaf\x82\x04\xf1\x08\x34\x4c\x8c\x90\xe1\x7e\xc9\x8b\x1e\x17"
#define GUID_PublicVideos			"\x3a\x18\x00\x24\x85\x61\xfb\x49\xa2\xd8\x4a\x39\x2a\x60\x2b\xa3"
#define GUID_QuickLaunch			"\x21\xf0\xa4\x52\x75\x7b\xa9\x48\x9f\x6b\x4b\x87\xa2\x10\xbc\x8f"
#define GUID_Recent					"\x81\xc0\x50\xae\xd2\xeb\x8a\x43\x86\x55\x8a\x09\x2e\x34\x98\x7a"
#define GUID_RecordedTV				"\x01\xe0\x85\xbd\x2e\x11\x1e\x43\x98\x3b\x7b\x15\xac\x09\xff\xf1"
#define GUID_RecordedTVLibrary		"\xa2\xdb\x6f\x1A\x2D\xf4\x58\x43\xA7\x98\xB7\x4D\x74\x59\x26\xC5"
#define GUID_RecycleBin				"\x46\x40\x53\xb7\xcb\x3e\x18\x4c\xbe\x4e\x64\xcd\x4c\xb7\xd6\xac"
#define GUID_ResourceDir			"\x31\x0c\xd1\x8a\xdb\x2a\x96\x42\xa8\xf7\xe4\x70\x12\x32\xc9\x72"
#define GUID_Ringtones				"\x4b\x04\x70\xC8\x9E\xf4\x26\x41\xA9\xC3\xB5\x2A\x1F\xF4\x11\xE8"
#define GUID_RoamingAppData			"\xdb\x85\xb6\x3e\xf9\x65\xf6\x4c\xa0\x3a\xe3\xef\x65\x72\x9f\x3d"
#define GUID_RoamingTiles			"\x5a\xfc\xbc\x00\x94\xed\x48\x4e\x96\xA1\x3F\x62\x17\xF2\x19\x90"
#define GUID_SampleMusic			"\x68\xc6\x50\xb2\x7d\xf5\xe1\x4e\xa6\x3c\x29\x0e\xe7\xd1\xaa\x1f"
#define GUID_SamplePictures			"\x40\x05\x90\xc4\x79\x23\x75\x4c\x84\x4b\x64\xe6\xfa\xf8\x71\x6b"
#define GUID_SamplePlaylists		"\xb3\x69\xca\x15\xee\x30\xc1\x49\xac\xe1\x6b\x5e\xc3\x72\xaf\xb5"
#define GUID_SampleVideos			"\x94\xad\x9e\x85\x85\x2e\xad\x48\xa7\x1a\x09\x69\xcb\x56\xa6\xcd"
#define GUID_SavedGames				"\xff\x32\x5c\x4c\x9d\xbb\xb0\x43\xb5\xb4\x2d\x72\xe5\x4e\xaa\xa4"
#define GUID_SavedSearches			"\x04\x3a\x1d\x7d\xbb\xde\x15\x41\x95\xcf\x2f\x29\xda\x29\x20\xda"
#define GUID_SEARCH_CSC				"\x46\xe4\x32\xee\xca\x31\xba\x4a\x81\x4f\xa5\xeb\xd2\xfd\x6d\x5e"
#define GUID_SEARCH_MAPI			"\x18\x0e\xec\x98\x98\x20\x44\x4d\x86\x44\x66\x97\x93\x15\xa2\x81"
#define GUID_SearchHome				"\xd1\x37\x03\x19\xca\xb8\x21\x41\xa6\x39\x6d\x47\x2d\x16\x97\x2a"
#define GUID_SendTo					"\x6c\x03\x83\x89\xc0\x27\x4b\x40\x8f\x08\x10\x2d\x10\xdc\xfd\x74"
#define GUID_SidebarDefaultParts	"\x54\x6e\x39\x7b\xc5\x9e\x00\x43\xbe\x0a\x24\x82\xeb\xae\x1a\x26"
#define GUID_SidebarParts			"\x2e\x36\x5d\xa7\xfc\x50\xb7\x4f\xac\x2c\xa8\xbe\xaa\x31\x44\x93"
#define GUID_StartMenu				"\xc3\x53\x5b\x62\x48\xab\xc1\x4e\xba\x1f\xa1\xef\x41\x46\xfc\x19"
#define GUID_Startup				"\xbb\x20\x7d\xb9\x6a\xf4\x97\x4c\xba\x10\x5e\x36\x08\x43\x08\x54"
#define GUID_SyncManagerFolder		"\xf8\x8b\x66\x43\x4e\xc1\xb2\x49\x97\xc9\x74\x77\x84\xd7\x84\xb7"
#define GUID_SyncResults			"\x43\x9a\x9a\x28\x44\xbe\x57\x40\xa4\x1b\x58\x7a\x76\xd7\xe7\xf9"
#define GUID_SyncSetupFolder		"\x38\x41\x21\x0f\xd3\xb1\x90\x4a\xbb\xa9\x27\xcb\xc0\xc5\x38\x9a"
#define GUID_System					"\x77\x4e\xc1\x1a\xe7\x02\x5d\x4e\xb7\x44\x2e\xb1\xae\x51\x98\xb7"
#define GUID_SystemX86				"\xb0\x31\x52\xd6\xf1\xb2\x57\x48\xa4\xce\xa8\xe7\xc6\xea\x7d\x27"
#define GUID_Templates				"\xe8\x93\x32\xa6\x4e\x66\xdb\x48\xa0\x79\xdf\x75\x9e\x05\x09\xf7"
#define GUID_TreeProperties			"\xad\x49\x37\x5b\x9f\xb4\xc1\x49\x83\xeb\x15\x37\x0f\xbd\x48\x82"
#define GUID_UserPinned				"\xab\x95\x39\x9E\x9C\x1f\x13\x4f\xB8\x27\x48\xB2\x4B\x6C\x71\x74"
#define GUID_UserProfiles			"\x72\xd2\x62\x07\x0a\xc5\xb0\x4b\xa3\x82\x69\x7d\xcd\x72\x9b\x80"
#define GUID_UserProgramFiles		"\xe2\xae\xd7\x5C\x19\x22\x67\x4a\xB8\x5D\x6C\x9C\xE1\x56\x60\xCB"
#define GUID_UserProgramFilesCommon	"\x57\x30\xbd\xBC\x5C\xca\x22\x46\xB4\x2D\xBC\x56\xDB\x0A\xE5\x16"
#define GUID_UsersFiles				"\x7c\x0f\xce\xf3\x01\x49\xcc\x4a\x86\x48\xd5\xd4\x4b\x04\xef\x8f"
#define GUID_UsersLibraries			"\x25\x48\x1e\x03\x94\x7b\xc3\x4d\xb1\x31\xe9\x46\xb4\x4c\x8d\xd5"
#define GUID_UsersLibrariesFolder	"\x5d\x54\x02\xA3\xFF\xde\x4b\x46\xAB\xE8\x61\xC8\x64\x8D\x93\x9B"
#define GUID_UserTiles				"\xb1\xa0\x8c\x00\xb4\x55\x56\x4c\xb8\xa8\x4d\xe4\xb2\x99\xd3\xbe"
#define GUID_Videos					"\x1d\x9b\x98\x18\xb5\x99\x5b\x45\x84\x1c\xab\x7c\x74\xe4\xdd\xfc"
#define GUID_VideosLibrary			"\x2f\x92\x1e\x49\x43\x56\xF4\x4a\xA7\xEB\x4E\x7A\x13\x8D\x81\x74"
#define GUID_Windows				"\x04\xf4\x8b\xf3\x43\x1d\xf2\x42\x93\x05\x67\xde\x0b\x28\xfc\x23"

// Additional IDs found on the net and in the registry.
#define GUID_CD_Burner				"\x05\x8a\xeb\xfb\xee\xbe\x42\x44\x80\x4e\x40\x9d\x6c\x45\x15\xe9"
#define GUID_Control_Panel			"\x94\xe6\x99\x53\xe5\x6c\x6c\x4d\x8f\xce\x1d\x88\x70\xfd\xcb\xa0"	// Control Panel command object.
#define GUID_Control_Panel2			"\x20\x20\xec\x21\xea\x3a\x69\x10\xa2\xdd\x08\x00\x2b\x30\x30\x9d"
#define GUID_CSC_Folder				"\x7b\x2e\x7a\xbd\xcb\x21\xb2\x41\xa0\x86\xb3\x09\x68\x0c\x6b\x7e"
#define GUID_Internet_Explorer		"\x80\x53\x1c\x87\xa0\x42\x69\x10\xa2\xea\x08\x00\x2b\x30\x30\x9d"
#define GUID_My_Computer			"\xe0\x4f\xd0\x20\xea\x3a\x69\x10\xa2\xd8\x08\x00\x2b\x30\x30\x9d"
#define GUID_My_Documents			"\xba\x8f\x0d\x45\x25\xad\xd0\x11\x98\xa8\x08\x00\x36\x1b\x11\x03"
#define GUID_My_Games				"\xdf\x8f\x22\xed\xa8\x9e\x70\x48\x83\xb1\x96\xb0\x2c\xfe\x0d\x52"
#define GUID_My_Network_Places		"\x60\x2c\x8d\x20\xea\x3a\x69\x10\xa2\xd7\x08\x00\x2b\x30\x30\x9d"
#define GUID_Network_Connections	"\xc7\xac\x07\x70\x02\x32\xd1\x11\xaa\xd2\x00\x80\x5f\xc1\x27\x0e"
#define GUID_Printers_and_Faxes		"\x80\xa2\x27\x22\xea\x3a\x69\x10\xa2\xde\x08\x00\x2b\x30\x30\x9d"
#define GUID_Dial_up_Connection		"\xd7\x6a\x12\xba\x66\x21\xd1\x11\xb1\xd0\x00\x80\x5f\xc1\x27\x0e"
#define GUID_Users					"\x47\x1a\x03\x59\x72\x3f\xa7\x44\x89\xc5\x55\x95\xfe\x6b\x30\xee"	// Same as UsersFiles?
#define GUID_Show_Desktop			"\x0d\xf9\x80\x30\xad\xd7\xd9\x11\xbd\x98\x00\x00\x94\x7b\x02\x57"
#define GUID_Window_Switcher		"\x0e\xf9\x80\x30\xad\xd7\xd9\x11\xbd\x98\x00\x00\x94\x7b\x02\x57"
#define GUID_Search					"\xf0\xa1\x59\x25\xd7\x21\xd4\x11\xbd\xaf\x00\xc0\x4f\x60\xb9\xf0"
#define GUID_Help_and_Support		"\xf1\xa1\x59\x25\xd7\x21\xd4\x11\xbd\xaf\x00\xc0\x4f\x60\xb9\xf0"
#define GUID_Windows_Security		"\xf2\xa1\x59\x25\xd7\x21\xd4\x11\xbd\xaf\x00\xc0\x4f\x60\xb9\xf0"
#define GUID_Run					"\xf3\xa1\x59\x25\xd7\x21\xd4\x11\xbd\xaf\x00\xc0\x4f\x60\xb9\xf0"
#define GUID_Email					"\xf5\xa1\x59\x25\xd7\x21\xd4\x11\xbd\xaf\x00\xc0\x4f\x60\xb9\xf0"
#define GUID_Set_Program_Access		"\xf7\xa1\x59\x25\xd7\x21\xd4\x11\xbd\xaf\x00\xc0\x4f\x60\xb9\xf0"
#define GUID_Start_Menu_Provider	"\x13\x53\xf9\xda\x4d\xe4\xaf\x46\xbe\x1b\xcb\xac\xea\x2c\x30\x65"
#define GUID_Search_Results			"\x1e\x1a\xde\x7f\x31\x8b\xa5\x49\x93\xb8\x6b\xe1\x4c\xfa\x49\x43"
#define GUID_Start_Menu				"\xcb\xb4\x87\xef\xce\xf2\x85\x47\x86\x58\x4c\xa6\xc6\x3e\x38\xc6"

#define GUID_Unknown				"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"

#pragma endregion DEFINES

#pragma region STRUCTS

struct ShellLinkHeader
{
	unsigned int HeaderSize;
	unsigned char LinkCLSID[ 16 ];
	unsigned int LinkFlags;
	unsigned int FileAttributes;
	unsigned int CreationTimeLow;
	unsigned int CreationTimeHigh;
	unsigned int AccessTimeLow;
	unsigned int AccessTimeHigh;
	unsigned int WriteTimeLow;
	unsigned int WriteTimeHigh;
	unsigned int FileSize;
	unsigned int IconIndex;
	unsigned int ShowCommand;
	unsigned short HotKey;
	unsigned short Reserved1;
	unsigned int Reserved2;
	unsigned int Reserved3;
};

struct LinkInfo
{
	unsigned int LinkInfoSize;	// Includes the size variable itself.
	unsigned int LinkInfoHeaderSize;
	unsigned int LinkInfoFlags;
	unsigned int VolumeIDOffset;
	unsigned int LocalBasePathOffset;
	unsigned int CommonNetworkRelativeLinkOffset;
	unsigned int CommonPathSuffixOffset;
};

struct VolumeID
{
	unsigned int VolumeIDSize;	// Includes the size variable itself.
	unsigned int DriveType;
	unsigned int DriveSerialNumber;
	unsigned int VolumeLabelOffset;
};

struct CommonNetworkRelativeLink
{
	unsigned int CommonNetworkRelativeLinkSize;	// Includes the size variable itself.
	unsigned int CommonNetworkRelativeLinkFlags;
	unsigned int NetNameOffset;
	unsigned int DeviceNameOffset;
	unsigned int NetworkProviderType;
};

struct EnvironmentVariableDataBlock
{
	char TargetAnsi[ MAX_PATH ];		// 260 bytes
	wchar_t TargetUnicode[ MAX_PATH ];	// 520 bytes
};

typedef struct EnvironmentVariableDataBlock IconEnvironmentDataBlock;

struct TrackerDataBlock1
{
	unsigned int Length;
	unsigned int Version;
};

// A variable length Machine ID (NetBIOS) exits between TrackerDataBlock1 and TrackerDataBlock2. It's generally 16 bytes.

struct TrackerDataBlock2
{
	unsigned char DroidVolumeID[ 16 ];
	unsigned char DroidFileID[ 16 ];
	unsigned char BirthDroidVolumeID[ 16 ];
	unsigned char BirthDroidFileID[ 16 ];
};

struct SerializedPropertyStorage
{
	unsigned int StorageSize;	// This int is excluded from the size.
	unsigned int Version;		// 0x53505331
	unsigned char FormatID[ 16 ];
};

struct SerializedPropertyValue
{
	unsigned int ValueSize;		// This int is included in the size.
	unsigned int NameIDSize;	// It'll either be an identifier, or the name size.
	unsigned char Reserved;		// Must be 0x00
};

struct ConsoleDataBlock
{
	unsigned short FillAttributes;
	unsigned short PopupFillAttributes;
	unsigned short ScreenBufferSizeX;
	unsigned short ScreenBufferSizeY;
	unsigned short WindowSizeX;
	unsigned short WindowSizeY;
	unsigned short WindowOriginX;
	unsigned short WindowOriginY;
	unsigned int _Unused1;
	unsigned int _Unused2;
	unsigned short FontSizeX;
	unsigned short FontSizeY;
	unsigned int FontFamily;
	unsigned int FontWeight;
	wchar_t FaceName[ 32 ];
	unsigned int CursorSize;
	unsigned int FullScreen;
	unsigned int QuickEdit;
	unsigned int InsertMode;
	unsigned int AutoPosition;
	unsigned int HistoryBufferSize;
	unsigned int NumberOfHistoryBuffers;
	unsigned int HistoryNoDup;
	unsigned int ColorTable[ 16 ];
};

struct SpecialFolderDataBlock
{
	unsigned int SpecialFolderID;
	unsigned int Offset;	// Specifies the location of the ItemID of the first child segment of the IDList specified by SpecialFolderID. This value is the offset, in bytes, into the link target IDList.
};

struct DarwinDataBlock
{
	char DarwinDataAnsi[ MAX_PATH ];		// 260 bytes
	wchar_t DarwinDataUnicode[ MAX_PATH ];	// 520 bytes
};

struct KnownFolderDataBlock
{
	unsigned char KnownFolderID[ 16 ];
	unsigned int Offset;	// Specifies the location of the ItemID of the first child segment of the IDList specified by KnownFolderID. This value is the offset, in bytes, into the link target IDList.
};

struct shitemid_header
{
	unsigned short size;	// Inclusive.
	unsigned char flag;		// Flag?
};

struct clsid_type
{
	const char *clsid;
	char *name;
};

#pragma endregion STRUCTS

#pragma region PROTOTYPES

wchar_t *get_extension_from_filename( wchar_t *filename );
void traverse_directory( wchar_t *path );
wchar_t *get_showwindow_value( unsigned int sw_value );
wchar_t *get_hot_key_value( unsigned short hk_value );
wchar_t *get_file_attributes( unsigned int fa_flags );
wchar_t *get_data_flags( unsigned int d_flags );
wchar_t *get_link_info_flags( unsigned int li_flags );
wchar_t *get_common_network_relative_link_flags( unsigned int cnrl_flags );
wchar_t *get_drive_type( unsigned int d_type );
wchar_t *get_network_provider_type( unsigned int np_type );
wchar_t *get_font_family_value( unsigned short ff_value );
wchar_t *get_font_weight( unsigned int fw );
wchar_t *get_color_flags( unsigned short c_flags );
wchar_t *get_special_folder_type( unsigned int sf_type );
wchar_t *get_property_type( unsigned int p_type );
char *get_prop_id_type( char *guid, unsigned int prop_id );

void buffer_to_guid( unsigned char *buffer, char *guid );
void buffer_to_mac( char *buffer, char *mac );

void hex_dump( unsigned char *buffer, unsigned int buf_length );
void parse_pidl( char *pidl, unsigned int pidl_size, unsigned int &pidl_offset, bool tab, bool use_csv );
void parse_shortcut( wchar_t *filepath );
char *get_clsid_type( char *clsid );

wchar_t *escape_html_unicode( wchar_t *buf );
char *escape_html_ascii( char *buf );
wchar_t *escape_string_unicode( wchar_t *buf );
char *escape_string_ascii( char *buf );

void check_config();

void write_csv( HANDLE hFile, wchar_t *buffer, unsigned int &offset );
void write_html( HANDLE hFile, wchar_t *buffer, unsigned int &offset );
void write_html_dump( HANDLE hFile, wchar_t *buffer, unsigned int &offset, wchar_t *text, unsigned char *dump_buffer, unsigned int dump_length );

void parse_metadata( char *metadata_property_store, unsigned int metadata_property_store_length );

#pragma endregion PROTOTYPES

#pragma region EXTERNS

extern wchar_t write_buf2[];
extern wchar_t write_buf[];
extern unsigned int write_offset2;
extern unsigned int write_offset;
extern HANDLE hFile_csv;
extern HANDLE hFile_html;
extern bool output_html;
extern bool output_csv;
extern bool dump_remnant;
extern bool silent;

extern char total_columns;
extern char column_count;

// Column distances. Used to determine how many commas to pad missing data in the CSV report.
extern char col_date_created;
extern char col_last_accessed;
extern char col_last_modified;
extern char col_file_size;
extern char col_file_attributes;
extern char col_icon_index;
extern char col_showwindow_value;
extern char col_hot_key_value;
extern char col_link_flags;
extern char col_link_target_id_list;
extern char col_location_flags;
extern char col_drive_type;
extern char col_drive_serial_number;
extern char col_volume_label_a;
extern char col_volume_label_u;
extern char col_local_path_a;
extern char col_network_share_flags;
extern char col_network_provider_type;
extern char col_network_share_name_a;
extern char col_device_name_a;
extern char col_network_share_name_u;
extern char col_device_name_u;
extern char col_common_path_a;
extern char col_local_path_u;
extern char col_common_path_u;
extern char col_comment_u;
extern char col_comment_a;
extern char col_relative_path_u;
extern char col_relative_path_a;
extern char col_working_directory_u;
extern char col_working_directory_a;
extern char col_arguments_u;
extern char col_arguments_a;
extern char col_icon_location_u;
extern char col_icon_location_a;
extern char col_color_flags;
extern char col_screen_buffer_width;
extern char col_screen_buffer_height;
extern char col_window_width;
extern char col_window_height;
extern char col_window_x_coordinate;
extern char col_window_y_coordinate;
extern char col_font_size;
extern char col_font_family_value;
extern char col_font_weight;
extern char col_font_face_name;
extern char col_cursor_size;
extern char col_fullscreen;
extern char col_quickedit_mode;
extern char col_insert_mode;
extern char col_automatic_positioning;
extern char col_history_buffer_size;
extern char col_number_of_history_buffers;
extern char col_duplicates_allowed_in_history;
extern char col_color_table_values;
extern char col_code_page;
extern char col_application_identifier_a;
extern char col_application_identifier_u;
extern char col_environment_variables_location_a;
extern char col_environment_variables_location_u;
extern char col_icon_location2_a;
extern char col_icon_location2_u;
extern char col_known_folder_guid;
extern char col_known_folder_first_child_segment_offset;
extern char col_metadata_property_store;
extern char col_shim_layer_u;
extern char col_special_folder_identifier;
extern char col_special_folder_first_child_segment_offset;
extern char col_version;
extern char col_netbios_name;
extern char col_droid_volume_identifier;
extern char col_droid_file_identifier;
extern char col_birth_droid_volume_identifier;
extern char col_birth_droid_file_identifier;
extern char col_mac_address;
extern char col_uuid_timestamp;
extern char col_uuid_sequence_number;
extern char col_distributed_link_tracker_notes;
extern char col_vista_and_above_id_list;
extern char col_output_notes;

// Configuration values for the CSV columns.
extern bool cfg_date_created;
extern bool cfg_last_accessed;
extern bool cfg_last_modified;
extern bool cfg_file_size;
extern bool cfg_file_attributes;
extern bool cfg_icon_index;
extern bool cfg_showwindow_value;
extern bool cfg_hot_key_value;
extern bool cfg_link_flags;
extern bool cfg_link_target_id_list;
extern bool cfg_location_flags;
extern bool cfg_drive_type;
extern bool cfg_drive_serial_number;
extern bool cfg_volume_label_a;
extern bool cfg_volume_label_u;
extern bool cfg_local_path_a;
extern bool cfg_network_share_flags;
extern bool cfg_network_provider_type;
extern bool cfg_network_share_name_a;
extern bool cfg_device_name_a;
extern bool cfg_network_share_name_u;
extern bool cfg_device_name_u;
extern bool cfg_common_path_a;
extern bool cfg_local_path_u;
extern bool cfg_common_path_u;
extern bool cfg_comment_u;
extern bool cfg_comment_a;
extern bool cfg_relative_path_u;
extern bool cfg_relative_path_a;
extern bool cfg_working_directory_u;
extern bool cfg_working_directory_a;
extern bool cfg_arguments_u;
extern bool cfg_arguments_a;
extern bool cfg_icon_location_u;
extern bool cfg_icon_location_a;
extern bool cfg_color_flags;
extern bool cfg_screen_buffer_width;
extern bool cfg_screen_buffer_height;
extern bool cfg_window_width;
extern bool cfg_window_height;
extern bool cfg_window_x_coordinate;
extern bool cfg_window_y_coordinate;
extern bool cfg_font_size;
extern bool cfg_font_family_value;
extern bool cfg_font_weight;
extern bool cfg_font_face_name;
extern bool cfg_cursor_size;
extern bool cfg_fullscreen;
extern bool cfg_quickedit_mode;
extern bool cfg_insert_mode;
extern bool cfg_automatic_positioning;
extern bool cfg_history_buffer_size;
extern bool cfg_number_of_history_buffers;
extern bool cfg_duplicates_allowed_in_history;
extern bool cfg_color_table_values;
extern bool cfg_code_page;
extern bool cfg_application_identifier_a;
extern bool cfg_application_identifier_u;
extern bool cfg_environment_variables_location_a;
extern bool cfg_environment_variables_location_u;
extern bool cfg_icon_location2_a;
extern bool cfg_icon_location2_u;
extern bool cfg_known_folder_guid;
extern bool cfg_known_folder_first_child_segment_offset;
extern bool cfg_metadata_property_store;
extern bool cfg_shim_layer_u;
extern bool cfg_special_folder_identifier;
extern bool cfg_special_folder_first_child_segment_offset;
extern bool cfg_version;
extern bool cfg_netbios_name;
extern bool cfg_droid_volume_identifier;
extern bool cfg_droid_file_identifier;
extern bool cfg_birth_droid_volume_identifier;
extern bool cfg_birth_droid_file_identifier;
extern bool cfg_mac_address;
extern bool cfg_uuid_timestamp;
extern bool cfg_uuid_sequence_number;
extern bool cfg_distributed_link_tracker_notes;
extern bool cfg_vista_and_above_id_list;
extern bool cfg_output_notes;

#pragma endregion EXTERNS

#endif
