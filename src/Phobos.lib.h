#pragma once

#pragma comment(lib, "Version.lib")
#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "Winmm.lib")

#ifdef _TTT
#pragma comment(linker, "/NODEFAULTLIB")

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
//#pragma comment(lib, "winspool.lib")
//#pragma comment(lib, "comdlg32.lib")
//#pragma comment(lib, "advapi32.lib")
//#pragma comment(lib, "shell32.lib")
//#pragma comment(lib, "oleaut32.lib")
//#pragma comment(lib, "uuid.lib")
//#pragma comment(lib, "odbc32.lib")
//#pragma comment(lib, "odbccp32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "libcmt.lib")
#pragma comment(lib, "msvcrt.lib")
#pragma comment(lib, "OLDNAMES.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "vcruntime.lib")
#pragma comment(lib, "atls.lib")
#pragma comment(lib, "msvcprt.lib")
#pragma comment(lib, "onecore.lib")
#pragma comment(lib, "dbghelp.lib")
//#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "comsuppw.lib")

//#pragma comment(lib, "msvcrtd.lib")

//#pragma comment(lib, "libcmtd.lib")

#pragma comment(lib, "vcruntime.lib")
//#pragma comment(lib, "vcruntimed.lib")

#pragma comment(lib, "libvcruntime.lib")
//#pragma comment(lib, "libvcruntimed.lib")

#pragma comment(lib, "ucrt.lib")
//#pragma comment(lib, "ucrtd.lib")

#pragma comment(lib, "libucrt.lib")
//#pragma comment(lib, "libucrtd.lib")

#endif