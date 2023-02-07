#pragma once
#include <HouseTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class HouseTypeExt
{
public:
	using base_type = HouseTypeClass;
	static constexpr DWORD Canary = 0x1111111A;

	class ExtData final : public TExtension<HouseTypeClass>
	{
	public:
		Nullable<int> NewTeamsSelector_MergeUnclassifiedCategoryWith;
		Nullable<double> NewTeamsSelector_UnclassifiedCategoryPercentage;
		Nullable<double> NewTeamsSelector_GroundCategoryPercentage;
		Nullable<double> NewTeamsSelector_NavalCategoryPercentage;
		Nullable<double> NewTeamsSelector_AirCategoryPercentage;

		ExtData(HouseTypeClass* OwnerObject) : TExtension<HouseTypeClass>(OwnerObject)
			, NewTeamsSelector_MergeUnclassifiedCategoryWith { }
			, NewTeamsSelector_UnclassifiedCategoryPercentage { }
			, NewTeamsSelector_GroundCategoryPercentage { }
			, NewTeamsSelector_NavalCategoryPercentage { }
			, NewTeamsSelector_AirCategoryPercentage { }
		{ }

		virtual ~ExtData() override = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void CompleteInitialization();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true;  };

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<HouseTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool Load(HouseTypeClass* pThis, IStream* pStm) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};