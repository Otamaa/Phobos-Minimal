#pragma once
#include <CaptureManagerClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <AnimTypeClass.h>
#include <RulesClass.h>
#include <FootClass.h>

class CaptureExt
{
public:
	/*
	class ExtData final : public Extension<CaptureManagerClass>
	{
	public:

		static COMPILETIMEEVAL size_t Canary = 0x87654121;
		using base_type = CaptureManagerClass;

	public:

		ExtData(CaptureManagerClass* OwnerObject) : Extension<base_type>(OwnerObject) { }
		virtual ~ExtData() override = default;

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<CaptureExt::ExtData>
	{
	public:
		CONSTEXPR_NOCOPY_CLASS(CaptureExt::ExtData, "CaptureManagerClass");
	};

	static ExtContainer ExtMap;
	*/

	static bool FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bSilent = false);
	static bool CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bRemoveFirst, bool bSilent , AnimTypeClass* pControlledAnimType, int threatDelay);
	static bool CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTechno, bool bSilent, int threatDelay);
	static void DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot , bool Captured);
	static AnimTypeClass* GetMindcontrollAnimType(TechnoClass* pController, TechnoClass* pTarget, AnimTypeClass* pFallback);
	static bool AllowDrawLink(TechnoClass* pTechno , TechnoTypeClass* pType);
};