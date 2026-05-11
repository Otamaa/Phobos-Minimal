#include "Body.h"

#include <CCToolTip.h>
#include <EventClass.h>
#include <ShapeButtonClass.h>

#include <Helpers/Macro.h>

#include <FactoryClass.h>
#include <Misc/PhobosToolTip.h>

#include <Ext/SWType/Body.h>
#include <Ext/Mouse/Body.h>

#include <Utilities/Cast.h>

#ifndef STRIPS

#ifndef _newIpml
static FORCEDINLINE void DoStuffs(int idx, StripClass* pStrip, int height, int width, int y, SelectClass* pBegin)
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

ASMJIT_PATCH(0x6A8220, StripClass_Initialize, 7)
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

ASMJIT_PATCH(0x6ABFB2, SidebarClass_InitGUI_Strip2, 0x6)
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

#else

static COMPILETIMEEVAL constant_ptr<SelectClass, 0xB07E80> const ButtonsPtr {};
static COMPILETIMEEVAL constant_ptr<SelectClass, 0xB0B300> const Buttons_endPtr {};

ASMJIT_PATCH(0x6ABFB2, SidebarClass_InitGUI_Strip2, 0x6)
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
ASMJIT_PATCH(0x6a96d9, StripClass_Draw_Strip, 7)
{
	GET(DWORD*, pSomething, EDI);
	GET(int, idx_first, ECX);
	GET(int, idx_Second, EDX);
	R->EAX(reinterpret_cast<SelectClass*>(56 * (idx_Second + 2 * idx_first) + ButtonsPtr.getAddrs()));
	return *reinterpret_cast<bool*>((((BYTE*)pSomething) + 0x3fu)) != 0 ? 0x6A9703 : 0x6A9714;
}

#endif

ASMJIT_PATCH(0x6AC02F, SidebarClass_InitGUI_Strip3, 0x8)
{
	GET_STACK(size_t, nCurIdx, 0x14);
	COMPILETIMEEVAL int Offset = 0x3E8;

	for (int i = 0; i < 0xF0; ++i)
		CCToolTip::Instance->Remove(i + Offset);

	if (nCurIdx > 0)
	{
		for (size_t a = 0; a < nCurIdx; ++a)
		{
			ToolTip _temp { a + Offset ,
				SidebarClass::SelectButtonCombined[a].Rect,
					nullptr,
				true };
			CCToolTip::Instance->Add(_temp);

		}
	}

	return 0x6AC0A7;
}

ASMJIT_PATCH(0x6ABF44, SidebarClass_InitGUI_Strip1, 0x5)
{
	R->ESI(SidebarClass::SelectButtonCombined.begin());
	return 0x6ABF49;
}

ASMJIT_PATCH(0x6A7EEE, SidebarClass_Active_Strip1, 0x6)
{
	GET(SidebarClass*, pThis, ESI);
	pThis->Tabs[pThis->ActiveTabIndex].Func_6A93F0_GScreenAddButton();
	return 0x6A7F9F;
}

ASMJIT_PATCH(0x6A801C, SidebarClass_Active_Strip2, 0x6)
{
	GET(SidebarClass*, pThis, ESI);
	pThis->Tabs[pThis->ActiveTabIndex].Deactivate();
	return 0x6A8061;
}

ASMJIT_PATCH(0x6A64C9, SidebarClass_AddCameo_Strip, 6)
{
	GET(SidebarClass*, pThis, EBX);
	GET(int, nStrip, EDI);
	pThis->ChangeTab(nStrip);
	return 0x6A65D6;
}

ASMJIT_PATCH(0x6A75B9, SidebarClass_SetActiveTab_Strip1, 6)
{
	GET(SidebarClass*, pThis, EBP);
	pThis->Tabs[pThis->ActiveTabIndex].Func_6A94B0_GScreenRemoveButton();
	return 0x6A7602;
}

ASMJIT_PATCH(0x6A7619, SidebarClass_SetActiveTab_Strip2, 6)
{
	GET(SidebarClass*, pThis, EBP);
	pThis->Tabs[pThis->ActiveTabIndex].Func_6A93F0_GScreenAddButton();
	return 0x6A76CA;
}

ASMJIT_PATCH(0x6A793F, SidebarClass_Update_Strip1, 6)
{
	GET(SidebarClass*, pThis, ESI);
	pThis->Tabs[pThis->ActiveTabIndex].Func_6A94B0_GScreenRemoveButton();
	return 0x6A7988;
}

ASMJIT_PATCH(0x6A79A0, SidebarClass_Update_Strip2, 6)
{
	GET(SidebarClass*, pThis, ESI);
	pThis->Tabs[pThis->ActiveTabIndex].Func_6A93F0_GScreenAddButton();
	return 0x6A7A51;
}

#endif

#ifndef CAMEOS_


ASMJIT_PATCH(0x6A4EA5, SidebarClass_CameosList, 6)
{
	MouseClassExt::ClearCameos();
	return 0;
}

ASMJIT_PATCH_AGAIN(0x6A4FD8, SidebarClass_CameosList, 6)


ASMJIT_PATCH(0x6A633D, SidebarClass_AddCameo_TabIndex, 0x5)
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

static COMPILETIMEEVAL NOINLINE BuildType* lower_bound(BuildType* first, int size, const BuildType& x)
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


// pointer #1
ASMJIT_PATCH(0x6A8D1C, StripClass_MouseMove_GetCameos1, 7)
{
	GET(int, CameoCount, EAX);

	GET(StripClass*, pTab, EBX);

	if (CameoCount <= 0)
	{
		return 0x6A8D8B;
	}

	R->EDI<BuildType*>(MouseClassExt::TabCameos[pTab->TabIndex].data());
	return 0x6A8D23;
}

// pointer #2
ASMJIT_PATCH(0x6A8DB5, StripClass_MouseMove_GetCameos2, 8)
{
	GET(int, CameoCount, EAX);
	GET(StripClass*, pTab, EBX);

	if (CameoCount <= 0)
	{
		return 0x6A8F64;
	}

	auto ptr = reinterpret_cast<byte*>(MouseClassExt::TabCameos[pTab->TabIndex].data());
	ptr += 0x10;
	R->EBP<byte*>(ptr);

	return 0x6A8DC0;
}

// pointer #3
ASMJIT_PATCH(0x6A8F6C, StripClass_MouseMove_GetCameos3, 9)
{
	GET(StripClass*, pTab, ESI);

	if (pTab->BuildableCount <= 0)
	{
		return 0x6A902D;
	}

	auto ptr = reinterpret_cast<byte*>(MouseClassExt::TabCameos[pTab->TabIndex].data());
	ptr += 0x1C;
	R->ESI<byte*>(ptr);
	R->EBP<int>(R->Stack<int>(0x20));

	return 0x6A8F7C;
}

#include <Ext/Rules/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/BuildingType/Body.h>

ASMJIT_PATCH(0x4F92DD, HouseClass_Update_RedrawSidebarWhenRecheckTechTree, 0x5)
{
	SidebarClass::Instance->SidebarBackgroundNeedsRedraw = true;
	return 0;
}

ASMJIT_PATCH(0x6ABBCB, StripClass_AbandonCameosFromFactory_GetPointer1, 7)
{
	GET(int, BuildableCount, EAX);
	GET(StripClass*, pTab, ESI);

	if (BuildableCount <= 0)
	{
		return 0x6ABC2F;
	}

	auto ptr = reinterpret_cast<byte*>(MouseClassExt::TabCameos[pTab->TabIndex].data());
	ptr += 0xC;
	R->ESI<byte*>(ptr);

	return 0x6ABBD2;
}

ASMJIT_PATCH(0x6AC67A, SidebarClass_FlashCameo_FixLimit, 5)
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
			cameo.FlashEndFrame = Unsorted::CurrentFrame.get() + Duration;
			break;
		}
	}

	return 0x6AC71A;
}


#endif
