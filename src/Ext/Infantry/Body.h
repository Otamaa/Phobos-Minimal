#pragma once
#include <InfantryClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <ExtraHeaders/CompileTimeDirStruct.h>

class InfantryExtData final
{
public:
	static constexpr size_t Canary = 0xACCAAAAA;
	using base_type = InfantryClass;
	static constexpr size_t ExtOffset = 0x6EC;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	bool IsUsingDeathSequence { false };
	int CurrentDoType { -1 };
	bool ForceFullRearmDelay { false };

	InfantryExtData() noexcept = default;
	~InfantryExtData() noexcept = default;

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class InfantryExtContainer final : public Container<InfantryExtData>
{
public:
	static InfantryExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(InfantryExtContainer, InfantryExtData, "InfantryClass");
};