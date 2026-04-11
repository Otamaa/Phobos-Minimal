#include "INIParser.h"

#include "Parser.h"

#include <CCINIClass.h>


#ifdef CHECK_
#include <Utilities/Debug.h>
#endif

#include <Phobos.h>

INI_EX::INI_EX() noexcept : IniFile { nullptr }
{}

INI_EX::~INI_EX() { IniFile = nullptr; }

INI_EX::INI_EX(CCINIClass* pIniFile) noexcept
	: IniFile { pIniFile }
{}

INI_EX::INI_EX(CCINIClass& iniFile) noexcept
	: IniFile { &iniFile }
{}

const char* INI_EX::c_str() const
{
	return Phobos::readBuffer;
}

char* INI_EX::value() const
{
	return Phobos::readBuffer;
}

size_t INI_EX::max_size() const
{
	return Phobos::readLength;
}

bool INI_EX::empty() const
{
	return !Phobos::readBuffer[0];
}

CCINIClass* INI_EX::GetINI() const
{
	return IniFile;
}

CCINIClass* INI_EX::operator->() const
{
	return IniFile;
}

INI_EX::operator bool() const
{
	return IniFile;
}
INI_EX::INI_EX(INI_EX const& other)
	: IniFile { other.IniFile }
{}

INI_EX& INI_EX::operator=(INI_EX const& other)
{
	// Use copy and swap idiom to implement assignment.
	INI_EX copy(other);
	swap(*this, copy);
	return *this;
}

INI_EX::INI_EX(INI_EX&& that) noexcept
	: IniFile { nullptr }               // Set the state so we know it is undefined
{
	swap(*this, that);
}

INI_EX& INI_EX::operator=(INI_EX&& that) noexcept
{
	swap(*this, that);
	return *this;
}

// parser template
template <typename T, size_t Count>
bool OPTIONALINLINE Read(INI_EX* pINI, const char* pSection, const char* pKey, T* pBuffer)
{
	if (pINI->ReadString(pSection, pKey) > 0)
	{
		return Parser<T, Count>::Parse(pINI->c_str(), pBuffer) == Count;
	}
	return false;
}

template <typename T, size_t Count>
size_t OPTIONALINLINE ReadAndCount(INI_EX* pINI , const char* pSection, const char* pKey, T* pBuffer)
{
	if (pINI->ReadString(pSection, pKey) > 0)
	{
		return Parser<T, Count>::Parse(pINI->c_str(), pBuffer);
	}

	return 0u;
}

int INI_EX::ReadString(const char* pSection, const char* pKey)
{
	const int ret = IniFile->ReadString(
		pSection, pKey, Phobos::readDefval, this->value(), this->max_size());

#ifdef CHECK_
	if (ret > 0 && (!*Phobos::readBuffer || !strlen(Phobos::readBuffer)))
	{
		Debug::LogInfo("ReadString returning empty strings![{}][{}] ,", pSection, pKey);
		DebugBreak();
	}
#endif
	return ret;
}

bool INI_EX::ReadBool(const char* pSection, const char* pKey, bool* bBuffer)
{
	return Read<bool, 1>(this,pSection, pKey, bBuffer);
}

bool INI_EX::Read2Bool(const char* pSection, const char* pKey, bool* bBuffer)
{
	return Read<bool, 2>(this, pSection, pKey, bBuffer);
}

bool INI_EX::Read3Bool(const char* pSection, const char* pKey, bool* bBuffer)
{
	return Read<bool, 3>(this, pSection, pKey, bBuffer);
}

bool INI_EX::ReadInteger(const char* pSection, const char* pKey, int* nBuffer)
{
	return Read<int, 1>(this, pSection, pKey, nBuffer);
}

bool INI_EX::Read2Integers(const char* pSection, const char* pKey, int* nBuffer)
{
	return Read<int, 2>(this, pSection, pKey, nBuffer);
}

bool INI_EX::Read2IntegerAndCount(const char* pSection, const char* pKey, int* nBuffer)
{
	return ReadAndCount<int, 2>(this, pSection, pKey, nBuffer);
}

bool INI_EX::Read3Integers(const char* pSection, const char* pKey, int* nBuffer)
{
	return Read<int, 3>(this, pSection, pKey, nBuffer);
}

size_t INI_EX::Read3IntegerAndCount(const char* pSection, const char* pKey, int* nBuffer)
{
	return ReadAndCount<int, 3>(this, pSection, pKey, nBuffer);
}

bool INI_EX::Read4Integers(const char* pSection, const char* pKey, int* nBuffer)
{
	return Read<int, 4>(this, pSection, pKey, nBuffer);
}

bool INI_EX::ReadBytes(const char* pSection, const char* pKey, BYTE* nBuffer)
{
	return Read<BYTE, 1>(this, pSection, pKey, nBuffer);
}

bool INI_EX::Read2Bytes(const char* pSection, const char* pKey, BYTE* nBuffer)
{
	return Read<BYTE, 2>(this, pSection, pKey, nBuffer);
}

bool INI_EX::Read3Bytes(const char* pSection, const char* pKey, BYTE* nBuffer)
{
	return Read<BYTE, 3>(this, pSection, pKey, nBuffer);
}

bool INI_EX::ReadDouble(const char* pSection, const char* pKey, double* nBuffer)
{
	return Read<double, 1>(this, pSection, pKey, nBuffer);
}

bool INI_EX::Read2Double(const char* pSection, const char* pKey, double* nBuffer)
{
	return Read<double, 2>(this, pSection, pKey, nBuffer);
}

size_t INI_EX::Read2DoubleAndCount(const char* pSection, const char* pKey, double* nBuffer)
{
	return ReadAndCount<double, 2>(this, pSection, pKey, nBuffer);
}

bool INI_EX::Read3Double(const char* pSection, const char* pKey, double* nBuffer)
{
	return Read<double, 3>(this, pSection, pKey, nBuffer);
}

size_t INI_EX::Read3DoubleAndCount(const char* pSection, const char* pKey, double* nBuffer)
{
	return ReadAndCount<double, 3>(this, pSection, pKey, nBuffer);
}

bool INI_EX::ReadFloat(const char* pSection, const char* pKey, float* nBuffer)
{
	return Read<float, 1>(this, pSection, pKey, nBuffer);
}

bool INI_EX::Read2Float(const char* pSection, const char* pKey, float* nBuffer)
{
	return Read<float, 2>(this, pSection, pKey, nBuffer);
}

bool INI_EX::Read3Float(const char* pSection, const char* pKey, float* nBuffer)
{
	return Read<float, 3>(this, pSection, pKey, nBuffer);
}

size_t INI_EX::Read3FloatAndCount(const char* pSection, const char* pKey, float* nBuffer)
{
	return ReadAndCount<float, 3>(this, pSection, pKey, nBuffer);
}

bool INI_EX::ReadShort(const char* pSection, const char* pKey, short* nBuffer)
{
	return Read<short, 1>(this, pSection, pKey, nBuffer);
}

bool INI_EX::Read2Short(const char* pSection, const char* pKey, short* nBuffer)
{
	return Read<short, 2>(this, pSection, pKey, nBuffer);
}

bool INI_EX::ReadSpeed(const char* pSection, const char* pKey, int* nBuffer)
{
	double parsedSpeed = IniFile->ReadDouble(pSection, pKey, -1.0);

	if (parsedSpeed >= 0.0)
	{
		int speed = int((MinImpl(parsedSpeed, 100.0) * 256.0) / 100.0);
		*nBuffer = MinImpl(speed, 255);
	}

	return (*nBuffer != -1);
}

bool INI_EX::ReadArmor(const char* pSection, const char* pKey, int* nBuffer)
{
	*nBuffer = IniFile->ReadArmorType(pSection, pKey, *nBuffer);
	return (*nBuffer != -1);
}

// WARNING : const char* memory address may temporary and can invalidated
bool INI_EX::ParseList(std::vector<const char*>& values, const char* pSection, const char* pKey)
{
	if (this->ReadString(pSection, pKey))
	{
		values.clear();
		char* context = nullptr;

		for (auto pCur = strtok_s(this->value(), Phobos::readDelims, &context); pCur; pCur = strtok_s(nullptr, Phobos::readDelims, &context))
			values.push_back(pCur);

		return true;
	}

	return false;
}

bool INI_EX::ParseList(std::vector<std::string>& values, const char* pSection, const char* pKey)
{
	if (this->ReadString(pSection, pKey))
	{
		values.clear();
		char* context = nullptr;

		for (auto pCur = strtok_s(this->value(), Phobos::readDelims, &context); pCur; pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			if (strlen(pCur))
			{
				values.push_back(pCur);
			}
		}

		return true;
	}

	return false;
}