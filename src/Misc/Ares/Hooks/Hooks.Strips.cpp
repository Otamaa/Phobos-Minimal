#include "Header.h"

#include <CCToolTip.h>
#include <EventClass.h>
#include <ShapeButtonClass.h>

#include <Helpers/Macro.h>

#include <FactoryClass.h>
#include <Misc/PhobosToolTip.h>
#include <Ext/SWType/Body.h>

DEFINE_HOOK(0x6A9304, StripClass_GetTip_Handle, 9)
{
	GET(StripClass*, pThis, ECX);
	GET(int, buildableCount, EAX);

	auto& cameo = MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][buildableCount];
	PhobosToolTip::Instance.IsCameo = true;

	if (PhobosToolTip::Instance.IsEnabled())
	{
		PhobosToolTip::Instance.HelpText(&cameo);
		R->EAX(L"X");
		return 0x6A93DE;
	}

	if (cameo.ItemType == AbstractType::Special)
	{

		auto pSW = SuperWeaponTypeClass::Array->Items[cameo.ItemIndex];
		const auto pData = SWTypeExtContainer::Instance.Find(pSW);

		if (pData->Money_Amount < 0)
		{

			// account for no-name SWs
			if (CCToolTip::HideName() || !wcslen(pSW->UIName))
			{
				const wchar_t* pFormat = StringTable::LoadStringA(GameStrings::TXT_MONEY_FORMAT_1);
				_snwprintf_s(SidebarClass::TooltipBuffer(), SidebarClass::TooltipLength - 1, pFormat, -pData->Money_Amount);
			}
			else
			{
				// then, this must be brand SWs
				const wchar_t* pFormat = StringTable::LoadStringA(GameStrings::TXT_MONEY_FORMAT_2);
				_snwprintf_s(SidebarClass::TooltipBuffer(), SidebarClass::TooltipLength - 1, pFormat, pSW->UIName, -pData->Money_Amount);
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

			R->EAX(SidebarClass::TooltipBuffer());
			return 0x6A93DE;
		}
		else
		{
			R->EAX(pSW->UIName);
		}

		return 0x6A93DE;
	}
	else if (auto pTechnoType = TechnoTypeClass::GetByTypeAndIndex(cameo.ItemType, cameo.ItemIndex))
	{
		const int Cost = pTechnoType->GetActualCost(HouseClass::CurrentPlayer);

		if (CCToolTip::HideName || !wcslen(pTechnoType->UIName))
		{
			const wchar_t* Format = StringTable::LoadStringA(GameStrings::TXT_MONEY_FORMAT_1);
			_snwprintf_s(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, SidebarClass::TooltipLength - 1, Format, Cost);
		}
		else
		{
			const wchar_t* UIName = pTechnoType->UIName;
			const wchar_t* Format = StringTable::LoadStringA(GameStrings::TXT_MONEY_FORMAT_2);
			_snwprintf_s(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, SidebarClass::TooltipLength - 1, Format, UIName, Cost);
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

		R->EAX(SidebarClass::TooltipBuffer());
		return 0x6A93DE;
	}

	return 0x6A93E2;
}

#ifndef STRIPS

#ifndef _newIpml
static FORCEINLINE void DoStuffs(int idx, StripClass* pStrip, int height, int width, int y, SelectClass* pBegin)
{
	int MaxShown = SidebarClass::Instance->Func_6AC430();

	if (MaxShown)
	{
		int a = 0;
		auto i = (pBegin);
		do
		{
			i->Index = a;
			i->ID = 202;
			i->Strip = pStrip;
			const auto nY_stuff = height * (a & 0xFFFFFFFE);
			const auto  nX_stuff = a++ & 1;
			i->Rect.X = pStrip->Location.X + width + nX_stuff;
			i->Rect.Y = y + nY_stuff;
			i->Rect.Height = 60;
			i->Rect.Width = 48;

			i++;

		}
		while (i != (MaxShown + pBegin));
	}
}

DEFINE_HOOK(0x6A8220, StripClass_Initialize, 7)
{
	GET(StripClass*, pThis, ECX);
	GET_STACK(int, nIdx, 0x4);

	pThis->TabIndex = nIdx;
	auto nInc_y = pThis->Location.X + 1;
	DoStuffs(nIdx,
		pThis,
		SidebarClass::ObjectHeight(),
		SidebarClass::ObjectWidth(),
		nInc_y,
		SidebarClass::SelectButtonCombined.begin()
	);
	return 0x6A8329;
}

int __fastcall SidebarClass_6AC430(SidebarClass*)
{
	JMP_THIS(0x6AC430)
}

// yeah , fuck it
// i cant reproduce the exact code
// so lets just dump the assembly code instead , lmao
// -otamaa
//declhook(0x6A8220, StripClass_Initialize, 0x7)
//extern "C" __declspec(naked, dllexport) DWORD __cdecl StripClass_Initialize(REGISTERS* R)
//{
//	__asm
//	{
//		push ecx
//		mov eax, [esp + 0x8]
//		mov ecx, 0x87F7E8
//		push ebx
//		push ebp
//		push esi
//		mov ebx, [eax + 0x20]
//		xor esi, esi
//		mov eax, [eax + 0x14]
//		push edi
//		mov ebp, [ebx + 0x24]
//		mov eax, [eax + 0x4]
//		inc ebp
//		mov[ebx + 0x38], eax
//		mov eax, [ebx + 0x20]
//		mov[esp + 0x18], eax
//		call SidebarClass_6AC430
//		lea edi, ds : 0[eax * 8]
//		sub edi, eax
//		shl edi, 3
//		mov[esp + 0x10], edi
//		test edi, edi
//		jz short retfunc_
//		mov ecx, dword ptr ds : 0xB0B500
//		mov eax, 0xB07E94
//		lea ebx, [ebx + 0x0]
//		loopfunc_ :
//		mov[eax + 0x1C], esi
//		mov edx, esi
//		mov dword ptr[eax + 0x10], 0xCA
//		and edx, 0xFFFFFFFE
//		mov[eax + 0x18], ebx
//		mov edi, dword ptr ds : 0xB0B4FC
//		imul edx, ecx
//		mov ecx, esi
//		and ecx, 1
//		inc esi
//		imul ecx, edi
//		add edx, ebp
//		add ecx, [esp + 0x18]
//		mov[eax - 8], ecx
//		mov ecx, [esp + 0x10]
//		mov[eax - 4], edx
//		add ecx, 0xB07E94
//		mov dword ptr[eax], 0x3C
//		mov dword ptr[eax + 4], 0x30
//		add eax, 0x38
//		cmp eax, ecx
//		lea ecx, [edx + 4]
//		jnz loopfunc_
//		retfunc_ :
//		pop edi
//			pop esi
//			pop ebp
//			mov eax, 0x6A8329
//			pop ebx
//			pop ecx
//			retn
//	}
//}

DEFINE_HOOK(0x6ABFB2, sub_6ABD30_Strip2, 0x6)
{
	enum
	{
		ContinueLoop = 0x6ABF66,
		BreakLoop = 0x6ABFC4,
	};

	GET(DWORD, pPtr, ESI);
	const DWORD pCur = pPtr + 0x3480;
	R->ESI(pCur);
	R->Stack(0x10, pCur);
	return (DWORD)pCur < (DWORD)SidebarClass::SelectButtonCombined.end() ?
		ContinueLoop : BreakLoop;
}

DEFINE_HOOK(0x6a96d9, StripClass_Draw_Strip, 7)
{
	GET(StripClass*, pThis, EDI);
	GET(int, idx_first, ECX);
	GET(int, idx_Second, EDX);
	R->EAX(&SidebarClass::SelectButtonCombined[idx_Second + 2 * idx_first]);
	return pThis->IsScrolling ? 0x6A9703 : 0x6A9714;
}
#else

static constexpr constant_ptr<SelectClass, 0xB07E80> const ButtonsPtr {};
static constexpr constant_ptr<SelectClass, 0xB0B300> const Buttons_endPtr {};

DEFINE_HOOK(0x6ABFB2, sub_6ABD30_Strip2, 0x6)
{
	enum
	{
		ContinueLoop = 0x6ABF66,
		BreakLoop = 0x6ABFC4,
	};

	GET(DWORD, pPtr, ESI);
	const DWORD pCur = pPtr + 0x3480;
	R->ESI(pCur);
	R->Stack(0x10, pCur);
	return (DWORD)pCur < Buttons_endPtr.getAddrs() ?
		ContinueLoop : BreakLoop;
}

//duuunno
DEFINE_HOOK(0x6a96d9, StripClass_Draw_Strip, 7)
{
	GET(DWORD*, pSomething, EDI);
	GET(int, idx_first, ECX);
	GET(int, idx_Second, EDX);
	R->EAX(reinterpret_cast<SelectClass*>(56 * (idx_Second + 2 * idx_first) + ButtonsPtr.getAddrs()));
	return *reinterpret_cast<bool*>((((BYTE*)pSomething) + 0x3fu)) != 0 ? 0x6A9703 : 0x6A9714;
}

#endif

DEFINE_HOOK(0x6AC02F, sub_6ABD30_Strip3, 0x8)
{
	GET_STACK(size_t, nCurIdx, 0x14);
	constexpr int Offset = 0x3E8;

	for (int i = 0; i < 0xF0; ++i)
		CCToolTip::Instance->Remove(i + Offset);

	if (nCurIdx > 0)
	{
		for (size_t a = 0; a < nCurIdx; ++a)
		{

			CCToolTip::Instance->Add(ToolTip { a + Offset ,
				SidebarClass::SelectButtonCombined[a].Rect,
					nullptr,
				true });

		}
	}

	return 0x6AC0A7;
}

DEFINE_HOOK(0x6a9822, StripClass_Draw_Power, 5)
{
	GET(FactoryClass*, pFactory, ECX);

	bool IsDone = pFactory->IsDone();

	if (IsDone)
	{
		if (auto pBuilding = cast_to<BuildingClass*, false>(pFactory->Object))
		{
			IsDone = pBuilding->FindFactory(true, true) != nullptr;
		}
	}

	R->EAX(IsDone);
	return 0x6A9827;
}

DEFINE_HOOK(0x6A83E0, StripClass_DisableInput, 6)
{
	for (auto begin = SidebarClass::SelectButtonCombined.begin();
		begin != SidebarClass::SelectButtonCombined.end();
		++begin)
		GScreenClass::Instance->RemoveButton(begin);

	return 0x6A8415;
}

DEFINE_HOOK(0x6A8330, StripClass_EnableInput, 5)
{
	GET(StripClass*, pThis, ECX);

	int const nIdx = SidebarClass::Instance->Func_6AC430();
	for (auto i = SidebarClass::SelectButtonCombined.begin();
		i != (&SidebarClass::SelectButtonCombined[nIdx]);
		++i)
	{
		(*i).Zap();
		(*i).Strip = pThis;
		GScreenClass::Instance->AddButton(i);
	}

	CCToolTip::Bound = true;
	return 0x6A83DA;
}

DEFINE_HOOK(0x6ABF44, sub_6ABD30_Strip1, 0x5)
{
	R->ESI(SidebarClass::SelectButtonCombined.begin());
	return 0x6ABF49;
}

DEFINE_HOOK(0x6A7EEE, sub_6A7D70_Strip1, 0x6)
{
	GET(SidebarClass*, pThis, ESI);
	pThis->Tabs[pThis->ActiveTabIndex].Func_6A93F0_GScreenAddButton();
	return 0x6A7F9F;
}

DEFINE_HOOK(0x6A801C, sub_6A7D70_Strip2, 0x6)
{
	GET(SidebarClass*, pThis, ESI);
	pThis->Tabs[pThis->ActiveTabIndex].Deactivate();
	return 0x6A8061;
}

DEFINE_HOOK(0x6A64C9, SidebarClass_AddCameo_Strip, 6)
{
	GET(SidebarClass*, pThis, EBX);
	GET(int, nStrip, EDI);
	pThis->ChangeTab(nStrip);
	return 0x6A65D6;
}

DEFINE_HOOK(0x6A75B9, SidebarClass_SetActiveTab_Strip1, 6)
{
	GET(SidebarClass*, pThis, EBP);
	pThis->Tabs[pThis->ActiveTabIndex].Func_6A94B0_GScreenRemoveButton();
	return 0x6A7602;
}

DEFINE_HOOK(0x6A7619, SidebarClass_SetActiveTab_Strip2, 6)
{
	GET(SidebarClass*, pThis, EBP);
	pThis->Tabs[pThis->ActiveTabIndex].Func_6A93F0_GScreenAddButton();
	return 0x6A76CA;
}

DEFINE_HOOK(0x6A793F, SidebarClass_Update_Strip1, 6)
{
	GET(SidebarClass*, pThis, ESI);
	pThis->Tabs[pThis->ActiveTabIndex].Func_6A94B0_GScreenRemoveButton();
	return 0x6A7988;
}

DEFINE_HOOK(0x6A79A0, SidebarClass_Update_Strip2, 6)
{
	GET(SidebarClass*, pThis, ESI);
	pThis->Tabs[pThis->ActiveTabIndex].Func_6A93F0_GScreenAddButton();
	return 0x6A7A51;
}

DEFINE_HOOK(0x6A93F0, StripClass_Activate, 6)
{
	GET(StripClass*, pThis, ECX);
	pThis->AllowedToDraw = true;
	pThis->Activate();
	return 0x6A94A0;
}

DEFINE_HOOK(0x6A94B0, StripClass_Deactivate, 6)
{
	GET(StripClass*, pThis, ECX);
	pThis->AllowedToDraw = false;
	pThis->Deactivate();
	return 0x6A94E9;
}
#endif

#ifndef CAMEOS_

DEFINE_HOOK_AGAIN(0x6A4FD8, SidebarClass_CameosList, 6)
DEFINE_HOOK(0x6A4EA5, SidebarClass_CameosList, 6)
{
	MouseClassExt::ClearCameos();
	return 0;
}

DEFINE_HOOK(0x6A6140, SidebarClass_FactoryLink_handle, 0x5)
{
	GET(SidebarClass*, pThis, ECX);
	GET_STACK(FactoryClass*, pFactory, 0x4);
	GET_STACK(AbstractType, rtti, 0x8);
	GET_STACK(int, typeIdx, 0xC);

	bool found = false;
	const int TabIndex = SidebarClass::GetObjectTabIdx(rtti, typeIdx, 0);
	auto& Tab = MouseClass::Instance->Tabs[TabIndex];

	for (auto& cameo : MouseClassExt::TabCameos[TabIndex])
	{
		if (cameo.ItemIndex == typeIdx && cameo.ItemType == rtti)
		{
			cameo.CurrentFactory = pFactory;
			Tab.NeedsRedraw = 1;
			Tab.IsBuilding = 1;
			MouseClass::Instance->RedrawSidebar(0);
			found = true;
			break;
		}
	}

	R->AL(found);
	return 0x6A6215;
}

DEFINE_HOOK(0x6A633D, SidebarClass_AddCameo_TabIndex, 0x5)
{
	enum { AlreadyExists = 0x6A65FF, NewlyAdded = 0x6A63FD };

	GET(AbstractType const, absType, ESI);
	GET(int const, typeIdx, EBP);

	const auto TabIndex = SidebarClass::GetObjectTabIdx(absType, typeIdx, 0);
	R->Stack(0x18, TabIndex);

	// don't check for 75 cameos in active tab
	for (auto const& cameo : MouseClassExt::TabCameos[TabIndex])
	{
		if (cameo.ItemIndex == typeIdx && cameo.ItemType == absType)
		{
			return AlreadyExists;
		}
	}

	R->EDI<StripClass*>(&MouseClass::Instance->Tabs[TabIndex]);
	return NewlyAdded;
}

DEFINE_HOOK(0x6A61B1, SidebarClass_SetFactoryForObject, 5)
{
	enum { Found = 0x6A6210, NotFound = 0x6A61E6 };

	GET(int, TabIndex, EAX);
	GET(AbstractType, ItemType, EDI);
	GET(int, ItemIndex, EBP);
	GET_STACK(FactoryClass*, Factory, 0x10);

	for (auto& cameo : MouseClassExt::TabCameos[TabIndex])
	{
		if (cameo.ItemIndex == ItemIndex && cameo.ItemType == ItemType)
		{
			cameo.CurrentFactory = Factory;
			auto& Tab = MouseClass::Instance->Tabs[TabIndex];
			Tab.NeedsRedraw = 1;
			Tab.IsBuilding = 1;
			MouseClass::Instance->RedrawSidebar(0);
			return Found;
		}
	}

	return NotFound;
}

static constexpr NOINLINE BuildType* lower_bound(BuildType* first, int size, const BuildType& x)
{
	BuildType* it;
	typename std::iterator_traits<BuildType*>::difference_type count, step;
	count = size;

	while (count > 0)
	{
		it = first;
		step = count / 2;
		std::advance(it, step);

		if (*it < x)
		{
			first = ++it;
			count -= step + 1;
		}
		else
			count = step;
	}

	return first;
}

DEFINE_HOOK(0x6A8710, StripClass_AddCameo_ReplaceItAll, 6)
{
	GET(StripClass*, pTab, ECX);
	GET_STACK(AbstractType, ItemType, 0x4);
	GET_STACK(int, ItemIndex, 0x8);

	BuildType newCameo(ItemIndex, ItemType);
	if (ItemType == BuildingTypeClass::AbsID)
	{
		newCameo.Cat = ObjectTypeClass::IsBuildCat5(BuildingTypeClass::AbsID, ItemIndex);
	}

	auto& cameo = MouseClassExt::TabCameos[pTab->TabIndex];
	auto lower = lower_bound(cameo.begin(), cameo.Count, newCameo);
	int idx = cameo.IsInitialized ? std::distance(cameo.begin(), lower) : 0;

	if (cameo.IsValidArray())
	{
		BuildType* added = cameo.Items + idx;
		BuildType* added_plusOne = std::next(added);
		std::memcpy(added_plusOne, added, (char*)(cameo.end()) - ((char*)added));
		cameo.Items[idx] = std::move_if_noexcept(newCameo);
		++cameo.Count;
	}

	++pTab->BuildableCount;

	return 0x6A87E7;
}

// pointer #1
DEFINE_HOOK(0x6A8D1C, StripClass_MouseMove_GetCameos1, 7)
{
	GET(int, CameoCount, EAX);

	GET(StripClass*, pTab, EBX);

	if (CameoCount <= 0)
	{
		return 0x6A8D8B;
	}

	R->EDI<BuildType*>(MouseClassExt::TabCameos[pTab->TabIndex].Items);
	return 0x6A8D23;
}

// pointer #2
DEFINE_HOOK(0x6A8DB5, StripClass_MouseMove_GetCameos2, 8)
{
	GET(int, CameoCount, EAX);
	GET(StripClass*, pTab, EBX);

	if (CameoCount <= 0)
	{
		return 0x6A8F64;
	}

	auto ptr = reinterpret_cast<byte*>(MouseClassExt::TabCameos[pTab->TabIndex].Items);
	ptr += 0x10;
	R->EBP<byte*>(ptr);

	return 0x6A8DC0;
}

// pointer #3
DEFINE_HOOK(0x6A8F6C, StripClass_MouseMove_GetCameos3, 9)
{
	GET(StripClass*, pTab, ESI);

	if (pTab->BuildableCount <= 0)
	{
		return 0x6A902D;
	}

	auto ptr = reinterpret_cast<byte*>(MouseClassExt::TabCameos[pTab->TabIndex].Items);
	ptr += 0x1C;
	R->ESI<byte*>(ptr);
	R->EBP<int>(R->Stack<int>(0x20));

	return 0x6A8F7C;
}

DEFINE_HOOK(0x6A9747, StripClass_Draw_GetCameo, 6)
{
	GET(int, CameoIndex, ECX);

	auto& Item = MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][CameoIndex];

	auto ptr = reinterpret_cast<byte*>(&Item);
	ptr -= 0x58;
	R->EAX<byte*>(ptr);
	R->Stack<byte*>(0x30, ptr);

	R->ECX(Item.ItemType);

	return (Item.ItemType == AbstractType::Special)
		? 0x6A9936
		: 0x6A9761
		;
}

DEFINE_HOOK(0x6A95C8, StripClass_Draw_Status, 8)
{
	GET(int, CameoIndex, EAX);

	R->EDX<DWORD*>(&MouseClassExt::TabCameos
		[MouseClass::Instance->ActiveTabIndex]
		[CameoIndex].Status
	);

	return 0x6A95D3;
}

DEFINE_HOOK(0x6A9866, StripClass_Draw_Status_1, 8)
{
	GET(int, CameoIndex, ECX);

	return (MouseClassExt::TabCameos
		[MouseClass::Instance->ActiveTabIndex]
		[CameoIndex].Status == 1)
		? 0x6A9874
		: 0x6A98CF
		;
}

DEFINE_HOOK(0x6A9886, StripClass_Draw_Status_2, 8)
{
	GET(int, CameoIndex, EAX);

	auto ptr = reinterpret_cast<byte*>(
		&MouseClassExt::TabCameos
		[MouseClass::Instance->ActiveTabIndex]
		[CameoIndex]
	);

	ptr += 0x10;
	R->EDI<byte*>(ptr);

	auto dwPtr = reinterpret_cast<DWORD*>(ptr);
	R->EAX<DWORD>(*dwPtr);

	return 0x6A9893;
}

DEFINE_HOOK(0x6A9EBA, StripClass_Draw_Status_3, 8)
{
	GET(int, CameoIndex, EAX);

	return (MouseClassExt::TabCameos
		[MouseClass::Instance->ActiveTabIndex]
		[CameoIndex].Status == 2
		)
		? 0x6A9ECC
		: 0x6AA01C
		;
}

DEFINE_HOOK(0x6A99BE, StripClass_Draw_BreakDrawLoop, 5)
{
	R->Stack8(0x12, 0);
	return 0x6AA01C;
}

DEFINE_HOOK(0x6A9B4F, StripClass_Draw_TestFlashFrame, 6)
{
	GET(int, CameoIndex, EAX);

	R->EAX(Unsorted::CurrentFrame());
	return (MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex]
		[CameoIndex].FlashEndFrame > Unsorted::CurrentFrame
		)
		? 0x6A9B67
		: 0x6A9BC5
		;
}

DEFINE_HOOK(0x6AAD2F, SelectClass_ProcessInput_LoadCameo1, 7)
{
	GET(int, CameoIndex, ESI);

	auto& cameos = MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex];

	if (CameoIndex >= cameos.Count)
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

DEFINE_HOOK(0x6AB0B0, SelectClass_ProcessInput_LoadCameo2, 8)
{
	GET(int, CameoIndex, ESI);

	R->EAX<DWORD*>(&MouseClassExt::TabCameos
		[MouseClass::Instance->ActiveTabIndex]
		[CameoIndex].Status
	);

	return 0x6AB0BE;
}

DEFINE_HOOK(0x6AB49D, SelectClass_ProcessInput_FixOffset1, 7)
{
	R->EDI<void*>(nullptr);
	R->ECX<void*>(nullptr);
	return 0x6AB4A4;
}

DEFINE_HOOK(0x6AB4E8, SelectClass_ProcessInput_FixOffset2, 7)
{
	R->ECX<int>(R->Stack<int>(0x14));
	R->EDX<void*>(nullptr);
	return 0x6AB4EF;
}

DEFINE_HOOK(0x6AB577, SelectClass_ProcessInput_FixOffset3, 7)
{
	GET(int, CameoIndex, ESI);
	GET_STACK(FactoryClass*, SavedFactory, 0x18);

	auto& Item = MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][CameoIndex];

	Item.Status = 1;

	auto Progress = (Item.CurrentFactory)
		? Item.CurrentFactory->GetProgress()
		: 0
		;

	R->EAX<int>(Progress);
	R->EBP<void*>(nullptr);

	if (Item.Status == 1)
	{
		if (Item.Progress.Value > Progress)
		{
			Progress = (Item.Progress.Value + Progress) / 2;
		}
	}

	Item.Progress.Value = Progress;
	R->EAX<int>(SavedFactory->GetBuildTimeFrames());
	R->ECX<void*>(nullptr);

	return 0x6AB5C6;
}

DEFINE_HOOK(0x6AB620, SelectClass_ProcessInput_FixOffset4, 7)
{
	R->ECX<void*>(nullptr);
	return 0x6AB627;
}

DEFINE_HOOK(0x6AB741, SelectClass_ProcessInput_FixOffset5, 7)
{
	R->EDX<void*>(nullptr);
	return 0x6AB748;
}

DEFINE_HOOK(0x6AB802, SelectClass_ProcessInput_FixOffset6, 8)
{
	GET(int, CameoIndex, EAX);
	MouseClassExt::TabCameos[MouseClass::Instance->ActiveTabIndex][CameoIndex].Status = 1;
	return 0x6AB814;
}

DEFINE_HOOK(0x6AB825, SelectClass_ProcessInput_FixOffset7, 5)
{
	R->ECX<int>(R->EBP<int>());
	R->EDX<void*>(nullptr);

	return 0x6AB82A;
}

DEFINE_HOOK(0x6AB920, SelectClass_ProcessInput_FixOffset8, 7)
{
	R->ECX<void*>(nullptr);
	return 0x6AB927;
}

DEFINE_HOOK(0x6AB92F, SelectClass_ProcessInput_FixOffset9, 7)
{
	R->EBX<byte*>(R->EBX<byte*>() + 0x6C);
	return 0x6AB936;
}

DEFINE_HOOK(0x6ABBCB, StripClass_AbandonCameosFromFactory_GetPointer1, 7)
{
	GET(int, BuildableCount, EAX);
	GET(StripClass*, pTab, ESI);

	if (BuildableCount <= 0)
	{
		return 0x6ABC2F;
	}

	auto ptr = reinterpret_cast<byte*>(MouseClassExt::TabCameos[pTab->TabIndex].Items);
	ptr += 0xC;
	R->ESI<byte*>(ptr);

	return 0x6ABBD2;
}

DEFINE_HOOK(0x6AC67A, SidebarClass_FlashCameo_FixLimit, 5)
{
	GET(int const, typeIdx, ESI);
	GET(AbstractType const, absType, EAX);
	GET_STACK(int, Duration, 0x10);
	const auto TabIndex = SidebarClass::GetObjectTabIdx(absType, typeIdx, 0);

	// don't limit to 75
	for (auto& cameo : MouseClassExt::TabCameos[TabIndex])
	{
		if (cameo.ItemIndex == typeIdx)
		{
			cameo.FlashEndFrame = Unsorted::CurrentFrame + Duration;
			break;
		}
	}

	return 0x6AC71A;
}

bool NOINLINE RemoveCameo(BuildType* item)
{
	auto TechnoType = ObjectTypeClass::FetchTechnoType(item->ItemType, item->ItemIndex);
	bool removeCameo = true;
	if (TechnoType)
	{
		if (auto Factory = TechnoType->FindFactory(true, false, false, HouseClass::CurrentPlayer()))
		{
			removeCameo = Factory->Owner->CanBuild(TechnoType, false, true) == CanBuildResult::Unbuildable;
		}
	}
	else
	{

		auto& Supers = HouseClass::CurrentPlayer->Supers;
		if (Supers.ValidIndex(item->ItemIndex))
		{
			removeCameo = !Supers.Items[item->ItemIndex]->Granted;
		}
	}

	if (!removeCameo)
		return false;

	if (item->CurrentFactory)
	{
		EventClass Event {
			HouseClass::CurrentPlayer->ArrayIndex  ,
			EventType::ABANDON ,
			item->ItemType ,
			item->ItemIndex,
			bool(TechnoType ? TechnoType->Naval : 0)
		};

		EventClass::AddEvent(&Event);
	}

	if (item->ItemType == BuildingTypeClass::AbsID || item->ItemType == BuildingClass::AbsID)
	{
		MouseClass::Instance->CurrentBuilding = nullptr;
		MouseClass::Instance->CurrentBuildingType = nullptr;
		MouseClass::Instance->CurrentBuildingOwnerArrayIndex = 0xFFFFFFFF;
		MouseClass::Instance->SetActiveFoundation(nullptr);
	}

	if (TechnoType)
	{
		auto Me = TechnoType->WhatAmI();
		if (HouseClass::CurrentPlayer->GetPrimaryFactory(Me, TechnoType->Naval, BuildCat::DontCare))
		{
			EventClass Event {
				HouseClass::CurrentPlayer->ArrayIndex  ,
				EventType::ABANDON_ALL ,
				item->ItemType ,
				item->ItemIndex,
				TechnoType->Naval
			};

			EventClass::AddEvent(&Event);
		}
	}

	return true;
}

DEFINE_HOOK(0x6aa600, StripClass_RecheckCameos, 5)
{
	GET(StripClass*, pThis, ECX);

	if (Unsorted::ArmageddonMode || pThis->BuildableCount <= 0)
	{
		R->EAX(0);
		return 0x6AACAE;
	}

	auto& tabs = MouseClassExt::TabCameos[pThis->TabIndex];
	const auto rtt = tabs[pThis->TopRowIndex].ItemType;
	const auto idx = tabs[pThis->TopRowIndex].ItemIndex;

	auto removeIter = std::remove_if(tabs.begin(), tabs.end(), [=](BuildType& item)
 {
	 return RemoveCameo(&item);
	});

	const auto count_after = std::distance(tabs.begin(), removeIter);
	tabs.Reset(count_after);

	if (count_after >= pThis->BuildableCount)
	{
		R->EAX(0);
		return 0x6AACAE;
	}

	pThis->BuildableCount = count_after;
	if (count_after <= 0)
	{
		SidebarClass::ShapeButtons[pThis->TabIndex].Disable();

		StripClass* begin_c = SidebarClass::Column.begin();
		bool IsBreak = false;
		while ((*begin_c).BuildableCount <= 0)
		{
			if (++begin_c == SidebarClass::Column.end())
			{
				SidebarClass::Instance->ToggleStuffs();
				if (SidebarClass::Shape_B0B478())
				{
					SidebarClass::something_884B80 = -1;
					SidebarClass::something_884B7C = SidebarClass::Shape_B0B478->Height;
				}

				IsBreak = true;
				break;
			}
		}

		if (!IsBreak && pThis->TabIndex == SidebarClass::something_884B84())
			SidebarClass::Instance->ChangeTab(std::distance(SidebarClass::Column.begin(), begin_c));
	}
	else
	{
		SidebarClass::Instance->ToggleStuffs();
	}

	auto iter = lower_bound(tabs.begin(), tabs.Count, { idx , rtt });
	auto idxLower = std::distance(tabs.begin(), iter);
	auto buildCount = pThis->BuildableCount - SidebarClass::Instance->Func_6AC430();
	int value = (buildCount >= 0 ? buildCount : 0) / 2;

	if (value >= idxLower)
		value = idxLower;

	pThis->TopRowIndex = value;
	CCToolTip::Bound = true;
	R->EAX(1);
	return 0x6AACAE;
}

#endif
