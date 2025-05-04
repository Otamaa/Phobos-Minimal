#include <CCToolTip.h>

enum class CommandBarTypes
{
	none = -1,
	Team01,
	Team02,
	Team03,
	TypeSelect,
	Deploy,
	AttackMove,
	Guard,
	Beacon,
	Stop,
	PlanningMode,
	Cheer,
	Test
};

static COMPILETIMEEVAL reference<ShapeButtonClass, 0xB0C1C0, 25u> CommandBarButtons {};
static COMPILETIMEEVAL reference<int, 0xB0CB78, 25u> CommandBarLinks {};
static COMPILETIMEEVAL reference<const char*, 0x8427D0, 11u> CommandBarNames {};
//constexpr const char* CommandBarNames[] = { "Team01" ,"Team02" ,"Team03" ,"TypeSelect" ,"Deploy" ,"AttackMove" , "Guard" ,"Beacon" ,"Stop" ,"PlanningMode" , "Cheer" };

static COMPILETIMEEVAL reference<ShapeButtonClass, 0xB0CCB0> TabThumbButtonActivated {};
static COMPILETIMEEVAL reference<ShapeButtonClass, 0xB0CC40> TabThumbButtonDeactivated {};
static COMPILETIMEEVAL reference<int, 0xB0CB20, 7u> ActiveCommandBarButtons {};

static COMPILETIMEEVAL constant_ptr<char, 0x842838> Tip_ThumbClosed {};
static COMPILETIMEEVAL constant_ptr<char, 0x842848> Tip_ThumbOpen {};
static COMPILETIMEEVAL constant_ptr<char, 0x842858> Tip_TypeSelect {};
static COMPILETIMEEVAL constant_ptr<char, 0x842868> Tip_Team03 {};
static COMPILETIMEEVAL constant_ptr<char, 0x842874> Tip_Team02 {};
static COMPILETIMEEVAL constant_ptr<char, 0x842880> Tip_Team01 {};
static COMPILETIMEEVAL constant_ptr<char, 0x84288C> Tip_Stop {};
static COMPILETIMEEVAL constant_ptr<char, 0x842898> Tip_PlanningMode {};
static COMPILETIMEEVAL constant_ptr<char, 0x8428AC> Tip_Guard {};
static COMPILETIMEEVAL constant_ptr<char, 0x8428B8> Tip_Deploy {};
static COMPILETIMEEVAL constant_ptr<char, 0x8428C4> Tip_Cheer {};
static COMPILETIMEEVAL constant_ptr<char, 0x8428D0> Tip_Beacon {};
static COMPILETIMEEVAL constant_ptr<char, 0x8428DC> Tip_AttackMove {};

static COMPILETIMEEVAL reference<int, 0xB0CD24> AttackMove_Index {};
static COMPILETIMEEVAL reference<int, 0xB0CB3C> Beacon_Index {};
static COMPILETIMEEVAL reference<int, 0xB0C1B8> Cheer_Index {};
static COMPILETIMEEVAL reference<int, 0xB0CB20> Deploy_Index {};
static COMPILETIMEEVAL reference<int, 0xB0CB68> Guard_Index {};
static COMPILETIMEEVAL reference<int, 0xB0CC1C> PlanningMode_Index {};
static COMPILETIMEEVAL reference<int, 0xB0CB6C> Stop_Index {};
static COMPILETIMEEVAL reference<int, 0xB0CC20> Team01_Index {};
static COMPILETIMEEVAL reference<int, 0xB0CC28> Team02_Index {};
static COMPILETIMEEVAL reference<int, 0xB0CD28> Team03_Index {};
static COMPILETIMEEVAL reference<int, 0xB0CB38> TypeSelect_Index {};

static COMPILETIMEEVAL reference<SHPStruct*, 0xB0C148, 25u> CommandBarButtonShapes {};
static COMPILETIMEEVAL reference<bool, 0xB0CBDC, 25u> CommandBarButtonShapesLoaded {};

static COMPILETIMEEVAL constant_ptr<char, 0x842828> Button__SHP {};

static COMPILETIMEEVAL reference<RectangleStruct*, 0xB0FC58> rect_B0FC58 {};
static COMPILETIMEEVAL reference<bool, 0xB0CD40> byte_B0CD40 {};
static COMPILETIMEEVAL reference<int, 0xB0CBF8> dword_B0CBF8 {};
static COMPILETIMEEVAL reference<int, 0xB0CBFC> dword_B0CBFC {};
static COMPILETIMEEVAL reference<int, 0xB0CD44> dword_B0CD44 {};
static COMPILETIMEEVAL reference<Surface*, 0xB0CC00, 4> dword_B0CC00 {};
static COMPILETIMEEVAL reference<int, 0xB0CC38> dword_B0CC38 {};
static COMPILETIMEEVAL reference<int, 0xB0CC24> dword_B0CC24 {};
static COMPILETIMEEVAL reference<RectangleStruct*, 0xB0FC64> tabclassrect_B0FC64 {};
static COMPILETIMEEVAL reference<RectangleStruct*, 0xB0FC68> rect_B0FC68 {};

class NOVTABLE FakeTabClass final : TabClass
{
public:

	// this thing bit complicated since it linked By gadget IDS
	// 6AC210
	wchar_t* _GetToooltipMessage()
	{
		//
		return L"Missing";
	}

	static void _InitCommandBarShapes()
	{

		char filename[256] {};
		for (size_t i = 0; i < CommandBarButtonShapes.size(); ++i)
		{
			sprintf(filename, Button__SHP, i);
			CommandBarButtonShapes[i] = (SHPStruct*)FileSystem::LoadWholeFileEx(filename, CommandBarButtonShapesLoaded[i]);
		}

		CommandBarButtonShapes[11] = CommandBarButtonShapes[0];
	}

	static void _DestroyCommandBarShapes()
	{
		for (size_t i = 0; i < CommandBarButtonShapes.size(); ++i)
		{
			if (CommandBarButtonShapesLoaded[i])
			{
				GameDelete(CommandBarButtonShapes[i]);
				CommandBarButtonShapesLoaded[i] = false;
				CommandBarButtonShapes[i] = nullptr;
			}
			else
			{
				CommandBarButtonShapes[i] = nullptr;
			}
		}
	}

	static void _InitDefaultIdx()
	{
		//start : 0x6CFE8E
		//this thing stupidly compare name ,..
	}

	static NOINLINE void _ParseButtonList(CCINIClass* pINI, const char* pSection)
	{
		static COMPILETIMEEVAL reference<const char*, 0x7F0CF0> ButtonList_str {};

		_InitAdvCommandBar();
		if (pINI->ReadString(pSection, ButtonList_str, Phobos::readDefval, Phobos::readBuffer))
		{
			int nCount = 0;
			char* context = nullptr;
			for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				Debug::LogInfo("Parsing Command List : {}", cur);
				CommandBarTypes idx = _GetCommandBarIndexByName(cur);
				if (idx != CommandBarTypes::none)
				{
					CommandBarLinks[(int)idx] = nCount;
				}
				++nCount;
			}

			TabClass::SetCommanbarRect(nCount);
		}
	}
	static NOINLINE CommandBarTypes _GetCommandBarIndexByName(const char* pName)
	{
		for (size_t i = 0; i < std::size(CommandBarNames); ++i)
		{
			if (IS_SAME_STR_N(CommandBarNames[i], pName))
			{
				return CommandBarTypes(i);
			}
		}

		static COMPILETIMEEVAL const char* add[] = { "Test" };

		for (size_t a = 0; a < std::size(add); ++a)
		{
			if (IS_SAME_STR_N(add[a], pName))
				return CommandBarTypes(a + std::size(CommandBarNames));
		}

		return CommandBarTypes::none;
	}

	static void _InitAdvCommandBar()
	{
		for (auto& link : CommandBarLinks)
			link = -1;
	}

	void _AddButtons()
	{
		for (auto& command : CommandBarButtons)
		{
			command.Zap();
			this->AddButton(&command);
		}
	}

	void _RemoveButtons()
	{
		for (auto& command : CommandBarButtons)
		{
			this->RemoveButton(&command);
		}
	}

	void _HideAdvCommand()
	{
		this->_RemoveButtons();

		for (auto& idx : CommandBarLinks)
		{
			if (auto pShape = _GetShapeButton(idx))
			{
				CCToolTip::Instance->Remove(pShape->ID);
			}
		}

		this->RemoveButton(TabThumbButtonActivated.operator->());

		CCToolTip::Instance->Remove(240);
	}

	void _ShowAdvCommand()
	{
		this->_AddButtons();
		this->AddButton(TabThumbButtonActivated.operator->());

		auto AttackMove = TabClass::GetCommandbarShape(AttackMove_Index);
		TabClass::LinkTooltip(AttackMove, "Tip:AttackMove");

		auto Beacon = TabClass::GetCommandbarShape(Beacon_Index);
		TabClass::LinkTooltip(Beacon, "Tip:Beacon");

		auto Cheer = TabClass::GetCommandbarShape(Cheer_Index);
		TabClass::LinkTooltip(Cheer, "Tip:Cheer");

		auto test = TabClass::GetCommandbarShape(std::size(CommandBarButtons));
		TabClass::LinkTooltip(test, "Tip:Fuck");

		auto Deploy = TabClass::GetCommandbarShape(Deploy_Index);
		TabClass::LinkTooltip(Deploy, "Tip:Deploy");

		auto Guard = TabClass::GetCommandbarShape(Guard_Index);
		TabClass::LinkTooltip(Guard, "Tip:Guard");

		auto PlanningMode = TabClass::GetCommandbarShape(PlanningMode_Index);
		TabClass::LinkTooltip(PlanningMode, "Tip:PlanningMode");

		auto Stop = TabClass::GetCommandbarShape(Stop_Index);
		TabClass::LinkTooltip(Stop, "Tip:Stop");

		auto Team01 = TabClass::GetCommandbarShape(Team01_Index);
		TabClass::LinkTooltip(Team01, "Tip:Team01");

		auto Team02 = TabClass::GetCommandbarShape(Team02_Index);
		TabClass::LinkTooltip(Team02, "Tip:Team02");

		auto Team03 = TabClass::GetCommandbarShape(Team03_Index);
		TabClass::LinkTooltip(Team03, "Tip:Team03");

		auto TypeSelect = TabClass::GetCommandbarShape(TypeSelect_Index);
		TabClass::LinkTooltip(TypeSelect, "Tip:TypeSelect");

		TabClass::LinkTooltip(TabThumbButtonActivated.operator->(), "Tip:ThumbOpen");
	}

	static NOINLINE ShapeButtonClass* __fastcall _GetShapeButton(int idx)
	{
		if (idx < 0 || idx >= 25)
			return nullptr;

		return &CommandBarButtons[idx];
	}

	static ShapeButtonClass* __fastcall _GetShapeButton2(int idx)
	{
		return _GetShapeButton(CommandBarLinks[idx]);
	}

	static NOINLINE SHPStruct* GetCommandButtonShape(int idx)
	{
		if (idx < 0 || idx >= 25)
			return nullptr;

		return CommandBarButtonShapes[idx];
	}

	static void InitAdvCommand()
	{
		for (auto& command : CommandBarButtons)
		{
			command.SetShape(nullptr, 0, 0);
		}

		int i = 0;
		int __val = tabclassrect_B0FC64->Width + tabclassrect_B0FC64->X;
		do
		{
			int idx_ = CommandBarLinks[i];
			if (auto pShpeBtn = _GetShapeButton(idx_))
			{
				int v5 = rect_B0FC68->X + dword_B0CC38 * idx_;
				int v6 = dword_B0CC24;

				if (v5 + rect_B0FC68->Width <= __val)
				{

					pShpeBtn->ID = i + 214;
					pShpeBtn->Drawer = FileSystem::SIDEBAR_PAL;
					pShpeBtn->IsOn = 0;
					pShpeBtn->ToggleType = 0;
					pShpeBtn->Flags = GadgetFlag::LeftPress | GadgetFlag::LeftRelease;
					pShpeBtn->SetPosition(v5, v6);
					pShpeBtn->SetShape(GetCommandButtonShape(i), 0, 0);
				}
			}

			++i;
		}
		while (i < 25);

		if (auto pShape = _GetShapeButton2(PlanningMode_Index))
		{
			pShape->ToggleType = 1;
			pShape->UseFlash = 1;
		}

		auto v9 = Team01_Index();
		if (Team01_Index <= Team03_Index)
		{
			auto v10 = &CommandBarLinks[Team01_Index];
			do
			{
				auto v11 = *v10;
				if (*v10 >= 0 && v11 < 25)
				{
					auto v12 = &CommandBarButtons[v11];
					if (v12)
					{
						v12->Flags = GadgetFlag(85);
					}
				}
				++v9;
				++v10;
			}
			while (v9 <= Team03_Index);
		}
	}
};

DEFINE_FUNCTION_JUMP(LJMP, 0x6CFD40, FakeTabClass::_GetShapeButton2));
DEFINE_FUNCTION_JUMP(LJMP, 0x6D1200, FakeTabClass::_ShowAdvCommand));
DEFINE_FUNCTION_JUMP(LJMP, 0x6D14F0, FakeTabClass::_HideAdvCommand));
DEFINE_FUNCTION_JUMP(LJMP, 0x6D0F70, FakeTabClass::_DestroyCommandBarShapes));
DEFINE_FUNCTION_JUMP(LJMP, 0x6D0F10, FakeTabClass::_InitCommandBarShapes));
DEFINE_FUNCTION_JUMP(LJMP, 0x6D04A0, FakeTabClass::_AddButtons));
DEFINE_FUNCTION_JUMP(LJMP, 0x6D04D0, FakeTabClass::_RemoveButtons));
DEFINE_FUNCTION_JUMP(LJMP, 0x6D0FD0, FakeTabClass::InitAdvCommand));

ASMJIT_PATCH(0x6D02C0, InitForHouse_RemoveInline, 0x5)
{
	FakeTabClass::_InitCommandBarShapes();
	return 0x6D0304;
}

ASMJIT_PATCH(0x6D1780, TabClass_noticeSink_Planning_TurnOn, 0x7)
{
	GET(int, index, EAX);
	if (auto pShape = FakeTabClass::_GetShapeButton(index))
		pShape->TurnOn();

	return 0x6D17AC;
}

ASMJIT_PATCH(0x6D17BE, TabClass_noticeSink_Planning_TurnOff, 0x7)
{
	GET(int, index, ECX);
	if (auto pShape = FakeTabClass::_GetShapeButton(index))
		pShape->TurnOff();

	return 0x6D17EA;
}

ASMJIT_PATCH(0x6D078C, TabClass_AI_Planning, 0x7)
{
	GET(int, index, ECX);
	R->EAX(FakeTabClass::_GetShapeButton(index));
	return 0x6D07AB;
}

ASMJIT_PATCH(0x67468B, RulesClass_AdcancedCommandBar_Parse, 0x6)
{
	GET(CCINIClass*, pINI, EBX);
	GET(const char*, pSection, EDI);
	FakeTabClass::_ParseButtonList(pINI, pSection);
	return 0x674710;
}

ASMJIT_PATCH(0x6D05CB, TabClass_Activate_RemoveInline, 0x6)
{
	GET(FakeTabClass*, pThis, EDI);
	pThis->_HideAdvCommand();
	return 0x6D0639;
}

ASMJIT_PATCH(0x6D1674, TabClass_ToggleThumb_RemoveInline, 0x6)
{
	GET(FakeTabClass*, pThis, EDI);
	pThis->_HideAdvCommand();
	return 0x6D16E2;
}

ASMJIT_PATCH(0x6D0D5A, TabClass_DrawIt_DrawCommandBar2, 0x5)
{
	GET(int, index, EAX);
	R->ESI(FakeTabClass::_GetShapeButton(index));
	return 0x6D0D6B;
}

ASMJIT_PATCH(0x6D0A87, TabClass_DrawIt_DrawCommandBar1, 0x5)
{
	GET(int, index, EAX);
	R->EAX(FakeTabClass::_GetShapeButton(index));
	return 0x6D0A97;
}

bool WhiteColorSearchedG = false;
int ColorIdxG = 5;

ASMJIT_PATCH(0x6D07E4, TabClass_AI_AdditionalAffect, 0x6)
{
	GET(int, index, EAX);
	if (index == 11)
	{
		if (!WhiteColorSearchedG)
		{
			const auto WhiteIndex = ColorScheme::FindIndex("White", 53);

			if (WhiteIndex != -1)
			{
				ColorIdxG = WhiteIndex;
			}

			WhiteColorSearchedG = true;
		}

		MessageListClass::Instance->PrintMessage(L"Hello world!", 600, ColorIdxG, true);
		return 0x6D0827;
	}

	return 0x0;
}