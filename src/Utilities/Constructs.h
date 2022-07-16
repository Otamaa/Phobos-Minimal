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

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>
#include <functional>

#include <Phobos.h>
#include <Phobos.CRT.h>

#include "PhobosFixedString.h"
#include "Savegame.h"
#include "Debug.h"

class ConvertClass;

template <typename T>
using UniqueGamePtr = std::unique_ptr<T, GameDeleter>;

//doesnt work with array !
//make new one !
template<typename T , typename... TArgs>
UniqueGamePtr<T> Make_UniqueGamePtr(TArgs&&... args)
{
	static_assert(std::is_constructible<T, TArgs...>::value, "Cannot construct T from TArgs.");
	GameAllocator<T> alloc;
	auto ptr = Memory::Create<T>(alloc, std::forward<TArgs>(args)...);
	return { ptr , GameDeleter() };
}

template <typename T>
using UniqueDLLPtr = std::unique_ptr<T, DLLDeleter>;

struct Leptons {
	Leptons() = default;
	//explicit Leptons(int value) noexcept : value(value) {}
	explicit Leptons(const int value) noexcept : value(value) { }
	explicit Leptons(double velue) noexcept : value(Game::F2I(velue * 256.0)){}

	operator int() const
	{ return this->value; }

	inline unsigned long ToLong() const
	{ return static_cast<std::make_unsigned<long>::type> (this->value); }

	inline double ToDouble() const
	{ return static_cast<double>(this->value / 256.0); }

	inline int ToCell() const
	{
		if (this->value >= (256 / 2))
		{
			return (this->value / 256) + 1;
		}
		return (this->value / 256);
	}

	inline int ToPixel()
	{ return (((int)(signed short)this->value * 48) + (256 / 2) - ((this->value < 0) ? (256 - 1) : 0)) / 256; }

	int value{ 0 };
};

class CustomPalette {
public:
	enum class PaletteMode : unsigned int {
		Default = 0,
		Temperate = 1
	};

	PaletteMode Mode{ PaletteMode::Default };
	UniqueGamePtr<ConvertClass> Convert{ nullptr };
	UniqueGamePtr<BytePalette> Palette{ nullptr };

	CustomPalette() = default;
	explicit CustomPalette(PaletteMode mode) noexcept : Mode(mode) {};

	ConvertClass* GetConvert() const {
		return this->Convert.get();
	}

	ConvertClass* GetOrDefaultConvert(ConvertClass* const& pDefault) const {
		return this->Convert.get() ? this->Convert.get() : pDefault;
	}

	bool Read(
		CCINIClass* pINI, const char* pSection, const char* pKey,
		const char* pDefault = "");
	bool LoadFromName(const char* PaletteName);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	bool CreateFromBytePalette(BytePalette nBytePal);

private:
	void Clear();
	void CreateConvert();
};

// vector of char* with builtin storage
class VectorNames {
protected:
	DynamicVectorClass<const char*> Strings;
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

	const DynamicVectorClass<const char*>& Entries() const {
		return this->Strings;
	}

	const char** ToString() const {
		return this->Strings.Items;
	}

	int Count() const {
		return this->Strings.Count;
	}

	void Clear() {
		if (this->Buffer) {
			this->Strings.Clear();
			free(this->Buffer);
			this->Buffer = nullptr;
		}
	}

	void Tokenize() {
		if (this->Buffer) {
			this->Strings.Clear();

			char* context = nullptr;
			for (auto cur = strtok_s(this->Buffer, ",", &context); cur && *cur; cur = strtok_s(nullptr, ",", &context)) {
				this->Strings.AddItem(cur);
			}
		}
	}

	void Tokenize(const char* pBuffer) {
		if (pBuffer) {
			this->Clear();
			this->Buffer = _strdup(pBuffer);
			this->Tokenize();
		}
	}
};

// a poor man's map with contiguous storage
template <typename TKey, typename TValue>
class PhobosMap {
	using container_t = std::vector<std::pair<TKey, TValue>>;
public:

	TValue& operator[] (const TKey& key) {
		if (auto pValue = this->find(key)) {
			return *pValue;
		}
		return this->insert_unchecked(key, TValue());
	}

	TValue* find(const TKey& key) {
		auto pValue = static_cast<const PhobosMap*>(this)->find(key);
		return const_cast<TValue*>(pValue);
	}

	const TValue* find(const TKey& key) const {
		auto it = this->get_key_iterator(key);
		if (it != this->values.end()) {
			return &it->second;
		}
		return nullptr;
	}

	TValue get_or_default(const TKey& key) const {
		if (auto pValue = this->find(key)) {
			return *pValue;
		}
		return TValue();
	}

	TValue get_or_default(const TKey& key, TValue def) const {
		if (auto pValue = this->find(key)) {
			return *pValue;
		}
		return def;
	}

	bool erase(const TKey& key) {
		auto it = this->get_key_iterator(key);
		if (it != this->values.end()) {
			this->values.erase(it);
			return true;
		}
		return false;
	}

	bool contains(const TKey& key) const {
		return this->get_key_iterator(key) != values.end();
	}

	bool insert(const TKey& key, TValue value) {
		if (!this->find(key)) {
			this->insert_unchecked(key, std::move(value));
			return true;
		}
		return false;
	}

	size_t size() const {
		return values.size();
	}

	bool empty() const {
		return values.empty();
	}

	void clear() {
		values.clear();
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

	using iterator = container_t::iterator;
	using const_iterator = container_t::const_iterator;

	[[nodiscard]] iterator begin() noexcept
	{
		return values.begin();
	}

	[[nodiscard]] const_iterator begin() const noexcept
	{
		return values.begin();
	}

	[[nodiscard]] iterator end() noexcept
	{
		return values.end();
	}

	[[nodiscard]] const_iterator end() const noexcept
	{
		return values.end();
	}

	typename container_t::const_iterator get_key_iterator(const TKey& key) const {
		return std::find_if(this->values.begin(), this->values.end(), [&](const container_t::value_type& item) {
			return item.first == key;
		});
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
	explicit PhobosPCXFile(bool autoResolve = true) : filename(), resolve(autoResolve), checked(false), exists(false) {
	}

	PhobosPCXFile(const char* pFilename, bool autoResolve = true) : PhobosPCXFile(autoResolve) {
		*this = pFilename;
	}

	PhobosPCXFile& operator = (const char* pFilename) {
		this->filename = pFilename;
		auto& data = this->filename.data();
		_strlwr_s(data);

		this->checked = false;
		this->exists = false;

		if (this->resolve) {
			this->Exists();
		}

		return *this;
	}

	const FixedString<Capacity>::data_type& GetFilename() const {
		return this->filename.data();
	}

	BSurface* GetSurface(BytePalette* pPalette = nullptr) const {
		return this->Exists() ? PCX::Instance->GetSurface(this->filename, pPalette) : nullptr;
	}

	bool Exists() const {
		if (!this->checked) {
			this->checked = true;
			if (this->filename) {
				auto pPCX = &PCX::Instance();
				this->exists = (pPCX->GetSurface(this->filename) || pPCX->LoadFile(this->filename));
			}
		}
		return this->exists;
	}

	bool Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "") {
		char buffer[Capacity];
		if (pINI->ReadString(pSection, pKey, pDefault, buffer)) {
			*this = buffer;

			if (this->checked && !this->exists) {
				Debug::INIParseFailed(pSection, pKey, this->filename, "PCX file not found.");
			}
		}
		return buffer[0] != 0;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) {
		this->filename = nullptr;
		if (Stm.Load(*this)) {
			if (this->checked && this->exists) {
				this->checked = false;
				if (!this->Exists()) {
					Debug::Log("PCX file '%s' was not found.\n", this->filename.data());
				}
			}
			return true;
		}
		return false;
	}

	bool Save(PhobosStreamWriter& Stm) const {
		Stm.Save(*this);
		return true;
	}

private:
	FixedString<Capacity> filename;
	bool resolve;
	mutable bool checked;
	mutable bool exists;
};

// provides storage for a csf label with automatic lookup.
class CSFText {
public:
	CSFText() noexcept {}
	explicit CSFText(nullptr_t) noexcept {}

	explicit CSFText(const char* label) noexcept {
		*this = label;
	}

	CSFText& operator = (CSFText const& rhs) = default;

	const CSFText& operator = (const char* label) {
		if (this->Label != label) {
			this->Label = label;
			this->Text = nullptr;

			if (this->Label) {
				this->Text = StringTable::LoadString(this->Label);
			}
		}

		return *this;
	}

	operator const wchar_t* () const {
		return this->Text;
	}

	bool empty() const {
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

	FixedString<0x20> Label;
	const wchar_t* Text{ nullptr };
};

// a wrapper for an optional value
template <typename T, bool Persistable = false>
struct OptionalStruct {
	OptionalStruct() = default;
	explicit OptionalStruct(T value) noexcept : Value(std::move(value)), HasValue(true) {}

	OptionalStruct& operator= (T value) {
		this->Value = std::move(value);
		this->HasValue = true;
		return *this;
	}

	operator T& () noexcept {
		return this->Value;
	}

	operator const T& () const noexcept {
		return this->Value;
	}

	void clear() {
		this->Value = T();
		this->HasValue = false;
	}

	bool empty() const {
		return !this->HasValue;
	}

	constexpr explicit operator bool() const noexcept {
		return this->HasValue;
	}

	constexpr bool has_value() const noexcept {
		return this->HasValue;
	}

	const T& get() const noexcept {
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

// owns a resource. not copyable, but movable.
template <typename T, typename Deleter, T Default = T()>
struct Handle {
	constexpr Handle() noexcept = default;

	constexpr explicit Handle(T value) noexcept
		: Value(value)
	{ }

	Handle(const Handle&) = delete;

	constexpr Handle(Handle&& other) noexcept
		: Value(other.release())
	{ }

	~Handle() noexcept {
		if (this->Value != Default) {
			Deleter{}(this->Value);
		}
	}

	Handle& operator = (const Handle&) = delete;

	Handle& operator = (Handle&& other) noexcept {
		this->reset(other.release());
		return *this;
	}

	constexpr explicit operator bool() const noexcept {
		return this->Value != Default;
	}

	constexpr operator T () const noexcept {
		return this->Value;
	}

	constexpr T get() const noexcept {
		return this->Value;
	}

	T release() noexcept {
		return std::exchange(this->Value, Default);
	}

	void reset(T value) noexcept {
		Handle(this->Value);
		this->Value = value;
	}

	void clear() noexcept {
		Handle(std::move(*this));
	}

	void swap(Handle& other) noexcept {
		using std::swap;
		swap(this->Value, other.Value);
	}

	friend void swap(Handle& lhs, Handle& rhs) noexcept {
		lhs.swap(rhs);
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange) {
		return Savegame::ReadPhobosStream(Stm, this->Value, RegisterForChange);
	}

	bool save(PhobosStreamWriter& Stm) const {
		return Savegame::WritePhobosStream(Stm, this->Value);
	}

private:
	T Value{ Default };
};
