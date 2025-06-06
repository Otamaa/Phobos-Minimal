#pragma once

#include <StringTable.h>
#include <Helpers/CompileTime.h>

class WWMessageBox
{
public:
	static COMPILETIMEEVAL reference<WWMessageBox, 0x82083C> Instance {};

	enum class Result : int
	{
		Button1 = 0, Button2, Button3,
		OK = Button1, Cancel = Button2
	};

	WWMessageBox(const wchar_t* pCaptain) : Captain { pCaptain } { }

	Result Process(
		const wchar_t* pMsg,
		const wchar_t* pBtn1 = StringTable::FetchString(GameStrings::TXT_OK),
		const wchar_t* pBtn2 = StringTable::FetchString(GameStrings::TXT_CANCEL),
		const wchar_t* pBtn3 = nullptr,
		bool bUkn = false
	)
	{
		JMP_THIS(0x5D3490);
	}

	Result Process(const char* pMsg, const char* pBtn1, const char* pBtn2, const char* pBtn3)
	{
		return Process(
			StringTable::FetchString(pMsg), StringTable::FetchString(pBtn1),
			StringTable::FetchString(pBtn2), StringTable::FetchString(pBtn3)
		);
	}

private:
	const wchar_t* Captain;
};