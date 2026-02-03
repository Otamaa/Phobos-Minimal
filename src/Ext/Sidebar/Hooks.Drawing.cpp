#include "Body.h"

#include <Misc/Ares/Hooks/Header.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Side/Body.h>
#include <Misc/PhobosToolTip.h>
#include <Misc/Ares/CSF.h>
#include <Utilities/Cast.h>
#include <Ext/Techno/Body.h>

#include <TechnoTypeClass.h>
#include <TextDrawing.h>
#include <CCToolTip.h>
#include <EventClass.h>

static COMPILETIMEEVAL int ObserverBackgroundWidth = 121;
static COMPILETIMEEVAL int ObserverBackgroundHeight = 96;

static COMPILETIMEEVAL int ObserverFlagPCXX = 70;
static COMPILETIMEEVAL int ObserverFlagPCXY = 70;
static COMPILETIMEEVAL int ObserverFlagPCXWidth = 45;
static COMPILETIMEEVAL int ObserverFlagPCXHeight = 21;

class FakeStripClass : public StripClass
{
public:
	void __Draw_It(bool forceRedraw);
	const wchar_t* __Help_Text(int index);
};


class FakeSelectClass : public SelectClass
{
public:

	int __Action(GadgetFlag flags,
		DWORD* key,
		KeyModifier a4);
};

// ASMJIT_PATCH(0x6ab773, SelectClass_ProcessInput_ProduceUnsuspended, 0xA)
// {
// 	GET(EventClass*, pEvent, EAX);
// 	GET_STACK(DWORD, flag, 0xB8);
//
// 	for (int i = ((4 * (flag & 1)) | 1); i > 0; --i) {
// 		EventClass::AddEvent(pEvent);
// 	}
//
// 	return 0x6AB7CC;
// }

// ASMJIT_PATCH(0x6AB689, SelectClass_Action_SkipBuildingProductionCheck, 0x5)
// {
// 	enum { SkipGameCode = 0x6AB6CE };
// 	return RulesExtData::Instance()->ExpandBuildingQueue ? SkipGameCode : 0;
// }

#define EPILOGUE() \
   this->ControlClass::Action(flags, key, KeyModifier::None); \
    return 1

int FakeSelectClass::__Action(GadgetFlag flags,
	DWORD* key,
	KeyModifier a4)
{

	if (!this->Strip) {
		return 1;
	}

	StripClass* strip = this->Strip;
	const int buildableIdx = this->Index + 2 * strip->TopRowIndex;
	auto& cameos = MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex];
	HouseClass* PlayerPtr = HouseClass::CurrentPlayer();
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
	KeyModifier shiftPressed = a4 & KeyModifier::Shift;
	BuildType* pBuild = &cameos[buildableIdx];

	if((size_t)buildableIdx >= cameos.size() || (size_t)buildableIdx >= (size_t)strip->BuildableCount) {
		flags = GadgetFlag::None;
		EPILOGUE();
	}

	if (buildableIdx < (int)cameos.size() && buildableIdx < strip->BuildableCount) {
	
		if (pBuild->ItemIndex >= 0) {
			auto pCurrentFactory = pBuild->CurrentFactory;

			if (pBuild->ItemType != AbstractType::Special) {
				if (auto Techno_Type = TechnoTypeClass::FetchTechnoType(pBuild->ItemType, pBuild->ItemIndex)) {

					flags &=  ~GadgetFlag::LeftUp;

					if (flags & GadgetFlag::RightPress) {
						if (pCurrentFactory) {
					
							if (Unsorted::PendingObject) {
								const AbstractType abs = Unsorted::PendingObject->WhatAmI();

								if (abs == AbstractType::Building 
									|| abs == AbstractType::Aircraft 
									|| abs == AbstractType::Unit 
									|| abs == AbstractType::Infantry) {
									Unsorted::PendingObject = nullptr;
									Unsorted::CurrentBuildingType = nullptr;
									Unsorted::unknown_11AC = -1;
									DisplayClass::Instance->SetActiveFoundation(0);
								}
							}

							if (!pCurrentFactory->Production.Timer.Rate || pCurrentFactory->IsSuspended) {
								VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0, 0);
								VoxClass::Play("EVA_Canceled");
								EventType ev = shiftPressed != KeyModifier::None ? EventType::ABANDON_ALL : EventType::ABANDON;

								EventClass Event {
									PlayerPtr->ArrayIndex ,
									ev ,
									pBuild->ItemType ,
									pBuild->ItemIndex,
									bool(Techno_Type->Naval)
								};

								EventClass::AddEvent(&Event);
							} else {
								VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0, 0);
								VoxClass::Play("EVA_OnHold");

								EventClass Event {
									PlayerPtr->ArrayIndex ,
									EventType::SUSPEND ,
									pBuild->ItemType ,
									pBuild->ItemIndex,
									bool(Techno_Type->Naval)
								};

								EventClass::AddEvent(&Event);
							}

							SidebarClass::Column[strip->TabIndex].NeedsRedraw = 1;
						} else {
							if (pBuild->Status == BuildState::Building) {
								pBuild->Status = BuildState::OnHold;
								strip->NeedsRedraw = 1;
								MapClass::Instance->RedrawSidebar(0);
								VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0, 0);
								VoxClass::Play("EVA_OnHold");

								EventClass Event {
									PlayerPtr->ArrayIndex ,
									EventType::SUSPEND ,
									pBuild->ItemType ,
									pBuild->ItemIndex,
									bool(Techno_Type->Naval)
								};

								EventClass::AddEvent(&Event);
							}

							auto pHouseFactory = PlayerPtr->GetPrimaryFactory(pBuild->ItemType, Techno_Type->Naval, pBuild->Cat);

							if (pHouseFactory && pHouseFactory->IsQueued(Techno_Type)) {
								VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0, 0);
								EventClass Event {
									PlayerPtr->ArrayIndex ,
									shiftPressed != KeyModifier::None ? EventType::ABANDON_ALL : EventType::ABANDON ,
									pBuild->ItemType ,
									pBuild->ItemIndex,
									bool(Techno_Type->Naval)
								};

								EventClass::AddEvent(&Event);
							}
						}
					}

					if (flags & GadgetFlag::LeftPress) {
						if (!pCurrentFactory || pCurrentFactory->Production.Timer.Rate && !pCurrentFactory->IsSuspended) {
							Techno_Type = TechnoTypeClass::FetchTechnoType(pBuild->ItemType, pBuild->ItemIndex);
							auto pHouseFactory = PlayerPtr->GetPrimaryFactory(pBuild->ItemType, Techno_Type->Naval, pBuild->Cat);
							bool ShouldDisableCameo = HouseExtData::ShouldDisableCameo(PlayerPtr,Techno_Type ,true);
							bool unable_to_comply = false;
							if (pHouseFactory && (pHouseFactory->Production.Timer.Rate && !pHouseFactory->IsSuspended || pHouseFactory->Object || pHouseFactory->QueuedObjects.Count > 0)) {
								unable_to_comply = 1;
								if (pBuild->ItemType == AbstractType::BuildingType && !RulesExtData::Instance()->ExpandBuildingQueue) {
									VoxClass::Play("EVA_UnableToComply");
									EPILOGUE();
								}

							} else {
								if (!ShouldDisableCameo) {
									VoxClass::Play(pBuild->ItemType == AbstractType::InfantryType ? "EVA_Training" : "EVA_Building");
								}
							}

							bool BusyStatus = PlayerPtr->IsBusy(pBuild->ItemType, Techno_Type->Naval, pBuild->Cat);

							bool v70 = 0;

							if (!ShouldDisableCameo) {
								VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0, 0);
								if (!pHouseFactory && !BusyStatus) {
									PlayerPtr->SetBusy(pBuild->ItemType, Techno_Type->Naval, pBuild->Cat);
									v70 = 1;
								}

								EventClass Event {
									PlayerPtr->ArrayIndex ,
									EventType::PRODUCE ,
									pBuild->ItemType ,
									pBuild->ItemIndex,
									bool(Techno_Type->Naval)
								};

								for (int i = ((4 * ((DWORD)shiftPressed)) | 1); i > 0; --i) {
									EventClass::AddEvent(&Event);
								}

								if (v70) {
									if (!unable_to_comply && !ShouldDisableCameo) {
										pBuild->Status = BuildState::Building;
										strip->NeedsRedraw = true;
										Techno_Type = TechnoTypeClass::FetchTechnoType(pBuild->ItemType, pBuild->ItemIndex);								

										if (Techno_Type) {
											const double divisor = 53.822631;
											const int maxAhead = Game::Network::MaxAhead();
											const int cost = Techno_Type->GetActualCost(PlayerPtr);
											const double raw = double(cost) * double(maxAhead) / divisor;
											const int corrected = (int)(raw >= 0 ? std::floor(raw) : std::ceil(raw));

											if (PlayerPtr->Available_Money() >= corrected) {
												if (Techno_Type->FindFactory(true, false, false, PlayerPtr)) {
													int time = Techno_Type->GetBuildSpeed();
													
													bool IsAWall = false;

													if (pBuild->ItemType == AbstractType::BuildingType) {
														auto pBuildingProduct = static_cast<BuildingTypeClass*>(Techno_Type);
														const auto pBuildingProductExt = TechnoTypeExtContainer::Instance.Find(pBuildingProduct);

														IsAWall = pBuildingProduct->Wall;

														if (IsAWall && pBuildingProductExt->BuildTime_Speed.isset())
															IsAWall = false;
													}

													if(IsAWall) {
														time = int(time * RulesClass::Instance->WallBuildSpeedCoefficient);
													}

													if (time <= maxAhead + 15) {
														time = maxAhead + 15;
													}

													int v52 = time / 54;
													if (54 * (v52) < maxAhead + 15) {
														++v52;
													}

													if (v52 >= 1) {
														if (v52 > 255) {
															v52 = 255;
														}
													} else {
														v52 = 1;
													}

													pBuild->Progress.Start(v52, 1 , 0);
													EPILOGUE();
												}
											}
										}
									}
								}
							}

							if (!BusyStatus) {
								if (!unable_to_comply && !ShouldDisableCameo) {
									pBuild->Status = BuildState::Building;
									strip->NeedsRedraw = true;
									Techno_Type = TechnoTypeClass::FetchTechnoType(pBuild->ItemType, pBuild->ItemIndex);

									if (Techno_Type) {
										const double divisor = 53.822631;
										const int maxAhead = Game::Network::MaxAhead();
										const int cost = Techno_Type->GetActualCost(PlayerPtr);
										const double raw = double(cost) * double(maxAhead) / divisor;
										const int corrected = (int)(raw >= 0 ? std::floor(raw) : std::ceil(raw));

										if (PlayerPtr->Available_Money() >= corrected) {
											if (Techno_Type->FindFactory(true, false, false, PlayerPtr)) {
												int time = Techno_Type->GetBuildSpeed();
												if (pBuild->ItemType == AbstractType::BuildingType && static_cast<BuildingTypeClass*>(Techno_Type)->Wall) {
													time = int(time * RulesClass::Instance->WallBuildSpeedCoefficient);
												}

												if (time <= maxAhead + 15) {
													time = maxAhead + 15;
												}

												int rate = time / 54;
												if (54 * (rate) < maxAhead + 15) {
													++rate;
												}

												if (rate >= 1) {
													if (rate > 255) {
														rate = 255;
													}
												} else {
													rate = 1;
												}

												pBuild->Progress.Start(rate, 1 , 0);
												EPILOGUE();
											} else {
												EPILOGUE();
											}
										} else {
											EPILOGUE();
										}
									} else {
										EPILOGUE();
									}
								} else {
									EPILOGUE();
								}
							} else {
								EPILOGUE();
							}
						}

						if (!pCurrentFactory->IsDone()) {
							VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0, 0);
							VoxClass::Play(pBuild->ItemType == AbstractType::InfantryType ? "EVA_Training" : "EVA_Building");
							Techno_Type = TechnoTypeClass::FetchTechnoType(pBuild->ItemType, pBuild->ItemIndex);
							PlayerPtr->SetBusy(pBuild->ItemType, Techno_Type->Naval, pBuild->Cat);
							VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0, 0);
							EventClass Event {
								PlayerPtr->ArrayIndex ,
								EventType::PRODUCE ,
								pBuild->ItemType ,
								pBuild->ItemIndex,
								bool(Techno_Type->Naval)
							};

							EventClass::AddEvent(&Event);

							pBuild->Status = BuildState::Building;

							auto Progress = (pBuild->CurrentFactory)
								? pBuild->CurrentFactory->GetProgress()
								: 0
								;

							if (pBuild->Status == BuildState::Building) {
								if (pBuild->Progress.Stage > Progress) {
									Progress = (pBuild->Progress.Stage + Progress) / 2;
								}
							}

							pBuild->Progress.Start(pCurrentFactory->GetBuildTimeFrames() + 1 , 1 , Progress);
							strip->NeedsRedraw = 1;
							MapClass::Instance->RedrawSidebar(0);
							CCToolTip::Bound = 1;
						} else {
							if(auto Object = pCurrentFactory->GetFactoryObject()) {
								VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0, 0);
								const auto nBuffer = HouseExtData::HasFactory(
													Object->GetOwningHouse(), 
									GET_TECHNOTYPE(Object),
									false,
									true, 
									false,
									true
								);
								
								if (nBuffer.first == NewFactoryState::Unpowered) {
									this->ControlClass::Action(flags, key, KeyModifier::None);
									return 1;
								}

								if(auto v23 = nBuffer.second) {
									if (auto pBld = cast_to<BuildingClass*, false>(Object)) {
										PlayerPtr->Manual_Place(v23, pBld);
									} else {
										VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0, 0);
										EventClass Event {
											Object->GetOwningHouseIndex() ,
											EventType::PLACE ,
											pBuild->ItemType ,
											-1,
											bool(Techno_Type->Naval),
											CellStruct::Empty
										};

										EventClass::AddEvent(&Event);
									}
								} else {
									EventClass Event {
										Object->GetOwningHouseIndex() ,
										EventType::ABANDON ,
										pBuild->ItemType ,
										pBuild->ItemIndex,
										bool(Techno_Type->Naval)
									};

									EventClass::AddEvent(&Event);

									VoxClass::Play("EVA_UnableToComply");
								} 
							} else {
								if (pCurrentFactory->SpecialItem != -1) {
									Unsorted::CurrentSWType = 1;
								}
							} 
						}
					}

				} else {
					flags = GadgetFlag::None;
				}

			} else {
				if (flags & GadgetFlag::LeftUp) {
					flags &=  ~GadgetFlag::LeftUp;
				}

				if (flags & GadgetFlag::RightPress) {
					Unsorted::CurrentSWType = -1;
				}

				if (flags & GadgetFlag::LeftPress || pBuild->ItemIndex < PlayerPtr->Supers.Count) {
					VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0, 0);
					SWTypeExtData::LauchSuper(PlayerPtr->Supers.Items[pBuild->ItemIndex]);
				}
			}
		} else {
			flags = GadgetFlag::None;
		}
	} else {
		flags = GadgetFlag::None;
	}

	this->ControlClass::Action(flags, key, KeyModifier::None);
	return 1;
}

#ifndef _backport
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F3048, FakeSelectClass::__Action)
#else 
ASMJIT_PATCH(0x6AAD2F, SelectClass_ProcessInput_LoadCameo1, 7)
{
	GET(int, CameoIndex, ESI);

	auto& cameos = MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex];

	if ((size_t)CameoIndex >= cameos.size())
	{
		return 0x6AB94F;
	}

	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);

	R->Stack(0x2C, CameoIndex);

	auto& Item = cameos[CameoIndex];
	R->Stack(0x14, Item.ItemIndex);
	R->Stack(0x18, Item.CurrentFactory);
	R->Stack(0x24, Item.Cat);
	R->EBP(Item.ItemType);

	auto ptr = reinterpret_cast<byte*>(&Item);
	ptr -= 0x58;
	R->EBX<byte*>(ptr);

	return 0x6AAD66;
}

ASMJIT_PATCH(0x6AB0B0, SelectClass_ProcessInput_LoadCameo2, 8)
{
	GET(int, CameoIndex, ESI);
	DWORD dmm = (DWORD)MouseClassExt::TabCameos
		[MouseClass::Instance->ActiveTabIndex]
		[CameoIndex].Status;

	R->EAX<DWORD*>(&dmm);

	return 0x6AB0BE;
}

ASMJIT_PATCH(0x6AB49D, SelectClass_ProcessInput_FixOffset1, 7)
{
	R->EDI<void*>(nullptr);
	R->ECX<void*>(nullptr);
	return 0x6AB4A4;
}

ASMJIT_PATCH(0x6AB4E8, SelectClass_ProcessInput_FixOffset2, 7)
{
	R->ECX<int>(R->Stack<int>(0x14));
	R->EDX<void*>(nullptr);
	return 0x6AB4EF;
}

ASMJIT_PATCH(0x6AB577, SelectClass_ProcessInput_FixOffset3, 7)
{
	GET(int, CameoIndex, ESI);
	GET_STACK(FactoryClass*, SavedFactory, 0x18);

	auto& Item = MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][CameoIndex];

	Item.Status = BuildState::Building;

	auto Progress = (Item.CurrentFactory)
		? Item.CurrentFactory->GetProgress()
		: 0
		;

	R->EAX<int>(Progress);
	R->EBP<void*>(nullptr);

	if (Item.Status == BuildState::Building)
	{
		if (Item.Progress.Stage > Progress)
		{
			Progress = (Item.Progress.Stage + Progress) / 2;
		}
	}

	Item.Progress.Stage = Progress;
	R->EAX<int>(SavedFactory->GetBuildTimeFrames());
	R->ECX<void*>(nullptr);

	return 0x6AB5C6;
}

ASMJIT_PATCH(0x6AB620, SelectClass_ProcessInput_FixOffset4, 7)
{
	R->ECX<void*>(nullptr);
	return 0x6AB627;
}

ASMJIT_PATCH(0x6AB741, SelectClass_ProcessInput_FixOffset5, 7)
{
	R->EDX<void*>(nullptr);
	return 0x6AB748;
}

ASMJIT_PATCH(0x6AB802, SelectClass_ProcessInput_FixOffset6, 8)
{
	GET(int, CameoIndex, EAX);
	MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][CameoIndex].Status = BuildState::Building;
	return 0x6AB814;
}

ASMJIT_PATCH(0x6AB825, SelectClass_ProcessInput_FixOffset7, 5)
{
	R->ECX<int>(R->EBP<int>());
	R->EDX<void*>(nullptr);

	return 0x6AB82A;
}

ASMJIT_PATCH(0x6AB920, SelectClass_ProcessInput_FixOffset8, 7)
{
	R->ECX<void*>(nullptr);
	return 0x6AB927;
}

ASMJIT_PATCH(0x6AB92F, SelectClass_ProcessInput_FixOffset9, 7)
{
	R->EBX<byte*>(R->EBX<byte*>() + 0x6C);
	return 0x6AB936;
}
#endif 

static COMPILETIMEEVAL reference<bool, 0xB0B518> const SidebarBlitRequested_FullRedraw {};
static COMPILETIMEEVAL reference<int, 0xB0B4F8> const Sidebar_B0B4F8 {};
static COMPILETIMEEVAL reference<int, 0x884CF8> const Sidebar_884CF8 {};
static COMPILETIMEEVAL reference<wchar_t, 0xB078B0, 64>  const Sidebar_UIName {};
static COMPILETIMEEVAL constant_ptr<int, 0x884C34> const Sidebar_Rank_ {};
static COMPILETIMEEVAL constant_ptr<int, 0x884BB4> const Sidebar_Kills_ {};
static COMPILETIMEEVAL constant_ptr<int, 0x884BD4> const Sidebar_Units_ {};
static COMPILETIMEEVAL constant_ptr<int, 0x884BF4> const Sidebar_Credits_ {};
static COMPILETIMEEVAL constant_ptr<HouseClass*, 0x884B94> const Sidebar_Houses_ {};
static COMPILETIMEEVAL constant_ptr<ColorScheme*, 0x884C14> const Sidebar_Converts_ {};

const wchar_t* FakeStripClass::__Help_Text(int index)
{
	if (!Game::IsActive())
		return nullptr;

	const auto& tab = MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex];

	if(tab.empty())
		return nullptr;

	auto& cameo = tab[index + 2 * this->TopRowIndex];

	if (cameo.ItemType == AbstractType::None)
	{
		return nullptr;
	}
	else if (Phobos::UI::ExtendedToolTips)
	{
		PhobosToolTip::Instance.IsCameo = true;
		PhobosToolTip::Instance.HelpText(&cameo);
		return (L"X");
	}
	else if (cameo.ItemType == AbstractType::Special)
	{

		PhobosToolTip::Instance.IsCameo = true;
		auto pSW = SuperWeaponTypeClass::Array->Items[cameo.ItemIndex];
		const auto pData = SWTypeExtContainer::Instance.Find(pSW);

		if (pData->Money_Amount < 0)
		{
			// account for no-name SWs
			if (CCToolTip::HideName() || !wcslen(pSW->UIName))
			{
				const wchar_t* pFormat = StringTable::FetchString(GameStrings::TXT_MONEY_FORMAT_1);
				_snwprintf_s(SidebarClass::TooltipBuffer(), SidebarClass::TooltipLength - 1, pFormat, -pData->Money_Amount);
			}
			else
			{
				// then, this must be brand SWs
				const wchar_t* pFormat = StringTable::FetchString(GameStrings::TXT_MONEY_FORMAT_2);
				_snwprintf_s(SidebarClass::TooltipBuffer(), SidebarClass::TooltipLength - 1, pFormat, pSW->UIName, -pData->Money_Amount);
			}
		}
		else
		{
			return pSW->UIName;
		}
	}
	else if (auto pTechnoType = TechnoTypeClass::GetByTypeAndIndex(cameo.ItemType, cameo.ItemIndex))
	{
		PhobosToolTip::Instance.IsCameo = true;

		const int Cost = pTechnoType->GetActualCost(HouseClass::CurrentPlayer);

		if (CCToolTip::HideName || !wcslen(pTechnoType->UIName))
		{
			const wchar_t* Format = StringTable::FetchString(GameStrings::TXT_MONEY_FORMAT_1);
			_snwprintf_s(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, SidebarClass::TooltipLength - 1, Format, Cost);
		}
		else
		{
			const wchar_t* UIName = pTechnoType->UIName;
			const wchar_t* Format = StringTable::FetchString(GameStrings::TXT_MONEY_FORMAT_2);
			_snwprintf_s(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, SidebarClass::TooltipLength - 1, Format, UIName, Cost);
		}
	}
	else
	{
		return nullptr;
	}

	SidebarClass::TooltipBuffer[SidebarClass::TooltipBuffer.size() - 1] = 0;

	// replace space by new line
	for (int i = wcslen(SidebarClass::TooltipBuffer()); i >= 0; --i)
	{
		if (SidebarClass::TooltipBuffer[i] == 0x20)
		{
			SidebarClass::TooltipBuffer[i] = 0xA;
			break;
		}
	}

	return SidebarClass::TooltipBuffer();
}

DEFINE_FUNCTION_JUMP(CALL, 0x6AC3C3, FakeStripClass::__Help_Text);

void __thiscall FakeStripClass::__Draw_It(bool forceRedraw)
{
	// Early exit checks
	// 006A954D: mov al, [esi+1Ch] - check AllowedToDraw
	if (!this->AllowedToDraw)
		return;

	// 006A9558: mov al, [esi+3Ch] - check NeedsRedraw
	// 006A955F: mov al, byte ptr [esp+480h+arg_0] - check forceRedraw
	if (!this->NeedsRedraw && !forceRedraw)
		return;

	// 006A9571: mov byte ptr [esi+3Ch], 0
	this->NeedsRedraw = false;

	// 006A957D: mov SidebarBlitRequested_FullRedraw, 1
	SidebarBlitRequested_FullRedraw = true;
	DSurface* SidebarSurface = DSurface::Sidebar();

	// Setup clip rect from sidebar surface
	// 006A9584-006A95AF: Setup argC rect and get surface dimensions
	RectangleStruct clipRect {
		.X = 0,
		.Y = 0,
		.Width = SidebarSurface->Get_Width(),
		.Height = SidebarSurface->Get_Height()
	};

	// 006A95A3: mov ebx, HouseClass *PlayerPtr
	// 006A95A9: mov ebp, AnotherPlayerPtr2
	HouseClass* pPlayer = HouseClass::CurrentPlayer();
	HouseClass* pAnotherPlayer = HouseClass::Observer();
	StripClass* pStrip = this;

	// 006A95B3-006A95BE: Check if PlayerPtr == AnotherPlayerPtr2 for starting index
	int buildableIdx = 0;
	bool hasActiveProduction = false;

	if (pPlayer == pAnotherPlayer)
	{
		buildableIdx = this->BuildableCount;
	}

	// ==========================================================================
	// HOOK: 0x6A95C8 - StripClass_Draw_Status (8 bytes)
	// Replaces status pointer calculation for extended cameo support
	// Original: lea ecx, [eax+eax*2+6] / lea edx, [eax+ecx*4+2] / lea edx, [esi+edx*4]
	// Hook reads from MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex] instead
	// ==========================================================================

	// 006A95C1-006A95E9: Loop to check if any buildable has active production
	int totalBuildables = this->BuildableCount;
	while (buildableIdx < totalBuildables)
	{
		// Hook: Reads from MouseClassExt::TabCameos[ActiveTabIndex][buildableIdx].Status
		BuildType* pBuildable = &MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][buildableIdx];
		if (pBuildable->Status == BuildState::Building || pBuildable->Status == BuildState::OnHold)
		{
			hasActiveProduction = true;
			break;
		}
		++buildableIdx;
	}

	// 006A95EE-006A9607: Calculate tab offset based on Scen->SideBarTypeseems
	const int tabOffset = ScenarioClass::Instance->PlayerSideIndex ? 18 : 26;

	// 006A9607-006A9644: Calculate visible rows
	// Formula: ((SidebarBounds.Height - dword_B0B4F8 - tabOffset + SidebarBounds.Y - 7) / 50) + IsScrolling
	int visibleRows = (DSurface::SidebarBounds->Height - Sidebar_B0B4F8 - tabOffset + DSurface::SidebarBounds->Y - 7) / 50;
	visibleRows += (this->IsScrolling != 0) ? 1 : 0;
	int maxRows = visibleRows;

	// 006A9646-006A964E: Branch based on player mode
	// ========================================================================
	// NORMAL MODE: Draw buildable items (PlayerPtr != AnotherPlayerPtr2)
	// ========================================================================
	if (pPlayer != pAnotherPlayer && maxRows > 0)
	{
		// 006A965C: Main row loop
		for (int rowIndex = 0; rowIndex < maxRows; ++rowIndex)
		{
			// 006A9664: Column loop (0 to 1)
			for (int column = 0; column < 2; ++column)
			{
				// 006A9664-006A96C9: Initialize per-item state
				bool shouldRedraw = forceRedraw;
				bool shouldDarken = false;
				bool isInProduction = false;
				bool isCompleted = false;
				bool isMouseOver = false;
				bool hasQueuedCount = false;

				int progressFrame = 0;
				int topIndex = pStrip->TopRowIndex;
				//int stripTabIndex = pStrip->TabIndex;
				int buildableIndex = column + 2 * (topIndex + rowIndex);
				int buildableCount = pStrip->BuildableCount;

				// Calculate screen position
				int stripX = pStrip->Location.X;
				int screenX = stripX + column * SidebarClass::ObjectWidth - DSurface::SidebarBounds->X;
				int screenY = rowIndex * SidebarClass::ObjectHeight + pStrip->Location.Y + 1;
				int originalScreenY = screenY;

				// Item data pointers
				SHPStruct* cameoShape = NULL;
				TechnoTypeClass* pFactoryType = NULL;
				TechnoTypeClass* pTechnoType = NULL;
				const wchar_t* statusText = NULL;
				const wchar_t* pUIName = NULL;
				BSurface* CameoPCXSurface = nullptr;
				ConvertClass* CameoConvert = nullptr;
				AbstractTypeClass* pBuildableItem = nullptr;

				auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[pPlayer->SideIndex]);

				SHPStruct* _GCLOCK_Shape = FileSystem::GCLOCK2_SHP();
				ConvertClass* _GCLOCK_Convert = FileSystem::SIDEBAR_PAL();
				BlitterFlags _GCLOCK_Trans = BlitterFlags::TransLucent50;

				if (pSideExt->GClock_Shape)
					_GCLOCK_Shape = pSideExt->GClock_Shape;

				if (auto pConvert = pSideExt->GClock_Palette.GetConvert())
					_GCLOCK_Convert = pConvert;

				if (pSideExt->GClock_Transculency->GetIntValue() > 0)
					_GCLOCK_Trans = pSideExt->GClock_Transculency->GetBlitterFlags();

				// ==========================================================================
				// HOOK: 0x6A96D9 - StripClass_Draw_Strip (7 bytes)
				// Simplifies SelectButton lookup and handles scrolling check
				// Original: Complex calculation with Rate * 15 * 4
				// Hook: R->EAX(&SidebarClass::SelectButtonCombined[column + 2 * rowIndex])
				//       Returns 0x6A9703 if scrolling, 0x6A9714 otherwise
				// ==========================================================================

				// 006A96C6-006A9701: Get SelectClass button
				SelectClass* pSelectButton = &SidebarClass::SelectButtonCombined[rowIndex * 2 + column];

				// 006A9701-006A9710: Adjust position if scrolling
				if (pStrip->IsScrolling)
				{
					screenY = screenY + pStrip->Slid - SidebarClass::ObjectHeight;
					originalScreenY = screenY;
				}

				if (buildableIndex >= buildableCount) { // dont proceed further if the index is out of bounds
					return;
				}

				BuildType* pBuildable = &MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][buildableIndex];

				// 006A9714-006A9727: Process buildable item if within bounds
				{
					// 006A972D-006A9742: Check tooltip state
					isMouseOver = (pSelectButton->__MouseOver && !ScenarioClass::IsUserInputLocked());

					// ==========================================================================
					// HOOK: 0x6A9747 - StripClass_Draw_GetCameo (6 bytes)
					// Replaces buildable lookup with MouseClassExt::TabCameos access
					// Original: lea edx, [ecx+ecx*2] / lea eax, [ecx+edx*4] / mov ecx, [edi+eax*4+5Ch]
					// Hook: 
					//   auto& Item = MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][CameoIndex];
					//   Returns 0x6A9936 for SuperWeapon (AbstractType::Special)
					//   Returns 0x6A9761 for Techno
					// ==========================================================================

					// 006A9747-006A975B: Get buildable info
					AbstractType buildableType = pBuildable->ItemType;
					// --------------------------------------------------------
					// SuperWeapon handling (AbstractType::Special == 0x1F)
					// 006A9936-006A99BC
					// --------------------------------------------------------

					if (buildableType == AbstractType::Special)
					{
						int superIndex = pBuildable->ItemIndex;
						SuperClass* pSuper = pPlayer->Supers.Items[superIndex];
						SuperWeaponTypeClass* pSuperType = pSuper->Type;
						pBuildableItem = pSuperType;
						pUIName = pSuperType->UIName;

						auto pSWTypeExt = SWTypeExtContainer::Instance.Find(pSuperType);

						// ==========================================================================
						// HOOK: 0x6A9948 - StripClass_Draw_SuperWeapon (6 bytes)
						// Stores custom palette from SWTypeExtContainer for later use
						// Hook: if (auto pManager = SWTypeExtContainer::Instance.Find(pSuper)->SidebarPalette.GetConvert())
						//           SWConvert = pManager;
						// ==========================================================================
						if (auto pManager = pSWTypeExt->SidebarPalette.GetConvert())
							CameoConvert = pManager;
						// ==========================================================================
						// HOOK: 0x6A9952 - StripClass_Draw_SuperWeapon_PCX (6 bytes)
						// Stores PCX surface from SWTypeExtContainer for later drawing
						// Hook: CameoPCXSurface = SWTypeExtContainer::Instance.Find(pSuper)->SidebarPCX.GetSurface();
						// ==========================================================================
						CameoPCXSurface = pSWTypeExt->SidebarPCX.GetSurface();

						// 006A9948-006A995C: Get cameo shape
						cameoShape = pSuperType->SidebarImage;

						if (pSWTypeExt->GClock_Shape)
							_GCLOCK_Shape = pSWTypeExt->GClock_Shape;

						if (auto pConvert = pSWTypeExt->GClock_Palette.GetConvert())
							_GCLOCK_Convert = pConvert;

						if (pSWTypeExt->GClock_Transculency->GetIntValue() > 0)
							_GCLOCK_Trans = pSWTypeExt->GClock_Transculency->GetBlitterFlags();

						isInProduction = true;

						// 006A9966-006A99B1: Get super weapon state
						isCompleted = !pSuper->ShouldDrawProgress();
						statusText = pSuper->NameReadiness();
						progressFrame = ((FakeSuperClass*)pSuper)->_GetAnimStage();

						// ==========================================================================
						// HOOK: 0x6A99B7 - StripClass_Draw_SuperDarken (5 bytes)
						// Darkens SW cameo if player can't afford it
						// Original: xor bl, bl (shouldDarken = false)
						// Hook: R->BL(SWTypeExtData::DrawDarken(HouseClass::CurrentPlayer->Supers.Items[idxSW]));
						// ==========================================================================
						shouldDarken = SWTypeExtData::DrawDarken(pSuper);
						shouldRedraw = true;
					}
					// --------------------------------------------------------
					// Techno handling
					// 006A9761-006A98D3
					// --------------------------------------------------------
					else
					{
						// 006A9761-006A976B: Fetch techno type
						pTechnoType = TechnoTypeClass::FetchTechnoType(buildableType, pBuildable->ItemIndex);

						if (pTechnoType)
						{
							pUIName = pTechnoType->UIName;
							bool shouldDisable = false;

							// ==========================================================================
							// HOOK: 0x6A9789 - StripClass_DrawStrip_NoGreyCameo (6 bytes)
							// Handles ExpandBuildingQueue feature
							// Original: cmp eax, 7 / jnz (check if BuildingType with active production)
							// Hook: Additional checks for BuildCat and factory state when
							//       RulesExtData::Instance()->ExpandBuildingQueue is enabled
							// ==========================================================================

							if (!RulesExtData::Instance()->ExpandBuildingQueue){
								if (pTechnoType->WhatAmI() == AbstractType::BuildingType && hasActiveProduction) {
									shouldDisable = true;
								}
							}
							else if (const auto pBuildingType = type_cast<BuildingTypeClass*, false>(pTechnoType)) {
								if (const auto pFactory = pPlayer->GetPrimaryFactory(AbstractType::BuildingType, pTechnoType->Naval, pBuildingType->BuildCat)) {
									if (const auto pProduct = cast_to<BuildingClass*>(pFactory->Object)) {
										if (pFactory->IsDone() && pProduct->Type != pTechnoType && ((pProduct->Type->BuildCat != BuildCat::Combat) ^ (pBuildingType->BuildCat == BuildCat::Combat)))
											shouldDisable = true;
									}
								}
							} 
							
							{// 006A978C-006A97B2: Check if cameo should be disabled			
								if (!pTechnoType->FindFactory(true, true, true, pPlayer)) {
									shouldDisable = true;
								} else {
									// 006A97B4-006A97F9: Check Can_Build and ShouldDisableCameo
									pFactoryType = TechnoTypeClass::FetchTechnoType(pBuildable->ItemType, pBuildable->ItemIndex);

									if (pPlayer->CanBuild(pFactoryType, false, false) == CanBuildResult::TemporarilyUnbuildable || HouseExtData::ShouldDisableCameo(pPlayer, pFactoryType, true))
										shouldDisable = true;
								}
							}

							// ==========================================================================
							// HOOK: 0x6A980A - StripClass_Draw_TechnoType_PCX (8 bytes)
							// Gets PCX surface for techno type cameo
							// Hook: CameoPCXSurface = TechnoTypeExt_ExtData::GetPCXSurface(pType, HouseClass::CurrentPlayer);
							// ==========================================================================
							auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

							if (pTechnoTypeExt->GClock_Shape)
								_GCLOCK_Shape = pTechnoTypeExt->GClock_Shape;

							if (auto pConvert = pTechnoTypeExt->GClock_Palette.GetConvert())
								_GCLOCK_Convert = pConvert;

							if (pTechnoTypeExt->GClock_Transculency->GetIntValue() > 0)
								_GCLOCK_Trans = pTechnoTypeExt->GClock_Transculency->GetBlitterFlags();

							CameoPCXSurface = TechnoTypeExt_ExtData::GetPCXSurface(pTechnoType, pPlayer);

							// 006A9800-006A980E: Get cameo data
							cameoShape = pTechnoType->GetCameo();
							if (auto pConv = TechnoTypeExtContainer::Instance.Find(pTechnoType)->CameoPal.GetConvert())
								CameoConvert = pConv;

							FactoryClass* pBuildCameo = pBuildable->CurrentFactory;

							// ------------------------------------------------
							// No factory - check status directly
							// 006A9879-006A98CF
							// ------------------------------------------------

							if (!pBuildCameo)
							{
								shouldDarken = shouldDisable;
								isInProduction = false;

								// ==========================================================================
								// HOOK: 0x6A9886 - StripClass_Draw_Status_2 (8 bytes)
								// Gets status from MouseClassExt::TabCameos
								// Hook: Calculates pointer to extended cameo data + 0x10 (Status offset)
								// ==========================================================================

								// 006A9891-006A9931: Switch on buildable status
								switch (pBuildable->Status)
								{
								case BuildState::Normal:
									if (!forceRedraw)
										continue;
									break;

								case BuildState::Building:
								{
									shouldRedraw = true;
									isInProduction = true;
									int completion = 0;
									if (pBuildable->Status != BuildState::Building) //transition check
										progressFrame = completion;
									else {
										int storedProgress = pBuildable->Progress.Stage;
										if (storedProgress > completion)
											completion = (storedProgress + completion) / 2;
									}
									progressFrame = completion;
									break;
								}

								case BuildState::OnHold:
								{
									isInProduction = true;
									isCompleted = false;
									shouldDarken = true;
									int completion = 0;
									if (pBuildable->Status == BuildState::Building) //transition check
									{
										int storedProgress = pBuildable->Progress.Stage;
										if (storedProgress > 0)
											completion = storedProgress / 2;
									}
									progressFrame = completion;
									break;
								}

								case BuildState::Ready:
									isInProduction = true;
									isCompleted = true;
									shouldDarken = false;
									break;
								}
							} else {
								// ------------------------------------------------
								// Factory exists - 006A981D-006A9877
								// ------------------------------------------------

								isInProduction = true;

								// ==========================================================================
								// HOOK: 0x6A9822 - StripClass_Draw_Power (5 bytes)
								// Checks if factory is done AND has power (for buildings)
								// Original: call Has_Completed
								// Hook: Also checks pBuilding->FindFactory(true, true) for power requirement
								// ==========================================================================

								// 006A9822-006A982D: Check if completed
								isCompleted = pBuildCameo->IsDone();

								if (isCompleted) {
									if (auto pBuilding = cast_to<BuildingClass*, false>(pBuildCameo->Object)) {
										isCompleted = pBuilding->FindFactory(true, true) != nullptr;
									}
								}

								if (isCompleted)
									statusText = CSFLoader::FetchStringManager("TXT_READY", NULL, NULL, 0);
								else
									shouldRedraw = true;

								// 006A9850-006A9877: Calculate progress
								int progress = pBuildable->CurrentFactory ? pBuildable->CurrentFactory->GetProgress() : 0;

								// ==========================================================================
								// HOOK: 0x6A9866 - StripClass_Draw_Status_1 (8 bytes)
								// Checks status from MouseClassExt::TabCameos
								// Hook: Returns 0x6A9874 if Building, 0x6A98CF otherwise
								// ==========================================================================

								if (pBuildable->Status == BuildState::Building)
								{
									int storedProgress = pBuildable->Progress.Stage;
									if (storedProgress > progress)
										progress = (storedProgress + progress) / 2;
								}
								progressFrame = progress;
								shouldDarken = false;
							}
						}

						pBuildableItem = pTechnoType;
					}
				}

				// ==========================================================================
				// HOOK: 0x6A99BE - StripClass_Draw_BreakDrawLoop (5 bytes)
				// Sets isInProduction = false and jumps to drawing section
				// ==========================================================================

				// ============================================================
				// DRAWING SECTION - 006A99C3-006AA01C
				// ============================================================

				RectangleStruct boundingRect = { 0, 0, screenX + 60, screenY + 48 };

				if (!shouldRedraw)
					continue;

				// ==========================================================================
				// HOOK: 0x6A99F3 - StripClass_Draw_SkipSHPForPCX (6 bytes)
				// Skips SHP drawing if PCX surface exists, also handles MissingCameo
				// ==========================================================================

				if (CameoPCXSurface)
				{
					RectangleStruct bounds { screenX, screenY, 60, 48 };
					PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, CameoPCXSurface, (WORD)Drawing::DefaultColors[6].ToInit());
					CameoPCXSurface = nullptr;
				}
				else
					// 006A99F3-006A9A3E: Draw cameo shape
					if (cameoShape)
					{
						// ==========================================================================
						// HOOK: 0x6A9A2A - StripClass_Draw_Main (6 bytes)
						// Gets custom palette for cameo drawing
						// Original: mov edx, ConvertClass * CameoDrawer
						// Hook: Uses TechnoTypeExtContainer palette or SWConvert if available
						// ==========================================================================
						bool drawSHP = true;
						auto pCameoRef = cameoShape->AsReference();
						char pFilename[0x20];
						strcpy_s(pFilename, RulesExtData::Instance()->MissingCameo.data());
						_strlwr_s(pFilename);

						if (!_stricmp(pCameoRef->Filename, GameStrings::XXICON_SHP())
							&& (strstr(pFilename, ".pcx")))
						{
							BSurface* pCXSurf = nullptr;

							if (PCX::Instance->LoadFile(pFilename))
								pCXSurf = PCX::Instance->GetSurface(pFilename);

							if (pCXSurf)
							{
								RectangleStruct bounds { screenX, screenY, 60, 48 };
								PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, pCXSurf);

								drawSHP = false;
							}
						}

						if (drawSHP)
						{
							Point2D cameoPos = { screenX, screenY };
							CC_Draw_Shape(SidebarSurface, CameoConvert ? CameoConvert : FileSystem::CAMEO_PAL(), cameoShape, 0, &cameoPos,
									  &boundingRect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
						}
					}

				// ==========================================================================
				// HOOK: 0x6A9A43 - StripClass_Draw_DrawPCX (6 bytes)
				// Draws PCX cameo if CameoPCXSurface was set by earlier hooks
				// ==========================================================================
				const int tooltipColor = Drawing::TooltipColor->ToInit();
				const int textColor = tooltipColor;

				// 006A9A43-006A9AFB: Draw tooltip highlight rectangle
				if (isMouseOver)
				{
					RectangleStruct highlightRect = { screenX, screenY, 60, 48 };
					SidebarSurface->Draw_Rect(boundingRect, highlightRect, textColor);
				}

				// 006A9AFB-006A9B4B: Draw darken overlay
				if (shouldDarken)
				{
					Point2D darkenPos = { screenX, screenY };
					CC_Draw_Shape(SidebarSurface, FileSystem::CAMEO_PAL(), FileSystem::DARKEN_SHP(), 0, &darkenPos,
								  &boundingRect, BlitterFlags::bf_400 | BlitterFlags::Darken, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
				}

				// ==========================================================================
				// HOOK: 0x6A9B4F - StripClass_Draw_TestFlashFrame (6 bytes)
				// Major hook: Flash check, grey cameo overlay, building count, statistics
				// ==========================================================================
				if (pBuildableItem)
				{
					//DrawGreyCameoExtraCover

					if (pTechnoType)
					{
						Point2D position { screenX + 30, screenY + 24 };
						const auto pRulesExt = RulesExtData::Instance();
						const Vector3D<int>& frames = pRulesExt->Cameo_OverlayFrames.Get();

						if (shouldDarken) // Only draw extras over grey cameos
						{
							auto frame = frames.Y;
							const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

							if (pTypeExt->Cameo_AlwaysExist.Get(pRulesExt->Cameo_AlwaysExist))
							{
								auto& vec = ScenarioExtData::Instance()->OwnedExistCameoTechnoTypes;

								if (vec.contains(pTechnoType))
								{
									if (const auto CameoPCX = pTypeExt->GreyCameoPCX.GetSurface())
									{
										auto drawRect = RectangleStruct { screenX, screenY, 60, 48 };
										PCX::Instance->BlitToSurface(&drawRect, DSurface::Sidebar, CameoPCX);
									}

									frame = frames.Z;
								}
							}

							if (frame >= 0)
							{
								ConvertClass* pConvert = FileSystem::PALETTE_PAL;
								if (pRulesExt->Cameo_OverlayPalette.GetConvert())
									pConvert = pRulesExt->Cameo_OverlayPalette.GetConvert();

								DSurface::Sidebar->DrawSHP(
									pConvert,
									pRulesExt->Cameo_OverlayShapes,
									frame,
									&position,
									&boundingRect,
									BlitterFlags(0x600),
									0, 0,
									ZGradient::Ground,
									1000, 0, 0, 0, 0, 0);
							}
						}

						//TODO : check bounding rect
						if (const auto pBuildingType = type_cast<BuildingTypeClass*, false>(pTechnoType)) // Only count owned buildings
						{
							const auto pHouse = HouseClass::CurrentPlayer();
							auto count = BuildingTypeExtData::GetUpgradesAmount(pBuildingType, pHouse);

							if (count == -1)
								count = pHouse->CountOwnedAndPresent(pBuildingType);

							if (count > 0)
							{
								if (frames.X >= 0)
								{
									ConvertClass* pConvert = FileSystem::PALETTE_PAL;
									if (pRulesExt->Cameo_OverlayPalette.GetConvert())
										pConvert = pRulesExt->Cameo_OverlayPalette.GetConvert();

									DSurface::Sidebar->DrawSHP(
										pConvert,
										pRulesExt->Cameo_OverlayShapes,
										frames.X,
										&position,
										&boundingRect,
										BlitterFlags(0x600),
										0, 0,
										ZGradient::Ground,
										1000, 0, 0, 0, 0, 0);
								}

								if (Phobos::Config::ShowBuildingStatistics
									&& BuildingTypeExtContainer::Instance.Find(pBuildingType)->Cameo_ShouldCount.Get(pBuildingType->BuildCat != BuildCat::Combat || (pBuildingType->BuildLimit != INT_MAX))
									)
								{

									const TextPrintType printType = TextPrintType::Background | TextPrintType::Right | TextPrintType::FullShadow | TextPrintType::Point8;
									auto textPosition = Point2D { screenX , screenY + 1 };
									static fmt::basic_memory_buffer<wchar_t> text;
									text.clear();
									fmt::format_to(std::back_inserter(text), L"{}", count);
									text.push_back(L'\0');
									DSurface::Sidebar->DrawText_Old(text.data(), &boundingRect, &textPosition, textColor, 0, (DWORD)printType);
								}
							}
						}

					}

					// 006A9B4B-006A9BC5: Draw flashing effect

					if (pBuildable->FlashEndFrame > Unsorted::CurrentFrame())
					{
						if ((Unsorted::CurrentFrame() % 16) > 8)
						{
							Point2D flashPos = { screenX, screenY };
							CC_Draw_Shape(SidebarSurface, FileSystem::CAMEO_PAL(), FileSystem::DARKEN_SHP(), 0, &flashPos,
										  &boundingRect, BlitterFlags::bf_400 | BlitterFlags::TransLucent50, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
						}
					}

					// 006A9BC5-006A9BF1: Draw UI name
					if (pUIName)
					{
						Point2D namePos = { screenX, screenY + 36 };
						TextDrawing::Draw_Text_On_Sidebar(pUIName, &namePos, &clipRect, 60);
					}

					// ==========================================================================
					// HOOK: 0x6A9C54 - StripClass_DrawStrip_FindFactoryDehardCode (6 bytes)
					// Gets BuildCat from BuildingType for factory lookup
					// ==========================================================================

					// 006A9BF1-006A9D88: Draw queued count

					if (pTechnoType)
					{
						bool isNaval = pTechnoType->Naval;
						AbstractType rtti = pTechnoType->WhatAmI();

						BuildCat cat = BuildCat::DontCare;
						if (const auto pBuildingType = type_cast<BuildingTypeClass*, false>(pTechnoType))
							cat = pBuildingType->BuildCat;

						if (FactoryClass* pHouseFactory = pPlayer->GetPrimaryFactory(rtti, isNaval, cat))
						{
							int queuedCount = pHouseFactory->CountTotal(pTechnoType);
							bool shouldDrawCount = (queuedCount > 1);

							if (!shouldDrawCount && queuedCount > 0)
							{
								TechnoClass* pFactoryObject = pHouseFactory->Object;
								if (!pFactoryObject)
								{
									shouldDrawCount = true;
								}
								else
								{
									TechnoTypeClass* pFactoryObjType = GET_TECHNOTYPE(pFactoryObject);
									if (pFactoryObjType && pFactoryObjType != pTechnoType)
										shouldDrawCount = true;
								}
							}

							if (shouldDrawCount)
							{
								static fmt::basic_memory_buffer<wchar_t> countBuffer;
								countBuffer.clear();
								fmt::format_to(std::back_inserter(countBuffer), L"{}", queuedCount);
								countBuffer.push_back(L'\0');

								int countX = screenX + 60;
								int countY = screenY + 1;
								Point2D countPos = { countX, countY };
								RectangleStruct countBgRect;
								Drawing::GetTextDimensions(&countBgRect, countBuffer.data(), countPos, TextPrintType::Right | TextPrintType::FullShadow | TextPrintType::Point8, 2, 1);
								LoadProgressManager::FillRectWithColor(countBgRect, SidebarSurface, 0, 0xAF);
								TextDrawing::Fancy_Text_Print_Wide_NoFormat(countBuffer.data(), SidebarSurface, &clipRect, &countPos,
													  textColor, 0, TextPrintType::Right | TextPrintType::FullShadow | TextPrintType::Point8);
								hasQueuedCount = true;
							}
						}
					}

					// 006A9D88-006AA01C: Draw status text and progress clock
					if (isInProduction)
					{
						// Draw status text ("Ready", etc.)
						if (statusText)
						{
							int statusX = screenX + 30;
							int statusY = screenY + 1;
							Point2D statusP { statusX, statusY };
							RectangleStruct statusBgRect;
							Drawing::GetTextDimensions(&statusBgRect, statusText, statusP, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point8, 2, 1);

							LoadProgressManager::FillRectWithColor(statusBgRect, SidebarSurface, 0, 0xAF);

							Point2D statusPos = { statusX, statusY };
							TextDrawing::Fancy_Text_Print_Wide_NoFormat(statusText, SidebarSurface, &clipRect, &statusPos,
												  textColor, 0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point8);
						}

						// Draw progress clock if not completed
						if (!isCompleted)
						{
							Point2D clockPos = { screenX, screenY };
							CC_Draw_Shape(SidebarSurface, _GCLOCK_Convert, _GCLOCK_Shape,
										  progressFrame + 1, &clockPos, &boundingRect,
										  BlitterFlags::bf_400 | _GCLOCK_Trans, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

							// ==========================================================================
							// HOOK: 0x6A9EBA - StripClass_Draw_Status_3 (8 bytes)
							// Checks status from MouseClassExt::TabCameos for HOLD text
							// ==========================================================================

							// Check if should draw "HOLD" text
							bool shouldDrawHold = false;
							FactoryClass* pCurrentFactory = pBuildable->CurrentFactory;

							if (pCurrentFactory && (!pCurrentFactory->Production.Timer.Rate || pCurrentFactory->IsSuspended))
								shouldDrawHold = true;

							if (pBuildable->Status == BuildState::OnHold)
								shouldDrawHold = true;

							if (shouldDrawHold)
							{
								const wchar_t* holdText = CSFLoader::FetchStringManager("TXT_HOLD", NULL, NULL, 0);

								if (hasQueuedCount)
								{
									// Draw at top-left (queued count shown)
									int holdX = screenX + 2;
									int holdY = originalScreenY + 1;
									Point2D holdPos = { holdX, holdY };
									RectangleStruct holdBgRect;
									Drawing::GetTextDimensions(&holdBgRect, holdText, holdPos, TextPrintType::FullShadow | TextPrintType::Point8, 2, 1);
									LoadProgressManager::FillRectWithColor(holdBgRect, SidebarSurface, 0, 0xAF);
									TextDrawing::Fancy_Text_Print_Wide_NoFormat(holdText, SidebarSurface, &clipRect, &holdPos,
														  textColor, 0, TextPrintType::FullShadow | TextPrintType::Point8);
								}
								else
								{
									// Draw centered (no queued count)
									int holdX = screenX + 30;
									int holdY = originalScreenY + 1;
									Point2D holdPos = { holdX, holdY };
									RectangleStruct holdBgRect;
									Drawing::GetTextDimensions(&holdBgRect, holdText, holdPos, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point8, 2, 1);
									LoadProgressManager::FillRectWithColor(holdBgRect, SidebarSurface, 0, 0xAF);
									TextDrawing::Fancy_Text_Print_Wide_NoFormat(holdText, SidebarSurface, &clipRect, &holdPos,
														  textColor, 0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point8);
								}
							}
						}
					}
				} // end column loop
			} // end row loop
		}
	}

	// ========================================================================
	// OBSERVER MODE: Draw house info panels
	// 006AA05B-006AA595
	// Only entered if PlayerPtr == AnotherPlayerPtr2
	// ========================================================================
	if (pPlayer == pAnotherPlayer && maxRows > 0)
	{
		int observerRows = maxRows / 2;

		for (int rowIndex = 0; rowIndex < observerRows; ++rowIndex)
		{
			int houseIndex = pStrip->TopRowIndex + rowIndex;
			int panelX = pStrip->Location.X - DSurface::SidebarBounds->X;
			int panelY = pStrip->Location.Y + 2 * rowIndex * SidebarClass::ObjectHeight + 1;

			// Bounds check
			if (houseIndex >= Sidebar_884CF8)
				break;

			HouseClass* pObservedHouse = Sidebar_Houses_[houseIndex];

			if (!pObservedHouse)
				break;

			HouseTypeClass* pHouseType = pObservedHouse->Type;
			const wchar_t* diff = nullptr;

			if(!pObservedHouse->ControlledByCurrentPlayer()){
				switch (pObservedHouse->AIDifficulty)
				{
				case AIDifficulty::Hard:
					diff = CSFLoader::FetchStringManager(GameStrings::TXT_HARD, NULL, NULL, 0);
					break;
				case AIDifficulty::Easy:
					diff = CSFLoader::FetchStringManager(GameStrings::TXT_EASY, NULL, NULL, 0);
					break;
				case AIDifficulty::Normal:
					diff = CSFLoader::FetchStringManager(GameStrings::TXT_NORMAL, NULL, NULL, 0);
					break;
				default:
					break;
				}
			}

			// --------------------------------------------------------------------
			// Draw faction shape (Allied/Soviet/Yuri)
			// --------------------------------------------------------------------
			if (pHouseType)
			{

				const auto pData = HouseTypeExtContainer::Instance.Find(pHouseType);

				if (pData->ObserverBackgroundSHP)
				{

					RectangleStruct factionBounds = { 0, 0, panelX + pData->ObserverBackgroundSHP->Width, panelY + pData->ObserverBackgroundSHP->Height };
					Point2D factionPos = { panelX, panelY };
					CC_Draw_Shape(SidebarSurface, FileSystem::ObserverDrawer(), pData->ObserverBackgroundSHP, 0, &factionPos,
								  &factionBounds, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

				}
				else if (auto PCXSurface = pData->ObserverBackground.GetSurface())
				{
					RectangleStruct bounds = { panelX, panelY, ObserverBackgroundWidth, ObserverBackgroundHeight };
					PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, PCXSurface, (WORD)Drawing::DefaultColors[6].ToInit());
				}
			}

			// --------------------------------------------------------------------
			// Draw country shape
			// --------------------------------------------------------------------
			if (pHouseType)
			{
				const auto idx = pHouseType->ParentIdx;
				SHPStruct* pCountryShape = NULL;
				int countryIndex = 0;

				//special cases
				if (idx == -2)
				{
					pCountryShape = FileSystem::RandomSideShape_SHP;
				}
				else if (idx == -3)
				{
					pCountryShape = FileSystem::ObserverSideShape_SHP;
				}
				else
				{

					const auto pData = HouseTypeExtContainer::Instance.Find(pHouseType);

					if (pData->ObserverFlagSHP)
					{
						pCountryShape = pData->ObserverFlagSHP;
						countryIndex = pData->ObserverFlagYuriPAL ? 9 : 0;
					}
					else if (auto PCXSurface = pData->ObserverFlag.GetSurface())
					{
						RectangleStruct bounds = { panelX + ObserverFlagPCXX , panelY + ObserverFlagPCXY,
								ObserverFlagPCXWidth, ObserverFlagPCXHeight
						};

						PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, PCXSurface, (WORD)Drawing::DefaultColors[6].ToInit());
					}
				}

				if (pCountryShape)
				{
					int countryShapeX = panelX + 70;
					int countryShapeY = panelY + 70;

					RectangleStruct countryBounds = { 0, 0, pCountryShape->Width + countryShapeX, pCountryShape->Height + countryShapeY };
					Point2D countryPos = { countryShapeX, countryShapeY };

					// Yuri uses different drawer
					if (countryIndex == 9)
					{
						CC_Draw_Shape(SidebarSurface, FileSystem::YuriObserverDrawer(), pCountryShape, 0, &countryPos,
									  &countryBounds, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
					}
					else
					{
						CC_Draw_Shape(SidebarSurface, FileSystem::ObserverDrawer(), pCountryShape, 0, &countryPos,
									  &countryBounds, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
					}
				}
			}

			// --------------------------------------------------------------------
			// Draw stats text
			// --------------------------------------------------------------------
			constexpr int TextPadding = 17;

			ColorScheme* pColorScheme = Sidebar_Converts_[houseIndex];

			// Adjust for scrolling
			int adjustedPanelY = panelY;
			if (pStrip->IsScrolling)
				adjustedPanelY += pStrip->Slid - SidebarClass::ObjectHeight;

			Point2D textPos = { panelX + 8, adjustedPanelY + 4 };

			// Draw house name
			if(diff)
			TextDrawing::Fancy_Text_Print_Wide_externalBuffer(Sidebar_UIName.get(), L"%ls (%ls)", SidebarSurface, &clipRect, &textPos,
								  pColorScheme, 0, TextPrintType::FullShadow | TextPrintType::Point8, pObservedHouse->UIName , diff);
			else
			TextDrawing::Fancy_Text_Print_Wide_externalBuffer(Sidebar_UIName.get(), L"%ls", SidebarSurface, &clipRect, &textPos,
								  pColorScheme, 0, TextPrintType::FullShadow | TextPrintType::Point8, pObservedHouse->UIName);

			wchar_t textBuffer[64];
			wchar_t rankStr[16], killsStr[16], unitsStr[16], creditsStr[16];

			_itow(Sidebar_Kills_[houseIndex], killsStr, 10);
			_itow(Sidebar_Units_[houseIndex], unitsStr, 10);
			_itow(Sidebar_Credits_[houseIndex], creditsStr, 10);

			// Draw rank (internet only)
			if (SessionClass::Instance->GameMode == GameMode::Internet)
			{
				textPos.Y += TextPadding;
				_itow(Sidebar_Rank_[houseIndex], rankStr, 10);

				if (Sidebar_Rank_[houseIndex] >= 1)
				{
					_swprintf(textBuffer, CSFLoader::FetchStringManager("GUI:ObsRank", NULL, NULL, 0), rankStr);
				}
				else
				{
					wcscpy(textBuffer, CSFLoader::FetchStringManager("TXT_UNRANKED", NULL, NULL, 0));
				}

				TextDrawing::Fancy_Text_Print_Wide_NoFormat(textBuffer, SidebarSurface, &clipRect, &textPos,
									  pColorScheme, 0, TextPrintType::FullShadow | TextPrintType::Point8);
			}

			// Draw kills
			textPos.Y += TextPadding;
			_swprintf(textBuffer, CSFLoader::FetchStringManager("GUI:ObsKills", NULL, NULL, 0), killsStr);
			TextDrawing::Fancy_Text_Print_Wide_NoFormat(textBuffer, SidebarSurface, &clipRect, &textPos,
								  pColorScheme, 0, TextPrintType::FullShadow | TextPrintType::Point8);

			// Draw units
			textPos.Y += TextPadding;
			_swprintf(textBuffer, CSFLoader::FetchStringManager("GUI:ObsUnits", NULL, NULL, 0), unitsStr);
			TextDrawing::Fancy_Text_Print_Wide_NoFormat(textBuffer, SidebarSurface, &clipRect, &textPos,
								  pColorScheme, 0, TextPrintType::FullShadow | TextPrintType::Point8);

			// Draw credits
			textPos.Y += TextPadding;
			_swprintf(textBuffer, CSFLoader::FetchStringManager("GUI:ObsCredits", NULL, NULL, 0), creditsStr);
			TextDrawing::Fancy_Text_Print_Wide_NoFormat(textBuffer, SidebarSurface, &clipRect, &textPos,
								  pColorScheme, 0, TextPrintType::FullShadow | TextPrintType::Point8);
		}
	}

	// 006AA59B-006AA5A5: Store Slid to __LastSlid
	pStrip->__LastSlid = pStrip->Slid;
}

DEFINE_FUNCTION_JUMP(CALL, 0x6A6FDF, FakeStripClass::__Draw_It)