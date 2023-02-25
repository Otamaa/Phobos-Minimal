#pragma once

#include <Ext/Abstract/Body.h>
#include <Utilities/Template.h>

#include <Helpers/Template.h>

#include <TActionClass.h>

class HouseClass;

enum class PhobosTriggerAction : unsigned int
{
	SaveGame = 500,
	EditVariable = 501,
	GenerateRandomNumber = 502,
	PrintVariableValue = 503,
	BinaryOperation = 504,
	AdjustLighting = 505,
	RunSuperWeaponAtLocation = 506,
	RunSuperWeaponAtWaypoint = 507,
	
	//#844
	ToggleMCVRedeploy = 510,

	//#658
	RandomTriggerPut = 12000,
	RandomTriggerRemove = 12001,
	RandomTriggerEnable = 12002,
	ScoreCampaignText = 19000,
	ScoreCampaignTheme = 19001,
	SetNextMission = 19002 ,
	//ES
	Something_700 = 700,
	Something_701 = 701,
	LauchSWAtWaypoint = 702,
	AISetMode = 703,
	Something_704 = 704,
	Something_705 = 705 ,
	DoFlash = 713 ,
	Something_716 = 716 ,
	Something_717 = 717,
	DoLighningStormStrike  = 720 ,
	DrawLaserBetweenWeaypoints = 9940
};

class TActionExt
{
public:
	static constexpr size_t Canary = 0x87154321;
	using base_type = TActionClass;

	class ExtData final : public Extension<base_type>
	{
	public:

		std::string Value1;
		std::string Value2;
		std::string Parm3;
		std::string Parm4;
		std::string Parm5;
		std::string Parm6;

		ExtData(TActionClass* const OwnerObject) : Extension<base_type>(OwnerObject)
		, Value1 { }
		, Value2 { }
		, Parm3 { }
		, Parm4 { }
		, Parm5 { }
		, Parm6 { }
		{ }

		virtual ~ExtData() override = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override { }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TActionExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual void Clear() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Trigger:
				return false;
			}
			
			return true;
		}
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static void RecreateLightSources();
	static bool Execute(TActionClass* pThis, HouseClass* pHouse,
			ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location, bool& bHandled);

#define ACTION_FUNC(name) \
	static bool name(TActionClass* pThis, HouseClass* pHouse, \
		ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)

	ACTION_FUNC(PlayAudioAtRandomWP);
	ACTION_FUNC(SaveGame);
	ACTION_FUNC(EditVariable);
	ACTION_FUNC(GenerateRandomNumber);
	ACTION_FUNC(PrintVariableValue);
	ACTION_FUNC(BinaryOperation);
	ACTION_FUNC(AdjustLighting);

	ACTION_FUNC(RunSuperWeaponAtLocation);
	ACTION_FUNC(RunSuperWeaponAtWaypoint);

	ACTION_FUNC(DrawLaserBetweenWaypoints);

	static bool RunSuperWeaponAt(TActionClass* pThis, int X, int Y);

	ACTION_FUNC(RandomTriggerPut);
	ACTION_FUNC(RandomTriggerEnable);
	ACTION_FUNC(RandomTriggerRemove);

	ACTION_FUNC(ScoreCampaignText);
	ACTION_FUNC(ScoreCampaignTheme);
	ACTION_FUNC(SetNextMission);

	ACTION_FUNC(ToggleMCVRedeploy);

#undef ACTION_FUNC

	static std::map<int, std::vector<TriggerClass*>> RandomTriggerPool;
};
