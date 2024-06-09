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

#include <Theater.h>
#include <CCINIClass.h>
#include <GeneralStructures.h>
#include <StringTable.h>
#include <Helpers/String.h>
#include <PCX.h>
#include <Leptons.h>

#include <algorithm>
#include <cstring>
#include <vector>
#include <functional>

#include <Phobos.h>
#include <Phobos.CRT.h>

#include "INIParser.h"

#include "PhobosFixedString.h"
#include "GameUniquePointers.h"

#include "Savegame.h"
#include "Debug.h"

template <typename T>
using UniqueDLLPtr = std::unique_ptr<T, DLLDeleter>;

// owns a resource. not copyable, but movable.
template <typename T, typename Deleter, T Default = T()>
struct Handle
{
	constexpr Handle() noexcept = default;

	constexpr explicit Handle(T value) noexcept
		: Value(value)
	{
	}

	Handle(const Handle&) = delete;

	constexpr Handle(Handle&& other) noexcept
		: Value(other.release())
	{
	}

	~Handle() noexcept
	{
		if (this->Value != Default)
		{
			Deleter {}(this->Value);
		}
	}

	Handle& operator = (const Handle&) = delete;

	Handle& operator = (Handle&& other) noexcept
	{
		this->reset(other.release());
		return *this;
	}

	constexpr explicit operator bool() const noexcept
	{
		return this->Value != Default;
	}

	constexpr operator T () const noexcept
	{
		return this->Value;
	}

	constexpr T get() const noexcept
	{
		return this->Value;
	}

	constexpr T operator->() const noexcept {
		return get();
	}

	constexpr T release() noexcept
	{
		return std::exchange(this->Value, Default);
	}

	void reset(T value) noexcept
	{
		Handle(this->Value);
		this->Value = value;
	}

	void clear() noexcept
	{
		Handle(std::move(*this));
	}

	void swap(Handle& other) noexcept
	{
		using std::swap;
		swap(this->Value, other.Value);
	}

	friend void swap(Handle& lhs, Handle& rhs) noexcept
	{
		lhs.swap(rhs);
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Savegame::ReadPhobosStream(Stm, this->Value, RegisterForChange);
	}

	bool save(PhobosStreamWriter& Stm) const
	{
		return Savegame::WritePhobosStream(Stm, this->Value);
	}

private:
	T Value { Default };
};

class TheaterSpecificSHP
{
public:
	constexpr TheaterSpecificSHP() noexcept = default;

	constexpr TheaterSpecificSHP(SHPStruct* pSHP)
		: value { pSHP }
	{ }

	constexpr operator SHPStruct* ()
	{
		return this->value;
	}

	constexpr SHPStruct* GetSHP()
	{
		return *this;
	}

	bool Read(INI_EX& parser, const char* pSection, const char* pKey);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Savegame::ReadPhobosStream(Stm, this->value, RegisterForChange);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return Savegame::WritePhobosStream(Stm, this->value);
	}

	~TheaterSpecificSHP() {
		GameDelete<true, true>(std::exchange(value , nullptr));
	}

private:
	SHPStruct* value { nullptr };

protected:
	TheaterSpecificSHP(const TheaterSpecificSHP& other) = delete;
	TheaterSpecificSHP& operator=(const TheaterSpecificSHP& other) = delete;
};

// vector of char* with builtin storage
class VectorNames {
protected:
	DynamicVectorClass<const char*> Strings {};
	char* Buffer{ nullptr };

public:
	VectorNames() = default;

	VectorNames(const char* pBuffer) {
		this->Tokenize(pBuffer);
	}

	~VectorNames() {
		this->Clear();
	}

	const char* operator[] (int index) const {
		return this->Strings.GetItemOrDefault(index);
	}

	constexpr const DynamicVectorClass<const char*>* Entries() const {
		return &this->Strings;
	}

	constexpr const char** ToString() const {
		return this->Strings.Items;
	}

	constexpr int Count() const {
		return this->Strings.Count;
	}

	void Clear() {
		if (this->Buffer) {
			this->Strings.Clear();
			YRMemory::Deallocate(this->Buffer);
			this->Buffer = nullptr;
		}
	}

	void Tokenize() {
		if (this->Buffer) {
			this->Strings.Reset();

			char* context = nullptr;
			for (auto cur = CRT::strtok(this->Buffer, ",", &context); cur && *cur; cur = CRT::strtok(nullptr, ",", &context)) {
				this->Strings.AddItem(cur);
			}
		}
	}

	void Tokenize(const char* pBuffer) {
		if (pBuffer) {
			this->Clear();
			this->Buffer = CRT::strdup(pBuffer);
			this->Tokenize();
		}
	}
protected:
	VectorNames(const VectorNames& other) = delete;
	VectorNames& operator=(const VectorNames& other) = delete;
};

// a poor man's map with contiguous storage
template <typename TKey, typename TValue , class customMem = std::allocator<std::pair<TKey , TValue>>>
class PhobosMap {
	using pair_t = std::pair<TKey, TValue>;
	using container_t = std::vector<pair_t , customMem>;
public:

	TValue& operator[] (const TKey& key) {
		if (auto pValue = this->tryfind(key)) {
			return *pValue;
		}
		return this->insert_unchecked(key, TValue());
	}

	//TValue* find__(const TKey& key) {
	//	auto pValue = static_cast<const PhobosMap*>(this)->tryfind(key);
	//	return const_cast<TValue*>(pValue);
	//}

	constexpr TValue* tryfind(const TKey& key) {
		auto it = this->get_key_iterator(key);
		if (it != this->values.end()) {
			return &it->second;
		}

		return nullptr;
	}

	// nonmodifiable
	constexpr const TValue* tryfind(const TKey& key) const {
		auto it = this->get_key_iterator(key);

		if (it != this->values.end()) {
			return &it->second;
		}

		return nullptr;
	}

	// nonmodifiable
	constexpr TValue get_or_default(const TKey& key) const {
		if (auto pValue = this->tryfind(key)) {
			return *pValue;
		}
		return TValue();
	}

	// nonmodifiable
	constexpr TValue get_or_default(const TKey& key, TValue def) const {
		if (auto pValue = this->tryfind(key)) {
			return *pValue;
		}
		return def;
	}

	constexpr void erase(container_t::iterator iter) {
		this->values.erase(iter, this->values.end());
	}

	constexpr bool erase(const TKey& key) {
		auto it = this->get_key_iterator(key);
		if (it != this->values.end()) {
			this->values.erase(it);
			return true;
		}
		return false;
	}

	constexpr bool contains(const TKey& key) const {
		return this->get_key_iterator(key) != values.end();
	}

	bool insert(const TKey& key, TValue value) {
		if (!this->tryfind(key)) {
			this->insert_unchecked(key, std::move(value));
			return true;
		}
		return false;
	}

	void emplace_unchecked(const TKey& key, TValue value) {
		this->insert_unchecked(key, std::move(value));
	}

	constexpr size_t size() const {
		return values.size();
	}

	constexpr bool empty() const {
		return values.empty();
	}

	constexpr void clear() {
		values.clear();
	}

	constexpr void reserve(size_t newsize) {
		values.reserve(newsize);
	}

	constexpr void resize(size_t newsize) {
		values.resize(newsize);
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange) {
		this->clear();

		size_t size = 0;
		auto ret = Stm.Load(size);

		if (ret && size) {
			this->values.resize(size);
			for (size_t i = 0; i < size; ++i) {
				if (!Savegame::ReadPhobosStream(Stm, this->values[i].first, RegisterForChange)
					|| !Savegame::ReadPhobosStream(Stm, this->values[i].second, RegisterForChange))
				{
					return false;
				}
			}
		}

		return ret;
	}

	bool save(PhobosStreamWriter& Stm) const {
		Stm.Save(this->values.size());

		for (const auto& [first,second] : this->values) {
			Savegame::WritePhobosStream(Stm, first);
			Savegame::WritePhobosStream(Stm, second);
		}

		return true;
	}

	constexpr [[nodiscard]] auto begin() noexcept
	{
		return values.begin();
	}

	constexpr [[nodiscard]] auto begin() const noexcept
	{
		return values.begin();
	}

	constexpr [[nodiscard]] auto end() noexcept
	{
		return values.end();
	}

	constexpr [[nodiscard]] auto end() const noexcept
	{
		return values.end();
	}

	constexpr auto back() const {
		return values.back();
	}

	constexpr auto back() {
		return values.back();
	}

	constexpr auto get_key_iterator(const TKey& key) {
		if constexpr (direct_comparable<TKey>){
			return std::find_if(this->values.begin(), this->values.end(), [&](const container_t::value_type& item) {
				return item.first == key;
			});
		} else {
			return std::find(this->values.begin(), this->values.end(), key);
		}
	}

	// nonmodifiable
	constexpr auto get_key_iterator(const TKey& key) const {
		if constexpr (direct_comparable<TKey>) {
			return std::find_if(this->values.begin(), this->values.end(), [&](const container_t::value_type& item) {
				return item.first == key;
			});
		} else {
			return std::find(this->values.begin(), this->values.end(), key);
		}
	}

	TValue& insert_unchecked(const TKey& key, TValue value) {
		this->values.emplace_back(key, std::move(value));
		return this->values.back().second;
	}

private:

	container_t values;
};

// pcx filename storage with optional automatic loading
class PhobosPCXFile {
	static constexpr const size_t Capacity = 0x20;
public:
	explicit PhobosPCXFile() : Surface(nullptr) , filename() {}

	PhobosPCXFile(const char* pFilename) : PhobosPCXFile() {
			*this = pFilename;
	}

	~PhobosPCXFile() = default;

	PhobosPCXFile(const PhobosPCXFile& other) = default;
	PhobosPCXFile& operator=(const PhobosPCXFile& other) = default;

	PhobosPCXFile& operator=(const char* pFilename) {

		// fucker
		if (!pFilename || !*pFilename || !strlen(pFilename)) {
			this->Clear();
			return *this;
		}

		this->filename = pFilename;
		auto& data = this->filename.data();
		_strlwr_s(data);

		BSurface* pSource = PCX::Instance->GetSurface(this->filename);
		if (!pSource && PCX::Instance->LoadFile(this->filename))
			pSource = PCX::Instance->GetSurface(this->filename);

		this->Surface = pSource;

		return *this;
	}

	PhobosPCXFile& operator=(std::string& pFilename) {

		// fucker
		if (pFilename.empty() || !*pFilename.data()) {
			this->Clear();
			return *this;
		}

		this->filename = pFilename.c_str();
		auto& data = this->filename.data();
		_strlwr_s(data);

		BSurface* pSource = PCX::Instance->GetSurface(this->filename);
		if (!pSource &&  PCX::Instance->ForceLoadFile(this->filename, 2, 0))
			pSource = PCX::Instance->GetSurface(this->filename);

		this->Surface = pSource;

		return *this;
	}

	constexpr const char* GetFilename() const {
		return this->filename.data();
	}

	constexpr BSurface* GetSurface() const {
		return this->Surface;
	}

	constexpr bool Exists() const {
		return this->Surface;
	}

	bool Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "") {
		char buffer[Capacity];
		if (pINI->ReadString(pSection, pKey, pDefault, buffer) > 0) {

			std::string cachedWithExt = _strlwr(buffer);

			if (cachedWithExt.find(".pcx") == std::string::npos)
				cachedWithExt += ".pcx";

			*this = cachedWithExt;

			if (this->filename && !this->Surface) {
				Debug::INIParseFailed(pSection, pKey, this->filename, "PCX file not found.");
			}
		}

		return buffer[0] != 0;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) {

		this->Clear();
		void* oldPtr;
		const auto ret = Stm.Load(oldPtr) && Stm.Load(this->filename);

		if (!ret)
			return false;

		if (oldPtr && this->filename) {
			BSurface* pSource = PCX::Instance->GetSurface(this->filename);
			if( !pSource && PCX::Instance->LoadFile(this->filename))
				pSource = PCX::Instance->GetSurface(this->filename, nullptr);

			this->Surface = pSource;

			if (!this->Surface) {
				Debug::Log("PCX file[%s] not found.\n",this->filename.data());
			}

			SwizzleManagerClass::Instance().Here_I_Am((long)oldPtr, this->Surface);
		}

		return true;
	}

	bool Save(PhobosStreamWriter& Stm) const {
		Stm.Save(this->Surface);
		Stm.Save(this->filename);
		return true;
	}

private:

	void Clear() {
		this->Surface = nullptr;
		this->filename = nullptr;
	}

	BSurface* Surface { nullptr };
	FixedString<Capacity> filename;
};

// provides storage for a csf label with automatic lookup.
class CSFText {
	static constexpr const size_t Capacity = 0x20;
public:
	CSFText() noexcept {}
	explicit CSFText(nullptr_t) noexcept {}

	explicit CSFText(const char* label) noexcept {
		*this = label;
	}

	~CSFText() noexcept = default;

	CSFText& operator = (CSFText const& rhs) = default;
	CSFText(const CSFText& other) = default;

	const CSFText& operator = (const char* label) {
		if (this->Label != label) {
			this->Label.assign(label);

			this->Text = nullptr;

			if (this->Label) {
				this->Text = StringTable::LoadString(this->Label);
			}
		}

		return *this;
	}

	template<bool check = true>
	void FORCEINLINE PrintAsMessage(int colorScheme) const {

		if constexpr (check) {
			if (this->empty())
				return;
		}

		MessageListClass::Instance->PrintMessage(this->Text, RulesClass::Instance->MessageDelay, colorScheme);
	}

	constexpr operator const wchar_t* () const {
		return this->Text;
	}

	constexpr bool empty() const {
		return !this->Text || !*this->Text;
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange) {
		this->Text = nullptr;
		if (Stm.Load(this->Label.data())) {
			if (this->Label) {
				this->Text = StringTable::LoadString(this->Label);
			}
			return true;
		}
		return false;
	}

	bool save(PhobosStreamWriter& Stm) const {
		Stm.Save(this->Label.data());
		return true;
	}

public:

	FixedString<0x20> Label;
	const wchar_t* Text{ nullptr };

};

// a wrapper for an optional value
template <typename T, bool Persistable = false>
struct OptionalStruct {
	constexpr OptionalStruct() = default;
	explicit OptionalStruct(T value) noexcept : Value(std::move(value)), HasValue(true) {}

	OptionalStruct& operator= (T value) {
		this->Value = std::move(value);
		this->HasValue = true;
		return *this;
	}

	~OptionalStruct() noexcept { this->clear(); }

	constexpr OptionalStruct(const OptionalStruct& other) = default;
	constexpr OptionalStruct& operator=(const OptionalStruct& other) = default;

	constexpr operator T& () noexcept {
		return this->Value;
	}

	constexpr operator const T& () const noexcept {
		return this->Value;
	}

	constexpr void clear() {
		this->Value = T();
		this->HasValue = false;
	}

	constexpr bool empty() const noexcept {
		return !this->HasValue;
	}

	explicit operator bool() const noexcept {
		return this->HasValue;
	}

	bool has_value() const noexcept {
		return this->HasValue;
	}

	constexpr bool isset() const noexcept {
		return this->HasValue;
	}

	constexpr const T& get() const noexcept {
		return this->Value;
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange) {
		this->clear();

		return load(Stm, RegisterForChange, std::bool_constant<Persistable>());
	}

	bool save(PhobosStreamWriter& Stm) const {
		return save(Stm, std::bool_constant<Persistable>());
	}

private:
	bool load(PhobosStreamReader& Stm, bool RegisterForChange, std::true_type) {
		if (Stm.Load(this->HasValue)) {
			if (!this->HasValue || Savegame::ReadPhobosStream(Stm, this->Value, RegisterForChange)) {
				return true;
			}
		}
		return false;
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChangestd, std::false_type) {
		return true;
	}

	bool save(PhobosStreamWriter& Stm, std::true_type) const {
		Stm.Save(this->HasValue);
		if (this->HasValue) {
			Savegame::WritePhobosStream(Stm, this->Value);
		}
		return true;
	}

	bool save(PhobosStreamWriter& Stm, std::false_type) const {
		return true;
	}

	T Value{};
	bool HasValue{ false };
};

struct GameConfig {

	GameConfig(const char* pFilename) noexcept : File { nullptr }
		, Ini { nullptr }
	{
		this->File.reset(GameCreate<CCFileClass>(pFilename));
	}

	~GameConfig() noexcept = default;

	bool OpenINI(FileAccessMode mode = FileAccessMode::Read) noexcept
	{
		if (!File->Exists() || !File->Open(mode))
		{
			Debug::Log("Failed to Open file %s \n", this->File->FileName);
			return false;
		}

		Ini.reset(GameCreate<CCINIClass>());
		Ini->ReadCCFile(this->File.get());
		Ini->CurrentSection = nullptr;
		Ini->CurrentSectionName = nullptr;

		return true;
	}

	template <typename Func>
	void OpenINIAction(Func&& action ,FileAccessMode mode = FileAccessMode::Read) noexcept
	{
		if (!File->Exists() || !File->Open(mode)) {
			Debug::Log("Failed to Open file %s \n", this->File->FileName);
			return;
		}

		Ini.reset(GameCreate<CCINIClass>());
		Ini->ReadCCFile(this->File.get());
		Ini->CurrentSection = nullptr;
		Ini->CurrentSectionName = nullptr;
		action(Ini.get());

		return;
	}

	bool OpenOrCreate(FileAccessMode mode = FileAccessMode::ReadWrite) noexcept
	{
		if (!File->Exists() || !File->CreateFileA() || !File->Open(mode))
		{
			Debug::Log("Failed to Open file %s \n", this->File->FileName);
			return false;
		}

		Ini.reset(GameCreate<CCINIClass>());
		Ini->ReadCCFile(this->File.get());
		Ini->CurrentSection = nullptr;
		Ini->CurrentSectionName = nullptr;

		return true;
	}

	FORCEINLINE void WriteCCFile()
	{
		Ini->WriteCCFile(File.get());
	}

	const char* filename() noexcept
	{
		return File->FileName;
	}

	CCINIClass* get() noexcept
	{
		return Ini.get();
	}

	CCINIClass* operator->() noexcept
	{
		return Ini.get();
	}

protected:
	UniqueGamePtr<CCFileClass> File;
	UniqueGamePtr<CCINIClass> Ini;
};