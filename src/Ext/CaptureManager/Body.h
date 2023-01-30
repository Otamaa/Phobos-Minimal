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

	static constexpr size_t Canary = 0x87654121;
	using base_type = CaptureManagerClass;

	class ExtData final : public TExtension<base_type>
	{
	public:

		ExtData(CaptureManagerClass* OwnerObject) : TExtension<base_type>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void InitializeConstants();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<CaptureExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget);
	static bool FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bSilent = false);
	static bool CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget,
		bool bRemoveFirst, AnimTypeClass* pControlledAnimType = RulesClass::Instance->ControlledAnimationType);
	static bool CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTechno,
		AnimTypeClass* pControlledAnimType = RulesClass::Instance->ControlledAnimationType);
	static void DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot);

	static int FixIdx(const Iterator<int>& iter, int nInput) {
		return iter.empty() ? 0 : iter[nInput > static_cast<int>(iter.size()) ? static_cast<int>(iter.size()) : nInput];
	}

	static bool AllowDrawLink(TechnoTypeClass* pType);
};