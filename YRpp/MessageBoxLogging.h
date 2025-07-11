#pragma once


#define EMIT_MSGBOXW(str , window)\
	MessageBoxW(0, str , window, MB_OK);

#define EMIT_MSGBOXA(str , window)\
	MessageBoxA(0, str , window, MB_OK);

#define MSGBOX_DEBUG(str)\
	EMIT_MSGBOXA(str , "Debug");
