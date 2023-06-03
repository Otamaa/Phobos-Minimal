#pragma once
#include <HouseTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class HouseTypeExt
{
public:
	class ExtData final : public Extension<HouseTypeClass>
	{
	public:
		using base_type = HouseTypeClass;
		static constexpr DWORD Canary = 0x1111111A;

	public:
		bool SettingsInherited;

		Nullable<int> SurvivorDivisor;
		Nullable<InfantryTypeClass*> Crew;
		Nullable<InfantryTypeClass*> Engineer;
		Nullable<InfantryTypeClass*> Technician;
		ValueableIdx<AircraftTypeClass> ParaDropPlane;

		Nullable<int> NewTeamsSelector_MergeUnclassifiedCategoryWith;
		Nullable<double> NewTeamsSelector_UnclassifiedCategoryPercentage;
		Nullable<double> NewTeamsSelector_GroundCategoryPercentage;
		Nullable<double> NewTeamsSelector_NavalCategoryPercentage;
		Nullable<double> NewTeamsSelector_AirCategoryPercentage;

		ExtData(HouseTypeClass* OwnerObject) : Extension<HouseTypeClass>(OwnerObject)
			, SettingsInherited { false }

			, SurvivorDivisor { }
			, Crew { }
			, Engineer { }
			, Technician { }
			, ParaDropPlane { -1 }

			, NewTeamsSelector_MergeUnclassifiedCategoryWith { }
			, NewTeamsSelector_UnclassifiedCategoryPercentage { }
			, NewTeamsSelector_GroundCategoryPercentage { }
			, NewTeamsSelector_NavalCategoryPercentage { }
			, NewTeamsSelector_AirCategoryPercentage { }
		{ }

		virtual ~ExtData() override = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

		void InheritSettings(HouseTypeClass* pThis);
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool Load(HouseTypeClass* pThis, IStream* pStm) override;
	};

	static ExtContainer ExtMap;
};