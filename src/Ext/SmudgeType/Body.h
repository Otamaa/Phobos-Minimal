#pragma once
#include <SmudgeTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>

class SmudgeTypeExtData final
{
public:
	static constexpr size_t Canary = 0xBEE75008;
	using base_type = SmudgeTypeClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	Valueable<bool> Clearable { true };

	SmudgeTypeExtData() noexcept = default;
	~SmudgeTypeExtData() noexcept = default;

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class SmudgeTypeExtContainer final : public Container<SmudgeTypeExtData>
{
public:
	static SmudgeTypeExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(SmudgeTypeExtContainer, SmudgeTypeExtData, "SmudgeTypeClass");
};