#pragma once
#include <TeamClass.h>

#include <Helpers/Enumerators.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Phobos.h>

class TeamExt
{
public:
	static constexpr size_t Canary = 0x414B4B41;
	using base_type = TeamClass;
//#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
//#endif

	class ExtData final : public Extension<TeamClass>
	{
	public:
		int WaitNoTargetAttempts;
		double NextSuccessWeightAward;
		int IdxSelectedObjectFromAIList;
		double CloseEnough;
		int Countdown_RegroupAtLeader;
		int MoveMissionEndMode;
		int WaitNoTargetCounter;
		TimerStruct WaitNoTargetTimer;
		TimerStruct ForceJump_Countdown;
		int ForceJump_InitialCountdown;
		bool ForceJump_RepeatMode;
		FootClass* TeamLeader;
		int GenericStatus;
		int FailedCounter;

		ExtData(TeamClass* OwnerObject) : Extension<TeamClass>(OwnerObject)
			, WaitNoTargetAttempts { 0 }
			, NextSuccessWeightAward { 0 }
			, IdxSelectedObjectFromAIList { -1 }
			, CloseEnough { -1 }
			, Countdown_RegroupAtLeader { -1 }
			, MoveMissionEndMode { 0 }
			, WaitNoTargetCounter { 0 }
			, WaitNoTargetTimer { 0 }
			, ForceJump_Countdown { -1 }
			, ForceJump_InitialCountdown { -1 }
			, ForceJump_RepeatMode { false }
			, TeamLeader { nullptr }
			, GenericStatus { 0 }
			, FailedCounter { -1 }
		{ }

		virtual ~ExtData() override = default;
		void InvalidatePointer(void* ptr, bool bRemoved);
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void InitializeConstants();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static TeamExt::ExtData* GetExtData(base_type* pThis);

	class ExtContainer final : public Container<TeamExt , true>
	{
	public:
		ExtContainer();
		~ExtContainer();

		bool InvalidateExtDataIgnorable(void* const ptr) const
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Aircraft:
			case AbstractType::Unit:
			case AbstractType::Infantry:
				return false;
			default:
				return true;
			}
		}

		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
