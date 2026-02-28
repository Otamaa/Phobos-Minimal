#pragma once

#include <Misc/Kratos/Common/INI/INIConfig.h>

#include <Misc/Kratos/Ext/Common/ExpandAnimsData.h>

class TerrainDestroyAnimData : public ExpandAnimsData
{
public:
	virtual void Read(INIBufferReader* reader) override
	{
		ExpandAnimsData::Read(reader, TITLE);
	}
private:
	inline static std::string TITLE = "Destroy.";
};
