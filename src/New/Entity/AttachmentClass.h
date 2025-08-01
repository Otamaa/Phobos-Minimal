#pragma once

#include <CDTimer.h>

#include <New/Type/AttachmentTypeClass.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/VectorHelper.h>

class TechnoClass;
class AttachmentClass
{
public:
	static HelperedVector<AttachmentClass*> Array;

	TechnoTypeExtData::AttachmentDataEntry* Data;
	TechnoClass* Parent;
	TechnoClass* Child;
	CDTimerClass RespawnTimer;

	AttachmentClass(TechnoTypeExtData::AttachmentDataEntry* data,
		TechnoClass* pParent, TechnoClass* pChild = nullptr) :
		Data { data },
		Parent { pParent },
		Child { pChild },
		RespawnTimer { }
	{
		Array.push_back(this);
	}

	AttachmentClass() :
		Data { },
		Parent { },
		Child { },
		RespawnTimer { }
	{
		Array.push_back(this);
	}

	~AttachmentClass();

	AttachmentTypeClass* GetType();
	TechnoTypeClass* GetChildType();
	CoordStruct GetChildLocation();

	void Initialize();
	void CreateChild();
	void AI();
	void Destroy(TechnoClass* pSource);
	void ChildDestroyed();

	void Unlimbo();
	void Limbo();

	bool AttachChild(TechnoClass* pChild);
	bool DetachChild();

	void InvalidatePointer(void* ptr);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};

template <>
struct Savegame::ObjectFactory<AttachmentClass>
{
	std::unique_ptr<AttachmentClass> operator() (PhobosStreamReader& Stm) const
	{
		return std::make_unique<AttachmentClass>();
	}
};