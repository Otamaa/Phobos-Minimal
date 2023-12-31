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

	bool operator==(const Point2DBYTE& that) const { return X == that.X && Y == that.Y; }
	bool operator!=(const Point2DBYTE& that) const { return X != that.X && Y != that.Y; }

	bool operator>(const Point2DBYTE& that) const { return X > that.X || X == that.X && Y > that.Y; }
	bool operator>=(const Point2DBYTE& that) const { return X >= that.X || X == that.X && Y >= that.Y; }

	bool operator<(const Point2DBYTE& that) const { return X < that.X || X == that.X && Y < that.Y; }
	bool operator<=(const Point2DBYTE& that) const { return X <= that.X || X == that.X && Y <= that.Y; }

	Point2DBYTE operator-(const Point2DBYTE& that) const { return {BYTE(X - that.X), BYTE(Y - that.Y)};}
	Point2DBYTE& operator-=(const Point2DBYTE& that) { X -= that.X; Y -= that.Y; return *this; }

	inline bool IsValid() const { return *this != (Point2DBYTE::Empty); }

	explicit operator DWORD() const {
		DWORD result = 0;
		std::memcpy(&result, this, sizeof(Point2DBYTE));
		return result;
	}

	inline DWORD Pack() const noexcept {
		return (DWORD)(*this);
	}

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	inline double pow() const {
		return double(X * X) + double(Y * Y);
	}

	inline double Length() const {
		return std::sqrt(this->pow());
	}

	inline double DistanceFrom(const Point2DBYTE& that) const{
		return (that - *this).Length();
	}

	inline double DistanceFromSquared(const Point2DBYTE& that) const {
		return (that - *this).pow();
	}

public:

	BYTE X, Y;
};