#pragma once

#include <unknwn.h>

__interface INoticeSource
{
	virtual void __stdcall INoticeSource_Unknown() PURE;
};

__interface INoticeSink
{
	virtual bool __stdcall INoticeSink_Unknown(DWORD dwUnknown) PURE;
};