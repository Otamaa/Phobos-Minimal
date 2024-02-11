#include "Body.h"

#include "SuperWeaponSidebar.h"

//DEFINE_HOOK(0x69300B, MouseClass_UpdateCursor, 0x6)
//{
//	const auto pCurrent = HouseClass::CurrentPlayer();
//
//	if (pCurrent->Defeated)
//		return 0;
//
//	int superCount = 0;
//
//	for (const auto pSuper : pCurrent->Supers) {
//
//		if (superCount > MaxSWShown)
//			break;
//
//		if(SWTypeExtContainer::Instance.Find(pSuper->Type)->IsAvailable(pCurrent))
//			superCount++;
//	}
//
//	if (superCount == 0)
//		return 0;
//
//	const Point2D crdCursor = { WWMouseClass::Instance->GetX(), WWMouseClass::Instance->GetY() };
//	const int cameoWidth = 60;
//	const int cameoHeight = 48;
//	Point2D location = { 0, (DSurface::ViewBounds->Height - std::min(superCount, MaxPerRow) * cameoHeight) / 2 };
//	int location_Y = location.Y;
//	int row = 0, line = 0;
//
//	for (int idx = 0; idx < superCount; idx++)
//	{
//		if (crdCursor.X > location.X
//			&& crdCursor.X < location.X + cameoWidth
//			&& crdCursor.Y > location.Y
//			&& crdCursor.Y < location.Y + cameoHeight)
//		{
//			R->EAX(Action::None);
//			return 0x69301A;
//		}
//
//		row++;
//
//		if (row >= MaxPerRow - line)
//		{
//			row = 0;
//			line++;
//			location_Y += cameoHeight / 2;
//			location = { location.X + cameoWidth, location_Y };
//		}
//		else
//		{
//			location.Y += cameoHeight;
//		}
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x6931A5, MouseClass_UpdateCursor_LeftPress, 0x6)
//{
//	const auto pCurrent = HouseClass::CurrentPlayer();
//	if (pCurrent->Defeated)
//		return 0;
//
//	int superCount = 0;
//
//	for (const auto pSuper : pCurrent->Supers)
//	{
//		if (superCount > MaxSWShown)
//			break;
//
//		if (SWTypeExtContainer::Instance.Find(pSuper->Type)->IsAvailable(pCurrent))
//			superCount++;
//	}
//
//	if (superCount == 0)
//		return 0;
//
//	const Point2D crdCursor = { WWMouseClass::Instance->GetX(), WWMouseClass::Instance->GetY() };
//	const int cameoWidth = 60;
//	const int cameoHeight = 48;
//	Point2D location = { 0, (DSurface::ViewBounds->Height - std::min(superCount, MaxPerRow) * cameoHeight) / 2 };
//	int location_Y = location.Y;
//	int row = 0, line = 0;
//
//	for (int idx = 0; idx < superCount; idx++)
//	{
//		if (crdCursor.X > location.X && crdCursor.X < location.X + cameoWidth && crdCursor.Y > location.Y && crdCursor.Y < location.Y + cameoHeight)
//		{
//			R->EAX(Action::None);
//			return 0x6931B4;
//		}
//
//		row++;
//
//		if (row >= MaxPerRow - line)
//		{
//			row = 0;
//			line++;
//			location_Y += cameoHeight / 2;
//			location = { location.X + cameoWidth, location_Y };
//		}
//		else
//		{
//			location.Y += cameoHeight;
//		}
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x693268, MouseClass_UpdateCursor_LeftRelease, 0x5)
//{
//	const auto pCurrent = HouseClass::CurrentPlayer();
//	if (pCurrent->Defeated)
//		return 0;
//
//	std::vector<SuperClass*> grantedSupers;
//
//	for (const auto pSuper : pCurrent->Supers) {
//
//		if (grantedSupers.size() > MaxSWShown)
//			break;
//
//		if(SWTypeExtContainer::Instance.Find(pSuper->Type)->IsAvailable(pCurrent))
//			grantedSupers.emplace_back(pSuper);
//	}
//
//	if (grantedSupers.empty())
//		return 0;
//
//	std::sort(grantedSupers.begin(), grantedSupers.end(),
//	[](SuperClass* a, SuperClass* b)
//	{
//		return BuildType::SortsBefore(AbstractType::Special, a->Type->ArrayIndex, AbstractType::Special, b->Type->ArrayIndex);
//	});
//
//	const Point2D crdCursor = { WWMouseClass::Instance->GetX(), WWMouseClass::Instance->GetY() };
//	const int cameoWidth = 60;
//	const int cameoHeight = 48;
//	const int superCount = static_cast<int>(grantedSupers.size());
//	Point2D location = { 0, (DSurface::ViewBounds->Height - std::min(superCount, MaxPerRow) * cameoHeight) / 2 };
//	int location_Y = location.Y;
//	int row = 0, line = 0;
//
//	for (const auto pSuper : grantedSupers)
//	{
//		if (crdCursor.X > location.X
//			&& crdCursor.X < location.X + cameoWidth
//			&& crdCursor.Y > location.Y
//			&& crdCursor.Y < location.Y + cameoHeight)
//		{
//			bool useAITargeting = false;
//
//			if (const auto pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type))
//			{
//				useAITargeting = pSWExt->SW_UseAITargeting;
//				VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0);
//				const bool manual = !pSWExt->SW_ManualFire && pSWExt->SW_AutoFire;
//				const bool unstopable = pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining
//					&& pSWExt->SW_Unstoppable;
//
//				if (!pSuper->CanFire() && !manual)
//				{
//					VoxClass::PlayIndex(pSuper->Type->ImpatientVoice);
//				}
//				else
//				{
//					if (!pCurrent->CanTransactMoney(pSWExt->Money_Amount))
//					{
//						VoxClass::PlayIndex(pSWExt->EVA_InsufficientFunds);
//						pSWExt->PrintMessage(pSWExt->Message_InsufficientFunds, pCurrent);
//					}
//					else if (!manual && !unstopable)
//					{
//						const auto swIndex = pSuper->Type->ArrayIndex;
//
//						if (pSuper->Type->Action == Action::None || useAITargeting)
//						{
//							EventClass::AddEvent(EventClass(pCurrent->ArrayIndex, EventType::SPECIAL_PLACE, swIndex, CellStruct::Empty));
//						}
//						else
//						{
//							DisplayClass::Instance->CurrentBuilding = nullptr;
//							DisplayClass::Instance->CurrentBuildingType = nullptr;
//							DisplayClass::Instance->unknown_11AC = static_cast<DWORD>(-1);
//							DisplayClass::Instance->SetRepairMode(0);
//							DisplayClass::Instance->SetSellMode(0);
//							DisplayClass::Instance->PowerToggleMode = false;
//							DisplayClass::Instance->PlanningMode = false;
//							DisplayClass::Instance->PlaceBeaconMode = false;
//							DisplayClass::Instance->CurrentSWTypeIndex = swIndex;
//							DisplayClass::UnselectAll();
//							VoxClass::PlayIndex(pSWExt->EVA_SelectTarget);
//						}
//					}
//				}
//			}
//			if (pSuper->CanFire())
//			{
//				DisplayClass::Instance->CurrentBuilding = nullptr;
//				DisplayClass::Instance->CurrentBuildingType = nullptr;
//				DisplayClass::Instance->unknown_11AC = static_cast<DWORD>(-1);
//				DisplayClass::Instance->SetRepairMode(0);
//				DisplayClass::Instance->SetSellMode(0);
//				DisplayClass::Instance->PowerToggleMode = false;
//				DisplayClass::Instance->PlanningMode = false;
//				DisplayClass::Instance->PlaceBeaconMode = false;
//				MapClass::UnselectAll();
//
//				if (!useAITargeting)
//					Unsorted::CurrentSWType = pSuper->Type->ArrayIndex;
//			}
//
//			R->EAX(Action::None);
//			return 0x693276;
//		}
//
//		row++;
//
//		if (row >=  - line)
//		{
//			row = 0;
//			line++;
//			location_Y += cameoHeight / 2;
//			location = { location.X + cameoWidth, location_Y };
//		}
//		else
//		{
//			location.Y += cameoHeight;
//		}
//	}
//
//	return 0;
//}

// DEFINE_HOOK(0x4F4583, GScreenClass_DrawOnTop_TheDarkSideOfTheMoon, 0x6)
// {
//
// 	if (auto pSuperBar = SuperWeaponSidebar::Instance())
// 		pSuperBar->Draws();
//
// 	return 0x0;
// }