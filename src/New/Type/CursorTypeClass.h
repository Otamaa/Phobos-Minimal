#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/Enum.h>

#include <WWMouseClass.h>
#include <GeneralDefinitions.h>

class CursorTypeClass final : public Enumerable<CursorTypeClass>
{
public:

	Valueable<MouseCursor> CursorData;
	CursorTypeClass(const char* const pTitle) : Enumerable<CursorTypeClass>(pTitle)
		, CursorData { }
	{ }

	virtual ~CursorTypeClass() override = default;
	static void AddDefaults();

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	static void LoadFromINIList_New(CCINIClass* pINI, bool bDebug = false);

	static std::array<const char* const, (size_t)MouseCursorType::count> MouseCursorTypeToStrings;
	static std::array<const char*, (size_t)NewMouseCursorType::count> NewMouseCursorTypeToStrings;
	static std::array<MouseCursor, (size_t)NewMouseCursorType::count> NewMouseCursorTypeData;

private:
	template <typename T>
	void Serialize(T& Stm);

};

template <>
void NOINLINE ValueableIdx<CursorTypeClass*>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		const char* val = parser.value();
		const bool IsBlank = GameStrings::IsBlank(val);
		const int idx = CursorTypeClass::FindIndexById(val);

		//invalid idx , is not blank , and the first value is number
		if (idx == -1 && !IsBlank && std::isdigit(val[0]))
		{
			const auto nEwIdx = CursorTypeClass::FindIndexById(pSection);

			if(nEwIdx == -1)
			{
				std::string copyed = parser.value();
				MouseCursor value;

				char* context = nullptr;
				if (char* const pFrame = strtok_s(parser.value(), Phobos::readDelims, &context))
				{
					Parser<int>::Parse(pFrame, &value.StartFrame);
				}
				if (char* const pCount = strtok_s(nullptr, Phobos::readDelims, &context))
				{
					Parser<int>::Parse(pCount, &value.FrameCount);
				}
				if (char* const pInterval = strtok_s(nullptr, Phobos::readDelims, &context))
				{
					Parser<int>::Parse(pInterval, &value.FrameRate);
				}
				if (char* const pFrame = strtok_s(nullptr, Phobos::readDelims, &context))
				{
					Parser<int>::Parse(pFrame, &value.SmallFrame);
				}
				if (char* const pCount = strtok_s(nullptr, Phobos::readDelims, &context))
				{
					Parser<int>::Parse(pCount, &value.SmallFrameCount);
				}
				if (char* const pHotX = strtok_s(nullptr, Phobos::readDelims, &context))
				{
					MouseCursorHotSpotX::Parse(pHotX, &value.X);
				}
				if (char* const pHotY = strtok_s(nullptr, Phobos::readDelims, &context))
				{
					MouseCursorHotSpotY::Parse(pHotY, &value.Y);
				}

				CursorTypeClass::Allocate(pSection)->CursorData = value;
				this->Value = CursorTypeClass::Array.size() - 1;
				Debug::Log("[Phobos]Parsing CusorTypeClass from raw Cursor value of [%s]%s=%s\n", pSection, pKey, copyed.c_str());
				return;
			}
			else
			{
				this->Value = nEwIdx;
				return;
			}
		}
		else if (idx != -1)
		{
			this->Value = idx;
			return;
		}

		Debug::INIParseFailed(pSection, pKey, val ,"Expect Valid CursorType");
	}
}