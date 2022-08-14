#pragma once
#include <CaptureManagerClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <AnimTypeClass.h>
#include <RulesClass.h>
#include <FootClass.h>

class CaptureExt
{
public:
	static constexpr size_t Canary = 0x87654121;
	using base_type = CaptureManagerClass;
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

	class ExtData final : public Extension<base_type>
	{
	public:

		ExtData(CaptureManagerClass* OwnerObject) : Extension<base_type>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		//virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void InvalidatePointer(void* ptr, bool bRemoved) { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static CaptureExt::ExtData* GetExtData(base_type* pThis);

	class ExtContainer final : public Container<CaptureExt>
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

	static void __stdcall DrawLinkTo(CoordStruct nFrom, CoordStruct nTo, ColorStruct color) {
		JMP_STD(0x704E40);
	}

	static inline int FixIdx(const Iterator<int>& iter, int nInput) {
		return iter.empty() ? 0 : iter[nInput > static_cast<int>(iter.size()) ? static_cast<int>(iter.size()) : nInput];
	}

	static bool AllowDrawLink(TechnoTypeClass* pType);
	static void __fastcall Overload_AI(CaptureManagerClass* pThis, void* _);
};