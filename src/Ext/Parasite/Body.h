#pragma once
#include <ParasiteClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

namespace ParasiteConstants
{
	constexpr int BridgeHeightOffset = 410;
	constexpr int AnimationFrameCount = 10;
	constexpr int AnimationTimingBase = 128;
	constexpr int SmokeZOffset = 100;
	constexpr int MaxHeightForFalling = 200;
	
	// Overlay index ranges for ice/special terrain
	constexpr int SpecialOverlayRangeStart1 = 74;
	constexpr int SpecialOverlayRangeEnd1 = 99;
	constexpr int SpecialOverlayRangeStart2 = 205;
	constexpr int SpecialOverlayRangeEnd2 = 230;
}

class ParasiteExtData
{
public:
	/*
	class ExtData final : public Extension<ParasiteClass>
	{
	public:
		static COMPILETIMEEVAL size_t Canary = 0x99954321;
		using base_type = ParasiteClass;

	public:

		CoordStruct LastVictimLocation {};
		ExtData(ParasiteClass* OwnerObject) : Extension<ParasiteClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};


	class ExtContainer final : public Container<ParasiteExt::ExtData>
	{
	public:
		CONSTEXPR_NOCOPY_CLASS(ParasiteExt::ExtData, "ParasiteClass");
	};

	static ExtContainer ExtMap;
	*/
};


class FakeParasiteClass : public ParasiteClass { 
public:

	FORCEDINLINE FakeParasiteClass* _AsParasite() const
	{
		return ((FakeParasiteClass*)this);
	}


	CoordStruct __Detach_From_Victim();
	bool __Victims_Cell_Valid();
	void __Infect(FootClass* target);
	void __Uninfect();
	void __Detach(AbstractClass* detachingObject, bool permanent);
	void __AI();
	void __Grapple_AI();
	bool __Update_GrappleAnim_Frame();
	void __ClearAnim();

	HRESULT __stdcall _Load(IStream* pStm)
	{
		return S_OK;
	}

	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty)
	{
		return S_OK;
	}

private:
	bool IsSpecialOverlay(int overlayIndex) const;
	void ResetOwnerMission(FootClass* owner);

};
static_assert(sizeof(FakeParasiteClass) == sizeof(ParasiteClass), "FakeParasiteClass size mismatch");