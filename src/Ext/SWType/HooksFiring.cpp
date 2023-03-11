#include "Body.h"

#include <Ext/Super/Body.h>
#include <Utilities/Macro.h>
#include <BitFont.h>
#include <HouseClass.h>

#ifdef ENABLE_NEWHOOKS

DEFINE_HOOK(0x6CDE40, SuperClass_Place, 0x5)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CoordStruct const, coords, 0x230); // I think?;

	if (auto const pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type))
		pSWExt->FireSuperWeapon(pSuper, pSuper->Owner, coords);

	return 0;
}
#else
//#include <Misc/InteractWithAres/Body.h>

//Ares hooked at 0x6CC390 and jumped to 0x6CDE40
// If a super is not handled by Ares however, we do it at the original entry point
//
//DEFINE_HOOK(0x6CC390, SuperClass_Place_NotSkippedByAres, 0x6)
//{
//	GET(SuperClass* const, pSuper, ECX);
//	GET_STACK(CellStruct const* const, pCell, 0x4);
//	//GET_STACK(bool const, isPlayer, 0x8);
//
//	if (auto const pSWExt = SWTypeExt::ExtMap.Find<true>(pSuper->Type)) {
//		SWTypeExt::Handled = false;
//		pSWExt->FireSuperWeapon(pSuper, pSuper->Owner, CellClass::Cell2Coord(*pCell), false);
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x6CDE40, SuperClass_Place_FireExt, 0x4)
//{
//	GET(SuperClass* const, pSuper, ECX);
//	GET_STACK(CellStruct const* const, pCell, 0x4);
//	//GET_STACK(bool const, isPlayer, 0x8);
//
//	if (SWTypeExt::Handled) {
//		if (auto const pSWExt = SWTypeExt::ExtMap.Find<true>(pSuper->Type)) {
//			pSWExt->FireSuperWeapon(pSuper, pSuper->Owner, CellClass::Cell2Coord(*pCell), false);
//		}
//
//		SWTypeExt::Handled = false;
//	}
//
//	return 0;
//}

DEFINE_HOOK_AGAIN(0x6CC390, SuperClass_Place_FireExt, 0x6)
DEFINE_HOOK(0x6CDE40, SuperClass_Place_FireExt, 0x4)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct const* const, pCell, 0x4);
	//GET_STACK(bool const, isPlayer, 0x8);

	if (!pSuper->Type)
		return 0x0;

	if (auto const pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type)){
		pSWExt->FireSuperWeapon(pSuper,pSuper->Owner, pCell ,true);
	}

	return 0;
}
#endif

#pragma region Otamaa
#ifdef SW_TIMER
//TODO : Better Code ?
namespace SWTimerTemp {
	SWTypeExt::ExtData* SuperExt;
}

DEFINE_HOOK(0x6D4A10, TacticalClass_Render_FetchSW, 0x6)
{
	SWTypeExt::TempSuper = R->ECX<SuperClass*>();
	SWTimerTemp::SuperExt = SWTypeExt::ExtMap.Find(R->ECX<SuperClass*>()->Type);
	return 0x0;
}

DEFINE_HOOK(0x6D4A71, TacticalClass_Render_ClearSW, 0x5)
{
	SWTypeExt::TempSuper = nullptr;
	SWTimerTemp::SuperExt = nullptr;
	return 0x0;
}

namespace Timer
{
	void __fastcall DrawTimer(int arg1, ColorScheme* scheme, int interval, const wchar_t* string, LARGE_INTEGER* pBlinkTimer, bool* pBlinkState)
	{
		if (!SWTypeExt::TempSuper || !SWTimerTemp::SuperExt || !SWTimerTemp::SuperExt->ChargeTimer.Get()) {
			TacticalClass::PrintTimer(arg1, scheme, interval, string, pBlinkTimer, pBlinkState);
			return;
		}

		auto const pFont = BitFont::BitFontPtr(
		TextPrintType::UseGradPal |
		TextPrintType::Right |
		TextPrintType::NoShadow |
		TextPrintType::Metal12 |
		TextPrintType::Background);

		std::wstring lpDisplay = string;
		lpDisplay += L"  ";
		wchar_t Buffer[0x100];

		//do
		//{
			int nTimeLeft = SWTypeExt::TempSuper->RechargeTimer.GetTimeLeft();
			double nTimePrec = (((nTimeLeft * 1.0) / SWTypeExt::TempSuper->Type->RechargeTime) * 100.0);
			double nRec = !SWTimerTemp::SuperExt->ChargeTimer_Backwards.Get() ? (100.0 - nTimePrec) : abs(nTimePrec);
			int nRes = (int)nRec;
			//lpTime = std::move(std::format(L"{} {}", nRes, Phobos::UI::PercentLabel.c_str()));
			swprintf_s(Buffer,L"%03d %ls", nRes, Phobos::UI::PercentLabel);
		//}
		//while (false);

		int nTimerIndex = arg1;

		// Most code below were previded by @Secsome !
		int nTimeWidth;
		pFont->GetTextDimension(Buffer, &nTimeWidth, nullptr, DSurface::ViewBounds->Width);

		ColorScheme* pTimeScheme = scheme;

		if (!interval && pBlinkTimer && pBlinkState)
		{
			auto currentTime = Game::AudioGetTime();
			if (pBlinkTimer->QuadPart <= currentTime.QuadPart)
			{
				pBlinkTimer->QuadPart = currentTime.QuadPart + 1000;
				*pBlinkState = !*pBlinkState;
			}

			if (*pBlinkState)
				pTimeScheme = ColorScheme::Array->GetItem(ColorScheme::White);
		}

		++nTimerIndex;

		auto bounds = DSurface::ViewBounds();
		auto nFlag = TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background | TextPrintType::Fonts;
		Point2D location { (bounds.Width - (nTimeWidth-1) - 3) , (bounds.Height - nTimerIndex * (pFont->field_1C + 2)) };
		DSurface::Composite->DrawColorSchemeText(lpDisplay.c_str(), bounds, location,scheme, nullptr, nFlag);

		location.X +=(nTimeWidth-1);
		DSurface::Composite->DrawColorSchemeText(Buffer, bounds, location, pTimeScheme, nullptr, nFlag);

	}
}

DEFINE_JUMP(CALL,0x6D4A6B, GET_OFFSET(Timer::DrawTimer));
#endif
#pragma endregion

template<bool Add = false>
static void SortTimer(SuperClass* const pThis)
{
	bool bSucceeded = false;
	const char* pString {};

	if constexpr (Add)
	{
		pString = "add";
		bSucceeded = SuperClass::ShowTimers->AddItem(pThis);
	} else {
		pString = "remove";
		bSucceeded = SuperClass::ShowTimers->Remove(pThis);
	}

	if (bSucceeded) {
		if(!Phobos::Otamaa::ExeTerminated){
			std::sort(SuperClass::ShowTimers->begin(), SuperClass::ShowTimers->end(),
				[](const SuperClass* pRightSuper, const SuperClass* pLeftSuper) {
					const auto aExt = SWTypeExt::ExtMap.Find(pRightSuper->Type);
					const auto bExt = SWTypeExt::ExtMap.Find(pLeftSuper->Type);
					return aExt->SW_Priority.Get() > bExt->SW_Priority.Get();
				});
		}
	}
	else
	{
		Debug::Log("Failed to %s super[%s] Timer ! \n", pString, pThis->get_ID());
	}
}

//DEFINE_HOOK(0x50B172, HouseClass_RemoveSuper_SortTimer, 0x5)
//{
//	//GET(HouseClass*, pThis, EBP);
//	//GET_STACK(int, nSuperIdx, STACK_OFFS(0x20 , 0xC));
//
//	if (!Phobos::Otamaa::ExeTerminated) {
//		std::sort(SuperClass::ShowTimers->begin(), SuperClass::ShowTimers->end(),
//			[](const SuperClass* pRightSuper, const SuperClass* pLeftSuper) {
//				const auto aExt = SWTypeExt::ExtMap.Find(pRightSuper->Type);
//				const auto bExt = SWTypeExt::ExtMap.Find(pLeftSuper->Type);
//				return aExt->SW_Priority.Get() > bExt->SW_Priority.Get();
//			});
//	}
//
//	return 0x0;
//}
//
//DEFINE_HOOK(0x6CB1AD, SuperClass_DTOR_SortTimer, 0x6)
//{
//	GET(SuperClass*, pThis, ESI);
//	SortTimer(pThis);
//	return 0x6CB1F6;
//}



//DEFINE_HOOK(0x6CB5D2, SuperClass_Grant_AddToShowTimer, 0x9)
//{
//	GET(SuperClass*, pThis, ESI);
//
//	enum { SkipGameCode = 0x6CB63E };
//
//	if (pThis->Type->ShowTimer && !pThis->Owner->Type->MultiplayPassive)
//	{
//		SuperClass::ShowTimers->AddItem(pThis);
//
//		const auto pTypeExt = SWTypeExt::ExtMap.Find(pThis->Type);
//		int priority = pTypeExt->SW_Priority;
//		int size = SuperClass::ShowTimers->Count;
//
//		for (int i = 0; i < size; i++)
//		{
//			int otherPriority = SWTypeExt::ExtMap.Find(SuperClass::ShowTimers->GetItem(i)->Type)->SW_Priority;
//
//			if (priority > otherPriority)
//			{
//				std::swap(SuperClass::ShowTimers->Items[i], SuperClass::ShowTimers->Items[size - 1]);
//
//				for (int j = i + 1; j < size - 1; j++)
//				{
//					std::swap(SuperClass::ShowTimers->Items[j], SuperClass::ShowTimers->Items[size - 1]);
//				}
//
//				break;
//			}
//		}
//	}
//
//	return SkipGameCode;
//}