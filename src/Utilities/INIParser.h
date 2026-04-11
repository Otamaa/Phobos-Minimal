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

#pragma once

#include <vector>
#include <string>
#include <Base/Always.h>

class CCINIClass;
class INI_EX {
	CCINIClass* IniFile;

public:

	INI_EX() noexcept;
	~INI_EX();
	explicit INI_EX(CCINIClass* pIniFile) noexcept;
	explicit INI_EX(CCINIClass& iniFile) noexcept;

	const char* c_str() const;
	char* value() const;
	size_t max_size() const;
	bool empty() const;
	CCINIClass* GetINI() const;
	INI_EX(INI_EX const& other);
	INI_EX& operator=(INI_EX const& other);
	INI_EX(INI_EX&& that) noexcept;

	INI_EX& operator=(INI_EX&& that) noexcept;

	friend void swap(INI_EX& lhs, INI_EX& rhs) noexcept
	{
		std::swap(lhs.IniFile, rhs.IniFile);
	}

	CCINIClass* operator->() const;
	operator bool() const;

	// basic string reader
	int ReadString(const char* pSection, const char* pKey);

	// helpers
	bool  ReadBool(const char* pSection, const char* pKey, bool* bBuffer);
	bool Read2Bool(const char* pSection, const char* pKey, bool* bBuffer);
	bool Read3Bool(const char* pSection, const char* pKey, bool* bBuffer);
	bool ReadInteger(const char* pSection, const char* pKey, int* nBuffer);
	bool Read2Integers(const char* pSection, const char* pKey, int* nBuffer);
	bool Read2IntegerAndCount(const char* pSection, const char* pKey, int* nBuffer);
	bool Read3Integers(const char* pSection, const char* pKey, int* nBuffer);
	size_t Read3IntegerAndCount(const char* pSection, const char* pKey, int* nBuffer);
	bool Read4Integers(const char* pSection, const char* pKey, int* nBuffer);
	bool ReadBytes(const char* pSection, const char* pKey, BYTE* nBuffer);
	bool Read2Bytes(const char* pSection, const char* pKey, BYTE* nBuffer);
	bool Read3Bytes(const char* pSection, const char* pKey, BYTE* nBuffer);
	bool ReadDouble(const char* pSection, const char* pKey, double* nBuffer);
	bool Read2Double(const char* pSection, const char* pKey, double* nBuffer);
	size_t Read2DoubleAndCount(const char* pSection, const char* pKey, double* nBuffer);
	bool Read3Double(const char* pSection, const char* pKey, double* nBuffer);
	size_t Read3DoubleAndCount(const char* pSection, const char* pKey, double* nBuffer);
	bool ReadFloat(const char* pSection, const char* pKey, float* nBuffer);
	bool Read2Float(const char* pSection, const char* pKey, float* nBuffer);
	bool Read3Float(const char* pSection, const char* pKey, float* nBuffer);
	size_t Read3FloatAndCount(const char* pSection, const char* pKey, float* nBuffer);
	bool ReadShort(const char* pSection, const char* pKey, short* nBuffer);
	bool Read2Short(const char* pSection, const char* pKey, short* nBuffer);
	bool ReadSpeed(const char* pSection, const char* pKey, int* nBuffer);
	bool ReadArmor(const char* pSection, const char* pKey, int* nBuffer);

	// WARNING : const char* memory address may temporary and can invalidated
	bool ParseList(std::vector<const char*>& values, const char* pSection, const char* pKey);
	bool ParseList(std::vector<std::string>& values, const char* pSection, const char* pKey);
};
