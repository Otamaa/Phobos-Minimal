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
	//AdjustLighting = 505,
	RunSuperWeaponAtLocation = 505,
	RunSuperWeaponAtWaypoint = 506,

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
	//Something_700 = 700,
	//Something_701 = 701,
	//LauchSWAtWaypoint = 702,
	//AISetMode = 703,
	//Something_704 = 704,
	//Something_705 = 705 ,
	//DoFlash = 713 ,
	//Something_716 = 716 ,
	//Something_717 = 717,
	//DoLighningStormStrike  = 720 ,
	//DrawLaserBetweenWeaypoints = 9940
};

class TActionExt
{
public:
	static std::map<int, std::vector<TriggerClass*>> RandomTriggerPool;

	class ExtData final : public Extension<TActionClass>
	{
	public:
		static constexpr size_t Canary = 0x87154321;
		using base_type = TActionClass;

	public:

		//std::string Value1 { };
		//std::string Value2 { };
		//std::string Parm3 { };
		//std::string Parm4 { };
		//std::string Parm5 { };
		//std::string Parm6 { };

		ExtData(TActionClass* const OwnerObject) : Extension<base_type>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TActionExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();

		void Clear() {
			RandomTriggerPool.clear();
		}

		void InvalidatePointer(void* ptr, bool bRemoved) {
			for (auto& nMap : RandomTriggerPool) {
				AnnounceInvalidPointer(nMap.second, ptr);
			}
		}

		bool InvalidateIgnorable(void* ptr)
		{
			switch (GetVtableAddr(ptr))
			{
			case TriggerClass::vtable:
				return false;
			}

			return true;
		}
	};

	static ExtContainer ExtMap;

	static void RecreateLightSources();
	static bool Occured(TActionClass* pThis, ActionArgs const& args , bool& bHandled);
	static bool RunSuperWeaponAt(TActionClass* pThis, int X, int Y);

#define ACTION_FUNC(name) \
	static bool name(TActionClass* pThis, HouseClass* pHouse, \
		ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* plocation)

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

	ACTION_FUNC(RandomTriggerPut);
	ACTION_FUNC(RandomTriggerEnable);
	ACTION_FUNC(RandomTriggerRemove);

	ACTION_FUNC(ScoreCampaignText);
	ACTION_FUNC(ScoreCampaignTheme);
	ACTION_FUNC(SetNextMission);

	ACTION_FUNC(ToggleMCVRedeploy);

#undef ACTION_FUNC
};
