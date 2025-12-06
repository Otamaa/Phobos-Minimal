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

	COMPILETIMEEVAL FORCEDINLINE bool SimilarTo(const CellStruct& a) const {return (X == a.X && Y == a.Y); }
	COMPILETIMEEVAL FORCEDINLINE bool DifferTo(const CellStruct& a)const { return (X != a.X || Y != a.Y); }
	COMPILETIMEEVAL FORCEDINLINE bool IsValid() const { return this->DifferTo(CellStruct::Empty); }

	//equality
	COMPILETIMEEVAL FORCEDINLINE bool operator==(const CellStruct& a) const {
		return (X == a.X && Y == a.Y);
	}

	COMPILETIMEEVAL CellStruct& operator++() {
		++X;
		++Y;
		return *this;
	}

	COMPILETIMEEVAL CellStruct& operator--() {
		--X;
		--Y;
		return *this;
	}

	COMPILETIMEEVAL CellStruct operator+(short nThat) const
	{ return { short(X + nThat), short(Y + nThat) }; }

	COMPILETIMEEVAL CellStruct operator+=(short nThat)
	{
		X += nThat;
		Y += nThat;
		return *this;
	}

	COMPILETIMEEVAL FORCEDINLINE DWORD Pack() const noexcept {
		return std::bit_cast<DWORD>(*this);
	}

	static CellStruct UnPack(const int nVal) {
		CellStruct nBuffer;
		std::memcpy(&nBuffer, &nVal, sizeof(int));
		return nBuffer;
	}

	//substraction
	COMPILETIMEEVAL CellStruct operator-(const CellStruct& a) const
	{
		return { static_cast<short>(X - a.X),  static_cast<short>(Y - a.Y) };
	}
	//substraction
	COMPILETIMEEVAL CellStruct& operator-=(const CellStruct& a)
	{
		X -= a.X;
		Y -= a.Y;
		return *this;
	}

	//addition
	COMPILETIMEEVAL CellStruct operator+(const CellStruct& a) const
	{
		return { static_cast<short>(X + a.X), static_cast<short>(Y + a.Y) };
	}

	//addition
	COMPILETIMEEVAL CellStruct& operator+=(const CellStruct& a)
	{
		X += a.X;
		Y += a.Y;
		return *this;
	}

	COMPILETIMEEVAL CellStruct operator--(int) { return --(*this); }

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	COMPILETIMEEVAL FORCEDINLINE double pow() const {
		return (double)(X * X) + (double)(Y * Y);
	}

	OPTIONALINLINE double Length() const {
		return Math::sqrt(this->pow());
	}

	OPTIONALINLINE double DistanceFrom(const CellStruct& that) const{
		return (that - *this).Length();
	}

	COMPILETIMEEVAL FORCEDINLINE double DistanceFromSquared(const CellStruct& that) const {
		return (that - *this).pow();
	}

};