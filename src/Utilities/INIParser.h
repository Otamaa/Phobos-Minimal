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

#include "Parser.h"

#include <Phobos.h>
#include <CCINIClass.h>

#ifdef CHECK_
#include <Utilities/Debug.h>
#endif

class INI_EX {
	CCINIClass* IniFile;

public:

	COMPILETIMEEVAL INI_EX() noexcept : IniFile { nullptr }
	{ }

	COMPILETIMEEVAL ~INI_EX() { IniFile = nullptr; }

	COMPILETIMEEVAL explicit INI_EX(CCINIClass* pIniFile) noexcept
		: IniFile { pIniFile }
	{ }

	COMPILETIMEEVAL explicit INI_EX(CCINIClass& iniFile) noexcept
		: IniFile { &iniFile }
	{ }

	OPTIONALINLINE const char* c_str() const {
		return Phobos::readBuffer;
	}

	OPTIONALINLINE char* value() const {
		return Phobos::readBuffer;
	}

	OPTIONALINLINE COMPILETIMEEVAL size_t max_size() const {
		return Phobos::readLength;
	}

	COMPILETIMEEVAL OPTIONALINLINE bool empty() const {
		return !Phobos::readBuffer[0];
	}

	COMPILETIMEEVAL OPTIONALINLINE CCINIClass* GetINI() const {
		return IniFile;
	}

	COMPILETIMEEVAL INI_EX(INI_EX const& other)
		: IniFile { other.IniFile }
	{ }

	INI_EX& operator=(INI_EX const& other) {
		// Use copy and swap idiom to implement assignment.
		INI_EX copy(other);
		swap(*this, copy);
		return *this;
	}

	INI_EX(INI_EX&& that) noexcept
		: IniFile { nullptr }               // Set the state so we know it is undefined
	{
		swap(*this, that);
	}

	INI_EX& operator=(INI_EX&& that) noexcept
	{
		swap(*this, that);
		return *this;
	}

	friend void swap(INI_EX& lhs, INI_EX& rhs) noexcept
	{
		std::swap(lhs.IniFile, rhs.IniFile);
	}

	CCINIClass* operator->() const {
		return IniFile;
	}

	operator bool() const {
		return IniFile;
	}

	// basic string reader
	int ReadString(const char* pSection, const char* pKey) {
		const int ret = IniFile->ReadString(
			pSection, pKey, Phobos::readDefval, this->value(), this->max_size());

#ifdef CHECK_
		if (ret > 0 && (!*Phobos::readBuffer || !strlen(Phobos::readBuffer))) {
			Debug::LogInfo("ReadString returning empty strings![{}][{}] ,", pSection, pKey);
			DebugBreak();
		}
#endif
		return ret;
	}

	// parser template
	template <typename T, size_t Count>
	bool Read(const char* pSection, const char* pKey, T* pBuffer) {
		if (this->ReadString(pSection, pKey) > 0) {
			return Parser<T, Count>::Parse(this->value(), pBuffer) == Count;
		}
		return false;
	}

	template <typename T, size_t Count>
	size_t ReadAndCount(const char* pSection, const char* pKey, T* pBuffer)
	{
		if (this->ReadString(pSection, pKey) > 0) {
			return Parser<T, Count>::Parse(this->value(), pBuffer);
		}

		return 0u;
	}

	// helpers

	bool ReadBool(const char* pSection, const char* pKey, bool* bBuffer) {
		return Read<bool, 1>(pSection, pKey, bBuffer);
	}

	bool Read2Bool(const char *pSection, const char *pKey, bool *bBuffer) {
		return Read<bool, 2>(pSection, pKey, bBuffer);
	}

	bool Read3Bool(const char *pSection, const char *pKey, bool *bBuffer) {
		return Read<bool, 3>(pSection, pKey, bBuffer);
	}

	bool ReadInteger(const char *pSection, const char *pKey, int *nBuffer) {
		return Read<int, 1>(pSection, pKey, nBuffer);
	}

	bool Read2Integers(const char* pSection, const char* pKey, int* nBuffer) {
		return Read<int, 2>(pSection, pKey, nBuffer);
	}

	bool Read2IntegerAndCount(const char* pSection, const char* pKey, int* nBuffer)
	{
		return ReadAndCount<int, 2>(pSection, pKey, nBuffer);
	}

	bool Read3Integers(const char* pSection, const char* pKey, int* nBuffer) {
		return Read<int, 3>(pSection, pKey, nBuffer);
	}

	size_t Read3IntegerAndCount(const char* pSection, const char* pKey, int* nBuffer)
	{
		return ReadAndCount<int, 3>(pSection, pKey, nBuffer);
	}

	bool Read4Integers(const char* pSection, const char* pKey, int* nBuffer) {
		return Read<int, 4>(pSection, pKey, nBuffer);
	}

	bool ReadBytes(const char *pSection, const char *pKey, BYTE*nBuffer) {
		return Read<BYTE, 1>(pSection, pKey, nBuffer);
	}

	bool Read2Bytes(const char *pSection, const char *pKey, BYTE*nBuffer) {
		return Read<BYTE, 2>(pSection, pKey, nBuffer);
	}

	bool Read3Bytes(const char *pSection, const char *pKey, BYTE*nBuffer) {
		return Read<BYTE, 3>(pSection, pKey, nBuffer);
	}

	bool ReadDouble(const char* pSection, const char* pKey, double* nBuffer) {
		return Read<double, 1>(pSection, pKey, nBuffer);
	}

	bool Read2Double(const char *pSection, const char *pKey, double *nBuffer) {
		return Read<double, 2>(pSection, pKey, nBuffer);
	}

	size_t Read2DoubleAndCount(const char* pSection, const char* pKey, double* nBuffer)
	{
		return ReadAndCount<double, 2>(pSection, pKey, nBuffer);
	}

	bool Read3Double(const char *pSection, const char *pKey, double *nBuffer)
	{
		return Read<double, 3>(pSection, pKey, nBuffer);
	}

	size_t Read3DoubleAndCount(const char* pSection, const char* pKey, double* nBuffer)
	{
		return ReadAndCount<double, 3>(pSection, pKey, nBuffer);
	}

	bool ReadFloat(const char *pSection, const char *pKey, float *nBuffer)
	{
		return Read<float, 1>(pSection, pKey, nBuffer);
	}

	bool Read2Float(const char *pSection, const char *pKey, float *nBuffer)
	{
		return Read<float, 2>(pSection, pKey, nBuffer);
	}

	bool Read3Float(const char *pSection, const char *pKey, float *nBuffer)
	{
		return Read<float, 3>(pSection, pKey, nBuffer);
	}

	bool ReadShort(const char* pSection, const char* pKey, short* nBuffer)
	{
		return Read<short, 1>(pSection, pKey, nBuffer);
	}

	bool Read2Short(const char* pSection, const char* pKey, short* nBuffer)
	{
		return Read<short, 2>(pSection, pKey, nBuffer);
	}

	bool ReadSpeed(const char* pSection, const char* pKey, int* nBuffer)
	{
		double parsedSpeed = IniFile->ReadDouble(pSection, pKey, -1.0);

		if (parsedSpeed >= 0.0)
		{
			int speed = int((MinImpl(parsedSpeed, 100.0) * 256.0) / 100.0);
			*nBuffer = MinImpl(speed, 255);
		}

		return (*nBuffer != -1);
	}

	bool ReadArmor(const char *pSection, const char *pKey, int *nBuffer) {
		*nBuffer = IniFile->ReadArmorType(pSection, pKey, *nBuffer);
		return (*nBuffer != -1);
	}

	// WARNING : const char* memory address may temporary and can invalidated
	bool ParseList(std::vector<const char*>& values, const char* pSection, const char* pKey) {
		if (this->ReadString(pSection, pKey)) {
			values.clear();
			char* context = nullptr;

			for (auto pCur = strtok_s(this->value(), Phobos::readDelims, &context); pCur; pCur = strtok_s(nullptr, Phobos::readDelims, &context))
				values.push_back(pCur);

			return true;
		}

		return false;
	}

	bool ParseList(std::vector<std::string>& values, const char* pSection, const char* pKey) {
		if (this->ReadString(pSection, pKey)) {
			values.clear();
			char* context = nullptr;

			for (auto pCur = strtok_s(this->value(), Phobos::readDelims, &context); pCur; pCur = strtok_s(nullptr, Phobos::readDelims, &context)){
				if(strlen(pCur)) {
					values.push_back(pCur);
				}
			}

			return true;
		}

		return false;
	}
};
