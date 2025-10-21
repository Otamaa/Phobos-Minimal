#pragma once

#include <Utilities/Container.h>
#include <Utilities/TemplateDefB.h>

class CCINIClass;
class AbstractTypeExtData : public AbstractExtended
{
public:

	AbstractTypeExtData(AbstractTypeClass* pObj) : AbstractExtended(pObj) {
		auto pIdent = Phobos::gEntt->try_get<ExtensionIdentifierComponent>(this->MyEntity);
		pIdent->Name = pObj->ID;
	}
	AbstractTypeExtData(AbstractTypeClass* pObj, noinit_t nn) : AbstractExtended(pObj, nn) { }

	virtual ~AbstractTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override {}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtended::Internal_LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->AbstractExtended::Internal_SaveToStream(Stm);
	}

	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	OPTIONALINLINE const char* Name() const {
		auto pIdent = Phobos::gEntt->try_get<ExtensionIdentifierComponent>(this->MyEntity);
		return pIdent->Name;
	}
	OPTIONALINLINE const char* Full_Name() const { return This()->Name; }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr) = 0;
	virtual bool WriteToINI(CCINIClass* pINI) const = 0;

	virtual AbstractTypeClass* This() const override { return reinterpret_cast<AbstractTypeClass*>(this->AbstractExtended::This()); }
	virtual const AbstractTypeClass* This_Const() const override { return reinterpret_cast<const AbstractTypeClass*>(this->AbstractExtended::This_Const()); }
};