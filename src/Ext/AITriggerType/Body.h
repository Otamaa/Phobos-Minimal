#pragma once

#include <AITriggerTypeClass.h>
#include <Ext/Abstract/Body.h>

//this is a 1-based index.
enum class PhobosAIConditionTypes : int
{
	CustomizableAICondition = 1,
};

enum class PhobosAINewConditionTypes : int
{
	CheckPrereq = 8,
	CheckBridgeCondition = 9
};

class AITriggerTypeExt
{
public:
	using base_type = AITriggerTypeClass;
	static constexpr size_t Canary = 0x2C2C2C2C;
	static constexpr size_t ExtOffset = 0x10C;

	class ExtData final : public Extension<AITriggerTypeClass>
	{
	public:

		//Valueable<HouseTypeClass*> NoneOF; String ?
		ExtData(AITriggerTypeClass* OwnerObject) : Extension<AITriggerTypeClass>(OwnerObject)
			//, NoneOF { }
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

	class ExtContainer final : public Container<AITriggerTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void ProcessCondition(AITriggerTypeClass* pAITriggerType, HouseClass* pHouse, int type, int condition);

	static void DisableAITrigger(AITriggerTypeClass* pAITriggerType);
	static void EnableAITrigger(AITriggerTypeClass* pAITriggerType);
	static bool ReadCustomizableAICondition(HouseClass* pHouse, int pickMode, int compareMode, int Number, TechnoTypeClass* TechnoType);
	static void CustomizableAICondition(AITriggerTypeClass* pAITriggerType, HouseClass* pHouse, int condition);

	static ExtContainer ExtMap;

};