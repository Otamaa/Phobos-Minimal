#include "Body.h"

#include <Ext/Rules/Body.h>
#include <Utilities/TemplateDef.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include <string>
// =============================
// container
AITriggerTypeExtContainer AITriggerTypeExtContainer::Instance;

int AITriggerTypeExtData::CheckConditions(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	int condition = (int)pThis->ConditionType;

	if (condition < -1) // Invalid, bail out early.
		return 0;
	else if (condition < (int)PhobosAINewConditionTypes::NumberOfTechBuildingsExist) // Not Phobos condition
		return -1;

	bool success = false;

	switch ((PhobosAINewConditionTypes)condition)
	{
	case PhobosAINewConditionTypes::NumberOfTechBuildingsExist:
		success = AITriggerTypeExtData::NumberOfTechBuildingsExist(pThis, pOwner);
		break;
	case PhobosAINewConditionTypes::NumberOfBridgeRepairHutsExist:
		success = AITriggerTypeExtData::NumberOfBridgeRepairHutsExist(pThis);
		break;
	}

	return success;
}

bool AITriggerTypeExtData::GetComparatorResult(int operand1, AITriggerConditionComparatorType operatorType, int operand2)
{
	switch (operatorType)
	{
	case AITriggerConditionComparatorType::Less:
		return operand1 < operand2;
		break;
	case AITriggerConditionComparatorType::LessOrEqual:
		return operand1 <= operand2;
		break;
	case AITriggerConditionComparatorType::Equal:
		return operand1 == operand2;
		break;
	case AITriggerConditionComparatorType::GreaterOrEqual:
		return operand1 >= operand2;
		break;
	case AITriggerConditionComparatorType::Greater:
		return operand1 > operand2;
		break;
	case AITriggerConditionComparatorType::NotEqual:
		return operand1 != operand2;
		break;
	default:
		return false;
		break;
	}
}

bool AITriggerTypeExtData::NumberOfTechBuildingsExist(AITriggerTypeClass* pThis, HouseClass* pOwner)
{
	int count = 0;

	for (auto const pHouse : *HouseClass::Array)
	{
		if (pHouse->IsAlliedWith(pOwner))
			continue;

		// Could possibly be optimized with bespoke tracking but
		// it didn't seem to make much of a difference in testing.
		for (auto const pBuilding : pHouse->Buildings)
		{
			if (!pBuilding->IsAlive || pBuilding->InLimbo)
				continue;

			auto const pType = pBuilding->Type;

			if (pType->NeedsEngineer && pType->Capturable)
				count++;
		}
	}

	return AITriggerTypeExtData::GetComparatorResult(count, pThis->Conditions[0].Type, pThis->Conditions[0].Operand);
}

bool AITriggerTypeExtData::NumberOfBridgeRepairHutsExist(AITriggerTypeClass* pThis)
{
	int count = 0;
	auto const pHouse = HouseClass::FindCivilianSide();

	for (auto const pBuilding : pHouse->Buildings)
	{
		if (!pBuilding->IsAlive || pBuilding->InLimbo)
			continue;

		auto const pType = pBuilding->Type;

		if (pType->BridgeRepairHut && MapClass::Instance->IsLinkedBridgeDestroyed(pBuilding->GetMapCoords()))
			count++;
	}

	return AITriggerTypeExtData::GetComparatorResult(count, pThis->Conditions[0].Type, pThis->Conditions[0].Operand);
}

template <typename T>
void AITriggerTypeExtData::Serialize(T& Stm)
{

}

#ifdef _NOT

void AITriggerTypeExt::ProcessCondition(AITriggerTypeClass* pAITriggerType, HouseClass* pHouse, int type, int condition)
{
	//AITriggerType is disabled by default
	DisableAITrigger(pAITriggerType);
	switch (static_cast<PhobosAIConditionTypes>(type))
	{
	case PhobosAIConditionTypes::CustomizableAICondition:
		AITriggerTypeExt::CustomizableAICondition(pAITriggerType, pHouse, condition);
		break;
	default:
		break;
	}
	return;
}

void AITriggerTypeExt::DisableAITrigger(AITriggerTypeClass* pAITriggerType)
{
	pAITriggerType->ConditionType = AITriggerCondition::AIOwns;
	pAITriggerType->ConditionObject = nullptr;
	return;
}

void AITriggerTypeExt::EnableAITrigger(AITriggerTypeClass* pAITriggerType)
{
	pAITriggerType->ConditionType = AITriggerCondition::Pool;
	pAITriggerType->ConditionObject = nullptr;
	return;
}

bool AITriggerTypeExt::ReadCustomizableAICondition(HouseClass* pHouse, int pickMode, int compareMode, int Number, TechnoTypeClass* TechnoType)
{
	//0 = pick enemies(except for neutral); 1 = pick allies(except for neutral); 2 = pick self; 3 = pick all(except for neutral);
	//4 = pick enemy human players; 5 = pick allied human players; 6 = pick all human players;
	//7 = pick enemy computer players(except for neutral); 8 = pick allied computer players(except for neutral); 9 = pick all computer players(except for neutral);
	//10 = pick neutral; 11 = pick all(including neutral);
	//int pickMode;

	//0 = "<"; 1 = "<="; 2 = "=="; 3 = ">="; 4 = ">"; 5 = "!=";
	//int compareMode;

	int count = 0;

	std::ranges::for_each(*TechnoClass::Array, [&](const TechnoClass* pTechno) {
		if (GET_TECHNOTYPE(pTechno) == TechnoType
			&& pTechno->IsAlive
			&& !pTechno->InLimbo
			&& pTechno->IsOnMap
			&& !pTechno->Absorbed
			&& pTechno->Owner
			&& ((!pTechno->Owner->IsAlliedWith(pHouse) && !pTechno->Owner->IsNeutral() && pickMode == 0)
				|| (pTechno->Owner->IsAlliedWith(pHouse) && !pTechno->Owner->IsNeutral() && pickMode == 1)
				|| (pTechno->Owner == pHouse && pickMode == 2)
				|| (!pTechno->Owner->IsNeutral() && pickMode == 3)
				|| (pTechno->Owner->IsControlledByHuman() && !pTechno->Owner->IsAlliedWith(pHouse) && pickMode == 4)
				|| (pTechno->Owner->IsControlledByHuman() && pTechno->Owner->IsAlliedWith(pHouse) && pickMode == 5)
				|| (pTechno->Owner->IsControlledByHuman() && pickMode == 6)
				|| (!pTechno->Owner->IsControlledByHuman() && !pTechno->Owner->IsNeutral() && !pTechno->Owner->IsAlliedWith(pHouse) && pickMode == 7)
				|| (!pTechno->Owner->IsControlledByHuman() && !pTechno->Owner->IsNeutral() && pTechno->Owner->IsAlliedWith(pHouse) && pickMode == 8)
				|| (!pTechno->Owner->IsControlledByHuman() && !pTechno->Owner->IsNeutral() && pickMode == 9)
				|| (pTechno->Owner->IsNeutral() && pickMode == 10)
				|| (pickMode == 11)
				))

		{
			count++;
		}
	});

	return ((count < Number && compareMode == 0)
		|| (count <= Number && compareMode == 1)
		|| (count == Number && compareMode == 2)
		|| (count >= Number && compareMode == 3)
		|| (count > Number && compareMode == 4)
		|| (count != Number && compareMode == 5)
		);
}

void AITriggerTypeExt::CustomizableAICondition(AITriggerTypeClass* pAITriggerType, HouseClass* pHouse, int condition)
{
	auto& AIConditionsLists = RulesExtData::Instance()->AIConditionsLists;

	int essentialRequirementsCount = -1;
	int leastOptionalRequirementsCount = -1;
	int essentialRequirementsMetCount = 0;
	int optionalRequirementsMetCount = 0;

	if ((size_t)condition < AIConditionsLists.size())
	{
		auto& thisAICondition = AIConditionsLists[condition];

		if (thisAICondition.size() < 2)
		{
			pAITriggerType->IsEnabled = false;
			Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition: [AIConditionsList]: Error parsing line [{}].", condition);
			return;
		}

		//parse first string
		char* context = nullptr;
		char* cur[3] {};
		cur[0] = strtok_s(thisAICondition[0].data(), Phobos::readDelims, &context);
		int j = 0;
		while (cur[j])
		{
			j++;
			cur[j] = strtok_s(NULL, Phobos::readDelims, &context);
		}

		if (cur[0])
			essentialRequirementsCount = atoi(cur[0]);
		else
			Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition : [AIConditionsList]: Error parsing Essential Requirements Count [0] !.");

		if (cur[1])
			leastOptionalRequirementsCount = atoi(cur[1]);
		else
			Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition : [AIConditionsList]: Error parsing Least Optional Requirements Count [1] !.");

		//parse other strings
		for (int i = 1; i < (int)thisAICondition.size(); i++)
		{
			int pickMode = -1;
			int compareMode = -1;
			int Number = -1;
			TechnoTypeClass* TechnoType;

			char* cur2[5] {};
			cur2[0] = strtok_s(thisAICondition[i].data(), Phobos::readDelims, &context);
			int k = 0;
			while (cur2[k])
			{
				k++;
				cur2[k] = strtok_s(NULL, Phobos::readDelims, &context);
			}
			TechnoTypeClass* buffer;
			if (Parser<TechnoTypeClass*>::TryParse(cur2[3], &buffer))
			{
				if (cur2[0])
					pickMode = atoi(cur2[0]);
				else
					Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition : [AIConditionsList]: Error parsing Pick [0] !.");

				if (cur2[1])
					compareMode = atoi(cur2[1]);
				else
					Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition : [AIConditionsList]: Error parsing Compare [1] !.");

				if (cur2[2])
					Number = atoi(cur2[2]);
				else
					Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition : [AIConditionsList]: Error parsing Number [2] !.");

				TechnoType = buffer;
			}
			else
			{
				Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition: [AIConditionsList][{}]: Error parsing [{}]", condition, cur2[3]);
				Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition: [AIConditionsList]: Error parsing line [{}].", condition);
				pAITriggerType->IsEnabled = false;
				return;
			}

			if (essentialRequirementsCount > -1
				&& leastOptionalRequirementsCount > -1
				&& essentialRequirementsCount + leastOptionalRequirementsCount < (int)thisAICondition.size()
				&& pickMode >= 0 && pickMode <= 11
				&& compareMode >= 0 && compareMode <= 5
				&& Number >= 0)
			{
				//essential requirements judgment
				if (i <= essentialRequirementsCount)
				{
					if (ReadCustomizableAICondition(pHouse, pickMode, compareMode, Number, TechnoType))
						essentialRequirementsMetCount++;
				}
				//optional requirements judgment
				else
				{
					if (ReadCustomizableAICondition(pHouse, pickMode, compareMode, Number, TechnoType))
						optionalRequirementsMetCount++;
				}
			}
			else
			{
				Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition: [AIConditionsList]: Error parsing line [{}].", condition);
				pAITriggerType->IsEnabled = false;
				return;
			}
		}
	}
	else
	{
		//thoroughly disable it
		pAITriggerType->IsEnabled = false;
		Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition: [AIConditionsList]: Condition number overflew!.");
		return;
	}
	if (essentialRequirementsCount == essentialRequirementsMetCount && leastOptionalRequirementsCount <= optionalRequirementsMetCount)
		EnableAITrigger(pAITriggerType);

	return;
}

#endif

void AITriggerTypeExtContainer::LoadFromINI(AITriggerTypeClass* key, CCINIClass* pINI, bool parseFailAddr)
{
	if (auto ptr = this->Find(key))
	{
		if (!pINI)
		{
			return;
		}

		// Rules first 
		// Other files 
		// when this doesnt match the case it will causing weirdd issues like some value wont be initialized or replaced to default value after parsing
		switch (ptr->Initialized)
		{
		case InitState::Blank:
		{
			if (pINI == CCINIClass::INI_Rules())
			{
				ptr->SetInitState(InitState::Inited);
				//ptr->Initialize();
			}
			[[fallthrough]];
		}
		case InitState::Inited:
		case InitState::Ruled:
		{
			ptr->LoadFromINI(pINI, parseFailAddr);
			ptr->SetInitState(InitState::Ruled);
			[[fallthrough]];
		}
		default:
			break;
		}
	}

}

void AITriggerTypeExtContainer::WriteToINI(AITriggerTypeClass* key, CCINIClass* pINI)
{

	if (auto ptr = this->TryFind(key))
	{
		if (!pINI)
		{
			return;
		}

		ptr->WriteToINI(pINI);
	}
}


ASMJIT_PATCH(0x41E471, AITriggerTypeClass_CTOR, 0x7)
{
	GET(AITriggerTypeClass*, pThis, ESI);
	if(!Phobos::Otamaa::DoingLoadGame)
	AITriggerTypeExtContainer::Instance.Allocate(pThis);
	return 0x0;
}

ASMJIT_PATCH(0x41E4AF, AITriggerTypeClass_DTOR, 0x6)
{
	GET(AITriggerTypeClass*, pThis, ESI);
	AITriggerTypeExtContainer::Instance.Remove(pThis);
	return 0x0;
}

HRESULT __stdcall FakeAITriggerTypeClass::__Load(IStream* pStm)
{
	HRESULT hr = this->AITriggerTypeClass::Load(pStm);

	if (SUCCEEDED(hr)) {
		if (!AITriggerTypeExtContainer::Instance.LoadByKey(this, pStm))
			return PHOBOS_E_EXTDATA_LOAD_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2A64, FakeAITriggerTypeClass::__Load)

HRESULT __stdcall FakeAITriggerTypeClass::__Save(IStream* pStm, BOOL fClearDirty)
{
	HRESULT hr = this->AITriggerTypeClass::Save(pStm, fClearDirty);

	if (SUCCEEDED(hr)) {
		if (!AITriggerTypeExtContainer::Instance.SaveByKey(this, pStm))
			return PHOBOS_E_EXTDATA_SAVE_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2A68, FakeAITriggerTypeClass::__Save)

ASMJIT_PATCH(0x41E8FF, AITriggerTypeClass_NewTeam_CheckConditions, 0x9) // ConditionMet() in YRpp
{
	enum { ContinueGameChecks = 0x41E908, ReturnFromFunction = 0x41E9E1, Success = 0x41E9EA };

	GET(AITriggerTypeClass*, pThis, ESI);
	GET(HouseClass*, pOwner, EDI);
	GET(HouseClass*, pEnemy, EBX);

	int result = AITriggerTypeExtData::CheckConditions(pThis, pOwner, pEnemy);

	switch (result)
	{
	case 0:
		return ReturnFromFunction;
		break;
	case 1:
		return Success;
		break;
	default:
		return ContinueGameChecks;
		break;
	}
}

//AbstractTypeClass* ResolveTechType(const char* name)
//{
//	int idx = InfantryTypeClass::From_Name(name);
//	if (idx != -1)
//		return InfantryTypes.Vector[idx]; // VERIFY: YRpp accessor name
//
//	idx = UnitTypeClass::From_Name(name);
//	if (idx != -1)
//		return UnitTypes.Vector[idx];     // VERIFY: YRpp accessor name
//
//	idx = AircraftTypeClass::From_Name(name);
//	if (idx != -1)
//		return AircraftTypes.Vector[idx]; // VERIFY: YRpp accessor name
//
//	idx = BuildingTypeClass::From_Name(name);
//	if (idx != -1)
//		return BuildingTypes.Vector[idx]; // VERIFY: YRpp accessor name
//
//	return nullptr;
//}
//
//void ParseConditions(const char* str, AITriggerTypeClass* self)
//{
//	// Vanilla lookup table initializer from stack:
//	//   var_270 = word_818170 (global 2-byte value)
//	//   var_270+2 = byte_818172 (global 1-byte value)
//	//   anonymous_0 (qword) = 0
//	// Together: a 3-entry sorted array used as the binary search range.
//	// VERIFY: these globals and their exact layout in YRpp/assembly
//	unsigned short lookup[2];
//	lookup[0] = '00'; // VERIFY: global name
//	reinterpret_cast<uint8_t*>(lookup)[2] = '0'; // VERIFY: global name
//	const unsigned short* const lookupEnd = reinterpret_cast<const unsigned short*>(
//		reinterpret_cast<const uint8_t*>(lookup) + sizeof(unsigned short) + 1);
//
//	unsigned int count = 0;
//	const char* p = str;
//
//	while (*p && count < 0x20)
//	{
//		// skip whitespace
//		while (*p && std::isspace(static_cast<unsigned char>(*p)))
//			++p;
//
//		// read two characters as a pair
//		unsigned short pair = 0;
//		reinterpret_cast<char*>(&pair)[0] = *p;
//		if (*p) ++p;
//
//		const char second = *p;
//		reinterpret_cast<char*>(&pair)[1] = second ? second : '\0';
//		if (second) ++p;
//
//		// binary search into lookup table
//		// vanilla: std::lower_bound(&var_270, &anonymous_0, 0x10)
//		// stores result byte (al) into Conditions[count]
//		const unsigned short* found = std::lower_bound(lookup, lookupEnd, static_cast<unsigned short>(0x10));
//		self->Conditions[count] = static_cast<uint8_t>(
//			reinterpret_cast<const uint8_t*>(found)[0]); // VERIFY: result extraction
//
//		++count;
//
//		if (!*p)
//			break;
//	}
//}

bool  FakeAITriggerTypeClass::_SaveToINI(CCINIClass* pINI)
{
	/*
	* 
	char v44[512];

	INIClass::Clear_Section_Cache(iniHandle);

	if (!INIClass::Get_String(iniHandle, "AITriggerTypes", this->IniName, // VERIFY: field name
		&Wstring::EmptyString, v44, 512))
		return 0;

	// --- token 1: name (48 chars + null) ---
	// Vanilla: strncpy into destination[48]+sentinel, then rep movsd(x12)+movsb
	// into this->at.Name. Stack bounce preserved semantically via strncpy limit.
	const char* tok = strtok(v44, ",");
	if (!tok)
		return 0;

	std::strncpy(this->Name, tok, 48u); // VERIFY: field name in YRpp
	this->Name[48] = '\0';

	// --- token 2: TeamTypeOne ---
	tok = strtok(nullptr, ",");
	if (!tok)
		return 0;

	{
		char buf[24];
		std::strncpy(buf, tok, 23u);
		buf[23] = '\0';
		strtrim(buf); // VERIFY: strtrim signature

		this->TeamTypeOne = nullptr; // VERIFY: field name
		if (_strcmpi(buf, none_str)) // VERIFY: none_str global
			this->TeamTypeOne = TeamTypeClass::From_Name(buf);
	}

	// --- token 3: owning house ---
	tok = strtok(nullptr, ",");
	if (!tok)
		return 0;

	{
		char buf[24];
		std::strncpy(buf, tok, 23u);
		buf[23] = '\0';
		strtrim(buf);

		this->OwnHouseType = 0;  // VERIFY: field name
		this->OwningHouse = -1; // VERIFY: field name

		if (!_strcmpi(buf, alllstring)) // VERIFY: alllstring global
		{
			this->OwnHouseType = 2;
		}
		else if (_strcmpi(buf, none_str))
		{
			const int houseIdx = HouseTypeClass::From_Name(buf);
			this->OwningHouse = houseIdx;
			if (houseIdx != -1)
				this->OwnHouseType = 1;
		}
	}

	// --- token 4: discarded (reserved field) ---
	// Assembly: strtok called, result checked for null but value unused.
	if (!strtok(nullptr, ","))
		return 0;

	// --- token 5: TechLevel (always initialized to 0 before token consumption) ---
	this->TechLevel = 0; // VERIFY: field name
	tok = strtok(nullptr, ",");
	if (!tok)
		return 0;
	this->ConditionType = std::atoi(tok); // VERIFY: field name

	// --- token 6: ConditionType ---
	tok = strtok(nullptr, ",");
	if (!tok)
		return 0;
	this->ConditionType = std::atoi(tok); // VERIFY: field name — assembly: [ebp+98h]

	// --- token 7: ConditionObject (tech type name) ---
	tok = strtok(nullptr, ",");
	if (!tok)
		return 0;

	{
		char buf[24];
		std::strncpy(buf, tok, 23u);
		buf[23] = '\0';
		strtrim(buf);

		// Vanilla: tries all four type vectors in order, stores raw pointer
		// Assembly: esi = resolved pointer, stored at [ebp+0D8h]
		this->ConditionObject = ResolveTechType(buf); // VERIFY: field name
	}

	// --- token 8: Conditions hex-pair string ---
	tok = strtok(nullptr, ",");
	if (!tok)
		return 0;

	ParseConditions(tok, this);

	// --- token 9: WeightCur (optional from here) ---
	// Vanilla: atof -> __ftol -> fild -> fstp double
	// Replaced with std::stod for equivalent precision.
	tok = strtok(nullptr, ",");
	if (tok)
		this->WeightCur = std::stod(tok); // VERIFY: field name, [ebp+0B8h]

	// --- token 10: WeightMin ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->WeightMin = std::stod(tok); // VERIFY: field name, [ebp+0C0h]

	// --- token 11: WeightMax ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->WeightMax = std::stod(tok); // VERIFY: field name, [ebp+0C8h]

	// --- token 12: IsForSkirmish ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->IsForSkirmish = std::atoi(tok) != 0; // VERIFY: field name, [ebp+0D0h]

	// --- token 13: discarded (second reserved slot) ---
	// Assembly: 0x41F93C — two consecutive strtok calls; first result unused.
	strtok(nullptr, ",");

	// --- token 14: OwningCountry ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->OwningCountry = std::atoi(tok); // VERIFY: field name, [ebp+0ACh]

	// --- token 15: IsForBaseDefense ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->IsForBaseDefense = std::atoi(tok) != 0; // VERIFY: field name, [ebp+0D1h]

	// --- token 16: TeamTypeTwo ---
	tok = strtok(nullptr, ",");
	if (tok)
	{
		char buf[24];
		std::strncpy(buf, tok, 23u);
		buf[23] = '\0';
		strtrim(buf);

		this->TeamTypeTwo = nullptr; // VERIFY: field name, [ebp+0E0h]
		if (_strcmpi(buf, none_str))
			this->TeamTypeTwo = TeamTypeClass::From_Name(buf);
	}

	// --- token 17: EnabledInEasy ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->EnabledInEasy = std::atoi(tok) != 0; // VERIFY: field name, [ebp+0D2h]

	// --- token 18: EnabledInMedium ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->EnabledInMedium = std::atoi(tok) != 0; // VERIFY: field name, [ebp+0D3h]

	// --- token 19: EnabledInHard ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->EnabledInHard = std::atoi(tok) != 0; // VERIFY: field name, [ebp+0D4h]

	// --- TechLevel derivation from TaskForce requirements ---
	// Vanilla: TechLevel = max(TechLevel, TaskForce::Tech_Level_Required(team->TaskForce))
	// Assembly: 0x41FA5C-0x41FAE3; both teams checked independently.
	// [TeamTypeClass+0E4h] = TaskForce pointer  VERIFY in YRpp

	if (this->TeamTypeOne)
	{
		const int required = TaskForceClass::Tech_Level_Required(
			this->TeamTypeOne->TaskForce); // VERIFY: field name
		this->TechLevel = std::max(this->TechLevel, required);
	}

	if (this->TeamTypeTwo)
	{
		const int required = TaskForceClass::Tech_Level_Required(
			this->TeamTypeTwo->TaskForce); // VERIFY: field name
		this->TechLevel = std::max(this->TechLevel, required);
	}

	return 1;
	*/
}