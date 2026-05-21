#pragma once
#include <DiskLaserClass.h>

#include <Utilities/Container.h>

class DiskLaserExtData : public AbstractExtended
{
public:
	using base_type = DiskLaserClass;
	static COMPILETIMEEVAL const char* ClassName = "DiskLaserExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "DiskLaserClass";

public :
	int WeaponIdx { 0 };

	DiskLaserExtData(DiskLaserClass* pObj) : AbstractExtended(pObj)
	{
		this->AbsType = DiskLaserClass::AbsID;
	}

	DiskLaserExtData(DiskLaserClass* pObj, noinit_t nn) : AbstractExtended(pObj, nn) {}

	virtual ~DiskLaserExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override {}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtended::Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->AbstractExtended::Internal_SaveToStream(Stm);
		const_cast<DiskLaserExtData*>(this)->Serialize(Stm);
	}

	virtual int GetSize() const { return sizeof(*this); };
	virtual void CalculateCRC(CRCEngine& crc) const {}

	DiskLaserClass* This() const { return reinterpret_cast<DiskLaserClass*>(this->AttachedToObject); }
	const DiskLaserClass* This_Const() const { return reinterpret_cast<const DiskLaserClass*>(this->AttachedToObject); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class DiskLaserExtContainer final : public Container<DiskLaserExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "DiskLaserExtContainer";

public:
	static DiskLaserExtContainer Instance;

	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm) { return true; }

};

class NOVTABLE FakeDiskLaserClass : public DiskLaserClass
{
public:

	void __AI();
	void __Fire(TechnoClass* firer, AbstractClass* target, WeaponTypeClass* weapon, int damage_multiplier);
};

static_assert(sizeof(FakeDiskLaserClass) == sizeof(DiskLaserClass), "Invalid Size !");