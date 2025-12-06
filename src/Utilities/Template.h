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
#include "Interpolation.h"

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

	COMPILETIMEEVAL Valueable() {
		if COMPILETIMEEVAL (std::is_pointer_v<T>) {
			Value = nullptr;
		}
	}

	COMPILETIMEEVAL explicit Valueable(T value) noexcept(noexcept(T { std::move(value) })) : Value(std::move(value)) { }
	COMPILETIMEEVAL Valueable(Valueable const& other) = default;
	COMPILETIMEEVAL Valueable(Valueable&& other) = default;

	COMPILETIMEEVAL ~Valueable() = default;

	COMPILETIMEEVAL Valueable& operator = (Valueable const& value) = default;
	COMPILETIMEEVAL Valueable& operator = (Valueable&& value) = default;

	template <typename Val, typename = std::enable_if_t<std::is_assignable<T&, Val&&>::value>>
	COMPILETIMEEVAL Valueable& operator = (Val value)
	{
		this->Value = std::move(value);
		return *this;
	}

	COMPILETIMEEVAL FORCEDINLINE operator const T& () const noexcept {
		return this->Get();
	}

	COMPILETIMEEVAL FORCEDINLINE auto operator->() noexcept {
		if COMPILETIMEEVAL (std::is_pointer<T>::type())
			return this->Value;
		else
			return &this->Value;
	}

	COMPILETIMEEVAL FORCEDINLINE auto operator->() const noexcept
	{
		if COMPILETIMEEVAL (std::is_pointer<T>::type())
			return this->Value;
		else
			return &this->Value;
	}


	//COMPILETIMEEVAL FORCEDINLINE bool operator!() const
	//{
	//	if COMPILETIMEEVAL (std::is_pointer<T>::type()
	//			|| std::is_integral<T>::type()
	//			|| std::is_same<T, bool>::value
	//	)
	//		return !this->Value;
	//	else {
	//		static_assert(true, "operator! not suitable!");
	//		return false;
	//	}
	//}

	COMPILETIMEEVAL FORCEDINLINE const T& Get() const noexcept
	{
		return this->Value;
	}

	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate = false);

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const;
};

template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
OPTIONALINLINE bool operator == (const Valueable<T>& val, const T& other)
{
	return val.Get() == other;
}

template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
OPTIONALINLINE bool operator == (const T& other, const Valueable<T>& val)
{
	return val.Get() == other;
}

template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
OPTIONALINLINE bool operator != (const Valueable<T>& val, const T& other)
{
	return !(val == other);
}

template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
OPTIONALINLINE bool operator != (const T& other, const Valueable<T>& val)
{
	return !(val == other);
}
// more fun
template<typename Lookuper>
class ValueableIdx : public Valueable<int>
{
public:
	COMPILETIMEEVAL ValueableIdx() noexcept : Valueable<int>(-1) { }
	COMPILETIMEEVAL explicit ValueableIdx(int value) noexcept : Valueable<int>(value) { }
	COMPILETIMEEVAL ValueableIdx(ValueableIdx const& other) = default;
	COMPILETIMEEVAL ValueableIdx(ValueableIdx&& other) = default;
	COMPILETIMEEVAL ~ValueableIdx() = default;

	COMPILETIMEEVAL ValueableIdx& operator = (ValueableIdx const& value) = default;
	COMPILETIMEEVAL ValueableIdx& operator = (ValueableIdx&& value) = default;

	template <typename Val, typename = std::enable_if_t<std::is_assignable<int&, Val&&>::value>>
	COMPILETIMEEVAL ValueableIdx& operator = (Val value)
	{
		this->Value = std::move(value);
		return *this;
	}

	COMPILETIMEEVAL FORCEDINLINE size_t ToUnsigned() const noexcept {
		return (size_t)this->Value;
	}

	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pKey);
};

template<typename T>
class Nullable : public Valueable<T>
{
protected:
	bool HasValue { false };
public:

	COMPILETIMEEVAL Nullable() = default;
	COMPILETIMEEVAL explicit Nullable(T value) noexcept(noexcept(Valueable<T>{std::move(value)})) : Valueable<T>(std::move(value)), HasValue(true) { }
	COMPILETIMEEVAL Nullable(Nullable const& other) = default;
	COMPILETIMEEVAL Nullable(Nullable&& other) = default;
	COMPILETIMEEVAL ~Nullable() = default;

	COMPILETIMEEVAL Nullable& operator = (Nullable const& value) = default;
	COMPILETIMEEVAL Nullable& operator = (Nullable&& value) = default;

	template <typename Val, typename = std::enable_if_t<std::is_assignable<T&, Val&&>::value>>
	COMPILETIMEEVAL Nullable& operator = (Val value)
	{
		this->Value = std::move(value);
		this->HasValue = true;
		return *this;
	}

	COMPILETIMEEVAL FORCEDINLINE bool isset() const noexcept
	{
		return this->HasValue;
	}

	COMPILETIMEEVAL FORCEDINLINE const T& Get() const noexcept
	{
		return this->Value;
	}

	// return a copy of the value instead
	// this can be used to fill an vector after reading
	COMPILETIMEEVAL FORCEDINLINE T GetCopy() const noexcept{
		return this->Value;
	}

	COMPILETIMEEVAL FORCEDINLINE T Get(const T& ndefault) const
	{
		return this->HasValue ? this->Get() : ndefault;
	}

	COMPILETIMEEVAL FORCEDINLINE const T& GetB(const T& ndefault) const
	{
		return this->HasValue ? this->Get() : ndefault;
	}

	COMPILETIMEEVAL FORCEDINLINE T Get(const Valueable<T>& ndefault) const
	{
		return this->HasValue ? this->Get() : ndefault.Get();
	}

	COMPILETIMEEVAL FORCEDINLINE const T& GetB(const Valueable<T>& ndefault) const
	{
		return this->HasValue ? this->Get() : ndefault.Get();
	}

	COMPILETIMEEVAL FORCEDINLINE T* GetEx(T* ndefault) noexcept
	{
		return this->isset() ? this->GetEx() : ndefault;
	}

	COMPILETIMEEVAL FORCEDINLINE const T* GetEx(const T* ndefault) const noexcept
	{
		return this->isset() ? this->GetEx() : ndefault;
	}

	COMPILETIMEEVAL FORCEDINLINE T* GetEx(Valueable<T>& ndefault) noexcept
	{
		const bool Isset = this->isset();
		if COMPILETIMEEVAL (std::is_pointer<T>::type())
			return Isset ? this->Value : ndefault.Value;
		else
			return Isset ? &this->Value : &ndefault.Value;
	}

	COMPILETIMEEVAL FORCEDINLINE const T* GetEx(const Valueable<T>& ndefault) const noexcept
	{
		const bool Isset = this->isset();
		if COMPILETIMEEVAL (std::is_pointer<T>::type())
			return Isset ? this->Value : ndefault.Value;
		else
			return Isset ? &this->Value : &ndefault.Value;
	}

	COMPILETIMEEVAL FORCEDINLINE void Reset()
	{
		this->Value = T();
		this->HasValue = false;
	}

	template <typename Val, typename = std::enable_if_t<std::is_assignable<T&, Val&&>::value>>
	COMPILETIMEEVAL void Reset(Val ndefault) const
	{
		this->Value = std::move(ndefault); // set the value to args
		this->HasValue = true;
	}

	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate = false);

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const;

private:

	COMPILETIMEEVAL FORCEDINLINE T* GetEx() noexcept {
		if COMPILETIMEEVAL (std::is_pointer<T>::type())
			return this->Value;
		else
			return &this->Value;
	}

	COMPILETIMEEVAL FORCEDINLINE const T* GetEx() const noexcept {
		if COMPILETIMEEVAL (std::is_pointer<T>::type())
			return this->Value;
		else
			return &this->Value;
	}
};

template<typename Lookuper, EnumCheckMode mode = EnumCheckMode::ignore>
class NullableIdx : public Nullable<int>
{
public:
	COMPILETIMEEVAL NullableIdx() noexcept : Nullable<int>(-1) { this->HasValue = false; }
	COMPILETIMEEVAL explicit NullableIdx(int value) noexcept : Nullable<int>(value) { }
	COMPILETIMEEVAL NullableIdx(NullableIdx const& other) = default;
	COMPILETIMEEVAL NullableIdx(NullableIdx&& other) = default;
	COMPILETIMEEVAL ~NullableIdx() = default;

	COMPILETIMEEVAL NullableIdx& operator = (NullableIdx const& value) = default;
	COMPILETIMEEVAL NullableIdx& operator = (NullableIdx&& value) = default;

	template <typename Val, typename = std::enable_if_t<std::is_assignable<int&, Val&&>::value>>
	COMPILETIMEEVAL NullableIdx& operator = (Val value)
	{
		this->Value = std::move(value);
		this->HasValue = true;
		return *this;
	}

	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pKey);
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

	COMPILETIMEEVAL Promotable() = default;
	COMPILETIMEEVAL explicit Promotable(T const& all) noexcept(noexcept(T { all })) : Rookie(all), Veteran(all), Elite(all) { }
	COMPILETIMEEVAL explicit Promotable(T const& r, T const& v, T const& e)
	noexcept(noexcept(T { r }) && noexcept(T { v }) && noexcept(T { e })) :
		Rookie(r), Veteran(v), Elite(e) { }

	COMPILETIMEEVAL Promotable(const Promotable&) = default;
	COMPILETIMEEVAL Promotable(Promotable&&) = default;
	COMPILETIMEEVAL Promotable& operator=(const Promotable& other) = default;

	COMPILETIMEEVAL ~Promotable() = default;

	COMPILETIMEEVAL FORCEDINLINE void SetAll(const T& val) {
		this->Elite = this->Veteran = this->Rookie = val;
	}

	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag, const char* pSingleFlag = nullptr, bool allocate = false);

	COMPILETIMEEVAL FORCEDINLINE const T* GetEx(TechnoClass* pTechno) const noexcept {
		const auto rank = pTechno->Veterancy.GetRemainingLevel();
		return &this->GetValue(rank);
	}

	COMPILETIMEEVAL FORCEDINLINE const T& GetFromSpecificRank(Rank rank)const noexcept
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

	COMPILETIMEEVAL FORCEDINLINE const T& Get(TechnoClass* pTechno) const noexcept {
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

	COMPILETIMEEVAL FORCEDINLINE const T& GetFromCurrentRank(TechnoClass* pTechno) const noexcept {
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

	// this mean 0 values will treat as false and alwas using default
	// and floating point value will suffer from accuracy problem with these
	// not sure how to fix it without breaking all shit
	//COMPILETIMEEVAL FORCEDINLINE const T& GetOrDefault(TechnoClass* pTechno, const T& nDefault) const noexcept {
	//	auto nRes = Get(pTechno);
	//	return nRes ? nRes : nDefault;
	//}

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const;

private:
	COMPILETIMEEVAL FORCEDINLINE T GetValue(Rank rank) {
		if (rank == Rank::Elite) {
			if COMPILETIMEEVAL (std::is_pointer<T>::type())
				return &this->Elite;
			else
				return this->Elite;
		}

		if (rank == Rank::Veteran) {
			if COMPILETIMEEVAL (std::is_pointer<T>::type())
				return &this->Veteran;
			else
				return this->Veteran;
		}

		if COMPILETIMEEVAL (std::is_pointer<T>::type())
			return &this->Rookie;
		else
			return this->Rookie;
	}
};

template<class T>
class ValueableVector : public std::vector<T>
{
public:
	using value_type = T;
	using base_type = std::remove_pointer_t<T>;

	//ValueableVector() noexcept = default;
	//ValueableVector(size_t Reserve) = delete;

	//~ValueableVector() = default;

	std::vector<T>& AsVector() {
		return *reinterpret_cast<std::vector<T>*>(this);
	}

	const std::vector<T>& AsVector() const{
		return *reinterpret_cast<const std::vector<T>*>(this);
	}

	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pKey, bool bAllocate = false);

	COMPILETIMEEVAL FORCEDINLINE bool Eligible(const T& other) const
	{
		if (this->empty())
			return true;

		return this->Contains(other);
	}

	COMPILETIMEEVAL FORCEDINLINE auto Find(const T& item) const
	{
		if COMPILETIMEEVAL (direct_comparable<T>) {
			auto i = this->begin();

			for (; i != this->end(); ++i) {
				if (*i == item) {
					break;
				}
			}

			return i;
		} else {
			return std::find(this->begin(), this->end(), item);
		}
	}

	COMPILETIMEEVAL FORCEDINLINE bool Contains(const T& other) const {
		return this->Find(other) != this->end();
	}

	COMPILETIMEEVAL FORCEDINLINE int IndexOf(const T& other) const
	{
		const auto it = this->Find(other);
		return it != this->end() ? std::distance(this->begin(), it) : -1;
	}

	COMPILETIMEEVAL FORCEDINLINE bool ValidIndex(int index) const {
		return static_cast<size_t>(index) < this->size();
	}

	COMPILETIMEEVAL FORCEDINLINE T GetItemAt(int nIdx) const {

		if (!this->ValidIndex(nIdx))
			return T();

		return *(this->begin() + nIdx);
	}

	COMPILETIMEEVAL size_t end_idx() const{
		return this->size() - 1;
	}

	COMPILETIMEEVAL FORCEDINLINE T GetItemAtOrMax(int nIdx) const
	{
		if (!this->ValidIndex(nIdx))
			nIdx = end_idx();

		return *(this->begin() + nIdx);
	}

	COMPILETIMEEVAL FORCEDINLINE T GetItemAtOrDefault(int nIdx , const T& other) const
	{
		if (!this->ValidIndex(nIdx))
			return other;

		return *(this->begin() + nIdx);
	}

	COMPILETIMEEVAL FORCEDINLINE void PushbackUnique(const T& other)
	{
		if (this->Contains(other)) return;
		else { this->push_back(other); }
	}

	COMPILETIMEEVAL FORCEDINLINE void EmplacebackUnique(const T& other)
	{
		if (this->Contains(other)) return;
		else { this->emplace_back(other); }
	}

	template <typename Func>
	COMPILETIMEEVAL FORCEDINLINE void For_Each(Func&& act) const
	{
		for (auto i = this->begin(); i != this->end(); ++i) {
        	act(*i);
    	}
	}

	template <typename Func>
	COMPILETIMEEVAL FORCEDINLINE void For_Each(Func&& act)
	{
		for (auto i = this->begin(); i != this->end(); ++i) {
        	act(*i);
    	}
	}

	template<typename func>
	COMPILETIMEEVAL FORCEDINLINE bool None_Of(func&& fn) const
	{
		for (auto i = this->begin(); i != this->end(); ++i) {
       	 	if (fn(*i)) {
           	 	return false;
        	}
    	}

    	return true;
	}

	template<typename func>
	COMPILETIMEEVAL FORCEDINLINE bool None_Of(func&& fn)
	{
		for (auto i = this->begin(); i != this->end(); ++i) {
       	 	if (fn(*i)) {
           	 	return false;
        	}
    	}

    	return true;
	}

	template<typename func>
	COMPILETIMEEVAL FORCEDINLINE bool Any_Of(func&& fn) const
	{
		for (auto i = this->begin(); i != this->end(); ++i) {
       		if (fn(*i)) {
            	return true;
			}
        }

		return false;
	}

	template<typename func>
	COMPILETIMEEVAL FORCEDINLINE bool Any_Of(func&& fn)
	{
		for (auto i = this->begin(); i != this->end(); ++i) {
       		if (fn(*i)) {
            	return true;
			}
        }

		return false;
	}

	template<typename func>
	COMPILETIMEEVAL FORCEDINLINE bool All_Of(func&& fn) const
	{
		for (auto i = this->begin(); i != this->end(); ++i) {
			if (!fn(*i)) {
				return false;
			}
		}

		return true;
	}

	template<typename func>
	COMPILETIMEEVAL FORCEDINLINE bool All_Of(func&& fn)
	{
		for (auto i = this->begin(); i != this->end(); ++i) {
			if (!fn(*i)) {
				return false;
			}
		}

		return true;
	}

	Iterator<T> GetElements() const noexcept
	{
		return Iterator<T>(*this);
	}

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const;
};

template<class T>
class NullableVector : public ValueableVector<T>
{
protected:
	bool hasValue { false };
public:
	//NullableVector() noexcept = default;
	//~NullableVector() = default;
	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pKey , bool allocate = false);

	COMPILETIMEEVAL FORCEDINLINE bool HasValue() const noexcept
	{
		return this->hasValue;
	}

	FORCEDINLINE void SetHasValue(bool bCond)
	{
		if (!bCond)
			Reset();
		else
			this->hasValue = true;
	}

	FORCEDINLINE void Reset()
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

	bool Eligible(const ValueableVector<T>& ndefault , const T& other) const
	{
		if (!this->hasValue)
			return ndefault.Eligible(other);

		return this->Contains(other);
	}

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const;
};

template<typename Lookuper>
class ValueableIdxVector : public ValueableVector<int>
{
public:
	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pKey);
};

template<typename Lookuper>
class NullableIdxVector : public NullableVector<int>
{
public:
	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pKey);
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

	COMPILETIMEEVAL Damageable() noexcept = default;

	COMPILETIMEEVAL explicit Damageable(T const& all)
		noexcept(noexcept(T { all }))
		: BaseValue { all }
	{
	}

	COMPILETIMEEVAL explicit Damageable(T const& undamaged, T const& damaged)
		noexcept(noexcept(T { undamaged }) && noexcept(T { damaged }))
		: BaseValue { undamaged }, ConditionYellow { damaged }
	{
	}

	COMPILETIMEEVAL explicit Damageable(T const& green, T const& yellow, T const& red)
		noexcept(noexcept(T { green }) && noexcept(T { yellow }) && noexcept(T { red }))
		: BaseValue { green }, ConditionYellow { yellow }, ConditionRed { red }
	{
	}

	COMPILETIMEEVAL ~Damageable() = default;

	COMPILETIMEEVAL Damageable(const Damageable&) = default;
	COMPILETIMEEVAL Damageable(Damageable&&) = default;
	COMPILETIMEEVAL Damageable& operator=(const Damageable& other) = default;

	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag, const char* pSingleFlag = nullptr , bool Alloc = false);

	COMPILETIMEEVAL const T& Get(TechnoClass* pTechno) const noexcept
	{
		return Get(pTechno->GetHealthPercentage());
	}

	COMPILETIMEEVAL const T& Get(double ratio, double conditionYellow , double conditionRed) const noexcept
	{
		if (this->ConditionRed.isset() && ratio <= conditionRed)
			return this->ConditionRed;
		else if (this->ConditionYellow.isset() && ratio <= conditionYellow)
			return this->ConditionYellow;

		return this->BaseValue;
	}

	COMPILETIMEEVAL const T& Get(HealthState const& nState) const noexcept
	{
		if (this->ConditionRed.isset() && (nState == HealthState::Red))
			return this->ConditionRed;
		else if (this->ConditionYellow.isset() && (nState == HealthState::Yellow))
			return this->ConditionYellow;

		return this->BaseValue;
	}

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const;
};

struct HealthOnFireData
{
protected:
	bool RedOnFire;
	bool GreenOnFire;
	bool YellowOnFire;

public:
	COMPILETIMEEVAL HealthOnFireData() noexcept :
		RedOnFire { true }, GreenOnFire { false }, YellowOnFire { true }
	{
	}

	COMPILETIMEEVAL HealthOnFireData(bool All) noexcept :
		RedOnFire { All }, GreenOnFire { All }, YellowOnFire { All }
	{
	}

	COMPILETIMEEVAL HealthOnFireData(bool R, bool G, bool Y) noexcept :
		RedOnFire { R }, GreenOnFire { G }, YellowOnFire { Y }
	{
	}

	COMPILETIMEEVAL ~HealthOnFireData() = default;

	COMPILETIMEEVAL HealthOnFireData(const HealthOnFireData&) = default;
	COMPILETIMEEVAL HealthOnFireData(HealthOnFireData&&) = default;
	COMPILETIMEEVAL HealthOnFireData& operator=(const HealthOnFireData& other) = default;

	COMPILETIMEEVAL OPTIONALINLINE bool Get(HealthState const& nState) const noexcept
	{
		return (nState == HealthState::Green && GreenOnFire)
			|| (nState == HealthState::Yellow && YellowOnFire)
			|| (nState == HealthState::Red && RedOnFire);
	}

	OPTIONALINLINE bool Read(INI_EX& parser, const char* pSection, const char* pKey);
	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const;
};

template<typename T>
class DamageableVector
{
	DamageableVector(const DamageableVector&) = delete;
	DamageableVector(DamageableVector&&) = delete;
	DamageableVector& operator=(const DamageableVector& other) = delete;
public:
	ValueableVector<T> BaseValue {};
	NullableVector<T> ConditionYellow {};
	NullableVector<T> ConditionRed {};
	NullableVector<T> MaxValue {};

	COMPILETIMEEVAL DamageableVector() noexcept  = default;

	COMPILETIMEEVAL explicit DamageableVector(ValueableVector<T> const& all)
		noexcept(noexcept(ValueableVector<T> { all }))
		: BaseValue { all }
	{
	}

	COMPILETIMEEVAL explicit DamageableVector(ValueableVector<T> const& undamaged, NullableVector<T> const& damaged)
		noexcept(noexcept(ValueableVector<T> { undamaged }) && noexcept(NullableVector<T> { damaged }))
		: BaseValue { undamaged }, ConditionYellow { damaged }
	{
	}

	COMPILETIMEEVAL explicit DamageableVector(ValueableVector<T> const& green, NullableVector<T> const& yellow, NullableVector<T> const& red)
		noexcept(noexcept(ValueableVector<T> { green }) && noexcept(NullableVector<T> { yellow }) && noexcept(NullableVector<T> { red }))
		: BaseValue { green }, ConditionYellow { yellow }, ConditionRed { red }
	{
	}

	COMPILETIMEEVAL explicit DamageableVector(ValueableVector<T> const& green, NullableVector<T> const& yellow, NullableVector<T> const& red, NullableVector<T> const& max)
		noexcept(noexcept(ValueableVector<T> { green }) && noexcept(NullableVector<T> { yellow }) && noexcept(NullableVector<T> { red }) && noexcept(NullableVector<T> { max }))
		: BaseValue { green }, ConditionYellow { yellow }, ConditionRed { red }, MaxValue { max }
	{
	}

	COMPILETIMEEVAL ~DamageableVector() = default;

	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag, const char* pSingleFlag = nullptr);

	COMPILETIMEEVAL const ValueableVector<T>* GetEx(TechnoClass* pTechno) const noexcept
	{
		return &this->Get(pTechno);
	}

	COMPILETIMEEVAL const ValueableVector<T>& Get(TechnoClass* pTechno) const noexcept
	{
		return Get(pTechno->GetHealthPercentage());
	}

	COMPILETIMEEVAL const ValueableVector<T>* GetEx(double ratio) const noexcept
	{
		return &this->Get(ratio);
	}

	COMPILETIMEEVAL const ValueableVector<T>& Get(double ratio) const noexcept
	{
		if (this->ConditionRed.HasValue() && ratio <= RulesClass::Instance->ConditionRed)
			return this->ConditionRed;
		else if (this->ConditionYellow.HasValue() && ratio <= RulesClass::Instance->ConditionYellow)
			return this->ConditionYellow;
		else if (this->MaxValue.HasValue() && Math::abs(ratio - 1.0) < 1e-6)
			return this->MaxValue;

		return this->BaseValue;
	}

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const;
};

/*
PromotableVector

Read: use ValueableVector<T>::Read, like promotable
ReadList: like gattling, remove last char if it is '.'

template <typename T>
class PromotableVector
{
	PromotableVector(const PromotableVector&) = delete;
	PromotableVector(PromotableVector&&) = delete;
	PromotableVector& operator=(const PromotableVector& other) = delete;
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

	//TODO : untested
	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag, const char* pSingleFlag = nullptr);

	//TODO : untested
	OPTIONALINLINE void ReadList(INI_EX& parser, const char* pSection, const char* pFlag, bool allocate = false);

	const T& Get(int index, double veterancy) const noexcept
	{
		if (2.0 <= veterancy)
		{
			if (this->Elite.contains(index))
				return this->Elite[index];
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

	OPTIONALINLINE bool Load(PhobosStreamReader& stm, bool registerForChange);

	OPTIONALINLINE bool Save(PhobosStreamWriter& stm) const;
};

template <typename T>
T PromotableVector<T>::Default = T();
*/

// Template to use for timed Warhead-applied values.
template<typename T>
class TimedWarheadValue
{
public:
	T Value { };
	CDTimerClass Timer { };
	AffectedHouse ApplyToHouses  { AffectedHouse::None };
	WarheadTypeClass* SourceWarhead { };

	COMPILETIMEEVAL TimedWarheadValue(const TimedWarheadValue&) = default;
	COMPILETIMEEVAL TimedWarheadValue(TimedWarheadValue&&) = default;
	COMPILETIMEEVAL TimedWarheadValue& operator=(const TimedWarheadValue& other) = default;

	COMPILETIMEEVAL TimedWarheadValue() = default;

	COMPILETIMEEVAL TimedWarheadValue(T const& value, int duration, AffectedHouse applyToHouses, WarheadTypeClass* sourceWarhead) :
		Value { value }
		, Timer {}
		, ApplyToHouses { applyToHouses }
		, SourceWarhead { sourceWarhead }
	{
		Timer.Start(duration);
	}

	COMPILETIMEEVAL TimedWarheadValue(T const& value, int duration, AffectedHouse applyToHouses)
	{
		TimedWarheadValue(value, duration, applyToHouses, nullptr);
	}

	COMPILETIMEEVAL TimedWarheadValue(T const& value, int duration)
	{
		TimedWarheadValue(value, duration, AffectedHouse::All, nullptr);
	}

	COMPILETIMEEVAL ~TimedWarheadValue() = default;

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const;
};


template<typename T>
class NullablePromotable
{
public:
	Nullable<T> Rookie {};
	Nullable<T> Veteran {};
	Nullable<T> Elite {};

	COMPILETIMEEVAL NullablePromotable() = default;
	COMPILETIMEEVAL explicit NullablePromotable(T const& all) noexcept(noexcept(T { all })) : Rookie(all), Veteran(all), Elite(all) { }
	COMPILETIMEEVAL explicit NullablePromotable(T const& r, T const& v, T const& e)
		noexcept(noexcept(T { r }) && noexcept(T { v }) && noexcept(T { e })) :
		Rookie(r), Veteran(v), Elite(e)
	{
	}

	COMPILETIMEEVAL NullablePromotable(const NullablePromotable&) = default;
	COMPILETIMEEVAL NullablePromotable(NullablePromotable&&) = default;
	COMPILETIMEEVAL NullablePromotable& operator=(const NullablePromotable& other) = default;
	COMPILETIMEEVAL ~NullablePromotable() = default;

	COMPILETIMEEVAL void SetAll(const T& val)
	{
		this->Elite = this->Veteran = this->Rookie = val;
	}

	OPTIONALINLINE void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag, const char* pSingleFlag = nullptr);

	COMPILETIMEEVAL const Nullable<T>* GetFromSpecificRank(Rank rank)const noexcept
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

	COMPILETIMEEVAL const Nullable<T>* Get(TechnoClass* pTechno) const noexcept
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

	COMPILETIMEEVAL const Nullable<T>* GetFromCurrentRank(TechnoClass* pTechno) const noexcept
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

	COMPILETIMEEVAL const T& GetOrDefault(TechnoClass* pTechno, const T& nDefault) const noexcept
	{
		return Get(pTechno)->GetB(nDefault);
	}

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const;
};

// Designates that the type can read it's value from multiple flags.
template<typename T, typename... TExtraArgs>
concept MultiflagReadable = requires(T obj, INI_EX & parser, const char* const pSection, const char* const pBaseFlag, TExtraArgs&... extraArgs)
{
	{ obj.Read(parser, pSection, pBaseFlag, extraArgs...) } -> std::same_as<bool>;
};

template<typename T, typename... TExtraArgs>
	requires MultiflagReadable<T, TExtraArgs...>
class MultiflagValueableVector : public ValueableVector<T>
{
public:
	OPTIONALINLINE void Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, TExtraArgs&... extraArgs);
};
template<typename T, typename... TExtraArgs>
	requires MultiflagReadable<T, TExtraArgs...>
class MultiflagNullableVector : public NullableVector<T>
{
public:
	OPTIONALINLINE void Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, TExtraArgs&... extraArgs);
};

template<typename TValue>
class Animatable
{
public:
	using absolute_length_t = int;

	class KeyframeDataEntry
	{
	public:
		double Percentage;
		Valueable<TValue> Value;

		OPTIONALINLINE bool Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, absolute_length_t absoluteLength = absolute_length_t(0));

		OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

		OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const;
	};

	InterpolationMode InterpolationMode;
	MultiflagValueableVector<KeyframeDataEntry, absolute_length_t> KeyframeData;

	// TODO ctors and stuff

	COMPILETIMEEVAL OPTIONALINLINE TValue Get(double const percentage) const noexcept;

	OPTIONALINLINE void Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, absolute_length_t absoluteLength = absolute_length_t(0));

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const;
};
