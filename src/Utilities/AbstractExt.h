#pragma once
#include <Base/Always.h>
#include <AbstractClass.h>
#include <AircraftClass.h>

#include <unordered_map>

typedef short ExtensionIndex;

typedef enum ExtensionRTTIType : char
{
	EXT_RTTI_NONE = -1,

	EXT_RTTI_AIRCRAFT = 0,
	EXT_RTTI_AIRCRAFTTYPE,
	EXT_RTTI_ANIM,
	EXT_RTTI_ANIMTYPE,
	EXT_RTTI_BUILDING,
	EXT_RTTI_BUILDINGTYPE,
	EXT_RTTI_BULLETCLASS,
	EXT_RTTI_BULLETTYPE,
	EXT_RTTI_CAMPAIGN,
	EXT_RTTI_EMPULSE,
	EXT_RTTI_FACTORY,
	EXT_RTTI_FOOT,
	EXT_RTTI_HOUSE,
	EXT_RTTI_HOUSETYPE,
	EXT_RTTI_INFANTRY,
	EXT_RTTI_INFANTRYTYPE,
	EXT_RTTI_ISOTILETYPE,
	EXT_RTTI_OBJECTTYPE,
	EXT_RTTI_OVERLAYTYPE,
	EXT_RTTI_PARTICLESYSTEM,
	EXT_RTTI_PARTICLESYSTEMTYPE,
	EXT_RTTI_PARTICLETYPE,
	EXT_RTTI_SMUDGETYPE,
	EXT_RTTI_SUPER,
	EXT_RTTI_SUPERWEAPONTYPE,
	EXT_RTTI_TECHNO,
	EXT_RTTI_TECHNOTYPE,
	EXT_RTTI_TERRAIN,
	EXT_RTTI_TERRAINTYPE,
	EXT_RTTI_TIBERIUM,
	EXT_RTTI_UNIT,
	EXT_RTTI_UNITTYPE,
	EXT_RTTI_VOXELANIMTYPE,
	EXT_RTTI_WARHEADTYPE,
	EXT_RTTI_WAVE,
	EXT_RTTI_WEAPONTYPE,

	EXT_RTTI_COUNT
};

class AbstractClassExtension
{
public:
	AbstractClassExtension(ExtensionRTTIType rtti) :
		RTTI(rtti),
		IsDirty(false)
	{
	}

	AbstractClassExtension(const noinit_t& noinit)
	{
	}

	virtual ~AbstractClassExtension()
	{
	}

	/**
	 *  Initializes an object from the stream where it was saved previously.
	 */
	virtual HRESULT Load(IStream* pStm);

	/**
	 *  Saves an object to the specified stream.
	 */
	virtual HRESULT Save(IStream* pStm, BOOL fClearDirty);

	/**
	 *  Return the raw size of class data for save/load purposes.
	 *
	 *  @note: This must be overridden by the extended class!
	 */
	virtual int Size_Of() const { return sizeof(AbstractClassExtension); }

	/**
	 *  Removes the specified target from any targeting and reference trackers.
	 */
	virtual void Detach(AbstractClass* target, bool all = true) { }

	/**
	 *  Compute a unique crc value for this instance.
	 */
	virtual void Compute_CRC(Checksummer& crc) const { }

public:
	/**
	 *  x
	 */
	ExtensionRTTIType RTTI;

	/**
	 *  Has the object changed since the last save?
	 */
	bool IsDirty;

private:
	AbstractClassExtension(const AbstractClassExtension&) = delete;
	void operator = (const AbstractClassExtension&) = delete;
};

AbstractClassExtension* Fetch_Extension(ExtensionRTTIType rtti, ExtensionIndex index);
AbstractClassExtension* Fetch_Extension(AbstractClass* abs);

class AircraftClassExtension;
extern DynamicVectorClass<AircraftClassExtension*> AircraftExtensions;

class TechnoClassExtensions
{
public:
	TechnoClassExtensions() { }
	TechnoClassExtensions(const noinit_t& noinit) { }
	~TechnoClassExtensions() = default;
};

class FootClassExtensions : public TechnoClassExtensions
{
public:
	FootClassExtensions() : TechnoClassExtensions() { }
	FootClassExtensions(const noinit_t& noinit) : TechnoClassExtensions(noinit) { }
	~FootClassExtensions() = default;
};

static void Set_Extension_Pointer(const AbstractClass* abs, const AbstractClassExtension* abstract_extension)
{
	if (!abstract_extension || !abs)
		return;

	(*(uintptr_t*)((char*)abs + 0x10)) = (uintptr_t)abstract_extension;
}


class AircraftClassExtension final : public AbstractClassExtension
{
public:

	FootClassExtensions* FootExt;

	AircraftClassExtension(AircraftClass* this_ptr) : AbstractClassExtension(EXT_RTTI_AIRCRAFT)
	, FootExt { nullptr }
	{
		Set_Extension_Pointer(this_ptr, this);
		AircraftExtensions.AddItem(this);
	}

	AircraftClassExtension(const noinit_t& noinit) : AbstractClassExtension(noinit) { }
	~AircraftClassExtension() { AircraftExtensions.Remove(this); }

	virtual HRESULT Load(IStream* pStm) override { return S_OK; }
	virtual HRESULT Save(IStream* pStm, BOOL fClearDirty) override { return S_OK; }
	virtual int Size_Of() const override { return sizeof(AircraftClassExtension); }

	virtual void Detach(AbstractClass* target, bool all = true) override { }
	virtual void Compute_CRC(Checksummer& crc) const override { }


	static AircraftClassExtension* Get_Extension_Pointer(const AircraftClass* abs)
	{
		if (!abs || abs->WhatAmI() != AbstractType::Aircraft)
			return nullptr;

		uintptr_t abstract_extension_address = (*(uintptr_t*)((char*)abs + 0x10));
		AbstractClassExtension* abstract_extension = (AbstractClassExtension*)abstract_extension_address;
		return (AircraftClassExtension*)abstract_extension;
	}

};

static AbstractClassExtension* Get_Extension_Pointer(const AbstractClass* abs)
{
	if (!abs)
		return nullptr;

	uintptr_t abstract_extension_address = (*(uintptr_t*)((char*)abs + 0x10));
	AbstractClassExtension* abstract_extension = (AbstractClassExtension*)abstract_extension_address;
	return abstract_extension;
}