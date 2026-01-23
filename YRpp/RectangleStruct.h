#pragma once

#include <GeneralStructures.h>
#include <tuple>
/*
		Otamaa : 04/08/2021

		Moved to separate header due to compile error
		Add More element known from Vinivera-TSpp
*/
//obvious
struct RectangleStruct
{
public:
	static const RectangleStruct Empty;

	/*
	RectangleStruct()
		: X(0), Y(0), Width(0), Height(0)
	{ }

	RectangleStruct(int const x, int const y, int const width, int const height)
		: X(x), Y(y), Width(width), Height(height)
	{ }

	RectangleStruct(const Point2D& nXY, const Point2D& nWH)
		:X(nXY.X), Y(nXY.Y), Width(nWH.X), Height(nWH.Y)
	{ }

	RectangleStruct(const Point2D& nXY, int const width, int const height)
		:X(nXY.X), Y(nXY.Y), Width(width), Height(height)
	{ }

	RectangleStruct(const RectangleStruct& that)
		:X(that.X), Y(that.Y), Width(that.Width), Height(that.Height)
	{ }

	//auto operator()() {
		// returns a tuple to make it work with std::tie
	//	return std::make_tuple(X, Y, Width, Height);
	//}

	RectangleStruct &operator=(const RectangleStruct &that)
	{
		if (this != &that) {
			X = that.X;
			Y = that.Y;
			Width = that.Width;
			Height = that.Height;
		}
		return *this;
	}*/

	COMPILETIMEEVAL bool IsEmpty() const {
		return(*this) == RectangleStruct::Empty;
	}

	COMPILETIMEEVAL bool operator==(const RectangleStruct &that) const
	{
		return (that.X == X) && (that.Width == Width)
			&& (that.Y == Y) && (that.Height == Height);
	}

	COMPILETIMEEVAL bool operator!=(const RectangleStruct &that) const
	{
		return (that.X != X) || (that.Width != Width)
			|| (that.Y != Y) || (that.Height != Height);
	}

	COMPILETIMEEVAL RectangleStruct &operator|=(const RectangleStruct &that) { *this = Union(*this, that); return *this; }
	COMPILETIMEEVAL RectangleStruct &operator+=(const Point2D &point) { X += point.X; Y += point.Y; return *this; }
	COMPILETIMEEVAL RectangleStruct &operator-=(const RectangleStruct &rect)
	{
		X -= rect.X;
		Y -= rect.Y;
		Width -= rect.Width;
		Height -= rect.Height;
		return *this;
	}

	COMPILETIMEEVAL RectangleStruct &operator-=(const Point2D &point) { X -= point.X; Y -= point.Y; return *this; }
	COMPILETIMEEVAL RectangleStruct &operator+=(const RectangleStruct &rect)
	{
		X += rect.X;
		Y += rect.Y;
		Width += rect.Width;
		Height += rect.Height;
		return *this;
	}

	COMPILETIMEEVAL RectangleStruct &operator&=(const RectangleStruct &that)
	{
		*this = Intersect(*this, that, nullptr, nullptr);
		return *this;
	}

	COMPILETIMEEVAL RectangleStruct operator+(const Point2D &point)
	{
		RectangleStruct tmp = *this;
		tmp.X = X + point.X;
		tmp.Y = Y + point.Y;
		return tmp;
	}

	COMPILETIMEEVAL RectangleStruct operator-(const Point2D &point)
	{
		RectangleStruct tmp = *this;
		tmp.X = X - point.X;
		tmp.Y = Y - point.Y;
		return tmp;
	}

	COMPILETIMEEVAL RectangleStruct operator+(const RectangleStruct &that)
	{
		RectangleStruct tmp = *this;
		tmp += that;
		return tmp;
	}

	COMPILETIMEEVAL RectangleStruct operator-(const RectangleStruct &that)
	{
		RectangleStruct tmp = *this;
		tmp -= that;
		return tmp;
	}

	COMPILETIMEEVAL RectangleStruct Bias_To(const RectangleStruct& rect)
	{
		Point2D xy = Top_Left() + rect.Top_Left();
		return { xy.X ,xy.Y , Width, Height };
	}

	COMPILETIMEEVAL FORCEDINLINE  void Reset_Position() { X = 0; Y = 0; }

	COMPILETIMEEVAL FORCEDINLINE void Move(int x, int y) { X += x; Y += y; }
	COMPILETIMEEVAL FORCEDINLINE void Move(const Point2D &point) { X += point.X; Y += point.Y; }

	COMPILETIMEEVAL FORCEDINLINE  int Size() const { return Width * Height; }
	COMPILETIMEEVAL FORCEDINLINE  bool IsValid() const { return Width > 0 && Height > 0; }
	COMPILETIMEEVAL FORCEDINLINE  bool IsWithin(const RectangleStruct &rect) const { return (rect.X >= X && rect.X < (X + Width)) && (rect.Width >= Y && rect.Width < (Y + Height)); }
	COMPILETIMEEVAL FORCEDINLINE  bool IsWithin(int x, int y) const { return (x >= X && x < (X + Width)) && (y >= Y && y < (Y + Height)); }
	COMPILETIMEEVAL FORCEDINLINE  bool IsWithin(const Point2D &point) const { return (point.X >= X && point.X < (X + Width)) && (point.Y >= Y && point.Y < (Y + Height)); }

	const RectangleStruct IntersectWith(const RectangleStruct &rectangle, int *x = nullptr, int *y = nullptr) { return Intersect(*this, rectangle, x, y); }

	COMPILETIMEEVAL FORCEDINLINE bool CanIntersectsWith(const RectangleStruct &with) const { return (X > with.Width) || (Width < with.X) || (Y > with.Height) || (Height < with.Y); }

	COMPILETIMEEVAL FORCEDINLINE  RectangleStruct InvalidRect() { return {0, 0, 0, 0}; }

	COMPILETIMEEVAL FORCEDINLINE bool IsOverlapping(const RectangleStruct &rect)
	{
		return X < rect.X + rect.Width
			&& Y < rect.Y + rect.Height
			&& X + Width > rect.X
			&& Y + Height > rect.Y;
	}

	COMPILETIMEEVAL FORCEDINLINE void Inflate(int w, int h, bool adjust_xy = false)
	{
		if (adjust_xy)
		{
			X -= w;
			Y -= h;
		}
		Width += w;
		Height += h;
	}

	COMPILETIMEEVAL FORCEDINLINE  Point2D Center_Point() const { return { X + (Width / 2), Y + (Height / 2) }; }
	COMPILETIMEEVAL FORCEDINLINE  Point2D Top_Left() const { return { X, Y }; }
	COMPILETIMEEVAL FORCEDINLINE  Point2D Top_Right() const { return { X + Width, Y }; }
	COMPILETIMEEVAL FORCEDINLINE  Point2D Bottom_Left() const { return { X, Y + Height }; }
	COMPILETIMEEVAL FORCEDINLINE  Point2D Bottom_Right() const { return { X + Width, Y + Height }; }
	COMPILETIMEEVAL FORCEDINLINE  Point2D Top_Center() const { return { (X + Width) / 2, Y }; }
	COMPILETIMEEVAL FORCEDINLINE  Point2D Bottom_Center() const { return { (X + Width) / 2, Height }; }
	COMPILETIMEEVAL FORCEDINLINE  Point2D Left_Center() const { return { X, (Y + Height) / 2 }; }
	COMPILETIMEEVAL FORCEDINLINE  Point2D Center_Right() const { return { Width, (Y + Height) / 2 }; }

	COMPILETIMEEVAL int& operator[](int i) { return (&X)[i]; }
	COMPILETIMEEVAL const int& operator[](int i) const { return (&X)[i]; }

	COMPILETIMEEVAL FORCEDINLINE int& At(int i) { return (&X)[i]; }
	COMPILETIMEEVAL FORCEDINLINE const int& At(int i) const { return (&X)[i]; }

	static COMPILETIMEEVAL FORCEDINLINE const RectangleStruct Union(const RectangleStruct &rect1, const RectangleStruct &rect2)
	{
		RectangleStruct r = rect1;

		if (r.X > rect2.X) {
			r.Width += r.X - rect2.X;
			r.X = rect2.X;
		}
		if (r.Y > rect2.Y) {
			r.Height += r.Y - rect2.Y;
			r.Y = rect2.Y;
		}
		if (r.X + r.Width < rect2.X + rect2.Width) {
			r.Width = ((rect2.X + rect2.Width) - r.X) + 1;
		}
		if (r.Y + r.Height < rect2.Y + rect2.Height) {
			r.Height = ((rect2.Y + rect2.Height) - r.Y) + 1;
		}
		return r;
	}

	static COMPILETIMEEVAL FORCEDINLINE const RectangleStruct Intersect(const RectangleStruct &rect1, const RectangleStruct &rect2, int *x, int *y)
	{
		RectangleStruct r = rect2;

		if (r.X < rect1.X) {
			r.Width -= rect1.X - r.X;
			r.X = rect1.X;
		}
		if (r.Width < 1) return { 0, 0, 0, 0 };

		if (r.Y < rect1.Y) {
			r.Height -= rect1.Y - r.Y;
			r.Y = rect1.Y;
		}
		if (r.Height < 1) return { 0, 0, 0, 0 };

		if (r.X + r.Width > rect1.X + rect1.Width) {
			r.Width = rect1.X + rect1.Width - r.X;
		}
		if (r.Width < 1) return { 0, 0, 0, 0 };

		if (r.Y + r.Height > rect1.Y + rect1.Height) {
			r.Height = rect1.Y + rect1.Height - r.Y;
		}
		if (r.Height < 1) return { 0, 0, 0, 0 };

		if (x != nullptr) {
			*x -= (r.X - rect2.X);
		}
		if (y != nullptr) {
			*y -= (r.Y - rect2.Y);
		}

		return r;
	}

public:

	int X, Y, Width, Height;
};

//typedef RectangleStruct Rect;