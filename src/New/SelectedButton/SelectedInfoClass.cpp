#include "SelectedInfoClass.h"

#include <AircraftClass.h>
#include <FactoryClass.h>
#include <SpawnManagerClass.h>
#include <SuperClass.h>
#include <MessageListClass.h>
#include <PCX.h>
#include <VocClass.h>

#include <Ext/Side/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/Cast.h>

#include <Map>

SelectedInfoClass SelectedInfoClass::Instance;

const wchar_t* SelectedInfoClass::Status[35] =
{
	L"Sleep", L"Attack", L"Move", L"QueueMove", L"Retreat",
	L"Guard", L"Sticky", L"Enter", L"Capture", L"Eaten",
	L"Harvest", L"AreaGuard", L"Return", L"Stop", L"Ambush",
	L"Hunt", L"Unload", L"Sabotage", L"Construction", L"Selling",
	L"Repair", L"Rescue", L"Missile", L"Harmless", L"Open",
	L"Patrol", L"Paradrop", L"AttackMove", L"Wait", L"Produce",
	L"Deactive", L"Locomotor", L"FollowGuard", L"Unknown", L"None"
};

const char* SelectedInfoClass::StatusEntry[35] =
{
	"Status:Sleep", "Status:Attack", "Status:Move", "Status:QueueMove", "Status:Retreat",
	"Status:Guard", "Status:Sticky", "Status:Enter", "Status:Capture", "Status:Eaten",
	"Status:Harvest", "Status:AreaGuard", "Status:Return", "Status:Stop", "Status:Ambush",
	"Status:Hunt", "Status:Unload", "Status:Sabotage", "Status:Construction", "Status:Selling",
	"Status:Repair", "Status:Rescue", "Status:Missile", "Status:Harmless", "Status:Open",
	"Status:Patrol", "Status:Paradrop", "Status:AttackMove", "Status:Wait", "Status:Produce",
	"Status:Deactive", "Status:Locomotor", "Status:FollowGuard", "Status:Unknown", "Status:None"
};

BSurface* SelectedInfoClass::MissingCameo;

SHPFile* SelectedInfoClass::SelectedInfo_Main;
SHPFile* SelectedInfoClass::SelectedInfo_Buff;
SHPFile* SelectedInfoClass::SelectedInfo_Button;
SHPFile* SelectedInfoClass::SelectedInfo_Bottom;
SHPFile* SelectedInfoClass::SelectedInfo_Toggle;

template<typename T>
void _removeButton(T&  item) {
	if (item) {
		
		GScreenClass::Instance->RemoveButton(item);
		GameDelete<true, false>(item);
		item = nullptr;
	}
}

template<typename T>
void _removeSHPs(T& item)
{
	if (item) {
		GameDelete<false, false>(item);
		item = nullptr;
	}
}

void SelectedInfoClass::InitClear()
{

	_removeButton(this->MainColumn);
	_removeButton(this->PushButton);
	_removeButton(this->AmmoButton);
	_removeButton(this->MainCameo);
	_removeButton(this->InfoIconA);
	_removeButton(this->InfoIconD);
	_removeButton(this->InfoIconS);
	_removeButton(this->MainBottom);
	_removeButton(this->ToggleV);
	_removeButton(this->ToggleE);
	_removeButton(this->ScrollL);
	_removeButton(this->ScrollR);

	for (int i = 0; i < 20; ++i) {
		_removeButton(this->Cameos[i]);
	}

	this->MaxCameo = 0;
	this->Current = 0;
	this->ShouldUpdate = false;
	this->SingleSelect = true;
	this->ObtainSelect = false;
	this->IsHovering = false;
	SelectedInfoClass::MissingCameo = nullptr;

	_removeSHPs(SelectedInfoClass::SelectedInfo_Main);
	_removeSHPs(SelectedInfoClass::SelectedInfo_Buff);
	_removeSHPs(SelectedInfoClass::SelectedInfo_Button);
	_removeSHPs(SelectedInfoClass::SelectedInfo_Bottom);
	_removeSHPs(SelectedInfoClass::SelectedInfo_Toggle);
}

void SelectedInfoClass::InitIO()
{
	SelectedInfoClass::SelectedInfo_Main = (SHPFile*)FakeFileLoader::_Retrieve("selectedmain.shp", false);
	SelectedInfoClass::SelectedInfo_Buff = (SHPFile*)FakeFileLoader::_Retrieve("selectedbuff.shp", false);
	SelectedInfoClass::SelectedInfo_Button = (SHPFile*)FakeFileLoader::_Retrieve("selectedbutton.shp", false);
	SelectedInfoClass::SelectedInfo_Bottom = (SHPFile*)FakeFileLoader::_Retrieve("selectedbottom.shp", false);
	SelectedInfoClass::SelectedInfo_Toggle = (SHPFile*)FakeFileLoader::_Retrieve("selectedtoggle.shp", false);

	if (Unsorted::MAP_DEBUG_MODE())
		return;

	const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	const auto pBottomSHP = pSideExt->SelectedInfo_Bottom.Get((SHPCaches*)SelectedInfoClass::SelectedInfo_Bottom);

	if (!pBottomSHP || pBottomSHP->GetFrameCount() < 3)
		return;

	const auto bottom = DSurface::Composite->Get_Height() - 32;

	if (const auto pMainSHP = pSideExt->SelectedInfo_Main.Get((SHPCaches*)SelectedInfoClass::SelectedInfo_Main))
	{
		{
			const auto pButton = GameCreate<SelectedColumnClass>(0, bottom - 100, 203, 79);
			pButton->Zap();
			GScreenClass::Instance->AddButton(pButton);
			this->MainColumn = pButton;
		}

		if (const auto pButtonSHP = pSideExt->SelectedInfo_Button.Get((SHPCaches*)SelectedInfoClass::SelectedInfo_Button))
		{
			if (pButtonSHP->GetFrameCount() >= 7)
			{
				{
					const auto pButton = GameCreate<SelectedButtonClass>(0, 197, bottom - 79);
					pButton->Zap();
					GScreenClass::Instance->AddButton(pButton);
					this->PushButton = pButton;
				}

				{
					const auto pButton = GameCreate<SelectedButtonClass>(1, 197, bottom - 50);
					pButton->Zap();
					GScreenClass::Instance->AddButton(pButton);
					this->AmmoButton = pButton;
				}
			}
		}

		{
			const auto pButton = GameCreate<SelectedMainCameoClass>(6, bottom - 95);
			pButton->Zap();
			GScreenClass::Instance->AddButton(pButton);
			this->MainCameo = pButton;
		}

		if (const auto pBuffSHP = pSideExt->SelectedInfo_Buff.Get((SHPCaches*)SelectedInfoClass::SelectedInfo_Buff))
		{
			if (pBuffSHP->GetFrameCount() >= 15)
			{
				{
					const auto pButton = GameCreate<SelectedNotButtonClass>(0, 179, bottom - 93);
					pButton->Zap();
					GScreenClass::Instance->AddButton(pButton);
					this->InfoIconA = pButton;
				}

				{
					const auto pButton = GameCreate<SelectedNotButtonClass>(1, 179, bottom - 77);
					pButton->Zap();
					GScreenClass::Instance->AddButton(pButton);
					this->InfoIconD = pButton;
				}

				{
					const auto pButton = GameCreate<SelectedNotButtonClass>(2, 179, bottom - 61);
					pButton->Zap();
					GScreenClass::Instance->AddButton(pButton);
					this->InfoIconS = pButton;
				}
			}
		}
	}

	{
		const auto pButton = GameCreate<SelectedBottomClass>(0, bottom - 21, 289, 21);
		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->MainBottom = pButton;
	}

	{
		const auto pButton = GameCreate<SelectedToggleClass>(0, Phobos::Config::SelectedDisplay_Enable ? 238 : 2, bottom - 17);
		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->ToggleV = pButton;
	}

	{
		const auto pButton = GameCreate<SelectedToggleClass>(1, 250, bottom - 17);
		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->ToggleE = pButton;
	}

	{
		const auto pButton = GameCreate<SelectedScrollClass>(0, 262, bottom - 17);
		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->ScrollL = pButton;
	}

	{
		const auto pButton = GameCreate<SelectedScrollClass>(1, 274, bottom - 17);
		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->ScrollR = pButton;
	}

	Point2D position { 0, (bottom - 69) };

	for (int i = 0; i < 20; ++i)
	{
		const auto pButton = GameCreate<SelectedCameoClass>(i, position.X, position.Y);
		position.X += 60;

		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->Cameos[i] = pButton;
	}

	this->ShouldUpdate = true;
	this->MaxCameo = std::min(Phobos::Config::SelectedDisplay_MaxCameo, (DSurface::Composite->Get_Width() - 150) / 60);

	for (int i = this->GetMaxCameo(); i < 20; ++i)
	{
		if (const auto& pButton = this->Cameos[i])
			pButton->Disabled = true;
	}
}

void SelectedInfoClass::SwitchExpand()
{
	if (!Phobos::Config::SelectedDisplay_Enable)
		return;

	Phobos::Config::SelectedDisplay_Expand = !Phobos::Config::SelectedDisplay_Expand;
	VocClass::PlayGlobal(RulesClass::Instance->GUIMainButtonSound, Panning::Center, 1.0);
	this->UpdateVisible();
	this->ShouldUpdate = true;
}

void SelectedInfoClass::SwitchVisible()
{
	Phobos::Config::SelectedDisplay_Enable = !Phobos::Config::SelectedDisplay_Enable;
	VocClass::PlayGlobal(RulesClass::Instance->GUIMainButtonSound, Panning::Center, 1.0);
	this->UpdateVisible();

	if (Phobos::Config::SelectedDisplay_Enable)
		this->ShouldUpdate = true;
}

void SelectedInfoClass::UpdateVisible()
{
	const bool enable = Phobos::Config::SelectedDisplay_Enable;
	auto disabled = !enable || !this->SingleSelect || !this->ObtainSelect;

	if (const auto& pButton = this->MainColumn)
		pButton->Disabled = disabled;

	if (const auto& pButton = this->PushButton)
		pButton->Disabled = disabled;

	if (const auto& pButton = this->AmmoButton)
		pButton->Disabled = disabled;

	if (const auto& pButton = this->MainCameo)
		pButton->Disabled = disabled;

	if (const auto& pButton = this->InfoIconA)
		pButton->Disabled = disabled;

	if (const auto& pButton = this->InfoIconD)
		pButton->Disabled = disabled;

	if (const auto& pButton = this->InfoIconS)
		pButton->Disabled = disabled;

	disabled = !enable || this->SingleSelect;
	int size = this->CurrentSelectCameo.size();
	int cameoCount = 0;

	if (size == 1 || Phobos::Config::SelectedDisplay_Expand)
		size = this->CurrentSelectTechno.size();

	for (int i = 0; i < this->GetMaxCameo(); ++i)
	{
		if (const auto& pButton = this->Cameos[i])
			pButton->Disabled = disabled || (i >= size);
	}

	const int overflow = size - this->GetMaxCameo();

	if (overflow > 0)
	{
		if (this->Current > overflow)
			this->Current = overflow;

		cameoCount = this->GetMaxCameo();
	}
	else
	{
		this->Current = 0;
		cameoCount = size;
	}

	if (const auto& pButton = this->MainBottom)
		pButton->Rect.Width = std::max((enable ? (this->SingleSelect ? 253 : 289) : 17), cameoCount * 60);

	if (const auto& pButton = this->ToggleV)
		pButton->Rect.X = enable ? 238 : 2;

	if (const auto& pButton = this->ToggleE)
		pButton->Disabled = disabled;

	if (const auto& pButton = this->ScrollL)
		pButton->Disabled = disabled;

	if (const auto& pButton = this->ScrollR)
		pButton->Disabled = disabled;
}

void SelectedInfoClass::UpdateSelected()
{
	this->ShouldUpdate = false;

	if (this->CurrentSelectCameo.size())
		this->CurrentSelectCameo.clear();

	if (this->CurrentSelectTechno.size())
		this->CurrentSelectTechno.clear();

	{
		std::map<int, SelectRecordStruct> CurrentSelectBuffer;

		for (const auto& pCurrent : ObjectClass::CurrentObjects())
		{
			if (const auto pType = pCurrent->GetTechnoType())
			{
				const auto count = CurrentSelectBuffer.contains(pType->UniqueID) ? CurrentSelectBuffer.at(pType->UniqueID).Count : 0;
				auto& _add = CurrentSelectBuffer[pType->UniqueID];
				_add.TypeExt = TechnoTypeExtContainer::Instance.Find(pType);
				_add.Count = count + 1;
				this->CurrentSelectTechno.emplace_back((TechnoClass*)pCurrent);
			}
		}

		if (CurrentSelectBuffer.size())
		{
			for (const auto& [ID, Record] : CurrentSelectBuffer)
				this->UpdateRecordCameo(Record);
		}
	}

	std::sort(this->CurrentSelectTechno.begin(), this->CurrentSelectTechno.end(),
		[](const TechnoClass* const pSelectA, const TechnoClass* const pSelectB)
		{
			const auto uniqueA = pSelectA->UniqueID;
			const auto uniqueB = pSelectB->UniqueID;
			if (uniqueA < uniqueB) return true;
			if (uniqueA > uniqueB) return false;
			return pSelectA->UniqueID < pSelectB->UniqueID;
		});

	const auto size = this->CurrentSelectTechno.size();
	this->SingleSelect = size <= 1;
	this->ObtainSelect = size > 0;
	this->UpdateVisible();
}

void SelectedInfoClass::UpdateRecordCameo(const SelectRecordStruct& Record)
{
	const auto groupID = Record.TypeExt->GetSelectionGroupID();
	const int currentCounts = this->CurrentSelectCameo.size();

	for (int i = 0; i < currentCounts; ++i)
	{
		if (this->CurrentSelectCameo[i].TypeExt->GetSelectionGroupID() == groupID)
		{
			this->CurrentSelectCameo[i].Count += Record.Count;
			return;
		}
	}

	this->CurrentSelectCameo.push_back(Record);
}

void SelectedInfoClass::DrawInfo()
{
	const bool drawAll = Phobos::Config::SelectedDisplay_Enable;

	if (drawAll)
	{
		if (this->ShouldUpdate)
			this->UpdateSelected();

		if (this->ObtainSelect)
		{
			if (this->SingleSelect)
			{
				if (const auto& pButton = this->MainColumn)
					pButton->DrawInfo();

				if (const auto& pButton = this->PushButton)
					pButton->DrawInfo();

				if (const auto& pButton = this->AmmoButton)
					pButton->DrawInfo();

				if (const auto& pButton = this->InfoIconA)
					pButton->DrawInfo();

				if (const auto& pButton = this->InfoIconD)
					pButton->DrawInfo();

				if (const auto& pButton = this->InfoIconS)
					pButton->DrawInfo();
			}
			else
			{
				for (int i = 0; i < this->GetMaxCameo(); ++i)
				{
					if (const auto& pButton = this->Cameos[i])
						pButton->DrawInfo();
				}
			}
		}
	}

	if (const auto& pButton = this->MainBottom)
		pButton->DrawInfo();

	if (const auto& pButton = this->ToggleV)
		pButton->DrawInfo();

	if (drawAll && !this->SingleSelect)
	{
		if (const auto& pButton = this->ToggleE)
			pButton->DrawInfo();

		if (const auto& pButton = this->ScrollL)
			pButton->DrawInfo();

		if (const auto& pButton = this->ScrollR)
			pButton->DrawInfo();
	}
}

BSurface* SelectedInfoClass::SearchMissingCameo(AbstractType absType, SHPCaches* pSHP)
{
	const auto pRulesExt = FakeRulesClass::Instance();
	char pFilename[0x20];
	strcpy_s(pFilename, pRulesExt->MissingCameo.data());
	_strlwr_s(pFilename);

	if (!_stricmp(pSHP->Filename, GameStrings::XXICON_SHP))
	{
		if (absType == AbstractType::InfantryType)
		{
			if (const auto MissingCameoPCX = pRulesExt->SelectedInfantryMissingPCX.GetSurface())
				return MissingCameoPCX;
		}
		else if (absType == AbstractType::UnitType)
		{
			if (const auto MissingCameoPCX = pRulesExt->SelectedVehicleMissingPCX.GetSurface())
				return MissingCameoPCX;
		}
		else if (absType == AbstractType::AircraftType)
		{
			if (const auto MissingCameoPCX = pRulesExt->SelectedAircraftMissingPCX.GetSurface())
				return MissingCameoPCX;
		}
		else if (absType == AbstractType::BuildingType)
		{
			if (const auto MissingCameoPCX = pRulesExt->SelectedBuildingMissingPCX.GetSurface())
				return MissingCameoPCX;
		}

		if (strstr(pFilename, ".pcx"))
		{
			if(!SelectedInfoClass::MissingCameo){
				if(PCXImages::Instance->LoadFile(pFilename)) {
					SelectedInfoClass::MissingCameo = PCXImages::Instance->GetSurface(pFilename);

					if (SelectedInfoClass::MissingCameo)
						return SelectedInfoClass::MissingCameo;
				}
			} else {
				return SelectedInfoClass::MissingCameo;
			}
		}
	}

	return nullptr;
}

void SelectedInfoClass::GetValuesForDisplay(TechnoClass* pThis, ObjectTypeClass* pFakeType, DisplayInfoType infoType, int& value, int& maxValue, int infoIndex)
{
	const auto pTrueType = pThis->GetTechnoType();
	ShieldClass* pShield = TechnoExtContainer::Instance.Find(pThis)->ShieldEntity.get();

	if (pTrueType == pFakeType)
		TechnoExtData::GetValuesForDisplay(pThis, infoType, value, maxValue, infoIndex, pShield);
	else
	{
		int fakeValue = 0;

		if (infoType == DisplayInfoType::Health) {
			value = pThis->Health;
			maxValue = pTrueType->Strength;
			fakeValue = pFakeType->Strength;
		} else if (auto pType = type_cast<TechnoTypeClass*>(pFakeType)) {
			TechnoExtData::GetValuesForDisplay(pThis, pType, infoType, value, maxValue, infoIndex, pShield);
			fakeValue = maxValue;
		}

		if (fakeValue <= 0) {
			maxValue = fakeValue;
		} else if (value >= 0 && maxValue > 0 && fakeValue != maxValue) {
			value = (value * fakeValue / maxValue);
			maxValue = fakeValue;
		}
	}
}

TechnoStatus SelectedInfoClass::GetCurrentStatus(TechnoClass* pThis)
{
	const auto pOwner = pThis->Owner;
	ObjectTypeClass* pDisguise = nullptr;

	if ((!pOwner || !pOwner->IsAlliedWith(HouseClass::CurrentPlayer())) && !HouseClass::IsCurrentPlayerObserver())
	{
		pDisguise = pThis->Disguise;

		if (!flag_cast_to<TechnoClass*>(pDisguise))
			return TechnoStatus::Unknown;
	}

	const auto mission = pThis->CurrentMission;

	if (pThis->IsUnderEMP() || pThis->Deactivated)
		return TechnoStatus::Deactive;

	if (pDisguise)
	{
		if (const auto pFoot = flag_cast_to<FootClass*, true>(pThis))
		{
			if (pFoot->LocomotorSource)
				return TechnoStatus::Locomotor;

			if (pFoot->Locomotor->Is_Moving_Now())
				return TechnoStatus::Move;
		}

		return TechnoStatus::Guard;
	}

	if (const auto pBuilding = cast_to<BuildingClass*, true>(pThis))
	{
		if (!pOwner->IsControlledByHuman())
		{
			if (const auto pFactory = pBuilding->Factory)
			{
				if (const auto pProduct = pFactory->Object)
					return TechnoStatus::Produce;
			}
		}
		else if (pBuilding->IsPrimaryFactory)
		{
			const auto pBuildingType = pBuilding->Type;
			const auto factoryType = pBuildingType->Factory;

			if (const auto pFactory = pOwner->GetPrimaryFactory(factoryType, pBuildingType->Naval, BuildCat::DontCare))
			{
				if (const auto pProduct = pFactory->Object)
					return TechnoStatus::Produce;
			}

			if (factoryType == AbstractType::BuildingType)
			{
				if (const auto pFactory = pOwner->Primary_ForDefenses)
				{
					if (const auto pProduct = pFactory->Object)
						return TechnoStatus::Produce;
				}
			}
		}
	}
	else if (const auto pFoot = flag_cast_to<FootClass*, true>(pThis))
	{
		if (pFoot->LocomotorSource)
			return TechnoStatus::Locomotor;
		else if (pFoot->MegaMission == Mission::AttackMove)
			return TechnoStatus::AttackMove;
		else if (mission == Mission::Area_Guard && flag_cast_to<FootClass*>(pFoot->ArchiveTarget))
			return TechnoStatus::FollowGuard;
	}

	switch (mission)
	{
	case Mission::ParadropApproach:
		return TechnoStatus::Paradrop;
	case Mission::ParadropOverfly:
	case Mission::SpyplaneApproach:
	case Mission::SpyplaneOverfly:
		return TechnoStatus::Move;
	default:
		break;
	}

	const auto status = static_cast<TechnoStatus>(mission);

	return (status > TechnoStatus::None || status < TechnoStatus::Sleep) ? TechnoStatus::Unknown : status;
}

int SelectedInfoClass::GetMaxCameo() const
{
	return this->MaxCameo;
}

bool SelectedInfoClass::CanScrollLeft() const
{
	if (this->SingleSelect)
		return false;

	return this->Current > 0;
}
 
bool SelectedInfoClass::CanScrollRight() const
{
	if (this->SingleSelect)
		return false;

	int size = this->CurrentSelectCameo.size();

	if (size == 1 || Phobos::Config::SelectedDisplay_Expand)
		size = this->CurrentSelectTechno.size();

	const int overflow = size - this->GetMaxCameo();

	return this->Current < overflow;
}

bool SelectedInfoClass::ScrollLeft()
{
	if (this->CanScrollLeft())
	{
		--this->Current;
		return true;
	}

	return false;
}

bool SelectedInfoClass::ScrollRight()
{
	if (this->CanScrollRight())
	{
		++this->Current;
		return true;
	}

	return false;
}
