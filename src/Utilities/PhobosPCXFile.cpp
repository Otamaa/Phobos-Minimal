#include "PhobosPCXFile.h"

#include "Savegame.h"
#include "Debug.h"

#include <CCINIClass.h>
#include <SwizzleManagerClass.h>

bool PhobosPCXFile::Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault)
{
	char buffer[Capacity];
	if (pINI->ReadString(pSection, pKey, pDefault, buffer) > 0)
	{

		std::string cachedWithExt = _strlwr(buffer);

		if (cachedWithExt.find(".pcx") == std::string::npos)
			cachedWithExt += ".pcx";

		*this = cachedWithExt;

		if (this->filename && !this->Surface)
		{
			Debug::INIParseFailed(pSection, pKey, this->filename, "PCX file not found.");
		}
	}

	return buffer[0] != 0;
}


bool PhobosPCXFile::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{

	this->Clear();
	void* oldPtr;
	const auto ret = Stm.Load(oldPtr) && Stm.Load(this->filename);

	if (!ret)
		return false;

	if (oldPtr && this->filename)
	{
		BSurface* pSource = PCX::Instance->GetSurface(this->filename);
		if (!pSource && PCX::Instance->LoadFile(this->filename))
			pSource = PCX::Instance->GetSurface(this->filename, nullptr);

		this->Surface = pSource;

		if (!this->Surface)
		{
			Debug::LogInfo("PCX file[{}] not found.", this->filename.data());
		}

		SwizzleManagerClass::Instance().Here_I_Am((long)oldPtr, this->Surface);
	}

	return true;
}

bool PhobosPCXFile::Save(PhobosStreamWriter& Stm) const
{
	Stm.Save(this->Surface);
	Stm.Save(this->filename);
	return true;
}
