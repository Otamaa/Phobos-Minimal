#pragma once

class Point2D
{
public:
	static const Point2D Empty;

	//Point2D& operator=(const Point2D& that)
	//{
	//	if (this != &that)
	//	{
	//		X = that.X;
	//		Y = that.Y;
	//	}
	//	return *this;
	//}

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

	Point2D operator+() const { return {+X, +Y};}
	Point2D operator-() const { return {-X, -Y};}

	Point2D operator+(const Point2D& that) const { return {X + that.X, Y + that.Y};}
	Point2D& operator+=(const Point2D& that) { X += that.X; Y += that.Y; return *this; }
	Point2D& operator+=(int factor) { X += factor; Y += factor; return *this; }

	Point2D operator-(const Point2D& that) const { return {X - that.X, Y - that.Y};}
	Point2D& operator-=(const Point2D& that) { X -= that.X; Y -= that.Y; return *this; }

	Point2D operator*(const Point2D& that) const { return {X * that.X, Y * that.Y};}
	Point2D operator*=(const Point2D& that) { X *= that.X; Y *= that.Y; return *this; }
	Point2D operator*(int factor) const { return {X * factor, Y * factor};}
	Point2D& operator*=(int factor) { X *= factor; Y *= factor; return *this; }

	Point2D operator*(double factor) const { return {static_cast<int>(X * factor), static_cast<int>(Y * factor)};}
	Point2D& operator*=(double factor) { X *= static_cast<int>(factor); Y *= static_cast<int>(factor); return *this; }

	Point2D operator/(const Point2D& that) const { return {X / that.X, Y / that.Y};}
	Point2D operator/=(const Point2D& that) { X /= that.X; Y /= that.Y; return *this; }
	Point2D operator/(int factor) const { return {X / factor, Y / factor};}
	Point2D& operator/=(int factor) { X /= factor; Y /= factor; return *this; }

	Point2D operator%(const Point2D& that) const { return {X / that.X, Y / that.Y};}
	Point2D operator%=(const Point2D& that) { X /= that.X; Y /= that.Y; return *this; }
	Point2D operator%(int factor) const { return {X / factor, Y / factor};}
	Point2D& operator%=(int factor) { X /= factor; Y /= factor; return *this; }

	Point2D operator&(const Point2D& that) const { return {X & that.X, Y & that.Y};}
	Point2D operator&=(const Point2D& that) { X &= that.X; Y &= that.Y; return *this; }
	Point2D operator&(int factor) const { return {X & factor, Y & factor};}
	Point2D& operator&=(int factor) { X &= factor; Y &= factor; return *this; }

	bool operator>(const Point2D& that) const { return X > that.X || X == that.X && Y > that.Y; }
	bool operator>=(const Point2D& that) const { return X >= that.X || X == that.X && Y >= that.Y; }

	bool operator<(const Point2D& that) const { return X < that.X || X == that.X && Y < that.Y; }
	bool operator<=(const Point2D& that) const { return X <= that.X || X == that.X && Y <= that.Y; }

	inline bool IsValid() const { return *this != (Point2D::Empty); }

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

	inline double DistanceFrom(const Point2D& that) const{
		return (that - *this).Length();
	}

	inline double DistanceFromSquared(const Point2D& that) const {
		return (that - *this).pow();
	}
public:

	int X, Y;
};