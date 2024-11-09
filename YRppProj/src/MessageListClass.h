/*
	Use this to print a message on the screen!
*/

#pragma once

#include <wchar.h>
#include <ColorScheme.h>

#include <Helpers/CompileTime.h>
#include <GadgetClass.h>

struct MessageArrayStruct
{
	__int16 buffer[162];
};

class MessageListClass
{
public:
	static constexpr reference<MessageListClass, 0xA8BC60u> const Instance{};

	MessageListClass();
	~MessageListClass();

	void Init(int x, int y, int max_msg, int maxchars, int height, int edit_x, int edit_y, bool overflow_on, int over_start, int over_end, int width) const
		;// { JMP_THIS(0x5D3A60); }

	TextLabelClass* AddMessage(wchar_t* name, int id, wchar_t* txt, int color, TextPrintType style, int timeout, bool single_player)
		;// { JMP_THIS(0x5D3BA0); }

	wchar_t* GetMessage(int id);// { JMP_THIS(0x5D3F60); }
	TextLabelClass* GetLabel(int id);// { JMP_THIS(0x5D3F90); }
	bool ConcatMessage(const wchar_t* name, int id, const wchar_t* txt, int timeout);// { JMP_THIS(0x5D3FC0); }
	TextLabelClass* AddEdit(int color, TextPrintType style, char* to, char cursor, int width);// { JMP_THIS(0x5D4210); }

	void SetEditFocus();// { JMP_THIS(0x5D41E0); }
	char HasEditFocus();// { JMP_THIS(0x5D41F0); }
	void RemoveEdit();// { JMP_THIS(0x5D4390); }
	wchar_t* GetEditBuffer();// { JMP_THIS(0x5D4400); }
	void SetEditColor(int a2);// { JMP_THIS(0x5D4410); }
	void Manage();// { JMP_THIS(0x5D4430); }
	int Input(int& Input, int DefInput = 0);// { JMP_THIS(0x5D4510); }
	void Draw();// { JMP_THIS(0x5D49A0); }
	int NumMessages();// { JMP_THIS(0x5D4AA0); }
	void SetWidth(int width);// { JMP_THIS(0x5D4AD0); }
	int TrimMessage(wchar_t* dest, wchar_t* src, int min_chars, int max_chars, int scandir);// { JMP_THIS(0x5D4B20); }
	void ComputeY();// { JMP_THIS(0x5D4BF0); }

	void PrintMessage(const wchar_t* pLabel, DWORD dwUnk1, const wchar_t* pMessage, int nColorSchemeIndex = ColorScheme::Yellow,
		DWORD dwUnk2 = 0x4046, int duration = 0x96, bool bSilent = false)
		;// { JMP_THIS(0x5D3BA0); };

	void PrintMessage(const wchar_t* pLabel, const wchar_t* pMessage, int durationFrames = 0x96, int nColorSchemeIndex = ColorScheme::White, bool bSilent = false)
		{ this->PrintMessage(pLabel, 0, pMessage, nColorSchemeIndex, 0x4046, durationFrames, bSilent); }

	void PrintMessage(const wchar_t* pMessage, int durationFrames = 0x96, int nColorSchemeIndex = ColorScheme::White, bool bSilent = false)
		{ this->PrintMessage(nullptr, 0, pMessage, nColorSchemeIndex, 0x4046, durationFrames, bSilent); }

	void PrintMessage(const wchar_t* pMessage, double durationMinutes, int nColorSchemeIndex = ColorScheme::White, bool bSilent = false)
		{ this->PrintMessage(nullptr, 0, pMessage, nColorSchemeIndex, 0x4046, static_cast<int>(durationMinutes * 900), bSilent); }

public :

	TextLabelClass* MessageList;
	Point2D MessagePos;
	int MaxMessageCount;
	int MaxCharacters;
	int Height;
	bool EnableOverflow;
	bool IsEdit;
	bool AdjustEdit;
	Point2D EditPos;
	TextLabelClass* EditLabel;
	wchar_t EditBuffer[162];
	wchar_t OverflowBuffer[162];
	DWORD EditCurrentPos;
	DWORD EditInitialPos;
	wchar_t CursorCharacter;
	DWORD OverflowStart;
	DWORD OverflowEnd;
	int Width;
	wchar_t MessageBuffers[14][162];
	wchar_t BufferAvail[14];
};

static_assert(sizeof(MessageListClass) == 0x149C, "Invalid Size !");