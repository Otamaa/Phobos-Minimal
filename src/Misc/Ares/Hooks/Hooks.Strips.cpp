#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

#include <CCToolTip.h>

// IDB says so ,..
static constexpr reference2D<SelectClass*, 0xB07E80, 0x1u , 0xEu> const SelectsDataArr {};

//Ares used these
static constexpr constant_ptr<SelectClass, 0xB07E80> const SelectsDatas {};
static constexpr constant_ptr<SelectClass, 0xB0B300> const SelectsDatas_endPtr {};

DEFINE_OVERRIDE_HOOK(0x6A7EEE, sub_6A7D70_Strip1, 0x6)
{
	GET(SidebarClass*, pThis, ESI);
	pThis->Tabs[pThis->ActiveTabIndex].Func_6A93F0_GScreenAddButton();
	return 0x6A7F9F;
}

DEFINE_OVERRIDE_HOOK(0x6A801C, sub_6A7D70_Strip2, 0x6)
{
	GET(SidebarClass*, pThis, ESI);
	pThis->Tabs[pThis->ActiveTabIndex].Deactivate();
	return 0x6A8061;
}

// Doest work for some reason 
// the compiled code producing exacly same result like ares one
DEFINE_OVERRIDE_HOOK(0x6AC02F, sub_6ABD30_Strip3, 0x8)
{
	GET_STACK(size_t, nCurIdx, 0x14);
	int Offset = 0x3E8;

	for (int i = 0; i < 0xF0; ++i)
		CCToolTip::Instance->Remove(i + Offset);

	if (nCurIdx < 0)
		return 0x6AC0A7;
	
	auto selects = SelectsDataArr[0][0];
	for (size_t a = 0; a < nCurIdx; ++a) {

		ToolTip nToolTip {
			a + Offset ,
			selects->Rect,
			nullptr,
			true
		};

		CCToolTip::Instance->Add(nToolTip);
		++selects;
	}	

	return 0x6AC0A7;
}

DEFINE_OVERRIDE_HOOK(0x6ABFB2, sub_6ABD30_Strip2, 0x6)
{
	enum
	{
		ContinueLoop = 0x6ABF66,
		BreakLoop = 0x6ABFC4,
	};

	GET(DWORD, pPtr, ESI);
	const auto pCur = (pPtr + 0x3480);
	R->ESI(pCur);
	R->Stack(0x10, pCur);
	return pCur < SelectsDatas_endPtr.getAddrs() ?
		ContinueLoop : BreakLoop;
}

DEFINE_OVERRIDE_HOOK(0x6ABF44, sub_6ABD30_Strip1, 0x5)
{
	R->ESI<void*>(SelectsDatas());
	return 0x6ABF49;
}
