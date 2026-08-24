#pragma once

#include <Helpers/CompileTime.h>
#include <Base/Macros.h>

struct VersionProtocolType
{
	int Version;
	int Protocol;
};

class NOVTABLE VersionClass
{
public:
	static constexpr reference<VersionProtocolType, 0x846140, 3u> const VersionProtocol {};

public:
	VersionClass() { JMP_THIS(0x74F730); }
	virtual ~VersionClass() = default;

	unsigned long Version_Number() { JMP_THIS(0x74F760); }
	unsigned short Major_Version() { JMP_THIS(0x74F960); }
	unsigned short Minor_Version() { JMP_THIS(0x74FA20); }
	const char* Version_Name() { JMP_THIS(0x74FAE0); }
	const char* Version_Text() { JMP_THIS(0x74FC80); }
	int Version_Protocol(unsigned long version) { JMP_THIS(0x74FD20); }
	void Init_Clipping()
	{
		this->MinClipVer = 0x20000;
		this->MaxClipVer = 0x20000;
	}
	unsigned long Clip_Version(unsigned long minver, unsigned long maxver)
	{
		unsigned __int32 v3 = this->MaxClipVer;
		unsigned __int32 v5 = this->MinClipVer;

		if (minver > v3)
		{
			return -1;
		}

		if (maxver < v5)
		{
			return 0;
		}
		if (minver > v5)
		{
			this->MinClipVer = minver;
		}
		if (maxver < v3)
		{
			this->MaxClipVer = maxver;
		}
		return this->MaxClipVer;
	}

	unsigned long Min_Version()
	{
		return 0x20000;
	}

	unsigned long Max_Version()
	{
		return 0x20000;
	}

private:
	unsigned long Version;
	unsigned short MajorVer;
	unsigned short MinorVer;
	char VersionName[30];
	char VersionText[16];
	unsigned long MinClipVer;
	unsigned long MaxClipVer;
	unsigned VersionInit : 1;
	unsigned MajorInit : 1;
	unsigned MinorInit : 1;
	unsigned TextInit : 1;
};
