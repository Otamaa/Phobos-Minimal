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
	static const CellStruct DefaultUnloadCell;

	__forceinline bool SimilarTo(const CellStruct& a) const {return (X == a.X && Y == a.Y); }
	__forceinline bool DifferTo(const CellStruct& a)const { return (X != a.X || Y != a.Y); }
	__forceinline bool IsValid() const { return this->DifferTo(CellStruct::Empty); }

	//equality
	__forceinline bool operator==(const CellStruct& a) const {
		return (X == a.X && Y == a.Y);
	}

	CellStruct operator+(short nThat) const
	{ return { short(X + nThat), short(Y + nThat) }; }

	CellStruct operator+(short nThat)
	{
		X += nThat;
		Y += nThat;
		return *this;
	}

	inline DWORD Pack() const noexcept {
		return std::bit_cast<DWORD>(*this);
	}

	static CellStruct UnPack(const int nVal) {
		CellStruct nBuffer;
		std::memcpy(&nBuffer, &nVal, sizeof(int));
		return nBuffer;
	}

	//substraction
	CellStruct operator-(const CellStruct& a) const
	{
		return { static_cast<short>(X - a.X),  static_cast<short>(Y - a.Y) };
	}
	//substraction
	CellStruct& operator-=(const CellStruct& a)
	{
		X -= a.X;
		Y -= a.Y;
		return *this;
	}

	//addition
	CellStruct operator+(const CellStruct& a) const
	{
		return { static_cast<short>(X + a.X), static_cast<short>(Y + a.Y) };
	}

	//addition
	CellStruct& operator+=(const CellStruct& a)
	{
		X += a.X;
		Y += a.Y;
		return *this;
	}
//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	inline double pow() const {
		return (double)std::pow(X,2) + (double)std::pow(Y,2);
	}

	inline double Length() const {
		return std::sqrt(this->pow());
	}

	inline double DistanceFrom(const CellStruct& that) const{
		return (that - *this).Length();
	}

	inline double DistanceFromSquared(const CellStruct& that) const {
		return (that - *this).pow();
	}

};