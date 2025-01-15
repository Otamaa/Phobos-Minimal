#pragma once

// an imaginary world where only one quarter is real.
// obviously, this class needs some serious expansion
// if it is to be used for serious stuff.
//#include <Matrix3D.h>
#include <tuple>

class Quaternion
{
public:
	//Constructor
	//Quaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f) : X(x), Y(y), Z(z), W(w) {}

	//auto operator()()
	//{
		// returns a tuple to make it work with std::tie
	//	return std::make_tuple(X, Y, Z, W);
	//}

	void Normalize() {
		auto len2 = this->X * this->X + this->Y * this->Y + this->Z * this->Z + this->W * this->W;

		if (0.0 != len2)
		{
			this->X = this->X / len2;
			this->Y = this->Y / len2;
			this->Z = this->Z / len2;
			this->W = this->W / len2;
		}
	}

	Quaternion Normalized(Quaternion rotation) { return rotation /= Norm(rotation); }
	float Norm(const Quaternion& rotation) { return (float)Math::sqrt(rotation.X * rotation.X + rotation.Y * rotation.Y + rotation.Z * rotation.Z + rotation.W * rotation.W); }
	static COMPILETIMEEVAL FORCEDINLINE float Dot(const Quaternion& A, const Quaternion& B) { return A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W; }

	COMPILETIMEEVAL FORCEDINLINE void Scale(float s)
	{
		this->X = s * this->X;
		this->Y = s * this->Y;
		this->Z = s * this->Z;
		this->W = s * this->W;
	}

	COMPILETIMEEVAL FORCEDINLINE void Set(float x = 0.0, float y = 0.0, float z = 0.0, float w = 1.0)
	{
		this->X = x;
		this->Y = y;
		this->Z = z;
		this->W = w;
	}

	COMPILETIMEEVAL FORCEDINLINE  void Make_Identity() { Set(); };
	COMPILETIMEEVAL FORCEDINLINE  float Length2() const { return (X*X + Y*Y + Z*Z + W*W); }
	OPTIONALINLINE float Length() const { return (float)Math::sqrt(Length2()); }

	//idk
	Quaternion* Func_645D60(Quaternion* B) const { JMP_THIS(0x645D60); }

	static Quaternion* __fastcall Multiply(Quaternion* ret, Quaternion* A, Quaternion* B) { JMP_STD(0x645ED0); }
	static Quaternion Multiply(const Quaternion& that1, const Quaternion& that2)
	{
		float that2X = that2.X;
		float that2Y = that2.Y;
		float that2Z = that2.Z;
		float that2W = that2.W;
		float v14 = that1.Y * that2W;
		float v15 = that1.Z * that2W;
		double v4 = that1.W;
		float v9 = static_cast<float>(v4 * that2X);
		float v11 = static_cast<float>(that2Y * v4);
		float v13 = static_cast<float>(that2Z * v4);
		float v16 = that1.Z * that2.X - that2.Z * that1.X;
		float v17 = that2.Y * that1.X - that1.Y * that2.X;

		Quaternion x {};
		x.X = that1.Y * that2.Z - that1.Z * that2.Y + v9 + that1.X * that2W;
		x.Y = v14 + v11 + v16;
		x.Z = v15 + v13 + v17;
		x.W = static_cast<float>(v4 * that2.W - (that1.Z * that2.Z + that1.Y * that2.Y + that1.X * that2.X));

		double len2 = x.Z * x.Z + x.Y * x.Y + x.W * x.W + x.X * x.X;// Normalize

		if (0.0 != len2)
		{
			x.X = static_cast<float>(x.X / len2);
			x.Y = static_cast<float>(x.Y / len2);
			x.Z = static_cast<float>(x.Z / len2);
			x.W = static_cast<float>(x.W / len2);
		}

		return x;
	}

	static Quaternion* __fastcall Func_646040(Quaternion* ret, Quaternion* A, Quaternion* B) { JMP_STD(0x646040); }
	static Quaternion Func_646040(const Quaternion& a, const Quaternion& b)
	{
		return Multiply(a, { -b.X , -b.Y , -b.Z , b.W });
	}

	static Quaternion* __fastcall Conjugate(Quaternion* ret, Quaternion* A) { JMP_STD(0x646110); }
	static Quaternion Conjugate(const Quaternion& a) {
		return { -a.X  ,-a.Y , -a.Z ,a.W };
	}

	static Quaternion* __fastcall Trackball(Quaternion* ret, float x0, float y0, float x1, float y1, float radius) { JMP_STD(0x646160); }
	//idk
	static Quaternion Trackball(float x0, float y0, float x1, float y1, float radius)
	{
		Quaternion buffer;
		Trackball(&buffer, x0, y0, x1, y1, radius);
		return buffer;
	}

	static Quaternion* __fastcall Slerp(Quaternion* ret, Quaternion* A, Quaternion* B, float alpha) { JMP_STD(0x646590); }
	static Quaternion Slerp(Quaternion& A, Quaternion& B, float alpha)
	{
		Quaternion buffer;
		Slerp(&buffer, &A, &B, alpha);
		return buffer;
	}

	static Quaternion* __fastcall FromAxis(Quaternion* ret, Vector3D<float>* vec, float phi) { JMP_STD(0x646480); }
	static Quaternion FromAxis(Vector3D<float>& a, float phi)
	{
		float y = a.Y;
		float zz = a.Z;
		float xx = a.X;
		float x = a.X;
		double v7 = Math::sqrt(zz * zz + y * y + x * x);
		if (v7 != 0.0) {
			xx = static_cast<float>(x / v7);
			y = static_cast<float>(y / v7);
			zz = static_cast<float>(zz / v7);
		}

		double s = Math::sin(phi * 0.5);

		return {
			static_cast<float>(xx * s) ,
			static_cast<float>(y * s) ,
			static_cast<float>(zz * s) ,
			static_cast<float>(Math::cos(phi * 0.5))
		};
	}

	COMPILETIMEEVAL FORCEDINLINE Quaternion& operator/=(const float rhs)
	{
		X /= rhs;
		Y /= rhs;
		Z /= rhs;
		W /= rhs;
		return *this;
	}

	COMPILETIMEEVAL FORCEDINLINE Quaternion& operator+=(const float rhs)
	{
		X += rhs;
		Y += rhs;
		Z += rhs;
		W += rhs;
		return *this;
	}

	COMPILETIMEEVAL FORCEDINLINE Quaternion& operator-=(const float rhs)
	{
		X -= rhs;
		Y -= rhs;
		Z -= rhs;
		W -= rhs;
		return *this;
	}

	COMPILETIMEEVAL FORCEDINLINE Quaternion& operator*=(const float rhs)
	{
		X *= rhs;
		Y *= rhs;
		Z *= rhs;
		W *= rhs;
		return *this;
	}

	COMPILETIMEEVAL FORCEDINLINE Quaternion& operator+=(const Quaternion B)
	{
		X += B.X;
		Y += B.Y;
		Z += B.Z;
		W += B.W;
		return *this;
	}

	COMPILETIMEEVAL FORCEDINLINE Quaternion& operator-=(const Quaternion B)
	{
		X -= B.X;
		Y -= B.Y;
		Z -= B.Z;
		W -= B.W;
		return *this;
	}

	COMPILETIMEEVAL FORCEDINLINE Quaternion& operator*=(const Quaternion B)
	{
		Quaternion buffer {
			X * B.W + W * B.X + Y * B.Z - Z * B.Y ,
			W * B.Y - X * B.Z + Y * B.W + Z * B.X ,
			W * B.Z + X * B.Y - Y * B.X + Z * B.W ,
			W* B.W - X * B.X - Y * B.Y - Z * B.Z
		};

		*this = buffer;
		return *this;
	}

	COMPILETIMEEVAL FORCEDINLINE bool operator==(const Quaternion B)
	{
		return
			X == B.X &&
			Y == B.Y &&
			Z == B.Z &&
			W == B.W;
	}

	COMPILETIMEEVAL FORCEDINLINE bool operator!=(const Quaternion B) { return !(*this == B); }

	COMPILETIMEEVAL FORCEDINLINE Quaternion operator* (const Quaternion &B)
	{
		return {
			W* B.X + X * B.W + Y * B.Z - Z * B.Y ,
			W* B.Y + Y * B.W + Z * B.X - X * B.Z ,
			W* B.Z + Z * B.W + X * B.Y - Y * B.X ,
			W* B.W - X * B.X - Y * B.Y - Z * B.Z
		};
	}

	Quaternion operator* (float scl)
	{
		Quaternion buffer = *this;
		return {scl * buffer[0], scl * buffer[1], scl * buffer[2], scl * buffer[3] };
	}

	Quaternion operator/ (const Quaternion& b)
	{
		return (*this) * Inverse(b);
	}

	Quaternion operator- (const Quaternion& b)
	{
		Quaternion a = *this;
		return { a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3] };
	}

	Quaternion operator+ (const Quaternion& b)
	{
		Quaternion a = *this;
		return  { a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3] };
	}

	float& operator[](int i) { JMP_THIS(0x645D00); }
	const float& operator[](int i) const { JMP_THIS(0x645D10); }

	COMPILETIMEEVAL FORCEDINLINE Quaternion operator-() const { return { -X, -Y, -Z, W }; }
	COMPILETIMEEVAL FORCEDINLINE Quaternion operator+() const { return *this; }

	float Angle(Quaternion B) const {
		double dot = (double)Dot(*this, B);
		return (float)Math::acos(fmin(fabs(dot), 1)) * 2;
	}

	Quaternion FromEuler(float x, float y, float z)
	{
		float cx = (float)Math::cos(x * 0.5f);
		float cy = (float)Math::cos(y * 0.5f);
		float cz = (float)Math::cos(z * 0.5f);
		float sx = (float)Math::cos(x * 0.5f);
		float sy = (float)Math::cos(y * 0.5f);
		float sz = (float)Math::cos(z * 0.5f);

		return {
			cx* sy* sz + cy * cz * sx ,
			cx* cz* sy - cy * sx * sz ,
			cx* cy* sz - cz * sx * sy ,
			sx* sy* sz + cx * cy * cz
		};
	}

	Quaternion Inverse(const Quaternion& rotation) { float n = Norm(rotation); return Conjugate(rotation) /= (n * n); }

	Quaternion RotateTowards(Quaternion to, float maxRadiansDelta)
	{
		float angle = Angle(to);
		if (angle == 0.0f)
			return to;
		maxRadiansDelta = (float)fmax(maxRadiansDelta, angle - Math::Pi);
		float t = static_cast<float>(fmin(1, maxRadiansDelta / angle));
		return SlerpUnclamped(to, t);
	}

	Quaternion Slerp(Quaternion B, float alpha) { return Slerp(*this, B, alpha); }

	Quaternion SlerpUnclamped(Quaternion B, float alpha)
	{
		float n1;
		float n2;
		float n3 = Dot(*this, B);
		bool flag = false;
		if (n3 < 0.0f)
		{
			flag = true;
			n3 = -n3;
		}
		if (n3 > 0.999999f)
		{
			n2 = 1.0f - alpha;
			n1 = flag ? -alpha : alpha;
		}
		else
		{
			float n4 = (float)Math::acos(n3);
			float n5 = 1.0f / (float)Math::sin(n4);
			n2 = (float)Math::sin((1 - alpha) * n4) * n5;
			n1 = flag ? -(float)Math::sin(alpha * n4) * n5 : (float)Math::sin(alpha * n4) * n5;
		}

		Quaternion buffer {
			(n2 * this->X) + (n1 * B.X) ,
			(n2 * this->Y) + (n1 * B.Y) ,
			(n2 * this->Z) + (n1 * B.Z) ,
			(n2 * this->W) + (n1 * B.W)
		};

		return Normalized(buffer);
	}

	void FromDouble(double *m) const
	{
		if (!m) return;
		m[0] = 1.0f - 2.0f * (Y * Y + Z * Z);
		m[1] = 2.0f * (X * Y + Z * W);
		m[2] = 2.0f * (X * Z - Y * W);
		m[3] = 0.0f;
		m[4] = 2.0f * (X * Y - Z * W);
		m[5] = 1.0f - 2.0f * (X * X + Z * Z);
		m[6] = 2.0f * (Z * Y + X * W);
		m[7] = 0.0f;
		m[8] = 2.0f * (X * Z + Y * W);
		m[9] = 2.0f * (Y * Z - X * W);
		m[10] = 1.0f - (2.0f * (X * X + Y * Y));
		m[11] = 0.0f;
		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1.0f;
	};

public:
	float X;
	float Y;
	float Z;
	float W; // the real part
};
