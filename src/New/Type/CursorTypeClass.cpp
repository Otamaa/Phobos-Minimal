#include "CursorTypeClass.h"

std::array<const char* const, (size_t)MouseCursorType::count> CursorTypeClass::MouseCursorTypeToStrings
{
{
	{ "Default" }, { "MoveN" },{ "MoveNE" }, { "MoveE" }, { "MoveSE" },
	{ "MoveS" }, { "MoveSW" }, { "MoveW" }, { "MoveNW" }, { "NoMoveN" },
	{ "NoMoveNE" }, { "NoMoveE" }, { "NoMoveSE" }, { "NoMoveS" },
	{ "NoMoveSW" }, { "NoMoveW" }, { "NoMoveNW" }, { "Select" },
	{ "Move" }, { "NoMove" }, { "Attack" }, { "AttackOutOfRange" },
	{ "CUR_16" }, { "DesolatorDeploy" }, { "CUR_18" }, { "Enter" },
	{ "NoEnter" }, { "Deploy" }, { "NoDeploy" }, { "CUR_1D" },
	{ "Sell" }, { "SellUnit" }, { "NoSell" }, { "Repair" },
	{ "EngineerRepair" }, { "NoRepair" }, { "CUR_24" }, { "Disguise" },
	{ "IvanBomb" }, { "MindControl" }, { "RemoveSquid" }, { "Crush" },
	{ "SpyTech" }, { "SpyPower" }, { "CUR_2C" }, { "GIDeploy" },
	{ "CUR_2E" }, { "Paradrop" }, { "CUR_30" }, { "CUR_31" },
	{ "LightningStorm" }, { "Detonate" }, { "Demolish" }, { "Nuke" },
	{ "CUR_36" }, { "Power" }, { "CUR_38" }, { "IronCurtain" }, { "Chronosphere" },
	{ "Disarm" }, { "CUR_3C" }, { "Scroll" }, { "ScrollESW" }, { "ScrollSW" },
	{ "ScrollNSW" }, { "ScrollNW" }, { "ScrollNEW" }, { "ScrollNE" },
	{ "ScrollNES" }, { "ScrollES" }, { "CUR_46" }, { "AttackMove" },
	{ "CUR_48" }, { "InfantryAbsorb" }, { "NoMindControl" }, { "CUR_4B" },
	{ "CUR_4C" }, { "CUR_4D" }, { "Beacon" }, { "ForceShield" }, { "NoForceShield" },
	{ "GeneticMutator" }, { "AirStrike" }, { "PsychicDominator" }, { "PsychicReveal" },
	{ "SpyPlane" }
}
};

std::array<const char*, (size_t)NewMouseCursorType::count> CursorTypeClass::NewMouseCursorTypeToStrings
{
{
		//86       //87					//88
	{ "Tote" }, { "EngineerDamage" }, { "TogglePower" },
	   //89					//90				//91
	{ "NoTogglePower" }, { "InfantryHeal" }, { "UnitRepair" },
	   //92					//93				//94
	{ "TakeVehicle" }, { "Sabotage" }, { "RepairTrench" },
}
};

std::array<const MouseCursor, (size_t)NewMouseCursorType::count> CursorTypeClass::NewMouseCursorTypeData
{
{
		{ 239,10,4,-1,-1,MouseHotSpotX::Center , MouseHotSpotY::Middle } ,
		{ 299,10,4,-1,-1,MouseHotSpotX::Center , MouseHotSpotY::Middle } ,
		{ 399,6,0,-1,-1,MouseHotSpotX::Center , MouseHotSpotY::Middle } ,
		{ 384,1,0,-1,-1,MouseHotSpotX::Center , MouseHotSpotY::Middle } ,
		{ 355,1,0,-1,-1,MouseHotSpotX::Center , MouseHotSpotY::Middle } ,
		{ 150,20,4,-1,-1,MouseHotSpotX::Center , MouseHotSpotY::Middle } ,
		{ 89,10,4,100,10,MouseHotSpotX::Center , MouseHotSpotY::Middle } ,
		{ 89,10,4,100,10,MouseHotSpotX::Center , MouseHotSpotY::Middle } ,
		{ 150,20,4,-1,-1,MouseHotSpotX::Center , MouseHotSpotY::Middle } ,
}
};

Enumerable<CursorTypeClass>::container_t Enumerable<CursorTypeClass>::Array;

const char* Enumerable<CursorTypeClass>::GetMainSection()
{
	return "MouseCursors";
}

void CursorTypeClass::AddDefaults()
{
	if (!Array.empty())
		return;

	for (size_t i = 0; i < MouseCursorTypeToStrings.size(); ++i) {
		AllocateWithDefault(MouseCursorTypeToStrings[i], MouseCursor::DefaultCursors[i]);
	}

	for (size_t a = 0; a < NewMouseCursorTypeToStrings.size(); ++a) {
		AllocateWithDefault(NewMouseCursorTypeToStrings[a], CursorTypeClass::NewMouseCursorTypeData[a]);
	}
}

void CursorTypeClass::LoadFromINI(CCINIClass* pINI)
{
	auto const pKey = this->Name.data();

	if (IS_SAME_STR_(pKey, MouseCursorTypeToStrings[0]))
		return;

	INI_EX exINI { pINI };
	auto const pSection = this->GetMainSection();

	this->CursorData.Read(exINI, pSection, pKey);
}

void CursorTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void CursorTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

void CursorTypeClass::LoadFromINIList_New(CCINIClass* pINI, bool bDebug)
{
	if (!pINI)
		return;

	const char* section = GetMainSection();

	if (!pINI->GetSection(section))
		return;

	auto const pKeyCount = pINI->GetKeyCount(section);
	if (!pKeyCount)
		return;

	Array.reserve(pKeyCount);

	for (int i = 0; i < pKeyCount; ++i) {
		if (auto const pItem = FindOrAllocate(pINI->GetKeyName(section, i))) {
			pItem->LoadFromINI(pINI);
		}
	}
}

template<typename T>
void CursorTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->CursorData)
		;
}
