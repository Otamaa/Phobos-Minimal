#pragma once

#include <utility>

class Point2DBYTE
{
public:
	static const Point2DBYTE Empty;

	//Point2DBYTE& operator=(const Point2DBYTE& that)
	//{
	//	if (this != &that)
	//	{
	//		X = that.X;
	//		Y = that.Y;
	//	}
	//	return *this;
	//}

	COMPILETIMEEVAL bool operator==(const Point2DBYTE& that) const { return X == that.X && Y == that.Y; }
	COMPILETIMEEVAL bool operator!=(const Point2DBYTE& that) const { return X != that.X && Y != that.Y; }

	COMPILETIMEEVAL bool operator>(const Point2DBYTE& that) const { return X > that.X || X == that.X && Y > that.Y; }
	COMPILETIMEEVAL bool operator>=(const Point2DBYTE& that) const { return X >= that.X || X == that.X && Y >= that.Y; }

	COMPILETIMEEVAL bool operator<(const Point2DBYTE& that) const { return X < that.X || X == that.X && Y < that.Y; }
	COMPILETIMEEVAL bool operator<=(const Point2DBYTE& that) const { return X <= that.X || X == that.X && Y <= that.Y; }

	COMPILETIMEEVAL Point2DBYTE operator-(const Point2DBYTE& that) const { return {BYTE(X - that.X), BYTE(Y - that.Y)};}
	COMPILETIMEEVAL Point2DBYTE& operator-=(const Point2DBYTE& that) { X -= that.X; Y -= that.Y; return *this; }

	COMPILETIMEEVAL bool IsValid() const { return *this != (Point2DBYTE::Empty); }

	explicit operator DWORD() const {
		DWORD result = 0;
		std::memcpy(&result, this, sizeof(Point2DBYTE));
		return result;
	}

	DWORD Pack() const noexcept {
		return (DWORD)(*this);
	}

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	COMPILETIMEEVAL  double pow() const {
		return double(X * X) + double(Y * Y);
	}

	OPTIONALINLINE double Length() const {
		return std::sqrt(this->pow());
	}

	OPTIONALINLINE double DistanceFrom(const Point2DBYTE& that) const{
		return (that - *this).Length();
	}

	COMPILETIMEEVAL double DistanceFromSquared(const Point2DBYTE& that) const {
		return (that - *this).pow();
	}

public:

	BYTE X, Y;
};