#pragma once

#include <TriggerTypeClass.h>
#include <Ext/Abstract/Body.h>

class TriggerClass;
class TriggerTypeExt
{
public:
	using base_type = TriggerTypeClass;
	static constexpr size_t Canary = 0x2C2C2C2C;

	class ExtData final : public Extension<TriggerTypeClass>
	{
	public:

		int HouseParam;
		ExtData(TriggerTypeClass* OwnerObject) : Extension<TriggerTypeClass>(OwnerObject) ,
			HouseParam { -1 }
		{ }

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {
			if (this->InvalidateIgnorable(ptr))
				return;
		}

		virtual bool InvalidateIgnorable(void* const ptr) const override { return true;  }
		virtual void LoadFromStream(PhobosStreamReader& Stm);
		virtual void SaveToStream(PhobosStreamWriter& Stm);
	};

	class ExtContainer final : public Container<TriggerTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static HouseClass* ResolveHouseParam(int const param, HouseClass* const pOwnerHouse);

};