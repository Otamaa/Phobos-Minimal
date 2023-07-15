#pragma once

class Point2D
{
public:
	static const Point2D Empty;

	Point2D& operator=(const Point2D& that)
	{
		if (this != &that)
		{
			X = that.X;
			Y = that.Y;
		}
		return *this;
	}

	bool operator==(const Point2D& that) const { return X == that.X && Y == that.Y; }
	bool operator!=(const Point2D& that) const { return X != that.X && Y != that.Y; }

	Point2D& operator++() { 
		++X; 
		++Y;
		return *this;
	}

	Point2D& operator--() { 
		--X;
		--Y;
		return *this;
	}

	Point2D operator++(int) { Point2D orig = *this; ++(*this); return orig; }
	Point2D operator--(int) { Point2D orig = *this; --(*this); return orig; }

	Point2D operator+() const { return Point2D{+X, +Y};}
	Point2D operator-() const { return Point2D{-X, -Y};}

	Point2D operator+(const Point2D& that) const { return Point2D{X + that.X, Y + that.Y};}
	Point2D& operator+=(const Point2D& that) { X += that.X; Y += that.Y; return *this; }
	Point2D& operator+=(int factor) { X += factor; Y += factor; return *this; }

	Point2D operator-(const Point2D& that) const { return Point2D{X - that.X, Y - that.Y};}
	Point2D& operator-=(const Point2D& that) { X -= that.X; Y -= that.Y; return *this; }

	Point2D operator*(const Point2D& that) const { return Point2D{X * that.X, Y * that.Y};}
	Point2D operator*=(const Point2D& that) { X *= that.X; Y *= that.Y; return *this; }
	Point2D operator*(int factor) const { return Point2D{X * factor, Y * factor};}
	Point2D& operator*=(int factor) { X *= factor; Y *= factor; return *this; }

	Point2D operator*(double factor) const { return Point2D{static_cast<int>(X * factor), static_cast<int>(Y * factor)};}
	Point2D& operator*=(double factor) { X *= static_cast<int>(factor); Y *= static_cast<int>(factor); return *this; }

	Point2D operator/(const Point2D& that) const { return Point2D{X / that.X, Y / that.Y};}
	Point2D operator/=(const Point2D& that) { X /= that.X; Y /= that.Y; return *this; }
	Point2D operator/(int factor) const { return Point2D{X / factor, Y / factor};}
	Point2D& operator/=(int factor) { X /= factor; Y /= factor; return *this; }

	Point2D operator%(const Point2D& that) const { return Point2D{X / that.X, Y / that.Y};}
	Point2D operator%=(const Point2D& that) { X /= that.X; Y /= that.Y; return *this; }
	Point2D operator%(int factor) const { return Point2D{X / factor, Y / factor};}
	Point2D& operator%=(int factor) { X /= factor; Y /= factor; return *this; }

	Point2D operator&(const Point2D& that) const { return Point2D{X & that.X, Y & that.Y};}
	Point2D operator&=(const Point2D& that) { X &= that.X; Y &= that.Y; return *this; }
	Point2D operator&(int factor) const { return Point2D{X & factor, Y & factor};}
	Point2D& operator&=(int factor) { X &= factor; Y &= factor; return *this; }

	bool operator>(const Point2D& that) const { return X > that.X || X == that.X && Y > that.Y; }
	bool operator>=(const Point2D& that) const { return X >= that.X || X == that.X && Y >= that.Y; }

	bool operator<(const Point2D& that) const { return X < that.X || X == that.X && Y < that.Y; }
	bool operator<=(const Point2D& that) const { return X <= that.X || X == that.X && Y <= that.Y; }

	inline bool IsValid() const { return *this != (Point2D::Empty); }

	const int DistanceFrom(Point2D const& nThat)
	{
		return abs((nThat.X - X) * (nThat.X - X)) + abs((nThat.Y - Y) * (nThat.Y - Y));
	}

	int Length() const
	{
		return static_cast<int>(Math::sqrt(static_cast<double>((X * X) + (Y * Y))));
	}

public:

	int X, Y;
};