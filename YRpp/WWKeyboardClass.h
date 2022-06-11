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

public:
    int MouseQX;
    int MouseQY;
    int MouseQX_2;
    int MouseQY_2;
    DWORD LastMessageElement;
    BYTE KeyState[0x100];
    WORD Buffer[0x100];
    int Head;
    int Tail;
    bool IsLibrary;
};