#pragma once
#include <CaptureManagerClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <AnimTypeClass.h>
#include <RulesClass.h>
#include <FootClass.h>

class CaptureExtData
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

	static AnimTypeClass* GetMindcontrollAnimType(TechnoClass* pController, TechnoClass* pTarget, AnimTypeClass* pFallback);
};


class NOVTABLE FakeCaptureManagerClass : public CaptureManagerClass
{
public:
	bool __FreeUnit(TechnoClass* pTarget, bool bSilent);
	bool __FreeUnit_Wrap(TechnoClass* pTarget);
	bool __CanCapture(TechnoClass* pTarget);
	bool __CaptureUnit(TechnoClass* pTarget, bool bRemoveFirst, bool bSilent, AnimTypeClass* pControlledAnimType, int threatDelay);
	bool __CaptureUnit(TechnoClass* pTechno, bool bSilent, int threatDelay);
	bool __CaptureUnit_Wrap(AbstractClass* pTechno);
	void __DecideUnitFate(TechnoClass* pTechno, bool Captured);
	void __DecideUnitFate_Wrap(TechnoClass* pTechno);
	int __GetControlledCount();
	int __GetControlledTotalSize();
	bool __IsOverloading(bool* isIt);
	bool __CanControlMore();
	bool __Should_Draw_Link();
	void __DrawControlLinks();
	bool __SetOwnerToCivilian(TechnoClass* pTarget);
};