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

#include "Iterator.h"
#include "Enum.h"

#include <MouseClass.h>
#include <FootClass.h>

#include "Savegame.h"

class INI_EX;

/**
 * More fancy templates!
 * This one is just a nicer-looking INI Parser... the fun starts with the next one
 */

template<typename T>
class Valueable
{
public:

	T Value {};

	//using value_type = T;
	//using base_type = std::remove_pointer_t<T>;

	Valueable() = default;
	explicit Valueable(T value) noexcept(noexcept(T { std::move(value) })) : Value(std::move(value)) { }
	Valueable(Valueable const& other) = default;
	Valueable(Valueable&& other) = default;

	~Valueable() = default;

	Valueable& operator = (Valueable const& value) = default;
	Valueable& operator = (Valueable&& value) = default;

	template <typename Val, typename = std::enable_if_t<std::is_assignable<T&, Val&&>::value>>
	Valueable& operator = (Val value)
	{
		this->Value = std::move(value);
		return *this;
	}

	operator const T& () const noexcept
	{
		return this->Get();
	}

	// only allow this when explict works, otherwise
	// the always-non-null pointer will be used in conditionals.
	//explicit operator T* () noexcept {
	//	return this->GetEx();
	//}

	//T operator -> () const
	//{
	//	return this->Get();
	//}

	auto operator->() const noexcept {
		if constexpr (std::is_pointer<T>::type())
			return this->Value;
		else
			return &this->Value;
	}

	//value_type* operator& () noexcept
	//{
	//	return this->GetEx();
	//}

	bool operator!() const
	{
		if constexpr (std::is_pointer<T>::type())
			return !this->Value;
		else
			return  this->Get() == 0;
	}

	const T& Get() const noexcept
	{
		return this->Value;
	}

	T* GetEx() noexcept
	{
		return &this->Value;
	}

	const T* GetEx() const noexcept
	{
		return &this->Value;
	}

	inline void Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate = false);

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	inline bool Save(PhobosStreamWriter& Stm) const;
};

template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
inline bool operator == (const Valueable<T>& val, const T& other)
{
	return val.Get() == other;
}

template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
inline bool operator == (const T& other, const Valueable<T>& val)
{
	return val.Get() == other;
}

template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
inline bool operator != (const Valueable<T>& val, const T& other)
{
	return !(val == other);
}

template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
inline bool operator != (const T& other, const Valueable<T>& val)
{
	return !(val == other);
}
// more fun
template<typename Lookuper>
class ValueableIdx : public Valueable<int>
{
public:
	ValueableIdx() noexcept : Valueable<int>(-1) { }
	explicit ValueableIdx(int value) noexcept : Valueable<int>(value) { }
	ValueableIdx(ValueableIdx const& other) = default;
	ValueableIdx(ValueableIdx&& other) = default;
	~ValueableIdx() = default;

	ValueableIdx& operator = (ValueableIdx const& value) = default;
	ValueableIdx& operator = (ValueableIdx&& value) = default;

	template <typename Val, typename = std::enable_if_t<std::is_assignable<int&, Val&&>::value>>
	ValueableIdx& operator = (Val value)
	{
		this->Value = std::move(value);
		return *this;
	}

	size_t ToUnsigned() const noexcept {
		return (size_t)this->Value;
	}

	inline void Read(INI_EX& parser, const char* pSection, const char* pKey);
};

template<typename T>
class Nullable : public Valueable<T>
{
protected:
	bool HasValue { false };
public:

	Nullable() = default;
	explicit Nullable(T value) noexcept(noexcept(Valueable<T>{std::move(value)})) : Valueable<T>(std::move(value)), HasValue(true) { }
	Nullable(Nullable const& other) = default;
	Nullable(Nullable&& other) = default;
	~Nullable() = default;

	Nullable& operator = (Nullable const& value) = default;
	Nullable& operator = (Nullable&& value) = default;

	template <typename Val, typename = std::enable_if_t<std::is_assignable<T&, Val&&>::value>>
	Nullable& operator = (Val value)
	{
		this->Value = std::move(value);
		this->HasValue = true;
		return *this;
	}

	bool isset() const noexcept
	{
		return this->HasValue;
	}

	const T& Get() const noexcept
	{
		return this->Value;
	}

	T* GetEx() noexcept
	{
		return &this->Value;
	}

	const T* GetEx() const noexcept
	{
		return &this->Value;
	}

	T Get(const T& ndefault) const
	{
		return this->HasValue ? this->Get() : ndefault;
	}

	const T& GetB(const T& ndefault) const
	{
		return this->HasValue ? this->Get() : ndefault;
	}

	T Get(const Valueable<T>& ndefault) const
	{
		return this->HasValue ? this->Get() : ndefault.Get();
	}

	const T& GetB(const Valueable<T>& ndefault) const
	{
		return this->HasValue ? this->Get() : ndefault.Get();
	}

	T* GetEx(T* ndefault) noexcept
	{
		return this->isset() ? this->GetEx() : ndefault;
	}

	const T* GetEx(const T* ndefault) const noexcept
	{
		return this->isset() ? this->GetEx() : ndefault;
	}

	T* GetEx(Valueable<T>& ndefault) noexcept
	{
		return this->isset() ? this->GetEx() : ndefault.GetEx();
	}

	const T* GetEx(const Valueable<T>& ndefault) const noexcept
	{
		return this->isset() ? this->GetEx() : ndefault.GetEx();
	}

	void Reset()
	{
		this->Value = T();
		this->HasValue = false;
	}

	template <typename Val, typename = std::enable_if_t<std::is_assignable<T&, Val&&>::value>>
	void Reset(Val ndefault) const
	{
		this->Value = std::move(ndefault); // set the value to args
		this->HasValue = true;
	}

	inline void Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate = false);

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	inline bool Save(PhobosStreamWriter& Stm) const;
};

template<typename Lookuper>
class NullableIdx : public Nullable<int>
{
public:
	NullableIdx() noexcept : Nullable<int>(-1) { this->HasValue = false; }
	explicit NullableIdx(int value) noexcept : Nullable<int>(value) { }
	NullableIdx(NullableIdx const& other) = default;
	NullableIdx(NullableIdx&& other) = default;
	~NullableIdx() = default;

	NullableIdx& operator = (NullableIdx const& value) = default;
	NullableIdx& operator = (NullableIdx&& value) = default;

	template <typename Val, typename = std::enable_if_t<std::is_assignable<int&, Val&&>::value>>
	NullableIdx& operator = (Val value)
	{
		this->Value = std::move(value);
		this->HasValue = true;
		return *this;
	}

	inline void Read(INI_EX& parser, const char* pSection, const char* pKey);
};

/*
 * This template is for something that varies depending on a unit's Veterancy Level
 * Promotable<int> PilotChance; // class def
 * PilotChance(); // ctor init-list
 * PilotChance->Read(..., "Base%s"); // load from ini
 * PilotChance->Get(Unit); // usage
 *
 * Use %s format specifier, exactly once. If pSingleFlag is null, pBaseFlag will
 * be used. For the single flag name, a trailing dot (after replacing %s) will
 * be removed. I.e. "Test.%s" will be converted to "Test".
 */
template<typename T>
class Promotable
{
public:
	T Rookie {};
	T Veteran {};
	T Elite {};

	Promotable() = default;
	explicit Promotable(T const& all) noexcept(noexcept(T { all })) : Rookie(all), Veteran(all), Elite(all) { }
	explicit Promotable(T const& r, T const& v, T const& e) 
	noexcept(noexcept(T { r }) && noexcept(T { v }) && noexcept(T { e })) :
		Rookie(r), Veteran(v), Elite(e) { }

	~Promotable() = default;

	void SetAll(const T& val) {
		this->Elite = this->Veteran = this->Rookie = val;
	}

	inline void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag, const char* pSingleFlag = nullptr, bool allocate = false);

	const T* GetEx(TechnoClass* pTechno) const noexcept {
		return &this->Get(pTechno);
	}

	const T& GetFromSpecificRank(Rank rank)const noexcept
	{
		if (rank == Rank::Elite)
		{
			return this->Elite;
		}

		if (rank == Rank::Veteran)
		{
			return this->Veteran;
		}

		return this->Rookie;
	}

	const T& Get(TechnoClass* pTechno) const noexcept {
		auto const rank = pTechno->Veterancy.GetRemainingLevel();
		if (rank == Rank::Elite)
		{
			return this->Elite;
		}
		if (rank == Rank::Veteran)
		{
			return this->Veteran;
		}
		return this->Rookie;
	}

	const T& GetFromCurrentRank(TechnoClass* pTechno) const noexcept {
		if (pTechno->CurrentRanking == Rank::Elite)
		{
			return this->Elite;
		}
		if (pTechno->CurrentRanking == Rank::Veteran)
		{
			return this->Veteran;
		}
		return this->Rookie;
	}

	const T& GetOrDefault(TechnoClass* pTechno, const T& nDefault) const noexcept {
		auto nRes = Get(pTechno);
		return nRes ? nRes : nDefault;
	}

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	inline bool Save(PhobosStreamWriter& Stm) const;
};

template<class T>
class ValueableVector : public std::vector<T>
{
public:
	using value_type = T;
	using base_type = std::remove_pointer_t<T>;

	ValueableVector() noexcept = default;
	ValueableVector(size_t Reserve) noexcept : std::vector<T>()
	{
		this->reserve(Reserve);
	}

	~ValueableVector() = default;

	inline void Read(INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate = false);

	bool Contains(const T& other) const
	{
		if constexpr (direct_comparable<T>) {
			for (auto pos = this->begin();
					pos != this->end();
					++pos)
			{
				if ((*pos) == other)
				{
					return true;
				}
			}

			return false;
		}
		else { return std::find(this->begin(), this->end(), other) != this->end(); }
	}

	int IndexOf(const T& other) const
	{
		if constexpr (direct_comparable<T>)
		{
			for (auto pos = this->begin();
				pos != this->end();
				++pos)
			{
				if ((*pos) == other) {
					return std::distance(this->begin(), pos);
				}
			}
		}
		else
		{
			auto const iter = std::find(this->begin(), this->end(), other);
			if (iter != this->end())
				return std::distance(this->begin(), iter);
		}

		return -1;
	}

	bool ValidIndex(int index) const {
		return static_cast<size_t>(index) < this->size();
	}

	T GetItemAt(int nIdx) const {

		if (!this->ValidIndex(nIdx))
			return T();

		return *(this->begin() + nIdx);
	}

	T GetItemAtOrMax(int nIdx) const
	{
		if (!this->ValidIndex(nIdx))
			nIdx = this->size();

		return *(this->begin() + nIdx);
	}

	T GetItemAtOrDefault(int nIdx , const T& other) const
	{
		if (!this->ValidIndex(nIdx))
			return other;

		return *(this->begin() + nIdx);
	}

	void PushbackUnique(const T& other) const
	{
		if (this->Contains(other)) return;
		else { this->push_back(other); }
	}

	void EmpalacebackUnique(const T& other) const
	{
		if (this->Contains(other)) return;
		else { this->empalace_back(other); }
	}

	Iterator<T> GetElements() const noexcept
	{
		return Iterator<T>(*this);
	}

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	inline bool Save(PhobosStreamWriter& Stm) const;
};

template<class T>
class NullableVector : public ValueableVector<T>
{
protected:
	bool hasValue { false };
public:
	NullableVector() noexcept = default;
	~NullableVector() = default;
	inline void Read(INI_EX& parser, const char* pSection, const char* pKey);

	bool HasValue() const noexcept
	{
		return this->hasValue;
	}

	void SetHasValue(bool bCond)
	{
		if (!bCond)
			Reset();
		else
			this->hasValue = true;
	}

	void Reset()
	{
		this->clear();
		this->hasValue = false;
	}
	using ValueableVector<T>::GetElements;

	Iterator<T> GetElements(Iterator<T> ndefault) const noexcept
	{
		if (!this->hasValue)
		{
			return ndefault;
		}

		return this->GetElements();
	}

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	inline bool Save(PhobosStreamWriter& Stm) const;
};

template<typename Lookuper>
class ValueableIdxVector : public ValueableVector<int>
{
public:
	inline void Read(INI_EX& parser, const char* pSection, const char* pKey);
};

template<typename Lookuper>
class NullableIdxVector : public NullableVector<int>
{
public:
	inline void Read(INI_EX& parser, const char* pSection, const char* pKey);
};

/*
 * This template is for something that varies depending on Technos damage state (or arbitrary 'health' ratio provided).
 * Damageable<AnimClass*> TestAnims; // class def
 * TestAnims(); // ctor init-list
 * TestAnims->Read(..., "Base%s"); // load from ini
 * TestAnims->Get(Techno); // usage
 * TestAnims->Get(healthRatio); // alternate usage
 *
 * Use %s format specifier, exactly once. If pSingleFlag is null, pBaseFlag will
 * be used. For the single flag name, a trailing dot (after replacing %s) will
 * be removed. I.e. "Test.%s" will be converted to "Test".
 */
template<typename T>
class Damageable
{
public:
	Valueable<T> BaseValue {};
	Nullable<T> ConditionYellow {};
	Nullable<T> ConditionRed {};

	Damageable() noexcept = default;

	explicit Damageable(T const& all)
		noexcept(noexcept(T { all }))
		: BaseValue { all }
	{
	}

	explicit Damageable(T const& undamaged, T const& damaged)
		noexcept(noexcept(T { undamaged }) && noexcept(T { damaged }))
		: BaseValue { undamaged }, ConditionYellow { damaged }
	{
	}

	explicit Damageable(T const& green, T const& yellow, T const& red)
		noexcept(noexcept(T { green }) && noexcept(T { yellow }) && noexcept(T { red }))
		: BaseValue { green }, ConditionYellow { yellow }, ConditionRed { red }
	{
	}

	~Damageable() = default;
	inline void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag, const char* pSingleFlag = nullptr , bool Alloc = false);

	const T* GetEx(TechnoClass* pTechno) const noexcept
	{
		return &this->Get(pTechno);
	}

	const T& Get(TechnoClass* pTechno) const noexcept
	{
		return Get(pTechno->GetHealthPercentage());
	}

	const T* GetEx(double ratio) const noexcept
	{
		return &this->Get(ratio);
	}

	const T& Get(double ratio) const noexcept
	{
		if (this->ConditionRed.isset() && ratio <= RulesClass::Instance->ConditionRed)
			return this->ConditionRed;
		else if (this->ConditionYellow.isset() && ratio <= RulesClass::Instance->ConditionYellow)
			return this->ConditionYellow;

		return this->BaseValue;
	}

	const T& Get(HealthState const& nState) const noexcept
	{
		if (this->ConditionRed.isset() && (nState == HealthState::Red))
			return this->ConditionRed;
		else if (this->ConditionYellow.isset() && (nState == HealthState::Yellow))
			return this->ConditionYellow;

		return this->BaseValue;
	}

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	inline bool Save(PhobosStreamWriter& Stm) const;
};

struct HealthOnFireData
{
protected:
	bool RedOnFire;
	bool GreenOnFire;
	bool YellowOnFire;

public:
	HealthOnFireData() noexcept :
		RedOnFire { true }, GreenOnFire { false }, YellowOnFire { true }
	{
	}

	HealthOnFireData(bool All) noexcept :
		RedOnFire { All }, GreenOnFire { All }, YellowOnFire { All }
	{
	}

	HealthOnFireData(bool R, bool G, bool Y) noexcept :
		RedOnFire { R }, GreenOnFire { G }, YellowOnFire { Y }
	{
	}

	~HealthOnFireData() = default;
	inline bool Get(HealthState const& nState) const noexcept
	{
		return (nState == HealthState::Green && GreenOnFire)
			|| (nState == HealthState::Yellow && YellowOnFire)
			|| (nState == HealthState::Red && RedOnFire);
	}

	inline bool Read(INI_EX& parser, const char* pSection, const char* pKey);
	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	inline bool Save(PhobosStreamWriter& Stm) const;
};

template<typename T>
class DamageableVector
{
public:
	ValueableVector<T> BaseValue {};
	NullableVector<T> ConditionYellow {};
	NullableVector<T> ConditionRed {};
	NullableVector<T> MaxValue {};

	DamageableVector() noexcept  = default;

	explicit DamageableVector(ValueableVector<T> const& all)
		noexcept(noexcept(ValueableVector<T> { all }))
		: BaseValue { all }
	{
	}

	explicit DamageableVector(ValueableVector<T> const& undamaged, NullableVector<T> const& damaged)
		noexcept(noexcept(ValueableVector<T> { undamaged }) && noexcept(NullableVector<T> { damaged }))
		: BaseValue { undamaged }, ConditionYellow { damaged }
	{
	}

	explicit DamageableVector(ValueableVector<T> const& green, NullableVector<T> const& yellow, NullableVector<T> const& red)
		noexcept(noexcept(ValueableVector<T> { green }) && noexcept(NullableVector<T> { yellow }) && noexcept(NullableVector<T> { red }))
		: BaseValue { green }, ConditionYellow { yellow }, ConditionRed { red }
	{
	}

	explicit DamageableVector(ValueableVector<T> const& green, NullableVector<T> const& yellow, NullableVector<T> const& red, NullableVector<T> const& max)
		noexcept(noexcept(ValueableVector<T> { green }) && noexcept(NullableVector<T> { yellow }) && noexcept(NullableVector<T> { red }) && noexcept(NullableVector<T> { max }))
		: BaseValue { green }, ConditionYellow { yellow }, ConditionRed { red }, MaxValue { max }
	{
	}

	~DamageableVector() = default;

	inline void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag, const char* pSingleFlag = nullptr);

	const ValueableVector<T>* GetEx(TechnoClass* pTechno) const noexcept
	{
		return &this->Get(pTechno);
	}

	const ValueableVector<T>& Get(TechnoClass* pTechno) const noexcept
	{
		return Get(pTechno->GetHealthPercentage());
	}

	const ValueableVector<T>* GetEx(double ratio) const noexcept
	{
		return &this->Get(ratio);
	}

	const ValueableVector<T>& Get(double ratio) const noexcept
	{
		if (this->ConditionRed.HasValue() && ratio <= RulesClass::Instance->ConditionRed)
			return this->ConditionRed;
		else if (this->ConditionYellow.HasValue() && ratio <= RulesClass::Instance->ConditionYellow)
			return this->ConditionYellow;
		else if (this->MaxValue.HasValue() && fabs(ratio - 1.0) < 1e-6)
			return this->MaxValue;

		return this->BaseValue;
	}

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	inline bool Save(PhobosStreamWriter& Stm) const;
};

/*
PromotableVector

Read: use ValueableVector<T>::Read, like promotable
ReadList: like gattling, remove last char if it is '.'
*/
template <typename T>
class PromotableVector
{
public:
	static T Default;

	ValueableVector<T> Base {};
	std::unordered_map<int, T> Veteran {};
	std::unordered_map<int, T> Elite {};

	PromotableVector() noexcept  = default;

	explicit PromotableVector(ValueableVector<T> const& all)
		noexcept(noexcept(ValueableVector<T> { all }))
		: Base { all }
	{
	}

	~PromotableVector() = default;

	inline void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag, const char* pSingleFlag = nullptr);

	inline void ReadList(INI_EX& parser, const char* pSection, const char* pFlag, bool allocate = false);

	const T& Get(int index, double veterancy) const noexcept
	{
		if (2.0 <= veterancy)
		{
			if (this->Elite.count(index))
				return this->Elite.at(index);
		}

		if (1.0 <= veterancy)
		{
			if (this->Veteran.count(index))
				return this->Veteran.at(index);
		}

		if (index < static_cast<int>(Base.size()))
			return this->Base.at(index);

		return Default;
	}

	const T& Get(int index, TechnoClass* pTechno) const noexcept
	{
		return this->Get(index, pTechno->Veterancy.Veterancy);
	}

	inline bool Load(PhobosStreamReader& stm, bool registerForChange);

	inline bool Save(PhobosStreamWriter& stm) const;
};

template <typename T>
T PromotableVector<T>::Default = T();

// Template to use for timed Warhead-applied values.
template<typename T>
class TimedWarheadValue
{
public:
	T Value { };
	CDTimerClass Timer { };
	AffectedHouse ApplyToHouses  { AffectedHouse::None };
	WarheadTypeClass* SourceWarhead { };

	TimedWarheadValue() = default;

	TimedWarheadValue(T const& value, int duration, AffectedHouse applyToHouses, WarheadTypeClass* sourceWarhead) :
		Value { value }
		, Timer {}
		, ApplyToHouses { applyToHouses }
		, SourceWarhead { sourceWarhead }
	{
		Timer.Start(duration);
	}

	TimedWarheadValue(T const& value, int duration, AffectedHouse applyToHouses)
	{
		TimedWarheadValue(value, duration, applyToHouses, nullptr);
	}

	TimedWarheadValue(T const& value, int duration)
	{
		TimedWarheadValue(value, duration, AffectedHouse::All, nullptr);
	}

	~TimedWarheadValue() = default;

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	inline bool Save(PhobosStreamWriter& Stm) const;
};


template<typename T>
class NullablePromotable
{
public:
	Nullable<T> Rookie {};
	Nullable<T> Veteran {};
	Nullable<T> Elite {};

	NullablePromotable() = default;
	explicit NullablePromotable(T const& all) noexcept(noexcept(T { all })) : Rookie(all), Veteran(all), Elite(all) { }
	explicit NullablePromotable(T const& r, T const& v, T const& e)
		noexcept(noexcept(T { r }) && noexcept(T { v }) && noexcept(T { e })) :
		Rookie(r), Veteran(v), Elite(e)
	{
	}

	~NullablePromotable() = default;

	void SetAll(const T& val)
	{
		this->Elite = this->Veteran = this->Rookie = val;
	}

	inline void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag, const char* pSingleFlag = nullptr);

	const Nullable<T>* GetFromSpecificRank(Rank rank)const noexcept
	{
		if (rank == Rank::Elite)
		{
			return &this->Elite;
		}

		if (rank == Rank::Veteran)
		{
			return &this->Veteran;
		}

		return &this->Rookie;
	}

	const Nullable<T>* Get(TechnoClass* pTechno) const noexcept
	{
		auto const rank = pTechno->Veterancy.GetRemainingLevel();
		if (rank == Rank::Elite)
		{
			return &this->Elite;
		}
		if (rank == Rank::Veteran)
		{
			return &this->Veteran;
		}
		return &this->Rookie;
	}

	const Nullable<T>* GetFromCurrentRank(TechnoClass* pTechno) const noexcept
	{
		if (pTechno->CurrentRanking == Rank::Elite)
		{
			return &this->Elite;
		}
		if (pTechno->CurrentRanking == Rank::Veteran)
		{
			return &this->Veteran;
		}
		return &this->Rookie;
	}

	const T& GetOrDefault(TechnoClass* pTechno, const T& nDefault) const noexcept
	{
		const auto nRes = Get(pTechno);
		return nRes->isset() ? nRes->Get() : nDefault;
	}

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	inline bool Save(PhobosStreamWriter& Stm) const;
};