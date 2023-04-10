#pragma once
#include <GeneralDefinitions.h>

class WWKeyboardClass
{
public:

    static constexpr reference<WWKeyboardClass*, 0x87F770u> const Instance{};

    ~WWKeyboardClass() {};

    WWKeyboardClass()
		{ JMP_THIS(0x54EE60); }

    WORD GetBuffer()
		{ JMP_THIS(0x54EE90); }

    bool IsMouseKey(short nKey)
		{ JMP_THIS(0x54EFC0); }

    WORD Check()
		{ JMP_THIS(0x54F000); }

    WORD Get()
		{ JMP_THIS(0x54F050); }

    bool Put(WORD nKey)
		{ JMP_THIS(0x54F1C0); }

    bool PutKeyMessage(WORD nKey , bool bRelease)
		{ JMP_THIS(0x54F200); }

    bool PutMouseMessage(WORD nKey , Point2D nXY , bool bRelease)
		{ JMP_THIS(0x54F2F0); }

    bool PutNewLineMessage(BYTE nChar)
		{ JMP_THIS(0x54F3D0); }

    char ToASCII(WORD nKey)
		{ JMP_THIS(0x54F450); }

    bool IsDown(int nKey)
		{ JMP_THIS(0x54F5C0); }

    WORD FetchElement()
		{ JMP_THIS(0x54F610); }

    WORD PeekElement()
		{ JMP_THIS(0x54F650); }

    bool PutElement()
		{ JMP_THIS(0x54F670); }

    bool IsBufferFull()
		{ JMP_THIS(0x54F6B0); }

    bool IsBufferEmpty()
		{ JMP_THIS(0x54F6D0); }

    void FillBufferFromSystem()
		{ JMP_THIS(0x54F6F0); }

    void Clear()
		{ JMP_THIS(0x54F720); }

    BOOL MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{ JMP_THIS(0x54F790); }

    int AvailableBuffer()
		{ JMP_THIS(0x54FB90); }

	bool IsKeyPressed(int key) const
		{ JMP_THIS(0x54F5C0); }

	bool IsForceFireKeyPressed() const {
		return this->IsKeyPressed(GameOptionsClass::Instance->KeyForceFire1)
			|| this->IsKeyPressed(GameOptionsClass::Instance->KeyForceFire2);
	}

	bool IsForceMoveKeyPressed() const {
		return this->IsKeyPressed(GameOptionsClass::Instance->KeyForceMove1)
			|| this->IsKeyPressed(GameOptionsClass::Instance->KeyForceMove2);
	}

	bool IsForceSelectKeyPressed() const {
		return this->IsKeyPressed(GameOptionsClass::Instance->KeyForceSelect1)
			|| this->IsKeyPressed(GameOptionsClass::Instance->KeyForceSelect2);
	}

public:
    int MouseQX;
    int MouseQY;
	uint16_t LastMessageElement; //is this even right ?
    int MouseQX_2;
    int MouseQY_2;
    BYTE KeyState[0x100];
    WORD Buffer[0x100];
    int Head;
    int Tail;
    bool IsLibrary;
};

static_assert(offsetof(WWKeyboardClass, KeyState) == 0x14 , "Class Member Shifted !");
static_assert(offsetof(WWKeyboardClass, Buffer) == 0x114, "Class Member Shifted !");
static_assert(offsetof(WWKeyboardClass, IsLibrary) == 0x31C, "Class Member Shifted !");
static_assert(sizeof(WWKeyboardClass) == 0x320, "Invalid Size !");

typedef WWKeyboardClass InputManagerClass;