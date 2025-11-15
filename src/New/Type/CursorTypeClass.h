#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/Enum.h>

#include <WWMouseClass.h>
#include <GeneralDefinitions.h>

#include <ranges>
#include <string_view>

class CursorTypeClass final : public Enumerable<CursorTypeClass>
{
public:

	Valueable<MouseCursor> CursorData;

	CursorTypeClass(const char* pTitle) : Enumerable<CursorTypeClass>(pTitle)
		, CursorData { }
	{ }

	static void AddDefaults();

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static void LoadFromINIList_New(CCINIClass* pINI, bool bDebug = false);

	static OPTIONALINLINE COMPILETIMEEVAL size_t AllocateWithDefault(const char* Title , const MouseCursor& cursor) {
		size_t sz = Array.size();
			Array.emplace_back((std::make_unique<CursorTypeClass>(Title)));
			Array.back()->CursorData = cursor;
		return sz;
	}

private:
	template <typename T>
	void Serialize(T& Stm);

};

template<>
struct IndexFinder<CursorTypeClass*>{

	static OPTIONALINLINE bool getindex(int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false)
	{
		if (parser.ReadString(pSection, pKey))
		{
			std::string_view val_(parser.value());

			if (GameStrings::IsBlank(val_.data())) {
				value = -1;
				return true;
			}

			if (int idx = CursorTypeClass::FindIndexById(val_.data());  idx != -1) {
				value = idx;
				return true;
			}

			if (!val_.empty()) {

				const size_t commaCount = std::ranges::count(val_, ',');

				if(commaCount == 6){

					std::string secondaryname = pSection;
					secondaryname += "_";
					secondaryname += pKey;

					CursorTypeClass* pCursor = nullptr;
					int outIndex = -1;

					//already registered by the secondary name
					if (int idxb = CursorTypeClass::FindIndexById(secondaryname.c_str());  idxb != -1) {
						pCursor = CursorTypeClass::Array[idxb].get();
						outIndex = idxb;
					} else {
						outIndex = (int)CursorTypeClass::Array.size();
						pCursor = CursorTypeClass::Array.emplace_back((std::make_unique<CursorTypeClass>(secondaryname.data()))).get();
					}

					auto contexes = PhobosCRT::split<7>(val_);
					auto cursor = pCursor->CursorData.operator->();
					Parser<int>::Parse(contexes[0].data(), &cursor->StartFrame);
					Parser<int>::Parse(contexes[1].data(), &cursor->FrameCount);
					Parser<int>::Parse(contexes[2].data(), &cursor->FrameRate);
					Parser<int>::Parse(contexes[3].data(), &cursor->SmallFrame);
					Parser<int>::Parse(contexes[4].data(), &cursor->SmallFrameCount);
					MouseCursorHotSpotX::Parse(contexes[5].data(), &cursor->X);
					MouseCursorHotSpotY::Parse(contexes[6].data(), &cursor->Y);

					value = outIndex;
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value());
		}

		return false;
	}
};
