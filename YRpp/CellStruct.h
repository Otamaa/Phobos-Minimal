#pragma once

#include <YRMath.h>
#include <utility>
#include <bit>

class CellStruct
{
public:
	short X { 0 };
	short Y { 0 };

	static const CellStruct Empty;
	static const CellStruct EOL;
	static const CellStruct DefaultUnloadCell;

	constexpr FORCEINLINE bool SimilarTo(const CellStruct& a) const {return (X == a.X && Y == a.Y); }
	constexpr FORCEINLINE bool DifferTo(const CellStruct& a)const { return (X != a.X || Y != a.Y); }
	constexpr FORCEINLINE bool IsValid() const { return this->DifferTo(CellStruct::Empty); }

	//equality
	constexpr FORCEINLINE bool operator==(const CellStruct& a) const {
		return (X == a.X && Y == a.Y);
	}

	constexpr CellStruct& operator++() {
		++X;
		++Y;
		return *this;
	}

	constexpr CellStruct& operator--() {
		--X;
		--Y;
		return *this;
	}

	constexpr CellStruct operator+(short nThat) const
	{ return { short(X + nThat), short(Y + nThat) }; }

	constexpr CellStruct operator+=(short nThat)
	{
		X += nThat;
		Y += nThat;
		return *this;
	}

	constexpr FORCEINLINE DWORD Pack() const noexcept {
		return std::bit_cast<DWORD>(*this);
	}

	static CellStruct UnPack(const int nVal) {
		CellStruct nBuffer;
		std::memcpy(&nBuffer, &nVal, sizeof(int));
		return nBuffer;
	}

	//substraction
	constexpr CellStruct operator-(const CellStruct& a) const
	{
		return { static_cast<short>(X - a.X),  static_cast<short>(Y - a.Y) };
	}
	//substraction
	constexpr CellStruct& operator-=(const CellStruct& a)
	{
		X -= a.X;
		Y -= a.Y;
		return *this;
	}

	//addition
	constexpr CellStruct operator+(const CellStruct& a) const
	{
		return { static_cast<short>(X + a.X), static_cast<short>(Y + a.Y) };
	}

	//addition
	constexpr CellStruct& operator+=(const CellStruct& a)
	{
		X += a.X;
		Y += a.Y;
		return *this;
	}

	constexpr CellStruct operator--(int) { return --(*this); }

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	constexpr FORCEINLINE double pow() const {
		return (double)(X * X) + (double)(Y * Y);
	}

	inline double Length() const {
		return std::sqrt(this->pow());
	}

	inline double DistanceFrom(const CellStruct& that) const{
		return (that - *this).Length();
	}

	constexpr FORCEINLINE double DistanceFromSquared(const CellStruct& that) const {
		return (that - *this).pow();
	}

};