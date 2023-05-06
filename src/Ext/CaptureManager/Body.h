#pragma once
#include <CaptureManagerClass.h>

#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

#include <AnimTypeClass.h>
#include <RulesClass.h>
#include <FootClass.h>

class CaptureExt
{
public:
	class ExtData final : public Extension<CaptureManagerClass>
	{
	public:

		static constexpr size_t Canary = 0x87654121;
		using base_type = CaptureManagerClass;

	public:

		ExtData(CaptureManagerClass* OwnerObject) : Extension<base_type>(OwnerObject)
		{ }

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
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget);
	static bool FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bSilent = false);
	static bool CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bRemoveFirst, bool bSilent , AnimTypeClass* pControlledAnimType);
	static bool CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTechno);
	static void DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot);
	static AnimTypeClass* GetMindcontrollAnimType(TechnoClass* pController, TechnoClass* pTarget, AnimTypeClass* pFallback);
	static bool AllowDrawLink(TechnoTypeClass* pType);
};