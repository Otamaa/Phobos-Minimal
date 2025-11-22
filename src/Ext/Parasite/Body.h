#pragma once
#include <ParasiteClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

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

	}

	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty)
	{

	}

};
static_assert(sizeof(FakeParasiteClass) == sizeof(ParasiteClass), "FakeParasiteClass size mismatch");