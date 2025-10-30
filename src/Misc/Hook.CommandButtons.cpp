#include <CCToolTip.h>
#include <FileFormats/SHP.h>
#include <ShapeButtonClass.h>
#include <CCFileClass.h>
#include <CCINIClass.h>

#include <MouseClass.h>

#include <Phobos.h>
#include <Syringe.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <MessageListClass.h>
#include <Phobos.Defines.h>

#include <Commands/DistributionMode.h>

#include <Utilities/Enum.h>

#define AS_STRING(name) #name,
#define AS_TIP(name) "Tip:" #name,

#define COMMAND_BAR_TYPES(X) \
    X(Team01) \
    X(Team02) \
    X(Team03) \
    X(TypeSelect) \
    X(Deploy) \
    X(AttackMove) \
    X(Guard) \
    X(Beacon) \
    X(Stop) \
    X(PlanningMode) \
    X(Cheer) \
    X(DistributionMode)

static COMPILETIMEEVAL const char* CommandBarTypes_ident[] = {
	COMMAND_BAR_TYPES(AS_STRING)
};

static COMPILETIMEEVAL const char* CommandBarTypes_Tip[] = {
	COMMAND_BAR_TYPES(AS_TIP)
};

#undef AS_STRING
#undef AS_TIP

static COMPILETIMEEVAL reference<ShapeButtonClass, 0xB0C1C0, 25u> CommandBarButtons {};

//link the button with the slot in sidebar
static COMPILETIMEEVAL reference<int, 0xB0CB78, 25u> CommandBarLinks {};

static COMPILETIMEEVAL reference<ShapeButtonClass, 0xB0CCB0> TabThumbButtonActivated {};
static COMPILETIMEEVAL reference<ShapeButtonClass, 0xB0CC40> TabThumbButtonDeactivated {};
static COMPILETIMEEVAL reference<int, 0xB0CB20, 7u> ActiveCommandBarButtons {};

static COMPILETIMEEVAL constant_ptr<char, 0x842838> Tip_ThumbClosed {};
static COMPILETIMEEVAL constant_ptr<char, 0x842848> Tip_ThumbOpen {};

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

	static void _InitCommandBarShapes()
	{
		char filename[256] {};
		for (size_t i = 0; i < CommandBarButtonShapes.size(); ++i)
		{
			sprintf(filename, Button__SHP, i);
			CommandBarButtonShapes[i] = (SHPStruct*)FileSystem::LoadWholeFileEx(filename, CommandBarButtonShapesLoaded[i]);
		}

	}

	static void _DestroyCommandBarShapes()
	{
		for (size_t i = 0; i < CommandBarButtonShapes.size(); ++i)
		{
			if (CommandBarButtonShapesLoaded[i]) {
				GameDelete<false,false>(CommandBarButtonShapes[i]);
			}

			CommandBarButtonShapesLoaded[i] = false;
			CommandBarButtonShapes[i] = nullptr;
		}
	}

	static void _InitDefaultIdx()
	{
		for (int i = 0; i < (int)CommandBarTypes::end; ++i)
			CommandBarLinks[i] = (int)_GetCommandBarIndexByName(CommandBarTypes_ident[i]);

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
		for (size_t i = 0; i < std::size(CommandBarTypes_ident); ++i) {
			if (IS_SAME_STR_N(CommandBarTypes_ident[i], pName)) {
				return CommandBarTypes(i);
			}
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

	void _RemoveButtons() {
		for (auto& command : CommandBarButtons) {
			this->RemoveButton(&command);
		}
	}

	void _HideAdvCommand()
	{
		this->_RemoveButtons();

		for (auto& idx : CommandBarLinks) {
			if (auto pShape = _GetShapeButton(idx)) {
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

		for (int i = 0; i < CommandBarLinks.size(); ++i) {
			int linked = CommandBarLinks[i];

			if (linked == -1 ||  !TabClass::GetCommandbarShape(linked) )
				continue;

			TabClass::LinkTooltip(TabClass::GetCommandbarShape(linked), (char*)CommandBarTypes_Tip[i]);
		}

		TabClass::LinkTooltip(TabThumbButtonActivated.operator->(), "Tip:ThumbOpen");
	}

	static NOINLINE ShapeButtonClass* __fastcall _GetShapeButton(int idx)
	{
		if (idx < 0 || idx >= 25)
			return nullptr;

		return &CommandBarButtons[idx];
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

		int __val = tabclassrect_B0FC64->Width + tabclassrect_B0FC64->X;
		for(int i = 0; i < std::size(CommandBarLinks); ++i){

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

					if(i == (int)CommandBarTypes::PlanningMode){
						pShpeBtn->ToggleType = 1;
						pShpeBtn->UseFlash = 1;
					} else{
						pShpeBtn->ToggleType = 0;
					}

					pShpeBtn->Flags = GadgetFlag::LeftPress | GadgetFlag::LeftRelease;
					if(i == (int)CommandBarTypes::Team01 || i == (int)CommandBarTypes::Team02 || i == (int)CommandBarTypes::Team03)
						pShpeBtn->Flags |= GadgetFlag::RightPress | GadgetFlag::RightRelease;

					pShpeBtn->SetPosition(v5, v6);
					pShpeBtn->SetShape(GetCommandButtonShape(i), 0, 0);
				}
			}
		}
	}

	static void __fastcall AttackMoveCommand() { JMP_STD(0x731AF0); } // AttackMoveCommand
	static void __fastcall BeaconCommand() { JMP_STD(0x731A30); } // BeaconCommand
	static void __fastcall CheerCommand() { JMP_STD(0x730F30); } // CheerCommand
	static void __fastcall DeployCommand() { JMP_STD(0x730AF0); } // DeployCommand
	static void __fastcall GuardCommand() { JMP_STD(0x730D60); } // GuardCommand
	static void __fastcall TurnOnPlanningMode(bool idk) { JMP_STD(0x731A50); } // TurnOffPlaningMode
	static void __fastcall TurnOffPlanningMode(bool idk) { JMP_STD(0x731A70); } // TurnOnPlanningMode
	static void __fastcall StopCommand() { JMP_STD(0x730EA0); } // StopCommand
	static void __fastcall TypeSelectCommand() { JMP_STD(0x732950); } // TypeSelectCommand

	// Team / group related helpers (argument renamed to groupnumber)
	static int __fastcall Get_Group_Index(int groupnumber) { JMP_STD(0x730A10); } // Get_Group_Index
	static bool __fastcall Is_Group_Assigned(int groupnumber) { JMP_STD(0x730990); } // Is_Group_Assigned
	static void __fastcall Center_On_Team_Command(int groupnumber) { JMP_STD(0x7313A0); } // Center_On_Team_Command
	static void __fastcall Select_Team_Command(int groupnumber) { JMP_STD(0x7311C0); } // Select_Team_Command
	static void __fastcall Create_Team_Command(int groupnumber) { JMP_STD(0x731060); } // Create_Team_Command

	static void TeamCommand(int teamNumber) {
		if (Get_Group_Index(teamNumber)) {
			if (Is_Group_Assigned(teamNumber)) {
				Center_On_Team_Command(teamNumber);
			} else {
				Select_Team_Command(teamNumber);
			}
		} else {
			Create_Team_Command(teamNumber);
		}
	}

	static void TestCommand()
	{
		static bool WhiteColorSearchedG = false;
		static int ColorIdxG = 5;

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
	}

	static void Proces(int key)
	{
		switch ((CommandBarTypes)key)
		{
		case CommandBarTypes::Team01: { TeamCommand(1); }break;
		case CommandBarTypes::Team02: { TeamCommand(2);  }break;
		case CommandBarTypes::Team03: { TeamCommand(3);  }break;
		case CommandBarTypes::TypeSelect: { TypeSelectCommand();  }break;
		case CommandBarTypes::Deploy: { DeployCommand();  }break;
		case CommandBarTypes::AttackMove: { AttackMoveCommand();  }break;
		case CommandBarTypes::Guard: { GuardCommand(); }break;
		case CommandBarTypes::Beacon: { BeaconCommand();  }break;
		case CommandBarTypes::Stop: { StopCommand();  }break;
		case CommandBarTypes::PlanningMode: {
			if (auto pButton = GetCommandbarShape(key)) {
				if (pButton->IsOn)
					TurnOffPlanningMode(1);
				else
					TurnOnPlanningMode(1);
			}
		}break;
		case CommandBarTypes::Cheer: { CheerCommand();  }break;
		case CommandBarTypes::DistributionMode: {
			if (auto pButton = GetCommandbarShape(key)) {
				if (pButton->IsOn)
					DistributionModeHoldDownCommandClass::DistributionModeOff(key);
				else
					DistributionModeHoldDownCommandClass::DistributionModeOn(key);
			}
		}break;
		default:break;
		}
	}
};

DEFINE_FUNCTION_JUMP(LJMP, 0x6CFD40, FakeTabClass::_GetShapeButton);
DEFINE_FUNCTION_JUMP(LJMP, 0x6D1200, FakeTabClass::_ShowAdvCommand);
DEFINE_FUNCTION_JUMP(LJMP, 0x6D14F0, FakeTabClass::_HideAdvCommand);
DEFINE_FUNCTION_JUMP(LJMP, 0x6D0F70, FakeTabClass::_DestroyCommandBarShapes);
DEFINE_FUNCTION_JUMP(LJMP, 0x6D0F10, FakeTabClass::_InitCommandBarShapes);
DEFINE_FUNCTION_JUMP(LJMP, 0x6D04A0, FakeTabClass::_AddButtons);
DEFINE_FUNCTION_JUMP(LJMP, 0x6D04D0, FakeTabClass::_RemoveButtons);
DEFINE_FUNCTION_JUMP(LJMP, 0x6D0FD0, FakeTabClass::InitAdvCommand);

ASMJIT_PATCH(0x6D02C0, InitForHouse_RemoveInline, 0x5)
{
	FakeTabClass::_InitCommandBarShapes();
	return 0x6D0304;
}

ASMJIT_PATCH(0x6D1770, TabClass_noticeSink_Planning_ChangeState, 0x7)
{
	GET_STACK(int, someID , 0x4);
	GET_STACK(int, something, 0x8);

	bool result = false;
	switch (someID)
	{
	case 4551:
	{
		if (auto pShape = FakeTabClass::_GetShapeButton((int)CommandBarTypes::PlanningMode))
			pShape->TurnOn();

		result = true;
		break;
	}
	case 4552:
	{
		if (auto pShape = FakeTabClass::_GetShapeButton((int)CommandBarTypes::PlanningMode))
			pShape->TurnOff();

		result = true;
		break;
	}
	default:break;
	}


	R->AL(result);
	return 0x6D17F1;
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

ASMJIT_PATCH(0x6D0D4F, TabClass_DrawIt_DrawCommandBar, 0x5)
{
	for (int i = 0; i < std::size(CommandBarLinks); ++i) {
		if (auto pShpeBtn = FakeTabClass::_GetShapeButton(CommandBarLinks[i])) {
			if (pShpeBtn->ShapeData) {
				pShpeBtn->Draw(true);
				pShpeBtn->IsDrawn = false;
			}

		}
	}

	return 0x6D0D8C;
}

ASMJIT_PATCH(0x6D0A87, TabClass_DrawIt_DrawCommandBar1, 0x5)
{
	GET(int, index, EAX);
	R->EAX(FakeTabClass::_GetShapeButton(index));
	return 0x6D0A97;
}

ASMJIT_PATCH(0x6D072C, TabClass_AI_AdditionalAffect, 0x6)
{
	GET(int, index, EAX);
	FakeTabClass::Proces(index);
	return 0x6D0827;
}

DEFINE_JUMP(LJMP, 0x6CFE8C, 0x6D0233)