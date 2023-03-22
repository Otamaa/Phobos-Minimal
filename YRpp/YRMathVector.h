//pd's Vector classes (math sense)
#pragma once

#include <Base/SSE_Math.h>
#include <YRMath.h>
#include <tuple>

/*==========================================
============ 2D Vector =====================
==========================================*/

template <typename T> class Vector2D
{
public:
	static const Vector2D Empty;

	//no constructor, so this class stays aggregate and can be initialized using the curly braces {}
	T X, Y;

	//auto operator()()
	//{
		// returns a tuple to make it work with std::tie
	//	return std::make_pair(X, Y);
	//}

	Vector2D& operator=(const Vector2D& v)
	{
		X = v.X;
		Y = v.Y;
		return *this;
	}
	//operator overloads
	//addition
	Vector2D operator+(const Vector2D& a) const
	{
		return Vector2D{ X + a.X, Y + a.Y };
	}
	//addition
	Vector2D& operator+=(const Vector2D& a)
	{
		X += a.X;
		Y += a.Y;
		return *this;
	}
	//substraction
	Vector2D operator-(const Vector2D& a) const
	{
		return Vector2D{ X - a.X, Y - a.Y };
	}
	//substraction
	Vector2D& operator-=(const Vector2D& a)
	{
		X -= a.X;
		Y -= a.Y;
		return *this;
	}
	//negation
	Vector2D operator-() const
	{
		return Vector2D{ -X, -Y };
	}
	//equality
	bool operator==(const Vector2D& a) const
	{
		return (X == a.X && Y == a.Y);
	}
	//unequality
	inline bool operator!=(const Vector2D& a) const
	{
		return (X != a.X || Y != a.Y);
	}
	//scalar multiplication
	Vector2D operator*(double r) const
	{
		return Vector2D{ static_cast<T>(X * r), static_cast<T>(Y * r) };
	}

	//scalar multiplication
	Vector2D& operator*=(double r)
	{
		X *= r;
		Y *= r;
		return *this;
	}

	inline T & operator [](int i) { return (&X)[i]; }
	inline const T &  operator [](int i) const { return (&X)[i]; }

	//vector multiplication
	double operator*(const Vector2D& a) const
	{
		return static_cast<double>(X) * a.X + static_cast<double>(Y) * a.Y;
	}

	inline bool IsValid() const { return *this != (Vector2D<T>::Empty); }

	//magnitude
	double Magnitude() const
	{
		return sqrt_fast_tpl(MagnitudeSquared());
	}
	//magnitude squared
	double MagnitudeSquared() const
	{
		return *this * *this;
	}
	//distance from another vector
	double DistanceFrom(const Vector2D& a) const
	{
		return (a - *this).Magnitude();
	}
	//distance from another vector, squared
	double DistanceFromSquared(const Vector2D& a) const
	{
		return (a - *this).MagnitudeSquared();
	}

	//and do sqrt when needed only
	double DistanceFromAutoMethod(const Vector2D& a, bool nQuick = false) const
	{
		if (nQuick)
			return Quick_Distance(a);

		return DistanceFrom(a);
	}

	//collinearity
	bool IsCollinearTo(const Vector2D& a) const
	{
		auto const c = static_cast<double>(X) * a.Y;
		return c == static_cast<double>(Y) * a.X;
	}
	//find scalar
	double FindScalar(const Vector2D& a) const
	{
		double r = static_cast<double>(a.X) / static_cast<double>(X);
		if(static_cast<T>(r * Y) == a.Y) {
			return r;
		}
		else {
			//the vectors are not collinear, return NaN!
			unsigned long NaN[2] = {0xFFFFFFFF,0x7FFFFFFF};
			return *reinterpret_cast<double*>(NaN);
		}
	}
	//=====================================================================
	double Angle(const Vector2D b) const
	{
		double Mag = (b).Magnitude();
		double MagT = this->Magnitude();
		double Dot = (this->X * b.X) + (this->Y * b.Y);
		double v = Dot / (Mag * MagT);

		v = fmax(v, -1.0);
		v = fmin(v, 1.0);

		return std::acos(v);
	}

	Vector2D FromPolar(double rad, double theta)
	{
		Vector2D v;
		v.X = rad * std::cos(theta);
		v.Y = rad * std::sin(theta);
		return v;
	}

	Vector2D Lerp(Vector2D b, double t)
	{
		if (t < 0) return *this;
		else if (t > 1) return b;
		return LerpUnclamped(b, t);
	}

	Vector2D LerpUnclamped(Vector2D b, double t)
	{
		return (b - *this) * t + *this;
	}

	Vector2D Max(Vector2D b)
	{
		double x = X > b.X ? X : b.X;
		double y = Y > b.Y ? Y : b.Y;

		Vector2D pBuffer;
		pBuffer.X += static_cast<T>(x);
		pBuffer.Y += static_cast<T>(y);
		return pBuffer;
	}

	Vector2D Min(Vector2D b)
	{
		double x = X > b.X ? b.X : X;
		double y = Y > b.Y ? b.Y : Y;

		Vector2D pBuffer;
		pBuffer.X += static_cast<T>(x);
		pBuffer.Y += static_cast<T>(y);
		return pBuffer;
	}

	Vector2D MoveTowards(Vector2D target, double maxDistanceDelta)
	{
		Vector2D d = target - *this;
		double m = MagnitudeFor(d);
		if (m < maxDistanceDelta || m == 0)
			return target;
		return *this += (d *= maxDistanceDelta * m);
	}

	double Dot(Vector2D rhs)
	{
		return X * rhs.X + Y * rhs.Y;
	}

	Vector2D Reflect(Vector2D planeNormal)
	{
		return *this - 2 * Project(planeNormal);
	}

	Vector2D Reject(Vector2D b)
	{
		return *this - Project(b);
	}

	void ToPolar(double &rad, double &theta)
	{
		rad = MagnitudeFor(*this);
		theta = atan2(Y, X);
	}

	void Scale(Vector2D b)
	{
		return Vector2D(X * b.X, Y * b.Y);
	}

	void Rotate(double theta)
	{
		return Rotate(std::sin(theta), std::cos(theta));;
	}

	void Rotate(double s, double c, bool Sin_Cos)
	{
		double new_x = X * c + Y * -s;
		double new_y = X * s + Y * c;
		X = new_x;
		Y = new_y;
	}

	inline void Set(const Vector2D & v) { X = v.X; Y = v.Y; }

	bool Rotate_Towards_Vector(Vector2D &target, double max_s, double max_c, bool &positive_turn, bool Sin_Cos)
	{
		if (Sin_Cos)
		{
			max_s = std::sin(max_s);
			max_c = std::cos(max_c);
		}

		bool return_value = false;

		positive_turn = Perp_Dot_Product(target, *this) > 0.0;

		if (Dot_Product(*this, target) >= max_c)
		{
			Set(target);
			return_value = true;

		}
		else {

			if (positive_turn)
			{
				Rotate(max_s, max_c);
			}
			else {
				Rotate(-max_s, max_c);
			}
		}

		return return_value;
	}

	inline void Swap(Vector2D & b)
	{
		Vector2D tmp(*this);
		*this = b;
		b = tmp;
	}

	void Update_Min(const Vector2D & a)
	{
		if (a.X < X) X = a.X;
		if (a.Y < Y) Y = a.Y;
	}


	void Update_Max(const Vector2D & a)
	{
		if (a.X > X) X = a.X;
		if (a.Y > Y) Y = a.Y;
	}

	void Scale(T a, T b)
	{
		X *= a;
		Y *= b;
	}

	double Quick_Distance(const Vector2D& b) const
	{
		Vector2D buffer = *this;
		double x_diff = abs(static_cast<double>(buffer.X - b.X));
		double y_diff = abs(static_cast<double>(buffer.Y - b.Y));

		if (x_diff > y_diff)
		{
			return (y_diff / 2) + x_diff;
		}
		else
		{
			return (x_diff / 2) + y_diff;
		}
	}

	bool Equal_Within_Epsilon(const Vector2D &b, double epsilon)
	{
		return(abs(X - b.X) < epsilon) && (abs(Y - b.Y) < epsilon);
	}

	Vector2D Normalize()
	{
		Vector2D buffer = Vector2D::Empty;
		double len2 = static_cast<double>(X*X + Y * Y);
		if (len2 != 0.0)
		{
			double oolen = static_cast<double>(Math::Q_invsqrt(static_cast<float>(len2)));
			buffer.X *= oolen;
			buffer.Y *= oolen;
		}

		return buffer;
	}

private:

	double Dot_Product(const Vector2D &a, const Vector2D &b)
	{
		return a * b;
	}

	double Perp_Dot_Product(const Vector2D &a, const Vector2D &b)
	{
		return a.X * -b.Y + a.Y * b.X;
	}

	double SqrMagnitude(Vector2D v)
	{
		return v.X * v.X + v.Y * v.Y;
	}

	Vector2D Project(Vector2D b)
	{
		double m = MagnitudeFor(b);
		return Dot(b) / (m * m) * b;
	}

	double MagnitudeFor(Vector2D v)
	{
		return sqrt_fast_tpl(SqrMagnitude(v));
	}

};

template <typename T>
const Vector2D<T> Vector2D<T>::Empty = {T(), T()};

/*==========================================
============ 3D Vector =====================
==========================================*/

template <typename T> class Vector3D
{
public:
	static const Vector3D Empty;

	//no constructor, so this class stays aggregate and can be initialized using the curly braces {}
	T X,Y,Z;

	//auto operator()()
	//{
		// returns a tuple to make it work with std::tie
	//	return std::make_tuple(X, Y, Z);
	//}

	//operator overloads
	//addition
	Vector3D operator+(const Vector3D& a) const
	{
		return Vector3D{ X + a.X, Y + a.Y, Z + a.Z };
	}
	//addition
	Vector3D& operator+=(const Vector3D& a)
	{
		X += a.X;
		Y += a.Y;
		Z += a.Z;
		return *this;
	}
	//substraction
	Vector3D operator-(const Vector3D& a) const
	{
		return Vector3D{ X - a.X, Y - a.Y, Z - a.Z };
	}
	//substraction
	Vector3D& operator-=(const Vector3D& a)
	{
		X -= a.X;
		Y -= a.Y;
		Z -= a.Z;
		return *this;
	}
	//negation
	Vector3D operator-() const
	{
		return Vector3D{ -X, -Y, -Z };
	}
	//equality
	bool operator==(const Vector3D& a) const
	{
		return (X == a.X && Y == a.Y && Z == a.Z);
	}
	//unequality
	bool operator!=(const Vector3D& a) const
	{
		return (X != a.X || Y != a.Y || Z != a.Z);
	}
	//scalar multiplication
	Vector3D operator*(double r) const
	{
		return Vector3D{
			static_cast<T>(X * r),
			static_cast<T>(Y * r),
			static_cast<T>(Z * r) };
	}

	Vector3D operator/(T nval) const
	{ return { (X / nval), (Y / nval), (Z / nval) }; }

	//scalar multiplication
	Vector3D& operator*=(double r)
	{
		X *= r;
		Y *= r;
		Z *= r;
		return *this;
	}

	inline T &operator[](int i) { return (&X)[i]; }
	inline const T &operator[](int i) const { return (&X)[i]; }

	//vector multiplication
	double operator*(const Vector3D& a) const
	{
		return static_cast<double>(X * a.X)
			+ static_cast<double>(Y * a.Y)
			+ static_cast<double>(Z * a.Z);
	}

	inline bool IsValid() const { return *this != (Vector3D<T>::Empty); }

	//magnitude
	double Magnitude() const
	{
		return sqrt_fast_tpl(MagnitudeSquared());
	}

	double MagnitudeXY()
	{ return sqrt_fast_tpl(X * X + Y * Y); }

	//magnitude squared
	double MagnitudeSquared() const
	{
		return *this * *this;
	}
	//distance from another vector
	double DistanceFrom(const Vector3D& a) const
	{
		return (a - *this).Magnitude();
	}
	//distance from another vector, squared
	double DistanceFromSquared(const Vector3D& a) const
	{
		return (a - *this).MagnitudeSquared();
	}

	double DistanceFromAutoMethod(const Vector3D& a, bool nQuick = false) const
	{
		if (nQuick)
			return Quick_distance(a);

		return DistanceFrom(a);
	}

	//collinearity
	bool IsCollinearTo(const Vector3D& a) const
	{
		return CrossProduct(a).MagnitudeSquared() == 0;
	}
	//find scalar
	double FindScalar(const Vector3D& a) const
	{
		double r = static_cast<double>(a.X) / static_cast<double>(X);
		if((static_cast<T>(r * Y) == a.Y) && (static_cast<T>(r * Z) == a.Z)) {
			return r;
		} else {
			//the vectors are not collinear, return NaN!
			unsigned long NaN[2] = {0xFFFFFFFF,0x7FFFFFFF};
			return *reinterpret_cast<double*>(NaN);
		}
	}
	//cross product
	Vector3D CrossProduct(const Vector3D& a) const
	{
		return Vector3D{
			Y * a.Z - Z * a.Y,
			Z * a.X - X * a.Z,
			X * a.Y - Y * a.X };
	}

	double Angle(const Vector3D b) const
	{
		double Mag = (b).Magnitude();
		double MagT = this->Magnitude();
		double Dot = (this->X * b.X) + (this->Y * b.Y) + (this->Z * b.Z);
		double v = Dot / (Mag * MagT);

		v = fmax(v, -1.0);
		v = fmin(v, 1.0);

		return std::acos(v);
	}

	Vector3D Lerp(Vector3D b, double t)
	{
		if (t < 0) return *this;
		else if (t > 1) return b;
		return LerpUnclamped(b, t);
	}

	Vector3D LerpUnclamped(Vector3D b, double t)
	{
		return (b - *this) * t + *this;
	}

	Vector3D Max(Vector3D b)
	{
		double x = X > b.X ? X : b.X;
		double y = Y > b.Y ? Y : b.Y;
		double z = Z > b.Z ? b.Z : Z;

		Vector3D pBuffer;
		pBuffer.X += static_cast<T>(x);
		pBuffer.Y += static_cast<T>(y);
		pBuffer.Z += static_cast<T>(z);
		return pBuffer;
	}

	Vector3D Min(Vector3D b)
	{
		double x = X > b.X ? b.X : X;
		double y = Y > b.Y ? b.Y : Y;
		double z = Z > b.Z ? b.Z : Z;

		Vector3D pBuffer;
		pBuffer.X += static_cast<T>(x);
		pBuffer.Y += static_cast<T>(y);
		pBuffer.Z += static_cast<T>(z);
		return pBuffer;
	}

	Vector3D MoveTowards(Vector3D target, double maxDistanceDelta)
	{
		Vector3D d = target - *this;
		double m = MagnitudeFor(d);
		if (m < maxDistanceDelta || m == 0)
			return target;
		return *this += (d *= maxDistanceDelta * m);
	}

	double Dot(Vector3D rhs)
	{
		return X * rhs.X + Y * rhs.Y + Z * rhs.Z;
	}

	Vector3D Reflect(Vector3D planeNormal)
	{
		return *this - 2 * Project(planeNormal);
	}

	Vector3D Reject(Vector3D b)
	{
		return *this - Project(b);
	}

	void Scale(Vector3D b)
	{
		return Vector3D(X * b.X, Y * b.Y, Z * b.Z);
	}

	bool Equal_Within_Epsilon(const Vector3D &b, double epsilon)
	{
		return ((abs(X - b.X) < epsilon) &&
			(abs(Y - b.Y) < epsilon) &&
			(abs(Z - b.Z) < epsilon));
	}

	T Cross_Product_X(const Vector3D &b)
	{
		return Y * b.Z - Z * b.Y;
	}


	T Cross_Product_Y(const Vector3D &b)
	{
		return Z * b.X - X * b.Z;
	}


	T Cross_Product_Z(const Vector3D &b)
	{
		return X * b.Y - Y * b.X;
	}

	Vector3D Normalize()
	{
		Vector3D buffer = Vector3D::Empty;
		double len2 = static_cast<double>(X * X + Y * Y + Z * Z);

		if (len2 != 0.0)
		{
			auto oolen = Math::Q_invsqrt(len2);
			buffer.X *= oolen;
			buffer.Y *= oolen;
			buffer.Z *= oolen;
		}

		return buffer;
	}

	double Quick_distance(const Vector3D &b) const
	{
		double max = abs(static_cast<double>(X - b.X));
		double mid = abs(static_cast<double>(Y - b.Y));
		double min = abs(static_cast<double>(Z - b.Z));

		double tmp;

		if (max < mid)
		{
			tmp = max;
			max = mid;
			mid = tmp;
		}
		if (max < min)
		{
			tmp = max;
			max = min;
			min = tmp;
		}
		if (mid < min)
		{
			tmp = mid;
			mid = min;
			min = mid;
		}

		return max + (11.0 / 32.0) * mid + (1.0 / 4.0) * min;
	}

	void Swap(Vector3D &b)
	{
		Vector3D tmp(*this);
		*this = b;
		b = tmp;
	}

	void Update_Min(const Vector3D &a)
	{
		if (a.X < X) X = a.X;
		if (a.Y < Y) Y = a.Y;
		if (a.Z < Z) Z = a.Z;
	}


	void Update_Max(const Vector3D &a) {
		if (a.X > X) X = a.X;
		if (a.Y > Y) Y = a.Y;
		if (a.Z > Z) Z = a.Z;
	}

	void Cap_Absolute_To(const Vector3D &a)
	{
		if (X > 0) {
			if (a.X < X) {
				X = a.X;
			}
		}
		else {
			if (-a.X > X) {
				X = -a.X;
			}
		}

		if (Y > 0) {
			if (a.Y < Y) {
				Y = a.Y;
			}
		}
		else {
			if (-a.Y > Y) {
				Y = -a.Y;
			}
		}

		if (Z > 0) {
			if (a.Z < Z) {
				Z = a.Z;
			}
		}
		else {
			if (-a.Z > Z) {
				Z = -a.Z;
			}
		}
	}

	void Rotate_X(double angle)
	{
		Rotate_X(std::sin(angle), std::cos(angle));
	}


	void Rotate_X(double s_angle, double c_angle)
	{
		double tmp_y = static_cast<double>(Y);
		double tmp_z = static_cast<double>(Z);

		Y = c_angle * tmp_y - s_angle * tmp_z;
		Z = s_angle * tmp_y + c_angle * tmp_z;
	}

	void Rotate_Y(double angle)
	{
		Rotate_X(std::sin(angle), std::cos(angle));
	}

	void Rotate_Y(double s_angle, double c_angle)
	{
		double tmp_x = static_cast<double>(X);
		double tmp_z = static_cast<double>(Z);

		X = c_angle * tmp_x + s_angle * tmp_z;
		Z = -s_angle * tmp_x + c_angle * tmp_z;
	}

	void Rotate_Z(double angle)
	{
		Rotate_X(std::sin(angle), std::cos(angle));
	}

	void Rotate_Z(double s_angle, double c_angle)
	{
		double tmp_x = static_cast<double>(X);
		double tmp_y = static_cast<double>(Y);

		X = c_angle * tmp_x - s_angle * tmp_y;
		Y = s_angle * tmp_x + c_angle * tmp_y;
	}

	T Find_X_At_Y(T y, const Vector3D &p2)
	{
		return (X + ((y - Y) * ((p2.X - X) / (p2.Y - Y))));
	}

	T Find_X_At_Z(T z, const Vector3D &p2)
	{
		return (X + ((z - Z) * ((p2.X - X) / (p2.Z - Z))));
	}

	T Find_Y_At_X(T x, const Vector3D &p2)
	{
		return (Y + ((x - X) * ((p2.Y - Y) / (p2.X - X))));
	}

	T Find_Y_At_Z(T z, const Vector3D &p2)
	{
		return (Y + ((z - Z) * ((p2.Y - Y) / (p2.Z - Z))));
	}

	T Find_Z_At_X(T x, const Vector3D &p2)
	{
		return (Z + ((x - X) * ((p2.Z - Z) / (p2.X - X))));
	}

	T Find_Z_At_Y(T y, const Vector3D &p2)
	{
		return (Z + ((y - Y) * ((p2.Z - Z) / (p2.Y - Y))));
	}

private:

	Vector3D Project(Vector3D b)
	{
		double m = MagnitudeFor(b);
		return Dot(b) / (m * m) * b;
	}

	double MagnitudeFor(Vector3D v)
	{
		return sqrt_fast_tpl(SqrMagnitude(v));
	}

	double SqrMagnitude(Vector3D v)
	{
		return v.X * v.X + v.Y * v.Y + v.Z * v.Z;
	}

};

template <typename T>
const Vector3D<T> Vector3D<T>::Empty = { T(), T(), T() };

template <typename T> class Vector4D
{
public:
	static const Vector4D Empty;
	//Vector4D(T x, T y, T z, T w) : X(x), Y(y), Z(z), W(w) {}
	//Vector4D(const Vector4D &that) : X(that.X), Y(that.Y), Z(that.Z), W(that.W) {}

	//no constructor, so this class stays aggregate and can be initialized using the curly braces {}
	T X, Y, Z, W;

	//auto operator()()
	//{
		// returns a tuple to make it work with std::tie
	//	return std::make_tuple(X, Y, Z, W);
	//}

	inline bool operator==(const Vector4D &b)
	{
		Vector4D a = *this;
		return ((a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]) && (a[3] == b[3]));
	}

	inline bool operator!=(const Vector4D &b)
	{
		Vector4D a = *this;
		return ((a[0] != b[0]) || (a[1] != b[1]) || (a[2] != b[2]) || (a[3] != b[3]));
	}

	double Length() const
	{
		return sqrt_fast_tpl(Length2());
	}

	inline void Swap(Vector4D &b)
	{
		Vector4D tmp = *this;
		*this = b;
		b = tmp;
	}

	inline Vector4D Lerp(const Vector4D &b, T alpha)
	{
		return Vector4D{ (X + (b.X - X) * alpha), (Y + (b.Y - Y) * alpha), (Z + (b.Z - Z) * alpha), (W + (b.W - W) * alpha) };
	}

	double Length2() const
	{
		return X * X + Y * Y + Z * Z + W * W;
	}

	inline bool IsValid() const { return *this != (Vector4D<T>::Empty); }

	inline Vector4D& operator=(const Vector4D &v)
	{
		X = v.X;
		Y = v.Y;
		Z = v.Z;
		W = v.W;
		return *this;
	}

	inline Vector4D &operator+=(const Vector4D &v)
	{
		X += v.X;
		Y += v.Y;
		Z += v.Z;
		W += v.W;
		return *this;
	}
	inline Vector4D &operator-=(const Vector4D &v)
	{
		X -= v.X;
		Y -= v.Y;
		Z -= v.Z;
		W += v.W;
		return *this;
	}
	inline Vector4D &operator*=(float k)
	{
		X = X * k;
		Y = Y * k;
		Z = Z * k;
		W = W * k;
		return *this;
	}
	inline Vector4D &operator/=(float k)
	{
		k = 1.0f / k;
		X = X * k;
		Y = Y * k;
		Z = Z * k;
		W = W * k;
		return *this;
	}

	inline Vector4D operator*(float k)
	{
		return Vector4D{ (X * k), (Y * k), (Z * k), (W * k) };
	}

	inline Vector4D operator/(float k)
	{
		float ook = 1.0f / k;
		Vector4D nThis = *this;
		return Vector4D{ (nThis[0] * ook), (nThis[1] * ook), (nThis[2] * ook), (nThis[3] * ook) };
	}


	inline Vector4D operator+(const Vector4D &b)
	{
		Vector4D nThis = *this;
		return Vector4D{ nThis[0] + b[0],nThis[1] + b[1],nThis[2] + b[2],nThis[3] + b[3] };
	}


	inline Vector4D operator-(const Vector4D &b)
	{
		Vector4D nThis = *this;
		return Vector4D{ nThis[0] - b[0], nThis[1] - b[1], nThis[2] - b[2], nThis[3] - b[3] };
	}


	inline double operator*(const Vector4D &b)
	{
		Vector4D nThis = *this;
		return nThis[0] * b[0] + nThis[1] * b[1] + nThis[2] * b[2] + nThis[3] * b[3];
	}

	double Dot_Product(const Vector4D &b)
	{
		return *this * b;
	}

	inline void Set(float x, float y, float z, float w)
	{
		X = x;
		Y = y;
		Z = z;
		W = w;
	}

	inline T &operator[](int i) { return (&X)[i]; }
	inline const T &operator[](int i) const { return (&X)[i]; }
};

template <typename T>
const Vector4D<T> Vector4D<T>::Empty = { T(), T(), T(), T() };


template<typename T>
class PartialVector2D : public Vector2D<T>
{
public:
	size_t ValueCount;
};

template<typename T>
class PartialVector3D : public Vector3D<T>
{
public:
	size_t ValueCount;
};

template<typename T>
class PartialVector4D : public Vector4D<T>
{
public:
	size_t ValueCount;
};
