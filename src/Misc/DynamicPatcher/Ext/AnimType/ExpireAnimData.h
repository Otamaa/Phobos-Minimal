#pragma once

#include <Misc/DynamicPatcher/Common/INI/INIConfig.h>

class ExpireAnimData : public INIConfig
{
public:
	std::string WakeAnimOnWater{ "" };
	std::string ExpireAnimOnWater{ "" };

	virtual void Read(INIBufferReader* reader) override
	{
		WakeAnimOnWater = reader->Get("WakeAnimOnWater", WakeAnimOnWater);
		ExpireAnimOnWater = reader->Get("ExpireAnimOnWater", ExpireAnimOnWater);
	}

};

