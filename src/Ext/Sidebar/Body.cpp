#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Rules/Body.h>

#include <CCToolTip.h>
#include <EventClass.h>

IStream* SidebarExtData::g_pStm = nullptr;
std::array<SHPReference*, 4u> SidebarExtData::TabProducingProgress {};
std::unique_ptr<SidebarExtData> SidebarExtData::Data = nullptr;

HelperedVector<TacticalButtonClass> TacticalButtonClass::Buttons {};
bool TacticalButtonClass::Initialized { false };
TacticalButtonClass* TacticalButtonClass::CurrentButton { nullptr };

void SidebarExtData::Allocate(SidebarClass* pThis)
{
	Data = std::make_unique<SidebarExtData>();
	Data->AttachedToObject = pThis;
}

void SidebarExtData::Remove(SidebarClass* pThis)
{
	Data = nullptr;
}

void SidebarExtData::DrawProducingProgress()
{
	const auto pPlayer = HouseClass::CurrentPlayer();

	if (HouseExtData::IsObserverPlayer(pPlayer))
		return;

	if (Phobos::UI::ShowProducingProgress) {
		const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[pPlayer->SideIndex]);

		if (!pSideExt)
			return;

		const int XOffset = pSideExt->Sidebar_GDIPositions ? 29 : 32;
		const int XBase = (pSideExt->Sidebar_GDIPositions ? 26 : 20) + pSideExt->Sidebar_ProducingProgress_Offset.Get().X;
		const int YBase = 197 + pSideExt->Sidebar_ProducingProgress_Offset.Get().Y;

		for (int i = 0; i < (int)SidebarExtData::TabProducingProgress.size(); i++) {
			if (auto pSHP = SidebarExtData::TabProducingProgress[i]) {

				const auto rtti = i == 0 || i == 1 ? AbstractType::BuildingType : AbstractType::InfantryType;
				FactoryClass* pFactory = nullptr;

				if (i != 3) {
					pFactory = pPlayer->GetPrimaryFactory(rtti, false, i == 1 ? BuildCat::Combat : BuildCat::DontCare);
				} else {
					pFactory = pPlayer->GetPrimaryFactory(AbstractType::UnitType, false, BuildCat::DontCare);
					if (!pFactory || !pFactory->Object)
						pFactory = pPlayer->GetPrimaryFactory(AbstractType::UnitType, true, BuildCat::DontCare);
					if (!pFactory || !pFactory->Object)
						pFactory = pPlayer->GetPrimaryFactory(AbstractType::AircraftType, false, BuildCat::DontCare);
				}

				if(pFactory) {

					const int idxFrame = (int)(((double)pFactory->GetProgress() / 54) * (pSHP->Frames - 1)) ;
					Point2D vPos = { XBase + i * XOffset, YBase };
					RectangleStruct sidebarRect = DSurface::Sidebar()->Get_Rect();

					if (idxFrame != -1)
					{
						DSurface::Sidebar()->DrawSHP(FileSystem::SIDEBAR_PAL, pSHP, idxFrame, &vPos,
							&sidebarRect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
					}
				}
			}
		}
	}
}

TacticalButtonClass::TacticalButtonClass(unsigned int id, int superIdx, int x, int y, int width, int height)
	: ToggleClass(id, x, y, width, height)
	, SuperIndex(superIdx)
{
	this->Zap();
	GScreenClass::Instance->AddButton(this);
}

TacticalButtonClass::~TacticalButtonClass()
{
	if (TacticalButtonClass::CurrentButton == this)
		TacticalButtonClass::CurrentButton = nullptr;

	GScreenClass::Instance->RemoveButton(this);
}

bool TacticalButtonClass::Draw(bool forced)
{
	/*if (!this->ControlClass::Draw(forced))
		return false;*/

	auto pSurface = DSurface::Composite();
	auto bounds = pSurface->Get_Rect();
	Point2D location = { this->Rect.X, this->Rect.Y };
	RectangleStruct destRect = { location.X, location.Y, this->Rect.Width, this->Rect.Height };

	const auto pCurrent = HouseClass::CurrentPlayer();
	const auto pSuper = pCurrent->Supers[this->SuperIndex];
	const auto pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type);

	// support for pcx cameos
	if (const auto pPCXCameo = pSWExt->SidebarPCX.GetSurface())
	{
		PCX::Instance->BlitToSurface(&destRect, pSurface, pPCXCameo);
	}
	else if (const auto pCameo = pSuper->Type->SidebarImage) // old shp cameos, fixed palette
	{
		const auto pCameoRef = pCameo->AsReference();
		char pFilename[0x20];
		strcpy_s(pFilename, RulesExtData::Instance()->MissingCameo.data());
		_strlwr_s(pFilename);

		if (!_stricmp(pCameoRef->Filename, GameStrings::XXICON_SHP) && strstr(pFilename, ".pcx"))
		{
			PCX::Instance->LoadFile(pFilename);

			if (const auto CameoPCX = PCX::Instance->GetSurface(pFilename))
				PCX::Instance->BlitToSurface(&destRect, pSurface, CameoPCX);
		}
		else
		{
			const auto pConvert = pSWExt->SidebarPalette ? pSWExt->SidebarPalette->GetConvert<PaletteManager::Mode::Default>() : FileSystem::CAMEO_PAL;
			pSurface->DrawSHP(pConvert, pCameo, 0, &location, &bounds, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		}
	}

	if (pSuper->IsCharged && !pCurrent->CanTransactMoney(pSWExt->Money_Amount) || (pSWExt->SW_UseAITargeting && !SWTypeExtData::IsTargetConstraintsEligible(pSuper, true)))
	{
		RectangleStruct darkenBounds { 0, 0, location.X + this->Rect.Width, location.Y + this->Rect.Height };
		pSurface->DrawSHP(FileSystem::SIDEBAR_PAL, FileSystem::DARKEN_SHP, 0, &location, &darkenBounds, BlitterFlags::bf_400 | BlitterFlags::Darken, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}

	if (this->IsHovering)
	{
		RectangleStruct cameoRect = { location.X, location.Y, this->Rect.Width, this->Rect.Height };
		const COLORREF tooltipColor = Drawing::RGB_To_Int(Drawing::TooltipColor());
		pSurface->Draw_Rect(cameoRect, tooltipColor);
	}

	if (!pSuper->RechargeTimer.Completed())
	{
		Point2D loc = { location.X, location.Y };
		pSurface->DrawSHP(FileSystem::SIDEBAR_PAL, FileSystem::GCLOCK2_SHP, pSuper->GetCameoChargeState() + 1, &loc, &bounds, BlitterFlags::bf_400 | BlitterFlags::TransLucent50, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}

	const auto buffer = pSuper->NameReadiness();;

	if (buffer && *buffer)
	{
		Point2D textLoc = { location.X + this->Rect.Width / 2, location.Y };
		const COLORREF foreColor = Drawing::RGB_To_Int(Drawing::TooltipColor);
		TextPrintType printType = TextPrintType::FullShadow | TextPrintType::Point8 | TextPrintType::Background | TextPrintType::Center;

		pSurface->DSurfaceDrawText(buffer, &bounds, &textLoc, foreColor, 0, printType);
	}

	return true;
}

void TacticalButtonClass::OnMouseEnter()
{
	this->IsHovering = true;
	TacticalButtonClass::CurrentButton = this;
}

void TacticalButtonClass::OnMouseLeave()
{
	this->IsHovering = false;
	this->IsPressed = false;
	TacticalButtonClass::CurrentButton = nullptr;
	CCToolTip::Instance->MarkToRedraw(CCToolTip::Instance->CurrentToolTipData);
}

bool TacticalButtonClass::Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier)
{
	if (!this->ControlClass::Action(flags, pKey, modifier))
		return false;

	if ((int)flags & (int)GadgetFlag::LeftPress)
		this->IsPressed = true;

	if ((int)flags & (int)GadgetFlag::LeftRelease && this->IsPressed)
	{
		this->IsPressed = false;
		return this->LaunchSuper(this->SuperIndex);
	}

	return true;
}

bool TacticalButtonClass::LaunchSuper(int superIdx)
{
	const auto pCurrent = HouseClass::CurrentPlayer();
	const auto pSuper = pCurrent->Supers[superIdx];
	const auto pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
	VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0);
	const bool manual = !pSWExt->SW_ManualFire && pSWExt->SW_AutoFire;
	const bool unstopable = pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining && pSWExt->SW_Unstoppable;

	if (!pSuper->CanFire() && !manual)
	{
		VoxClass::PlayIndex(pSuper->Type->ImpatientVoice);
		return false;
	}

	if (!pCurrent->CanTransactMoney(pSWExt->Money_Amount))
	{
		VoxClass::PlayIndex(pSWExt->EVA_InsufficientFunds);
		pSWExt->PrintMessage(pSWExt->Message_InsufficientFunds, pCurrent);
	}
	else if (!pSWExt->SW_UseAITargeting || (SWTypeExtData::IsTargetConstraintsEligible(pSuper, true)))
	{
		if (!manual && !unstopable)
		{
			const auto swIndex = pSuper->Type->ArrayIndex;

			if (pSuper->Type->Action == Action::None || pSWExt->SW_UseAITargeting)
			{
				EventClass Event { pCurrent->ArrayIndex, EventType::SPECIAL_PLACE, swIndex, CellStruct::Empty };
				EventClass::AddEvent(&Event);
			}
			else
			{
				DisplayClass::Instance->CurrentBuilding = nullptr;
				DisplayClass::Instance->CurrentBuildingType = nullptr;
				DisplayClass::Instance->unknown_11AC = static_cast<DWORD>(-1);
				DisplayClass::Instance->SetActiveFoundation(nullptr);
				MapClass::Instance->SetRepairMode(0);
				DisplayClass::Instance->SetSellMode(0);
				DisplayClass::Instance->PowerToggleMode = false;
				DisplayClass::Instance->PlanningMode = false;
				DisplayClass::Instance->PlaceBeaconMode = false;
				DisplayClass::Instance->CurrentSWTypeIndex = swIndex;
				MapClass::Instance->UnselectAll();
				VoxClass::PlayIndex(pSWExt->EVA_SelectTarget);
			}

			return true;
		}
	}
	else
	{
		pSWExt->PrintMessage(pSWExt->Message_CannotFire, pCurrent);
	}

	return false;
}

bool TacticalButtonClass::AddButton(int superIdx)
{
	TacticalButtonClass::Initialized = true;

	if (!Phobos::UI::ExclusiveSuperWeaponSidebar || Unsorted::ArmageddonMode)
		return false;

	auto& buttons = TacticalButtonClass::Buttons;

	if(buttons.any_of([superIdx](const TacticalButtonClass& button) { return button.SuperIndex == superIdx; }))
		return false;

	buttons.emplace_back(superIdx + 2200, superIdx, 0, 0, 60, 48);
	SortButtons();
	return true;
}

bool TacticalButtonClass::RemoveButton(int superIdx)
{
	auto& buttons = TacticalButtonClass::Buttons;

	buttons.remove_if([superIdx](const TacticalButtonClass& button) { return button.SuperIndex == superIdx; });
	SortButtons();
	return true;
}

void TacticalButtonClass::SortButtons()
{
	auto& buttons = TacticalButtonClass::Buttons;

	if (!buttons.empty()) {

		if (TacticalButtonClass::CurrentButton)
			TacticalButtonClass::CurrentButton->OnMouseLeave();

		std::stable_sort(buttons.begin(), buttons.end(), [](const TacticalButtonClass&  a, const TacticalButtonClass&  b) {
			return BuildType::SortsBefore(AbstractType::Special, a.SuperIndex, AbstractType::Special, b.SuperIndex);
		});

		const int buttonCount = static_cast<int>(buttons.size());
		const int cameoWidth = 60, cameoHeight = 48;
		const int maximum = Phobos::UI::ExclusiveSuperWeaponSidebar_Max;
		Point2D location = { 0, (DSurface::ViewBounds().Height - std::min(buttonCount, maximum) * cameoHeight) / 2 };
		int location_Y = location.Y;
		int row = 0, line = 0;

		for (int idx = 0; idx < buttonCount && maximum - line > 0; idx++)
		{
			const auto button = &buttons[idx];
			button->SetPosition(location.X, location.Y);
			row++;

			if (row >= maximum - line)
			{
				row = 0;
				line++;
				location_Y += cameoHeight / 2;
				location = { location.X + cameoWidth, location_Y };
			}
			else
			{
				location.Y += cameoHeight;
			}
		}
	}
}

// =============================
// load / save

template <typename T>
void SidebarExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		;
}

// =============================
// container hooks

DEFINE_HOOK(0x6A4F0B, SidebarClass_CTOR, 0x5)
{
	GET(SidebarClass*, pItem, EAX);

	SidebarExtData::Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x6AC82F, SidebarClass_DTOR, 0x5)
{
	GET(SidebarClass*, pItem, EBX);

	SidebarExtData::Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6AC5D0, SidebarClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6AC5E0, SidebarClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(IStream*, pStm, 0x4);

	SidebarExtData::g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(0x6AC5DA, SidebarClass_Load_Suffix, 0x6)
{
	auto buffer = SidebarExtData::Instance();

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(SidebarExtData::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(SidebarExtData::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x6AC5EA, SidebarClass_Save_Suffix, 0x6)
{
	auto buffer = SidebarExtData::Instance();
	// negative 4 for the AttachedToObjectPointer , it doesnot get S/L
	PhobosByteStream saver(sizeof(SidebarExtData) - 4u);
	PhobosStreamWriter writer(saver);

	writer.Save(SidebarExtData::Canary);
	writer.Save(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(SidebarExtData::g_pStm);

	return 0;
}
