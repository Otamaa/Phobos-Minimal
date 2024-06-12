#pragma once
#include <string>
#include <vector>
#include <map>

#include <GeneralStructures.h>

#include <Misc/DynamicPatcher/Common/INI/INIConfig.h>

#include <Misc/DynamicPatcher/Ext/Common/CommonStatus.h>

class AircraftAttitudeData : public INIConfig
{
public:

	bool Disable = false; // 关闭俯仰姿态自动调整，但不影响俯冲

	int SpawnTakeoffDir = 0;
	int SpawnLandDir = 0;


	virtual void Read(INIBufferReader* reader) override
	{
		// 读取全局
		INIBufferReader* av = INI::GetSection(INI::Rules, GameStrings::AudioVisual());
		Read(av, "");
		// 读取个体
		Read(reader, "");
	}

	virtual void Read(INIBufferReader* reader, std::string title)
	{
		Disable = reader->Get("DisableAircraftAutoPitch", Disable);

		SpawnTakeoffDir = reader->Get("SpawnTakingOffDir", SpawnTakeoffDir);
		SpawnLandDir = reader->Get("SpawnLandingDir", SpawnTakeoffDir);

		Enable = !Disable;
	}
};

class AircraftDockingOffsetData : public INIConfig
{
public:
	std::map<int, int> Direction{};

	virtual void Read(INIBufferReader* reader) override
	{
		// 读取全局
		int poseDir = RulesClass::Instance->PoseDir;
		for (int i = 0; i < 128; i++)
		{
			std::string key = "DockingOffset" + std::to_string(i) + ".Dir";
			int dir = poseDir;
			if (reader->TryGet<int>(key, dir))
			{

				if (dir < 0)
				{
					dir = 0;
				}
				else if (dir > 7)
				{
					dir = 7;
				}
				Direction[i] = dir;
			}
		}
		Enable = !Direction.empty();
	}
};
