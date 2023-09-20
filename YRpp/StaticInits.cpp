//This file initializes static constant values.

#include <YRPP.h>
#include <YRPPGlobal.h>
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
#include <Surface.h>

#include <WWKeyboardClass.h>

const CoordStruct CoordStruct::Empty = { 0,0,0 };
const ColorStruct ColorStruct::Empty = { 0,0,0 };
const Color16Struct Color16Struct::Empty = { 0,0,0 };
const CellStruct CellStruct::Empty = { 0,0 };
const CellStruct CellStruct::DefaultUnloadCell = { 3 , 1 };
const Point2D Point2D::Empty = { 0,0 };
const Point2DBYTE Point2DBYTE::Empty = { 0u,0u };
const Point3D Point3D::Empty = { 0,0,0 };

std::array< ColorStruct, (size_t)DefaultColorList::count> Drawing::DefaultColors
{
{
		// gery  , red , green
		{ 128,128,128 } , { 255,0,0 } , { 0,255,0 } ,
		// blue , yellow , white
		{ 0,0,255 } , { 255,255,0 } , { 255 , 255 , 255 } ,
		// ares cameo transparent color
		{ 255 , 0 , 255 }
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

const char* ObjectClass::get_ID() const
{
	const auto pType = this->GetType();
	return pType ? pType->get_ID() : GameStrings::NoneStr();
}

Point2D ObjectClass::GetScreenLocation() const
{
	CoordStruct crdAbsolute = this->GetCoords();
	Point2D  posScreen = { 0,0 };
	TacticalClass::Instance->CoordsToScreen(&posScreen, &crdAbsolute);
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


int HouseClass::GetSpawnPosition()
{
	for (int i = 0; i < HouseClass::MaxPlayers; i++)
	{
		if (HouseClass::Array->GetItemOrDefault(ScenarioClass::Instance->HouseIndices[i], nullptr) == this)
			return i;
	}

	return -1;
}

int HouseClass::CountOwnedNow(const TechnoTypeClass* const pItem) const {
	switch(pItem->WhatAmI()) {
	case AbstractType::BuildingType:
		return this->CountOwnedNow(
			static_cast<BuildingTypeClass const*>(pItem));
	case AbstractType::UnitType:
		return this->CountOwnedNow(
			static_cast<UnitTypeClass const*>(pItem));
	case AbstractType::InfantryType:
		return this->CountOwnedNow(
			static_cast<InfantryTypeClass const*>(pItem));
	case AbstractType::AircraftType:
		return this->CountOwnedNow(
			static_cast<AircraftTypeClass const*>(pItem));
	default:
		return 0;
	}
}

int HouseClass::CountOwnedAndPresent(TechnoTypeClass* pItem) const {
	switch(pItem->WhatAmI()) {
	case AbstractType::BuildingType:
		return this->CountOwnedAndPresent((BuildingTypeClass*)pItem);
	case AbstractType::UnitType:
		return this->CountOwnedAndPresent((UnitTypeClass*)pItem);
	case AbstractType::InfantryType:
		return this->CountOwnedAndPresent((InfantryTypeClass*)pItem);
	case AbstractType::AircraftType:
		return this->CountOwnedAndPresent((AircraftTypeClass*)pItem);
	default:
		return 0;
	}
}

int HouseClass::CountOwnedEver(TechnoTypeClass* pItem) const {
	switch(pItem->WhatAmI()) {
	case AbstractType::BuildingType:
		return this->CountOwnedEver((BuildingTypeClass*)pItem);
	case AbstractType::UnitType:
		return this->CountOwnedEver((UnitTypeClass*)pItem);
	case AbstractType::InfantryType:
		return this->CountOwnedEver((InfantryTypeClass*)pItem);
	case AbstractType::AircraftType:
		return this->CountOwnedEver((AircraftTypeClass*)pItem);
	default:
		return 0;
	}
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
	return this->Supers.GetItemOrDefault(index);
}

bool HouseClass::IsIonCannonEligibleTarget(const TechnoClass* const pTechno) const {
	if(pTechno->IsAlive && !pTechno->InLimbo && pTechno->InWhichLayer() == Layer::Ground) {
		return true;
	}

	// hard difficulty shoots the tank in the factory
	if(this->AIDifficulty == AIDifficulty::Hard) {
		for(const auto* pFactory : *FactoryClass::Array) {
			if(pFactory->Object == pTechno
				&& pFactory->Production.Timer.Duration
				&& !pFactory->IsSuspended)
			{
				return true;
			}
		}
	}

	return false;
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
		else if (rules.BuildConst.FindItemIndex(pType->DeploysInto) != -1)
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
		else if (rules.BuildConst.FindItemIndex(pType) != -1)
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

	return pValues ? pValues->GetItemOrDefault(static_cast<int>(difficulty), value) : value;
}

TechnoTypeClass* BuildingClass::GetSecretProduction() const {
	auto const pType = this->Type;

	if(pType->SecretInfantry) {
		return pType->SecretInfantry;
	}
	if(pType->SecretUnit) {
		return pType->SecretUnit;
	}
	if(pType->SecretBuilding) {
		return pType->SecretBuilding;
	}
	return this->SecretProduction;
}

void InfantryClass::RemoveMe_FromGunnerTransport()
{
	if (auto pTransport = this->Transporter)
	{
		if (auto pUnit = specific_cast<UnitClass*>(pTransport))
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
		Game::RaiseError(E_POINTER);
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
				Game::RaiseError(res);
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

void LoadProgressManager::DrawText(const wchar_t *pText, int X, int Y, DWORD dwColor)
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
					list.AddItem(ptr);
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

	unsigned short color = RGBA_To_Pixel(rgb.R, rgb.G, rgb.B);

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

	if (!file.Exists()) {
		return nullptr;
	}

	file.Open(FileAccessMode::Read);

	void* data = CCFileClass::Load_Alloc_Data(file);
	if (!data)
	{
		return nullptr;
	}

	BytePalette loaded_pal { };
	std::memcpy(&loaded_pal, data, sizeof(BytePalette));

	ConvertClass* drawer = GameCreate<ConvertClass>(loaded_pal, FileSystem::TEMPERAT_PAL(), DSurface::Primary(), 1 , false);
	return drawer;
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
		ObjectClass::CurrentObjects->GetItem(index)->Deselect();

		if (count_before <= ObjectClass::CurrentObjects->Count)
		{
			ObjectClass::CurrentObjects->Remove(ObjectClass::CurrentObjects->Items[index]);
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
			ObjectClass::CurrentObjects->Remove(ObjectClass::CurrentObjects->Items[index]);
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
			ObjectClass::CurrentObjects->Remove(ObjectClass::CurrentObjects->Items[index]);
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

void TechnoClass::SpillTiberium(int& value ,int idx , CellClass* pCenter, Point2D const& nMinMax)
{
	if (!pCenter)
		return;

	constexpr FacingType Neighbours[] = {
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
	if (auto pAnim = GameCreate<AnimClass>(this, CoordStruct::Empty)) {
		pAnim->SetHouse(owner);
		return (ObjectClass*)pAnim;
	}

	return nullptr;
}; // ! this just returns NULL instead of creating the anim, fucking slackers

bool GameStrings::IsBlank(const char* pValue)
{
	return CRT::strcmpi(pValue, NoneStr()) == 0
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

	// setting up the nodes. Funnily, nothing else from the manager is needed
	const auto& SpawnNodes = this->SpawnOwner->SpawnManager->SpawnedNodes;

	//find the specific spawnee in the node
	for (auto SpawnNode : SpawnNodes) {

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

	if (InfantryClass* pSlave = specific_cast<InfantryClass*>(this)) {
		auto Manager = pSlave->SlaveOwner->SlaveManager;

		//LostSlave can free the unit from the miner, so we're awesome.
		Manager->LostSlave(pSlave);
		pSlave->SlaveOwner = nullptr;

		//OK, delinked, Now relink it to the side which separated the slave from the miner
		pSlave->SetOwningHouse(Affector);
	}
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