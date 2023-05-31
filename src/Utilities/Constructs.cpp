#pragma region Ares Copyrights
/*
 *Copyright (c) 2008+, All Ares Contributors
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *3. All advertising materials mentioning features or use of this software
 *   must display the following acknowledgement:
 *   This product includes software developed by the Ares Contributors.
 *4. Neither the name of Ares nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY ITS CONTRIBUTORS ''AS IS'' AND ANY
 *EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE ARES CONTRIBUTORS BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma endregion

#include "Constructs.h"

#include <ConvertClass.h>
#include <FileSystem.h>
#include <ScenarioClass.h>
#include "GeneralUtils.h"
#include "TranslucencyLevel.h"

//#include <Ext/Convert/Body.h>

bool CustomPalette::Read(
	CCINIClass* pINI, const char* pSection, const char* pKey,
	const char* pDefault)
{
	if (pINI->ReadString(pSection, pKey, pDefault, Phobos::readBuffer)) {
		//dont init anything if it is empty
		if (GeneralUtils::IsValidString(Phobos::readBuffer))
		{
			Debug::Log("Loading Palette[%s] for [%s] ! \n", Phobos::readBuffer , pSection);
			GeneralUtils::ApplyTheaterSuffixToString(Phobos::readBuffer);
			return this->LoadFromName(Phobos::readBuffer);
		}
	}
	return false;
}

bool FC CustomPalette::Read_Static(CustomPalette* pThis, DWORD, CCINIClass* pINI, const char* pSection, const char* pKey, const char* pDefault)
{
	return pThis->Read(pINI , pSection,pKey,pDefault);
}

bool CustomPalette::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Clear();

	bool hasPalette = false;
	auto ret = Stm.Load(this->Mode) && Stm.Load(hasPalette);

	if (ret && hasPalette)
	{
		this->Palette.reset(GameCreate<BytePalette>());
		ret = Stm.Load(*this->Palette);

		if (ret)
		{
			this->CreateConvert();
		}
	}

	return ret;
}

bool CustomPalette::Save(PhobosStreamWriter& Stm) const
{
	Stm.Save(this->Mode);
	Stm.Save(this->Palette != nullptr);
	if (this->Palette) {
		Stm.Save(*this->Palette);
	}
	return true;
}

bool CustomPalette::LoadFromName(const char* PaletteName)
{
	this->Clear();

	if (auto pPal = FileSystem::AllocatePalette(PaletteName))
	{
		this->Palette.reset(pPal);
		this->CreateConvert();
	}

	return this->Convert != nullptr;
}

void CustomPalette::Clear()
{
	this->Convert = nullptr;
	this->Palette = nullptr;
}

void CustomPalette::CreateConvert()
{
	if (!this->Palette)
		Debug::Log("Missing Palette Data ! \n");

	ConvertClass* buffer = nullptr;
	if (this->Mode == PaletteMode::Temperate) {
		buffer = GameCreate<ConvertClass>(this->Palette.get(), &FileSystem::TEMPERAT_PAL(), DSurface::Primary(), 53, false);
	}
	else {
		buffer = GameCreate<ConvertClass>(this->Palette.get(), this->Palette.get(), DSurface::Alternate(), 1, false);
	}

	this->Convert.reset(buffer);
}

bool CustomPalette::CreateFromBytePalette(BytePalette nBytePal)
{
	this->Clear();
	ConvertClass* buffer = nullptr;
	if (this->Mode == PaletteMode::Temperate)
	{
		buffer = GameCreate<ConvertClass>(
			nBytePal, FileSystem::TEMPERAT_PAL, DSurface::Primary,
			53, false);
	}
	else
	{
		buffer = GameCreate<ConvertClass>(
			nBytePal, nBytePal, DSurface::Alternate,
			1, false);
	}

	//ConvertExt::GetOrSetName(buffer, Name);

	this->Convert.reset(buffer);
	return this->Convert != nullptr;
}

bool TheaterSpecificSHP::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		auto pValue = parser.value();
		GeneralUtils::ApplyTheaterSuffixToString(pValue);

		std::string Result = pValue;
		if (!CRT::strstr(pValue, ".shp"))
			Result += ".shp";

		if (auto const pImage = FileSystem::LoadSHPFile(Result.c_str()))
		{
			value = pImage;
			return true;
		}
		else
		{
			Debug::Log("Failed to find file %s referenced by [%s]%s=%s\n", Result.c_str(), pSection, pKey, pValue);
		}
	}

	return false;
}