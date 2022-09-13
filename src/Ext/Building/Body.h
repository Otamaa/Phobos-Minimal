#pragma once
#include <BuildingClass.h>
#include <HouseClass.h>
#include <TiberiumClass.h>
#include <FactoryClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.Otamaa.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>

class BuildingExt
{
public:
	static constexpr size_t Canary = 0x87654321;
	using base_type = BuildingClass;
#ifndef ENABLE_NEWEXT
	static constexpr size_t ExtOffset = 0x6FC;
#endif

	class ExtData final : public Extension<BuildingClass>
	{
	public:

		bool DeployedTechno;
		int LimboID;
		int GrindingWeapon_LastFiredFrame;
		BuildingClass* CurrentAirFactory;
		int AccumulatedGrindingRefund;

		std::vector<AnimClass*> DamageFireAnims;
		TimerStruct AutoSellTimer;

		ExtData(BuildingClass* OwnerObject) : Extension<BuildingClass>(OwnerObject)
			, DeployedTechno { false }
			, LimboID { -1 }
			, GrindingWeapon_LastFiredFrame { 0 }
			, CurrentAirFactory { nullptr }
			, AccumulatedGrindingRefund { 0 }
			, DamageFireAnims { }
			, AutoSellTimer { }

		{ }

		virtual ~ExtData() = default;

		void InvalidatePointer(void* ptr, bool bRemoved);

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void InitializeConstants();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingExt
#ifndef ENABLE_NEWEXT
, true
, true
#endif
	>
	{
	public:
		ExtContainer();
		~ExtContainer();

		bool InvalidateExtDataIgnorable(void* const ptr) const
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Building:
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

	static void StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType);

	static void UpdatePrimaryFactoryAI(BuildingClass* pThis);
	static int CountOccupiedDocks(BuildingClass* pBuilding);
	static bool HasFreeDocks(BuildingClass* pBuilding);
	static bool CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno);
	static bool DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno);
};