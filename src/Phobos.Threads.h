#pragma once
#ifdef ENABLE_TLS
#include <Base/Always.h>

struct TLS_Thread
{
private:
	NO_CONSTRUCT_CLASS(TLS_Thread)
public:

	static DWORD dwTlsIndex_SHPDRaw_1;
	static DWORD dwTlsIndex_SHPDRaw_2;
};
#endif