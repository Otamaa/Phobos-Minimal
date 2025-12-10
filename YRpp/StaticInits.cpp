//This file initializes static constant values.

#include <YRPP.h>
#include <ASMMacros.h>
#include <YRPPCore.h>
#include <Unsorted.h>
#include <Helpers/Macro.h>

#include <ArrayClasses.h>
#include <TacticalClass.h>
#include <TechnoClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <BuildingClass.h>
#include <SlaveManagerClass.h>
#include <RulesClass.h>
#include <Drawing.h>
#include <MapClass.h>
#include <WarheadTypeClass.h>
#include <HouseClass.h>
#include <SuperClass.h>
#include <FactoryClass.h>
#include <ScenarioClass.h>
#include <FootClass.h>
#include <GameOptionsClass.h>
#include <WWMouseClass.h>
#include <CoordStruct.h>
#include <Fixed.h>
#include <CCINIClass.h>
#include <TextDrawing.h>

#include <WWKeyboardClass.h>
#include <DisplayClass.h>
#include <MapClass.h>

#include <array>
#include <Windows.h>

#include <Helpers/Iterators.h>

/*
# Tiberian Sun CLSIDs and IIDs

## Interface IDs (IIDs)

| Interface Name | GUID |
|---|---|
| IID_IApplication | 96F02EC3-6FE8-11D1-B6FD-00A024DDAFD1 |
| IID_ILinkStream | 0D5CD78E-6470-11D2-9B74-00104B972FE8 |
| IID_IPowerEvents | 56272740-89BB-11D1-B707-00A024DDAFD1 |
| IID_IRTTITypeInfo | 170DAC82-12E4-11D2-8175-0060080505B |
| IID_IAIHouse | 96F02EC4-6FE8-11D1-B6FD-00A024DDAFD1 |
| IID_IPublicHouse | CAACF210-86E3-11D1-B706-00A024DDAFD1 |
| IID_IPiggyback | 92FEA800-A184-11D1-B70A-00A024DDAFD1 |
| IID_ISwizzle | 5FF0CA70-8B12-11D1-B708-00A024DDAFD1 |
| IID_IBlockCipher | E0113100-6A7C-11D1-B6F9-00A024DDAFD1 |
| IID_IFlyControl | 820F501C-4F39-11D2-9B70-00104B972FE8 |

## Core Library CLSIDs

| Class Name | GUID |
|---|---|
| CLSID_BlowfishLibrary | E7F91750-8861-11D1-B707-00A024DDAFD1 |
| CLSID_BlowfishObject | 1440AD10-6AA8-11D1-B6F9-00A024DDAFD1 |
| CLSID_TiberianSunLibrary | B45A4A80-86DA-11D1-B706-00A024DDAFD1 |
| CLSID_Tiberian_Sun | B45A4A81-86DA-11D1-B706-00A024DDAFD1 |
| CLSID_CompressStream | B48FA168-646F-11D2-9B74-00104B972FE8 |

## Game Object Type Classes

| Class Name | GUID |
|---|---|
| CLSID_SuperWeaponTypeClass | 0CF2BCE7-36E4-11D2-B8D8-006008C809ED |
| CLSID_UnitTypeClass | DCBD42EA-0546-11D2-ACA4-00600805B5B |
| CLSID_InfantryTypeClass | AE8B33D8-061C-11D2-ACA4-00600805B5B |
| CLSID_AircraftTypeClass | AE8B33D9-061C-11D2-ACA4-00600805B5B |
| CLSID_BuildingTypeClass | AE8B33DB-061C-11D2-ACA4-00600805B5B |
| CLSID_BulletTypeClass | 5AF2CE77-0634-11D2-ACA4-00600805B5B |
| CLSID_TerrainTypeClass | 5AF2CE7B-0634-11D2-ACA4-00600805B5B |
| CLSID_IsometricTileTypeClass | 5AF2CE7A-0634-11D2-ACA4-00600805B5B |
| CLSID_OverlayTypeClass | 5AF2CE79-0634-11D2-ACA4-00600805B5B |
| CLSID_SmudgeTypeClass | 5AF2CE78-0634-11D2-ACA4-00600805B5B |
| CLSID_AnimTypeClass | AE8B33DA-061C-11D2-ACA4-00600805B5B |
| CLSID_HouseTypeClass | 1DD43928-046B-11D2-ACA4-00600805B5B |
| CLSID_ParticleSystemTypeClass | 703E044A-0FB1-11D2-8172-00600805B5B |
| CLSID_ParticleTypeClass | 703E044B-0FB1-11D2-8172-00600805B5B |
| CLSID_VoxelAnimTypeClass | 2EBB6D66-0D4D-11D2-8172-00600805B5B |
| CLSID_WeaponTypeClass | 9FD219CA-0F7B-11D2-8172-00600805B5B |
| CLSID_WarheadTypeClass | A8C54DA4-0F7B-11D2-8172-00600805B5B |

## Game Object Instance Classes

| Class Name | GUID |
|---|---|
| CLSID_HouseClass | D9D4A910-87C6-11D1-B707-00A024DDAFD1 |
| CLSID_SuperWeaponClass | D7F754C6-391C-11D2-9B64-00104B972FE8 |
| CLSID_IsometricTileClass | 0E272DC0-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_VoxelAnimClass | 0E272DC1-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_AircraftClass | 0E272DC2-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_AnimClass | 0E272DC3-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_InfantryClass | 0E272DC4-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_SmudgeClass | 0E272DC5-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_BuildingClass | 0E272DC6-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_OverlayClass | 0E272DC7-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_ParticleSystemClass | 0E272DC8-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_BulletClass | 0E272DC9-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_UnitClass | 0E272DCA-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_ParticleClass | 0E272DCC-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_WaveClass | 0E272DCD-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_BuildingLightClass | 54822258-D8A8-11D1-B462-006097C6A979 |
| CLSID_TerrainClass | 0E272DCE-9C0F-11D1-B709-00A024DDAFD1 |
| CLSID_TubeClass | 0B4CA41C-B3A7-11D1-B457-006097C6A979 |
| CLSID_TeamClass | 0E272DCF-9C0F-11D1-B709-00A024DDAFD1 |

## Mission & AI Classes

| Class Name | GUID |
|---|---|
| CLSID_TaskForceClass | 61DE341E-0774-11D2-ACA5-00600805B5B |
| CLSID_TeamTypeClass | D1DBA64E-0778-11D2-ACA5-00600805B5B |
| CLSID_ScriptClass | 42F3A646-0789-11D2-ACA5-00600805B5B |
| CLSID_ScriptTypeClass | 42F3A647-0789-11D2-ACA5-00600805B5B |
| CLSID_TagClass | 54F6E432-09ED-11D2-ACA5-00600805B5B |
| CLSID_TagTypeClass | 54F6E433-09ED-11D2-ACA5-00600805B5B |
| CLSID_TriggerClass | C02D1590-0A2A-11D2-ACA7-00600805B5B |
| CLSID_TriggerTypeClass | C02D1591-0A2A-11D2-ACA7-00600805B5B |
| CLSID_ActionClass | 4F0EC392-0A55-11D2-ACA7-00600805B5B |
| CLSID_EventClass | 4F0EC393-0A55-11D2-ACA7-00600805B5B |
| CLSID_FactoryClass | 34ECD9A8-0AB0-11D2-ACA7-00600805B5B |
| CLSID_AITriggerTypeClass | BA093524-4CF4-11D2-BC26-00104B8FB04D |
| CLSID_AITriggerClass | 03C4CE76-4CF5-11D2-BC26-00104B8FB04D |
| CLSID_NeuronClass | 241AB316-4CF5-11D2-BC26-00104B8FB04D |

## AI House Classes

| Class Name | GUID |
|---|---|
| CLSID_AILibrary | E97EC0D0-86DA-11D1-B706-00A024DDAFD1 |
| CLSID_AIHouse | F706E6E0-86DA-11D1-B706-00A024DDAFD1 |
| CLSID_AIMeade | 9E0F6120-87C1-11D1-B707-00A024DDAFD1 |
| CLSID_AIJackson | C6004D80-87D1-11D1-B707-00A024DDAFD1 |
| CLSID_AIGrant | FBE6D4A0-87D1-11D1-B707-00A024DDAFD1 |
| CLSID_AIHooker | FBE6D4A1-87D1-11D1-B707-00A024DDAFD1 |

## Locomotion Classes

| Class Name | GUID |
|---|---|
| CLSID_LocomotionLibrary | 4A582740-9839-11D1-B709-00A024DDAFD1 |
| CLSID_DriveLocomotion | 4A582741-9839-11D1-B709-00A024DDAFD1 |
| CLSID_HoverLocomotion | 4A582742-9839-11D1-B709-00A024DDAFD1 |
| CLSID_TunnelLocomotion | 4A582743-9839-11D1-B709-00A024DDAFD1 |
| CLSID_WalkLocomotion | 4A582744-9839-11D1-B709-00A024DDAFD1 |
| CLSID_BallisticLocomotion | 4A582745-9839-11D1-B709-00A024DDAFD1 |
| CLSID_FlyerLocomotion | 4A582746-9839-11D1-B709-00A024DDAFD1 |
| CLSID_TeleportLocomotion | 4A582747-9839-11D1-B709-00A024DDAFD1 |
| CLSID_MechLocomotion | 55D141B8-DB94-11D1-AC98-00600805B5B |
| CLSID_ShipLocomotion | 2BEA74E1-7CCA-11D3-BE14-00104B62A16C |
| CLSID_JumpjetLocomotion | 92612C46-F71F-11D1-AC9F-00600805B5B |
| CLSID_RocketLocomotion | B7B49766-E576-11D3-9BD9-00104B972FE8 |

## Manager Classes

| Class Name | GUID |
|---|---|
| CLSID_SpawnManagerClass | 06679E981-AD9D-11D3-BE16-00104B62A16C |
| CLSID_SlaveManagerClass | 335AAFE4-2DA6-11D5-BE22-00104B62A16C |
| CLSID_CaptureManagerClass | 06679E982-AD9D-11D3-BE16-00104B62A16C |

## Special Effect Classes

| Class Name | GUID |
|---|---|
| CLSID_DiskLaserClass | 5230C9A8-846A-47EC-BDA2-7E95445E1D49 |
| CLSID_ParasiteClass | 1D016B81-B24B-11D3-BE16-00104B62A16C |
| CLSID_BombClass | 06679E983-AD9D-11D3-BE16-00104B62A16C |
| CLSID_RadSiteClass | 4104D740-D507-11D3-8C38-00A0C933BE44 |
| CLSID_TemporalClass | 94112424-E403-11D3-8E6E-005004AAB2FB |
| CLSID_AirstrikeClass | 70DE3921-1E26-11D5-8F95-00A024834B9C |
| CLSID_LightSource | 6F9C48F0-1207-11D2-8174-00600805B5B |
| CLSID_EMPulseClass | B825CB22-200E-11D2-9FA9-0060089AD458 |

## Map & World Classes

| Class Name | GUID |
|---|---|
| CLSID_CampaignClass | FFDAC848-1517-11D2-8175-00600805B5B |
| CLSID_SideClass | C53DD372-151E-11D2-8175-00600805B5B |
| CLSID_TiberiumClass | C53DD373-151E-11D2-8175-00600805B5B |
| CLSID_CellClass | C1BF99CE-1A8C-11D2-8175-00600805B5B |
| CLSID_TacticalMapClass | CF56B38A-240D-11D2-817C-00600805B5B |
| CLSID_WaypointPath | F73125BA-1054-11D2-8172-00600805B5B |
| CLSID_FoggedObjectClass | 1C470B0E-69D7-11D2-B8F2-006008C809ED |
| CLSID_AlphaShapeClass | 623C7584-74E7-11D2-B8F5-006008C809ED |
| CLSID_VeinholeMonsterClass | 5192D06A-C632-11D2-B90B-006008C809ED |
*/

const CoordStruct CoordStruct::Empty = {};
const ColorStruct ColorStruct::Empty = {};
const CellStruct CellStruct::Empty = {};
const CellStruct CellStruct::EOL = { 0x7FFF , 0x7FFF };
const VelocityClass VelocityClass::Empty = {};
const CellStruct CellStruct::DefaultUnloadCell = { 3 , 1 };
const Point2D Point2D::Empty = {};
const Point2DBYTE Point2DBYTE::Empty = {};
const Point3D Point3D::Empty = {};
const RectangleStruct RectangleStruct::Empty = {};
const HSVClass BlackColor = { 0, 0, 0 };

std::array< ColorStruct, (size_t)DefaultColorList::Black + 1> Drawing::DefaultColors
{
{
		// gery  , red , green
		{ 128,128,128 } , { 255,0,0 } , { 0,255,0 } ,
		// blue , yellow , white
		{ 0,0,255 } , { 255,255,0 } , { 255 , 255 , 255 } ,
		// ares cameo transparent color
		{ 255 , 0 , 255 } ,
		// black
		{ 3 , 3 , 3}
}
};

//#pragma region GlobalVarDeclaration
//ALIAS(MouseClass, Map, 0x87F7E8)
//ALIAS(GScreenClass, GScreen, 0x87F7E8)
//ALIAS(CellClass, WorkingCellInstance, 0xABDC50)
//ALIAS(RulesClass*,RulesGlobal,0x8871E0)
//ALIAS(ScenarioClass*, ScenarioGlobal ,0xA8B230)
//ALIAS(Random2Class, Random2Global ,0x886B88)
//ALIAS(ParticleSystemClass*, ParticleSystemGlobal ,0xA8ED78)
//ALIAS(GameOptionsClass, GameOptions,0xA8EB60)
//ALIAS(GameModeOptionsClass, GameModeOptions ,0xA8B250)
//ALIAS(TacticalClass*, TacticalGlobal,0x887324)
//ALIAS(MessageListClass, MessageListGlobal,0xA8BC60)
//ALIAS(SessionClass, SessionGlobal,0xA8B238)
//ALIAS(WWMouseClass*, WWMouse,0x887640)
//ALIAS(BombListClass , BombList , 0x87F5D8u)
//#pragma endregion

AnimClass* BulletClass::CreateDamagingBulletAnim(HouseClass* pHouse, CellClass* pTarget, BulletClass* pBullet, AnimTypeClass* pAnimType)
{
	if (!pAnimType)
		return nullptr;

	auto loc = pTarget->GetCoords();

	auto pAnim = GameCreate<AnimClass>(pAnimType, loc);
	pAnim->SetBullet(pBullet);
	pAnim->SetHouse(pHouse);
	pAnim->Make_Invisible();

	return pAnim;
}

bool WWKeyboardClass::IsForceFireKeyPressed() const
{
	return this->IsKeyPressed(GameOptionsClass::Instance->KeyForceFire1)
		|| this->IsKeyPressed(GameOptionsClass::Instance->KeyForceFire2);
}

bool WWKeyboardClass::IsForceMoveKeyPressed() const
{
	return this->IsKeyPressed(GameOptionsClass::Instance->KeyForceMove1)
		|| this->IsKeyPressed(GameOptionsClass::Instance->KeyForceMove2);
}

bool WWKeyboardClass::IsForceSelectKeyPressed() const
{
	return this->IsKeyPressed(GameOptionsClass::Instance->KeyForceSelect1)
		|| this->IsKeyPressed(GameOptionsClass::Instance->KeyForceSelect2);
}
void SlaveManagerClass::ZeroOutSlaves() {
	for(const auto& pNode : this->SlaveNodes) {
		if(auto pSlave = pNode->Slave) {
			pSlave->SlaveOwner = nullptr;
		}
		pNode->Slave = nullptr;
		pNode->State = SlaveControlStatus::Dead;
		pNode->RespawnTimer.Start(this->RegenRate);
	}
}

DamageState ObjectClass::TakeDamage(int damage, bool crewed, bool ignoreDefenses , ObjectClass* pAttacker , HouseClass* pAttackingHouse)
{
	return TakeDamage(damage, RulesClass::Instance->C4Warhead, crewed, ignoreDefenses, pAttacker, pAttackingHouse);
}

const char* ObjectClass::get_ID() const
{
	const auto pType = this->GetType();
	return pType ? pType->get_ID() : GameStrings::NoneStr();
}

Point2D ObjectClass::GetScreenLocation() const
{
	CoordStruct crdAbsolute = this->GetCoords();
	Point2D  posScreen = TacticalClass::Instance->CoordsToScreen(crdAbsolute);
	posScreen -= TacticalClass::Instance->TacticalPos;

	return posScreen;
}

bool ObjectClass::IsOnMyView() const
{
	auto const coords = this->GetCoords();
	auto const Point = TacticalClass::Instance->CoordsToView(coords);
	return Point.X > Drawing::SurfaceDimensions_Hidden().X
		&& Point.Y > Drawing::SurfaceDimensions_Hidden().Y
		&& Point.X < Drawing::SurfaceDimensions_Hidden().X + Drawing::SurfaceDimensions_Hidden().Width
		&& Point.Y < Drawing::SurfaceDimensions_Hidden().Y + Drawing::SurfaceDimensions_Hidden().Height;

}

bool ObjectClass::IsGreenToYellowHP() const
{
	return this->GetHealthPercentage() >= RulesClass::Instance->ConditionYellow;
}

bool ObjectClass::IsFullHP() const
{ return this->GetHealthPercentage() >= RulesClass::Instance->ConditionGreen; }

double ObjectClass::GetHealthPercentage_() const
{ JMP_THIS(0x5F5C60); }

double ObjectClass::GetHealthPercentage() const
{
	return (double)this->Health / (double)this->GetType()->Strength;
}

bool HouseClass::CanExpectToBuild(const TechnoTypeClass* const pItem) const {
	auto const parentOwnerMask = this->Type->FindParentCountryIndex();
	return this->CanExpectToBuild(pItem, parentOwnerMask);
}

bool HouseClass::CanExpectToBuild(const TechnoTypeClass* const pItem, int const idxParent) const {
	auto const parentOwnerMask = 1u << idxParent;
	if(pItem->InOwners(parentOwnerMask)) {
		if(this->InRequiredHouses(pItem)) {
			if(!this->InForbiddenHouses(pItem)) {
				auto const BaseSide = pItem->AIBasePlanningSide;
				if(BaseSide < 0 || BaseSide == this->Type->SideIndex) {
					return true;
				}
			}
		}
	}
	return false;
}

int HouseClass::FindSuperWeaponIndex(SuperWeaponType const type) const {
	for(int i = 0; i < this->Supers.Count; ++i) {
		if(this->Supers.Items[i]->Type->Type == type) {
			return i;
		}
	}
	return -1;
}

SuperClass* HouseClass::FindSuperWeapon(SuperWeaponType const type) const {
	auto index = this->FindSuperWeaponIndex(type);
	return this->Supers.get_or_default(index);
}

SuperClass* HouseClass::FindSuperWeapon(SuperWeaponTypeClass* pType) const {
	for (int i = 0; i < this->Supers.Count; ++i) {
		if (this->Supers.Items[i]->Type == pType) {
			return this->Supers.Items[i];
		}
	}

	return nullptr;
}

CellStruct FootClass::GetRandomDirection(FootClass* pFoot)
{
	CellStruct nRet = CellStruct::Empty;

	if (auto pCell = MapClass::Instance->GetCellAt(pFoot->GetCoords())) {

		const int rnd = ScenarioClass::Instance->Random.RandomFromMax(7);

		for (int j = 0; j < 8; ++j)
		{
			// get the direction in an overly verbose way
			FacingType dir = FacingType(((j + rnd) % 8) & 7);

			if (auto pNeighbour = pCell->GetNeighbourCell(dir))
			{
				if (pFoot->IsCellOccupied(pNeighbour, FacingType::None, -1, nullptr, true) == Move::OK)
				{
					nRet = pNeighbour->MapCoords;
					break;
				}
			}
		}
	}

	return nRet;
}

bool TechnoClass::CanICloakByDefault() const
{
	const auto tType = this->GetTechnoType();
	return tType->Cloakable || this->HasAbility(AbilityType::Cloak);
}

int TechnoClass::GetIonCannonValue(AIDifficulty const difficulty) const {
	const auto& rules = *RulesClass::Instance;

	const TypeList<int>* pValues = nullptr;
	int value = 1;

	switch (this->WhatAmI())
	{
	case AbstractType::Unit:
	{
		const auto pType = static_cast<const UnitClass*>(this)->Type;

		if (pType->Harvester)
		{
			pValues = &rules.AIIonCannonHarvesterValue;
		}
		else if (rules.BuildConst.contains(pType->DeploysInto))
		{
			pValues = &rules.AIIonCannonMCVValue;
		}
		else if (pType->Passengers > 0)
		{
			pValues = &rules.AIIonCannonAPCValue;
		}
		else
		{
			value = 2;
		}
		break;
	}
	case AbstractType::Building:
	{
		const auto pType = static_cast<const BuildingClass*>(this)->Type;

		if (pType->Factory == AbstractType::BuildingType)
		{
			pValues = &rules.AIIonCannonConYardValue;
		}
		else if (pType->Factory == AbstractType::UnitType && !pType->Naval)
		{
			pValues = &rules.AIIonCannonWarFactoryValue;
		}
		else if (pType->PowerBonus > pType->PowerDrain)
		{
			pValues = &rules.AIIonCannonPowerValue;
		}
		else if (pType->IsBaseDefense)
		{
			pValues = &rules.AIIonCannonBaseDefenseValue;

		}
		else if (pType->IsPlug)
		{
			pValues = &rules.AIIonCannonPlugValue;
		}
		else if (pType->IsTemple)
		{
			pValues = &rules.AIIonCannonTempleValue;
		}
		else if (pType->HoverPad)
		{
			pValues = &rules.AIIonCannonHelipadValue;
		}
		else if (rules.BuildConst.contains(pType))
		{
			pValues = &rules.AIIonCannonTechCenterValue;
		}
		else
		{
			value = 4;
		}
		break;
	}
	case AbstractType::Infantry:
	{
		const auto pType = static_cast<const InfantryClass*>(this)->Type;

		if (pType->Engineer)
		{
			pValues = &rules.AIIonCannonEngineerValue;
		}
		else if (pType->VehicleThief)
		{
			pValues = &rules.AIIonCannonThiefValue;
		}
		else
		{
			value = 2;
		}

		break;
	}
	default:
		break;
	}

	return pValues ? pValues->get_or_default(static_cast<int>(difficulty), value) : value;
}

void InfantryClass::RemoveMe_FromGunnerTransport()
{
	if (auto pTransport = this->Transporter)
	{
		if (auto pUnit = cast_to<UnitClass* , false>(pTransport))
		{
			if (pUnit->GetTechnoType()->Gunner)
			{
				pUnit->RemoveGunner(this);
			}
		}
	}
}

bool BuildingClass::BuildingUnderAttack()
{
	if (this->Owner)
	{
		this->Owner->BuildingUnderAttack(this);
		return true;
	}

	return false;
}

#pragma warning(push)
#pragma warning(disable : 4244)

const Fixed Fixed::_1_2(1, 2);		// 1/2
const Fixed Fixed::_1_3(1, 3);		// 1/3
const Fixed Fixed::_1_4(1, 4);		// 1/4
const Fixed Fixed::_3_4(3, 4);		// 3/4
const Fixed Fixed::_2_3(2, 3);		// 2/3

Fixed::Fixed(int numerator, int denominator)
{
	if (denominator == 0)
	{
		Data.Raw = 0U;
	}
	else
	{
		Data.Raw = (unsigned int)(((unsigned __int64)numerator * PRECISION) / denominator);
	}
}

Fixed::Fixed(const char* ascii)
{
	if (ascii == nullptr)
	{
		Data.Raw = 0U;
		return;
	}

	char const* wholepart = ascii;

	while (isspace(*ascii))
	{
		ascii++;
	}

	char const* tptr = ascii;
	while (isdigit(*tptr))
	{
		tptr++;
	}

	if (*tptr == '%')
	{
		Data.Raw = (unsigned short)(((unsigned __int64)CRT::atoi(ascii) * PRECISION) / 100ULL);
	}
	else
	{

		Data.Composite.Whole = Data.Composite.Fraction = 0U;
		if (wholepart && *wholepart != '.')
		{
			Data.Composite.Whole = (unsigned char)CRT::atoi(wholepart);
		}

		const char* fracpart = CRT::strchr(ascii, '.');
		if (fracpart) fracpart++;
		if (fracpart)
		{
			unsigned short frac = (unsigned short)CRT::atoi(fracpart);

			int len = 0;
			unsigned int base = 1;
			char const* fptr = fracpart;
			while (isdigit(*fptr))
			{
				fptr++;
				len++;
				base *= 10U;
			}

			Data.Composite.Fraction = (unsigned char)(((unsigned __int64)frac * PRECISION) / base);
		}
	}
}

int Fixed::To_ASCII(char* buffer, int maxlen) const
{
	if (buffer == nullptr) return 0;

	unsigned int whole = Data.Composite.Whole;
	unsigned int frac = ((unsigned int)Data.Composite.Fraction * 1000U) / PRECISION;
	char tbuffer[32];

	if (frac == 0)
	{
		sprintf_s(tbuffer, "%u", whole);
	}
	else
	{
		sprintf_s(tbuffer, "%u.%02u", whole, frac);

		char* ptr = &tbuffer[CRT::strlen(tbuffer) - 1];
		while (*ptr == '0')
		{
			*ptr = '\0';
			ptr--;
		}
	}

	if (maxlen == -1)
	{
		maxlen = CRT::strlen(tbuffer) + 1;
	}

	CRT::strncpy(buffer, tbuffer, maxlen);

	int len = CRT::strlen(tbuffer);
	if (len < maxlen - 1) return(len);
	return(maxlen - 1);
}

const char* Fixed::As_ASCII() const
{
	static char buffer[32];

	To_ASCII(buffer, sizeof(buffer));
	return buffer;
}
#pragma warning(pop)

CoordStruct WWMouseClass::GetCoordsUnderCursor()
{
	CoordStruct nbuffer { -1,-1,-1 };
	Point2D nBuffer2D {};
	WWMouseClass::Instance->GetCoords_(nBuffer2D);

	if (nBuffer2D.X >= 0 && nBuffer2D.Y >= 0)
	{
		CellStruct nBufferCell {};
		TacticalClass::Instance->Coordmap_viewportpos_tocellpos_Click_Cell_Calc(nBufferCell, nBuffer2D);
		nbuffer.X = nBufferCell.X * 256;
		nbuffer.Y = nBufferCell.Y * 256;
		nbuffer.Z = 0;
	}

	return nbuffer;
}

CellStruct WWMouseClass::GetCellUnderCursor()
{
	CellStruct nbuffer { -1,-1 };
	Point2D nBuffer2D {};
	WWMouseClass::Instance->GetCoords_(nBuffer2D);

	if (nBuffer2D.X >= 0 && nBuffer2D.Y >= 0)
		TacticalClass::Instance->Coordmap_viewportpos_tocellpos_Click_Cell_Calc(nbuffer, nBuffer2D);

	return nbuffer;
}

bool LocomotionClass::End_Piggyback(ILocomotionPtr &pLoco)
{
	if (!pLoco)
	{
		_com_issue_error(E_POINTER);
	}

	if (IPiggybackPtr pPiggy = pLoco)
	{
		if (pPiggy->Is_Piggybacking())
		{
			// _com_ptr_t releases the old pointer automatically,
			// so we just use it without resetting it
			auto res = pPiggy->End_Piggyback(&pLoco);
			if (FAILED(res))
			{
				_com_issue_error(res);
			}
			return (res == S_OK);
		}
	}

	return false;
}

void LocomotionClass::ChangeLocomotorTo(FootClass *Object, const CLSID &clsid)
{
	// remember the current one
	ILocomotionPtr Original(Object->Locomotor);

	// create a new locomotor and link it
	auto NewLoco = CreateInstance(clsid);
	NewLoco->Link_To_Object(Object);

	// get piggy interface and piggy original
	IPiggybackPtr Piggy(NewLoco);
	Piggy->Begin_Piggyback(Original.GetInterfacePtr());

	// replace the current locomotor
	Object->Locomotor = NewLoco;
}


void TechnoClass::ReleaseCaptureManager() const
{
	if (auto pManager = this->CaptureManager)
		pManager->FreeAll();
}

void TechnoClass::SuspendWorkSlaveManager() const
{
	if (auto pManager = this->SlaveManager)
		pManager->SuspendWork();
}

void TechnoClass::ResumeWorkSlaveManager() const
{
	if (auto pManager = this->SlaveManager)
		pManager->ResumeWork();
}

void TechnoClass::DetechMyTemporal() const
{
	if (this->IsWarpingSomethingOut())
		if (auto pTemporal = this->TemporalImUsing)
			pTemporal->LetGo();
}

/*
MissionControlClass* TechnoClass::GetCurrentMissionControl() const
{
	return this->GetMissionControl(MissionFlags::CurrentMission);
}

double TechnoClass::GetCurrentMissionRate() const
{
	auto const control = this->GetMissionControl(MissionFlags::CurrentMission);
	auto const doubleval = 900.0; // 0x7E27F8
	return control->Rate * doubleval;
}*/

int TechnoClass::GetIonCannonValue(AIDifficulty difficulty, int maxHealth) const {
	// what TS does
	if (maxHealth > 0 && this->Health > maxHealth) {
		return (this->WhatAmI() == AbstractType::Building) ? 3 : 1;
	}

	return this->GetIonCannonValue(difficulty);
}

bool PCX::LoadFile(const char *pFileName, int flag1, int flag2)
{
	if (Instance->GetSurface(pFileName, nullptr)) {
		return true;
	}
	return Instance->ForceLoadFile(pFileName, flag1, flag2);
}

void LoadProgressManager::DrawTheText(const wchar_t *pText, int X, int Y, DWORD dwColor)
{
	if (auto pManager = LoadProgressManager::Instance())
	{
		if (auto pSurface = pManager->ProgressSurface)
		{
			pSurface->DrawText_Old(pText, X, Y, dwColor);
		}
	}
}

bool FootClass::LiberateMember(int idx, byte count)
{
	if (this->BelongsToATeam())
	{
		this->Team->LiberateMember(this, idx, count);
		return true;
	}

	return false;
}

void InfantryClass::UnslaveMe()
{
	if (auto pSlave = this->SlaveOwner)
	{
		if (auto pManager = pSlave->SlaveManager)
		{
			pManager->LostSlave(this);
		}
	}
}

DWORD INIClass::CalculateTextCRCChecksums(const char* pText) {
	return SafeChecksummer()(pText);
}

template<class T>
TypeList<T*> CCINIClass::Get_TypeList(const char* section, const char* entry, const TypeList<T*> defvalue, const DynamicVectorClass<T*>& heap)
{
	char buffer[1024];

	if (INIClass::ReadString(section, entry, "", buffer, sizeof(buffer)) > 0)
	{

		TypeList<T> list;

		char* name = CRT::strtok(buffer, ",");
		while (name)
		{

			for (int index = 0; index < heap.Count; ++index)
			{
				T* ptr = const_cast<T*>(T::FindOrAllocate(name));
				if (ptr)
				{
					list.push_back(ptr);
				}
			}

			name = CRT::strtok(nullptr, ",");
		}

		return list;
	}

	return defvalue;
}

template<class T>
bool CCINIClass::Put_TypeList(const char* section, const char* entry, const TypeList<T*> value)
{
	char buffer[1024] = { '\0' };

	for (int index = 0; index < value.Count; ++index)
	{
		if (buffer[0] != '\0')
		{
			CRT::strcat(buffer, ",");
		}
		CRT::strcat(buffer, value[index]->Name);
	}

	return WriteString(section, entry, buffer);
}

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4239)
#pragma warning(disable : 4838)
static void Sort_Vertices(Point2D* p1, Point2D* p2, Point2D* p3)
{
	Point2D* temp;
	if (p1->Y > p2->Y)
	{
		temp = p1;
		p1 = p2;
		p2 = temp;
	}
	if (p1->Y > p3->Y)
	{
		temp = p1;
		p1 = p3;
		p3 = temp;
	}
	if (p2->Y > p3->Y)
	{
		temp = p2;
		p2 = p3;
		p3 = temp;
	}
}


static void Fill_Triangle_Top(Surface& surface, Point2D& point1, Point2D& point2, Point2D& point3, unsigned color)
{
	if (point2.X > point3.X)
	{
		Point2D temp = point2;
		point2 = point3;
		point3 = temp;
	}
	float a = (float(point2.X - point1.X) / float(point2.Y - point1.Y));
	float b = (float(point3.X - point1.X) / float(point3.Y - point1.Y));
	float left = point1.X;
	float right = point1.X;
	for (int idy = point1.Y; idy <= point2.Y; ++idy)
	{
		for (int idx = left; idx <= right; ++idx)
		{
			Point2D nBuffer = { idx, idy };
			surface.Put_Pixel(nBuffer, color);
		}
		left += a;
		right += b;
	}
}


static void Fill_Triangle_Bottom(Surface& surface, Point2D& point1, Point2D& point2, Point2D& point3, unsigned color)
{
	if (point1.X > point2.X)
	{
		Point2D temp = point2;
		point2 = point1;
		point1 = temp;
	}
	float a = (float(point3.X - point1.X) / float(point3.Y - point1.Y));
	float b = (float(point3.X - point2.X) / float(point3.Y - point2.Y));
	float left = point3.X;
	float right = point3.X;
	for (int idy = point3.Y; idy > point2.Y; --idy)
	{
		for (int idx = left; idx <= right; ++idx)
		{
			Point2D nBuffer = { idx, idy };
			surface.Put_Pixel(nBuffer, color);
		}
		left -= a;
		right -= b;
	}
}


bool DSurface::Draw_Triangle(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, unsigned color)
{
	Draw_Line_Rect(rect, point1, point2, color);
	Draw_Line_Rect(rect, point2, point3, color);
	Draw_Line_Rect(rect, point3, point1, color);

	return true;
}


/**
 *
 *
 *  @author: Darth Jane
 */
bool DSurface::Fill_Triangle(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, unsigned color)
{
	if (!rect.IsValid())
	{
		return false;
	}

	RectangleStruct r1 = RectangleStruct::Intersect(Get_Rect(), rect, nullptr, nullptr);

	if (!r1.IsValid())
	{
		return false;
	}

	Point2D r1_tl = r1.Top_Left();
	unsigned short* buffptr = (unsigned short*)Lock(r1_tl.X, r1_tl.Y);
	if (buffptr == nullptr)
	{
		return false;
	}

	/**
	 *  At first sort the three vertices by y-coordinate ascending so v1 is the topmost vertice.
	 */
	Sort_Vertices(&point1, &point2, &point3);

	/**
	 *  Here we know that point1.Y <= point2.Y <= point3.Y
	 *  check for trivial case of bottom-flat triangle.
	 */
	if (point2.Y == point3.Y)
	{
		Fill_Triangle_Top(*this, point1, point2, point3, color);

		/**
		 *  Check for trivial case of top-flat triangle.
		 */
	}
	else if (point1.Y == point2.Y)
	{
		Fill_Triangle_Bottom(*this, point1, point2, point3, color);

		/**
		 *  General case - split the triangle in a topflat and bottom-flat one.
		 */
	}
	else
	{
		Point2D point4 { (int)(point1.X + ((float)(point2.Y - point1.Y) / (float)(point3.Y - point1.Y)) * (point3.X - point1.X)), point2.Y };
		Fill_Triangle_Top(*this, point1, point2, point4, color);
		Fill_Triangle_Bottom(*this, point2, point4, point3, color);
	}

	return true;
}


bool DSurface::Fill_Triangle_Trans(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, ColorStruct& rgb, unsigned opacity)
{
	// TODO
	return false;
}


bool DSurface::Draw_Quad(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, Point2D& point4, unsigned color)
{
	Draw_Line_Rect(rect, point1, point2, color);
	Draw_Line_Rect(rect, point2, point3, color);
	Draw_Line_Rect(rect, point3, point4, color);
	Draw_Line_Rect(rect, point4, point1, color);

	return true;
}


bool DSurface::Fill_Quad(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, Point2D& point4, unsigned color)
{
	Fill_Triangle(rect, point1, point2, point3, color);
	Fill_Triangle(rect, point2, point3, point4, color);

	return true;
}


bool DSurface::Fill_Quad_Trans(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, Point2D& point4, ColorStruct& rgb, unsigned opacity)
{
	// TODO
	return true;
}


/**
 *  Draw a circle.
 *
 *  Uses a modified Bresenham's Circle Drawing algorithm.
 */
void DSurface::Fill_Circle(const Point2D center, unsigned radius, RectangleStruct rect, unsigned color)
{
	Point2D pt { radius, 0 };
	Point2D sxy = Point2D::Empty;
	Point2D dxy = Point2D::Empty;

	/**
	 *  The roundness factor of the circle.
	 *  0 is circle. 50 is rect.
	 */
	int roundness_val = 2;

	/**
	 *  Calculate start decision delta.
	 */
	int d = 3 - (roundness_val * radius);

	do
	{

		dxy = center + Point2D { pt.X, pt.Y };
		sxy = center + Point2D { -pt.X, pt.Y };
		Draw_Line_Rect(rect, sxy, dxy, color);

		dxy = center + Point2D { pt.Y, pt.X };
		sxy = center + Point2D { -pt.Y, pt.X };
		Draw_Line_Rect(rect, sxy, dxy, color);

		dxy = center + Point2D { pt.X, -pt.Y };
		sxy = center + Point2D { -pt.X, -pt.Y };
		Draw_Line_Rect(rect, sxy, dxy, color);

		dxy = center + Point2D { pt.Y, -pt.X };
		sxy = center + Point2D { -pt.Y, -pt.X };
		Draw_Line_Rect(rect, sxy, dxy, color);

		/**
		 *  Check decision and update it, x and y.
		 */
		if (d < 0)
		{

			/**
			 *  Calculate delta for vertical pixel.
			 */
			d += (4 * pt.Y) + 6;

		}
		else
		{

			/**
			 *  Calculate delta for diagonal pixel.
			 */
			d += 4 * (pt.Y - pt.X) + 10;
			pt.X--;
		}

		++pt.Y;

	}
	while (pt.X >= pt.Y);
}


void DSurface::Fill_Circle_Trans(const Point2D center, unsigned radius, RectangleStruct rect, ColorStruct& rgb, unsigned opacity)
{
	Fill_Ellipse_Trans(center, radius, radius, rect, rgb, opacity);
}


void DSurface::Draw_Circle(const Point2D center, unsigned radius, RectangleStruct rect, unsigned color)
{
	Draw_Ellipse(center, radius, radius, rect, color);
}


bool DSurface::Fill_Ellipse(Point2D point, int radius_x, int radius_y, RectangleStruct clip, unsigned color)
{
	// TODO
	return false;
}


bool DSurface::Fill_Ellipse_Trans(Point2D point, int radius_x, int radius_y, RectangleStruct clip, ColorStruct& rgb, unsigned opacity)
{
	// TODO
	return false;
}


bool DSurface::Put_Pixel_Trans(Point2D& point, ColorStruct& rgb, unsigned opacity)
{
	int bpp = Get_Bytes_Per_Pixel();
	if (bpp != 2)
	{
		return false;
	}

	opacity = MinImpl((int)opacity, 100);

	unsigned scale = (opacity * 255) / 100;
	unsigned short delta = (255 - scale) & 0xFFFF;

	unsigned int red_max = (unsigned int)(255 >> RedRight) << RedLeft;
	unsigned int green_max = (unsigned int)(255 >> GreenRight) << GreenLeft;
	unsigned int blue_max = (unsigned int)(255 >> BlueRight) << BlueLeft;

	unsigned short color = rgb.ToInit();

	unsigned rscaled = scale * (color & red_max);
	unsigned gscaled = scale * (color & green_max);
	unsigned bscaled = scale * (color & blue_max);

	unsigned short rmax = red_max & 0xFFFF;
	unsigned short gmax = green_max & 0xFFFF;
	unsigned short bmax = blue_max & 0xFFFF;

	unsigned short* current_pixel = (unsigned short*)Lock(point.X, point.Y);
	*current_pixel = (unsigned short)
		(((*current_pixel & rmax) * (delta + rscaled) >> 8) & rmax)
		| (((*current_pixel & gmax) * (delta + gscaled) >> 8) & gmax)
		| (((*current_pixel & bmax) * (delta + bscaled) >> 8) & bmax);

	Unlock();

	return true;
}

#pragma warning(pop)

CellStruct TechnoClass::FindExitCell(TechnoClass* pDocker, CellStruct nDefault) const
{ JMP_THIS(0x70AD50); }

ConvertClass* ConvertClass::CreateFromFile(const char* pal_filename) {

	CCFileClass file{ pal_filename };

	if (!file.Exists() || !file.Open(FileAccessMode::Read)) {
		return nullptr;
	}

	if (void* data = CCFileClass::Load_Alloc_Data(file)) {
		BytePalette loaded_pal { };
		std::memcpy(&loaded_pal, data, sizeof(BytePalette));
		delete data;
		return GameCreate<ConvertClass>(loaded_pal, FileSystem::TEMPERAT_PAL(), DSurface::Primary(), 1, false);
	}

	return nullptr;
}

void Game::Unselect_All_Except(AbstractType rtti)
{
	int index = 0;
	while (index < ObjectClass::CurrentObjects->Count)
	{

		if (ObjectClass::CurrentObjects->Items[index]->What_Am_I() == rtti)
		{
			++index;
			continue;
		}

		int count_before = ObjectClass::CurrentObjects->Count;
		ObjectClass::CurrentObjects->Items[index]->Deselect();

		if (count_before <= ObjectClass::CurrentObjects->Count)
		{
			ObjectClass::CurrentObjects->erase(ObjectClass::CurrentObjects->Items[index]);
		}
	}
}

void Game::Unselect_All_Except(ObjectTypeClass* objecttype)
{
	int index = 0;
	while (index < ObjectClass::CurrentObjects->Count)
	{

		if (ObjectClass::CurrentObjects->Items[index]->GetType() == objecttype)
		{
			++index;
			continue;
		}

		int count_before = ObjectClass::CurrentObjects->Count;
		ObjectClass::CurrentObjects->Items[index]->Deselect();

		if (count_before <= ObjectClass::CurrentObjects->Count)
		{
			ObjectClass::CurrentObjects->erase(ObjectClass::CurrentObjects->Items[index]);
		}
	}
}

void Game::Unselect_All_Except(ObjectClass* object)
{
	int index = 0;
	while (index < ObjectClass::CurrentObjects->Count)
	{

		if (ObjectClass::CurrentObjects->Items[index] == object)
		{
			++index;
			continue;
		}

		int count_before = ObjectClass::CurrentObjects->Count;
		ObjectClass::CurrentObjects->Items[index]->Deselect();

		if (count_before <= ObjectClass::CurrentObjects->Count)
		{
			ObjectClass::CurrentObjects->erase(ObjectClass::CurrentObjects->Items[index]);
		}
	}
}

	std::array<const DWORD, 21> CellClass::TileArray =
	{ {
		{0x0},
		{0x484AB0},
		{0x485060},
		{0x486380},
		{0x4863A0},
		{0x4863D0},
		{0x4865B0},
		{0x4865D0},
		{0x486650},
		{0x486670},
		{0x486690},
		{0x4866D0},
		{0x4866F0},
		{0x486710},
		{0x486730},
		{0x486750},
		{0x486770},
		{0x486790},
		{0x4867B0},
		{0x4867E0},
		{0x486900},
	}};

	void CellClass::ReduceTiberiumWithinCircularArea(int radius, int reduceAmount)
	{
		// Iterate through a square area from -radius to +radius in both directions
		for (short offsetY = -radius; offsetY <= radius; offsetY++) {
			for (short offsetX = -radius; offsetX <= radius; offsetX++) {
				// Calculate distance from center using Pythagorean theorem
				// Only process cells within circular radius
				if ((int)Math::sqrt((double)(offsetX * offsetX) + (double)(offsetY * offsetY)) <= radius) {
					// Calculate target cell position
					const CellStruct targetCell {
						this->MapCoords.X + offsetX , this->MapCoords.Y + offsetY
					};

					// Get the cell at this position
					if (CellClass* cell = MapClass::Instance->GetCellAt(targetCell)) {
						cell->ReduceTiberium(reduceAmount);
					}
				}
			}
		}
	}

	std::array<TActionClass::ActionFuncEntry, (size_t)TriggerAction::count> TActionClass::ActionFuncTable =
	{ {
		{ "Win", 0x6E0440 },
		{ "Lose", 0x6E0460 },
		{ "ProductionBegins", 0x6E1E40 },
		{ "CreateTeam", 0x6E1F60 },
		{ "DestroyTeam", 0x6E1F90 },
		{ "AllToHunt", 0x6E1FF0 },
		{ "Reinforcement", 0x6E1FB0 },
		{ "DropZoneFlare", 0x6E1CC0 },
		{ "FireSale", 0x6E1EA0 },
		{ "PlayMovie", 0x6E16D0 },
		{ "TextTrigger", 0x6E0D60 },
		{ "DestroyTrigger", 0x6E2A10 },
		{ "AutocreateBegins", 0x6E1F00 },
		{ "ChangeHouse", 0x6E0AA0 },
		{ "AllowWin", 0x6E1DC0 },
		{ "RevealAllMap", 0x6E1330 },
		{ "RevealAroundWaypoint", 0x6E0FE0 },
		{ "RevealWaypointZone", 0x6E11C0 },
		{ "PlaySoundEffect", 0x6E1760 },
		{ "PlayMusicTheme", 0x6E1B90 },
		{ "PlaySpeech", 0x6E1BB0 },
		{ "ForceTrigger", 0x6E2AA0 },
		{ "TimerStart", 0x6E13A0 },
		{ "TimerStop", 0x6E13E0 },
		{ "TimerExtend", 0x6E1440 },
		{ "TimerShorten", 0x6E14A0 },
		{ "TimerSet", 0x6E1530 },
		{ "GlobalSet", 0x6E0FA0 },
		{ "GlobalClear", 0x6E0FC0 },
		{ "AutoBaseBuilding", 0x6E0EF0 },
		{ "GrowShroud", 0x6E0F90 },
		{ "DestroyAttachedObject", 0x6E2050 },
		{ "AddOneTimeSuperWeapon", 0x6E1BD0 },
		{ "AddRepeatingSuperWeapon", 0x6E1C40 },
		{ "PreferredTarget", 0x6E0ED0 },
		{ "AllChangeHouse", 0x6E0B60 },
		{ "MakeAlly", 0x6E0DF0 },
		{ "MakeEnemy", 0x6E0E60 },
		{ "ChangeZoomLevel", 0 }, // Not found
		{ "ResizePlayerView", 0x6E21E0 },
		{ "PlayAnimAt", 0x6E2290 },
		{ "DoExplosionAt", 0x6E2390 },
		{ "CreateVoxelAnim", 0x6E2520 },
		{ "IonStormStart", 0x6E2600 },
		{ "IonStormStop", 0x6E2640 },
		{ "LockInput", 0x6E2660 },
		{ "UnlockInput", 0x6E26C0 },
		{ "MoveCameraToWaypoint", 0x6E26D0 },
		{ "ZoomIn", 0x6E2860 },
		{ "ZoomOut", 0x6E28E0 },
		{ "ReshroudMap", 0x6E2950 },
		{ "ChangeLightBehavior", 0x6E2970 },
		{ "EnableTrigger", 0x6E2AF0 },
		{ "DisableTrigger", 0x6E2B70 },
		{ "CreateRadarEvent", 0x6E2BB0 },
		{ "LocalSet", 0x6E2BE0 },
		{ "LocalClear", 0x6E2C00 },
		{ "MeteorShower", 0x6E2C40 },
		{ "ReduceTiberium", 0x6E1180 },
		{ "SellBuilding", 0x6E08B0 },
		{ "TurnOffBuilding", 0x6E09A0 },
		{ "TurnOnBuilding", 0x6E0A20 },
		{ "Apply100Damage", 0x6E0490 },
		{ "SmallLightFlash", 0x6E0790 },
		{ "MediumLightFlash", 0x6E07F0 },
		{ "LargeLightFlash", 0x6E0850 },
		{ "AnnounceWin", 0x6E0440 },
		{ "AnnounceLose", 0x6E0460 },
		{ "ForceEnd", 0x6E0480 },
		{ "DestroyTag", 0x6E2A50 },
		{ "SetAmbientStep", 0x6E2E20 },
		{ "SetAmbientRate", 0x6E2E40 },
		{ "SetAmbientLight", 0x6E2F50 },
		{ "AITriggersBegin", 0x6E2FA0 },
		{ "AITriggersStop", 0x6E3000 },
		{ "RatioOfAITriggerTeams", 0x6E3300 },
		{ "RatioOfTeamAircraft", 0x6E3320 },
		{ "RatioOfTeamInfantry", 0x6E3340 },
		{ "RatioOfTeamUnits", 0x6E3360 },
		{ "ReinforcementAt", 0x6E1FD0 },
		{ "WakeupSelf", 0x6E01C0 },
		{ "WakeupAllSleepers", 0x6E02B0 },
		{ "WakeupAllHarmless", 0x6E0330 },
		{ "WakeupGroup", 0x6E03B0 },
		{ "VeinGrowth", 0x6E0250 },
		{ "TiberiumGrowth", 0x6E0270 },
		{ "IceGrowth", 0x6E0290 },
		{ "ParticleAnim", 0x6E0110 },
		{ "RemoveParticleAnim", 0x6E0080 },
		{ "LightningStrike", 0x6E0050 },
		{ "GoBerzerk", 0x6E0930 },
		{ "ActivateFirestorm", 0 }, // Not found
		{ "DeactivateFirestorm", 0 }, // Not found
		{ "IonCannonStrike", 0x6E3380 },
		{ "NukeStrike", 0x6E3410 },
		{ "ChemMissileStrike", 0x6E38F0 },
		{ "ToggleTrainCargo", 0x6E3B20 },
		{ "PlaySoundEffectRandom", 0x6E1780 },
		{ "PlaySoundEffectAtWaypoint", 0x6E18B0 },
		{ "PlayIngameMovie", 0x6E1720 },
		{ "ReshroudMapAtWaypoint", 0x6E1A70 },
		{ "LightningStormStrike", 0x6E35F0 },
		{ "TimerText", 0x6E15F0 },
		{ "FlashTeam", 0x6E4020 },
		{ "TalkBubble", 0x6E4040 },
		{ "SetObjectTechLevel", 0x6E37F0 },
		{ "ReinforcementByChrono", 0x6E3890 },
		{ "CreateCrate", 0x6E38B0 },
		{ "IronCurtain", 0x6E36E0 },
		{ "PauseGame", 0x6E4080 },
		{ "EvictOccupiers", 0x6E4090 },
		{ "CenterCameraAtWaypoint", 0x6E2790 },
		{ "MakeHouseCheer", 0x6E3060 },
		{ "SetTabTo", 0x6E4100 },
		{ "FlashCameo", 0x6E4150 },
		{ "StopSounds", 0x6E1980 },
		{ "PlayIngameMovieAndPause", 0x6E1740 },
		{ "ClearAllSmudges", 0x6E2C20 },
		{ "DestroyAll", 0x6E3180 },
		{ "DestroyAllBuildings", 0x6E31E0 },
		{ "DestroyAllLandUnits", 0x6E3240 },
		{ "DestroyAllNavalUnits", 0x6E32A0 },
		{ "MindControlBase", 0x6E0CA0 },
		{ "RestoreMindControlledBase", 0x6E0D00 },
		{ "CreateBuilding", 0x6E4200 },
		{ "RestoreStartingUnits", 0x6E30C0 },
		{ "StartChronoScreenEffect", 0x6E2F90 },
		{ "TeleportAll", 0x6E1A40 },
		{ "SetSuperWeaponCharge", 0x6E42D0 },
		{ "RestoreStartingBuildings", 0x6E3120 },
		{ "FlashBuildingsOfType", 0x6E4560 },
		{ "SuperWeaponSetRechargeTime", 0x6E4320 },
		{ "SuperWeaponResetRechargeTime", 0x6E4360 },
		{ "SuperWeaponReset", 0x6E43A0 },
		{ "SetPreferredTargetCell", 0x6E43E0 },
		{ "ClearPreferredTargetCell", 0x6E4440 },
		{ "SetBaseCenterCell", 0x6E44E0 },
		{ "ClearBaseCenterCell", 0x6E4540 },
		{ "BlackoutRadar", 0x6E3B40 },
		{ "SetDefensiveTargetCell", 0x6E4460 },
		{ "ClearDefensiveTargetCell", 0x6E44C0 },
		{ "RetintRed", 0x6E2E60 },
		{ "RetintGreen", 0x6E2EB0 },
		{ "RetintBlue", 0x6E2F00 },
		{ "JumpCameraHome", 0x6E2850 }
	}};

	std::array<const DWORD, (size_t)TeamMissionType::count> TeamClass::TMissionFuncTable = {
		0x6ED090, // Attack
		0x6EC9A0, // Att_waypt
		0x6EDD90, // Go_bezerk
		0x6EC7D0, // Move
		0x6EC770, // Movecell
		0x6ED770, // Guard
		0x6EDE10, // Loop
		0x6EDE40, // Player_wins
		0x6EF110, // Unload
		0x6ED4D0, // Deploy
		0x6EDB50, // Hound_dog
		0x6ED7E0, // Do
		0x6EDA90, // Set_global
		0x6EDDC0, // Idle_anim
		0x6ED200, // Load
		0x6ECE60, // Spy
		0x6ECCE0, // Patrol
		0x6ED030, // Change_script
		0x6ECFB0, // Change_team
		0x6EDD60, // Panic
		0x6ECF50, // Change_house
		0x6ECF10, // Scatter
		0x6EC730, // Goto_nearby_shroud
		0x6EDE60, // Player_loses
		0x6EDE80, // Play_speech
		0x6EDE90, // Play_sound
		0x6EDEC0, // Play_movie
		0x6EDEF0, // Play_music
		0x6EDF10, // Reduce_tiberium
		0x6EDF90, // Begin_production
		0x6EDFB0, // Fire_sale
		0x6EDFD0, // Self_destruct
		0x6EE0A0, // Ion_storm_start_in
		0x6EE0E0, // Ion_storn_end
		0x6EE100, // Center_view_on_team
		0x6EE1B0, // Reshroud_map
		0x6EE1D0, // Reveal_map
		0x6EE050, // Delete_team_members
		0x6EDAC0, // Clear_global
		0x6EDAF0, // Set_local
		0x6EDB20, // Clear_local
		0x6EDC70, // Unpanic
		0x6EDCA0, // Force_facing
		0x6EE1F0, // Wait_till_fully_loaded
		0x6EE230, // Truck_unload
		0x6EE2A0, // Truck_load
		0x6EE310, // Attack_enemy_building
		0x6EE3F0, // Moveto_enemy_building
		0x6EE800, // Scout
		0x6EF450, // Success
		0x6EF5C0, // Flash
		0x6EF610, // Play_anim
		0x6EF6D0, // Talk_bubble
		0x6EF700, // Gather_at_enemy
		0x6EFA10, // Gather_at_base
		0x6EFC70, // Iron_curtain_me
		0x6EFE60, // Chrono_prep_for_abwp
		0x6F0130, // Chrono_prep_for_aq
		0x6EE5C0, // Move_to_own_building
		0x6ECA70, // Attack_building_at_waypoint
		0x6ECB50, // Enter_grinder
		0x6ECBA0, // Occupy_tank_bunker
		0x6ECBF0, // Enter_bio_reactor
		0x6ECC40, // Occupy_battle_bunker
		0x6ECC90  // Garrison_building
	};


const char* const FileClass::FileErrorToString[] =
 {
		  "Non-error. "
		, "Operation not permitted. "
		, "No such file or directory. "
		, "No such process. "
		, "Interrupted function call. "
		, "Input/output error. "
		, "No such device or address. "
		, "Argument list too long. "
		, "Exec format error. "
		, "Bad file descriptor. "
		, "No child processes. "
		, "Resource temporarily unavailable. "
		, "Not enough space/cannot allocate memory. "
		, "Permission denied. "
		, "Bad address. "
		, "Unknown error 15. "
		, "Device or resource busy. "
		, "File exists. "
		, "Improper link. "
		, "No such device. "
		, "Not a directory. "
		, "Is a directory. "
		, "Invalid argument. "
		, "Too many open files in system. "
		, "Too many open files. "
		, "Unknown error 26. "
		, "Inappropriate I/O control operation. "
		, "File too large. "
		, "No space left on device. "
		, "Invalid seek. "
		, "Read-only filesystem. "
		, "Too many links. "
		, "Broken pipe. "
		, "Mathematics argument out of domain of function. "
		, "Result too large. "
		, "Unknown error 36. "
		, "Resource deadlock avoided. "
		, "Filename too long. "
		, "No locks available. "
		, "Function not implemented. "
		, "Directory not empty. "
		, "Invalid or incomplete multibyte or wide character. "
};

HouseClass* HouseClass::FindByCountryName(const char* name)
{
	auto idx = HouseTypeClass::FindIndexByIdAndName(name);
	return FindByCountryIndex(idx);
}

// gets the first house of a type with name Neutral
HouseClass* HouseClass::FindNeutral()
{
	return FindByCountryName(GameStrings::Neutral());
}

// gets the first house of a type with name Special
HouseClass* HouseClass::FindSpecial()
{
	return FindByCountryName(GameStrings::Special());
}

// gets the first house of a type from the Civilian side
HouseClass* HouseClass::FindCivilianSide()
{
	return FindBySideName(GameStrings::Civilian());
}

void CellClass::CreateGap(HouseClass* pHouse, int range, CoordStruct& coords)
{
	DisplayClass::Instance->Sub_4ADEE0(0, 0);
	CellRangeIterator<CellClass>{}(CellClass::Coord2Cell(coords), range + 0.5, [](CellClass* pCell) {
		pCell->Flags &= ~CellFlags::Revealed;
		pCell->AltFlags &= ~AltCellFlags::Clear;
		pCell->ShroudCounter = 1;
		pCell->GapsCoveringThisCell = 0;
		return true;
	});
	DisplayClass::Instance->Sub_4ADCD0(0, 0);
	pHouse->Visionary = 0;
	MapClass::Instance->Map_AI();
	MapClass::Instance->MarkNeedsRedraw(2);
}

void TechnoClass::SpillTiberium(int& value ,int idx , CellClass* pCenter, Point2D const& nMinMax)
{
	if (!pCenter)
		return;

	COMPILETIMEEVAL FacingType Neighbours[] = {
		FacingType::NorthWest,
		FacingType::East,
		FacingType::NorthWest,
		FacingType::NorthEast,
		FacingType::South,
		FacingType::SouthEast,
		FacingType::North,
		FacingType::SouthWest,
		FacingType::West
	};

	for (auto const& neighbour : Neighbours) {
		// spill random amount
		const int amount = ScenarioClass::Instance->Random.RandomRanged(nMinMax.X , nMinMax.Y);
		CellClass* pCell = pCenter->GetNeighbourCell(neighbour);

		if (!pCell)
			continue;

		pCell->IncreaseTiberium(idx, amount);
		value -= amount;

		// stop if value is reached
		if (value <= 0) {
			break;
		}
	}
}

ObjectClass* AnimTypeClass::CreateObject(HouseClass* owner)
{
	auto pAnim = GameCreate<AnimClass>(this, CoordStruct::Empty);
	pAnim->SetHouse(owner);
	return (ObjectClass*)pAnim;
}; // ! this just returns NULL instead of creating the anim, fucking slackers

bool GameStrings::IsBlank(const char* pValue)
{
	if(!pValue)
		return true;

	return CRT::strcmpi(pValue, NoneStr.get()) == 0
		|| CRT::strcmpi(pValue, NoneStrb()) == 0;
}

void TechnoClass::TurnFacing(const DirStruct& nDir)
{
	if (this->GetTechnoType()->Turret)
	{
		this->SecondaryFacing.Set_Current(nDir);
	}
	else
	{
		this->PrimaryFacing.Set_Current(nDir);
	}
}

void TechnoClass::ClearAllTarget()
{
	this->Target = nullptr;
	this->SetTarget(nullptr);
	this->QueueMission(Mission::Stop, true);

	if (auto pManager = this->SpawnManager)
	{
		pManager->Target = nullptr;
		pManager->NewTarget = nullptr;
		pManager->SetTarget(nullptr);
	}

	if (auto pTemporal = this->TemporalImUsing)
	{
		pTemporal->LetGo();
	}
}

bool TechnoClass::IsCloaked() const
{
	if (this->CloakState == CloakState::Cloaked) {
		return true;
	}
	else if (this->WhatAmI() == AbstractType::Building) {
		return (reinterpret_cast<const BuildingClass*>(this)->Translucency == 15);
	}

	return false;
}

void TechnoClass::DetachSpecificSpawnee(HouseClass* NewSpawneeOwner)
{
	if (!this->SpawnOwner)
		return;

	//find the specific spawnee in the node
	for (auto& SpawnNode : this->SpawnOwner->SpawnManager->SpawnedNodes) {

		if (this == SpawnNode->Unit) {

			SpawnNode->Unit = nullptr;
			this->SpawnOwner = nullptr;

			SpawnNode->Status = SpawnNodeStatus::Dead;

			this->SetOwningHouse(NewSpawneeOwner);
		}
	}
}

void TechnoClass::FreeSpecificSlave(HouseClass* Affector) {

	if (!this->SlaveOwner)
		return;

	//If you're a slave, you're an InfantryClass. But since most functions use TechnoClasses and the check can be done in that level as well
	//it's easier to set up the recasting in this function
	//Anybody who writes 357, take note that SlaveManager uses InfantryClasses everywhere, SpawnManager uses TechnoClasses derived from AircraftTypeClasses
	//as I wrote it in http://bugs.renegadeprojects.com/view.php?id=357#c10331
	//So, expand that one instead, kthx.

	if (InfantryClass* pSlave = cast_to<InfantryClass* , false>(this)) {
		auto Manager = pSlave->SlaveOwner->SlaveManager;

		//LostSlave can free the unit from the miner, so we're awesome.
		Manager->LostSlave(pSlave);
		pSlave->SlaveOwner = nullptr;

		//OK, delinked, Now relink it to the side which separated the slave from the miner
		pSlave->SetOwningHouse(Affector);
	}
}

bool TechnoClass::CanThisCloakByDefault() const
{
	return (GetTechnoType()) && (GetTechnoType()->Cloakable || HasAbility(AbilityType::Cloak));
}

bool SuperClass::IsDisabledFromShell() const
{
	if (SessionClass::Instance->GameMode != GameMode::Campaign && !Unsorted::SWAllowed) {
		if (this->Type->DisableableFromShell) {
			return true;
		}
	}

	return false;
}

const char* TeamClass::get_ID() const
{
	return Type ? Type->get_ID() : GameStrings::NoneStr;
}

const char* ScriptClass::get_ID() const
{
	return Type ? Type->get_ID() : GameStrings::NoneStr;
}

DEFINE_IMPLEMENTATION(void TechnoClass::Draw_Object(SHPStruct*,
	int,
	Point2D*,
	RectangleStruct*,
	DirType,  //unused
	int, //unused
	int,
	ZGradient,
	bool,
	int,
	int,
	SHPStruct*,
	int,
	int,
	int,
	BlitterFlags), 0x705E00);

void AITriggerTypeClass::FormatForSaving(char* buffer, size_t size) const
{
	const char* Team1Name = GameStrings::NoneStr();
	const char* Team2Name = GameStrings::NoneStr();
	const char* HouseName = GameStrings::NoneStr();
	const char* ConditionName = GameStrings::NoneStr();

	TeamTypeClass* T = this->Team1;
	if (T)
	{
		Team1Name = T->get_ID();
	}
	T = this->Team2;
	if (T)
	{
		Team2Name = T->get_ID();
	}

	if (this->OwnerHouseType == AITriggerHouseType::Single)
	{
		auto const idxHouse = this->HouseIndex;
		if (idxHouse != -1)
		{
			HouseName = HouseTypeClass::Array->operator[](idxHouse)->get_ID();
		}
	}
	else if (this->OwnerHouseType == AITriggerHouseType::Any)
	{
		HouseName = "<all>";
	}

	TechnoTypeClass* O = this->ConditionObject;
	if (O)
	{
		ConditionName = O->get_ID();
	}

	char ConditionString[68];
	int idx = 0;
	char* condStr = ConditionString;
	auto buf = reinterpret_cast<const byte*>(&this->Conditions);
	do
	{
		sprintf_s(condStr, 4, "%02x", *buf);
		++buf;
		++idx;
		condStr += 2;
	}
	while (idx < 0x20);
	*condStr = '\0';

	sprintf_s(buffer, size, "%s = %s,%s,%s,%d,%d,%s,%s,%lf,%lf,%lf,%u,%d,%d,%u,%s,%u,%u,%u\n",
		this->ID,
		this->Name,
		Team1Name,
		HouseName,
		this->TechLevel,
		this->ConditionType,
		ConditionName,
		ConditionString,
		this->Weight_Current,
		this->Weight_Minimum,
		this->Weight_Maximum,
		this->IsForSkirmish,
		0,
		this->SideIndex,
		this->IsForBaseDefense,
		Team2Name,
		this->Enabled_Easy,
		this->Enabled_Normal,
		this->Enabled_Hard
	);

}

void DSurface::DSurfaceDrawText(const wchar_t* pText, RectangleStruct* pBounds, Point2D* pLocation,
	COLORREF ForeColor, COLORREF BackColor, TextPrintType Flag)
{
	TextDrawing::Fancy_Text_Print_Wide_NoFormat(pText, this, pBounds, pLocation, (unsigned int)ForeColor, (unsigned int)BackColor, Flag);
}

void DSurface::DrawColorSchemeText(const wchar_t* pText, RectangleStruct& pBounds, Point2D& pLocation,
ColorScheme* ForeColor, COLORREF BackColor, TextPrintType Flag)
{
	TextDrawing::Fancy_Text_Print_Wide_NoFormat(pText, this, &pBounds, &pLocation, ForeColor, (unsigned int)BackColor, Flag);
}

void DSurface::DSurfaceDrawText(const wchar_t* pText, Point2D* pLoction, COLORREF Color)
{
	RectangleStruct rect = this->Get_Rect();
	TextDrawing::Fancy_Text_Print_Wide_NoFormat(pText, this, &rect, pLoction, (unsigned int)Color, 0, TextPrintType::NoShadow);
}
