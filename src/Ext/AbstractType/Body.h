#pragma once

#include <Utilities/Container.h>
#include <Utilities/TemplateDefB.h>

class CCINIClass;
class AbstractTypeExtData : public AbstractExtended
{
public:
	PhobosFixedString<0x18> Name {};
	DWORD CameoPriority_Houses { 0u };
	Valueable<int> CameoPriority { 0 };
public:

	AbstractTypeExtData(AbstractTypeClass* pObj) : AbstractExtended(pObj) {
		this->Name = pObj->ID;
	}
	AbstractTypeExtData() = default;

	virtual ~AbstractTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override {}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtended::Internal_LoadFromStream(Stm);
		Stm
			.Process(Name)
			.Process(CameoPriority_Houses)
			.Process(CameoPriority)
			;
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->AbstractExtended::Internal_SaveToStream(Stm);
		Stm.Process(Name)
			.Process(CameoPriority_Houses)
			.Process(CameoPriority)
			;
	}

	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	OPTIONALINLINE const char* Full_Name() const { return This()->Name; }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const = 0;

	AbstractTypeClass* This() const { return reinterpret_cast<AbstractTypeClass*>(this->AttachedToObject); }
	const AbstractTypeClass* This_Const() const { return reinterpret_cast<const AbstractTypeClass*>(this->AttachedToObject); }
};