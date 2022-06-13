#pragma once

// an imaginary world where only one quarter is real.
// obviously, this class needs some serious expansion
// if it is to be used for serious stuff.
#include <Matrix3D.h>
#include <tuple>

class Quaternion
{
public:
	//Constructor
	Quaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f) : X(x), Y(y), Z(z), W(w) {}

	auto operator()()
	{
		// returns a tuple to make it work with std::tie
		return std::make_tuple(X, Y, W);
	}

	void Normalize() { JMP_THIS(0x645C70); }
	//
	Quaternion Normalized(Quaternion rotation) { return rotation /= Norm(rotation); }
	float Norm(Quaternion rotation) { return Math::sqrt(rotation.X * rotation.X + rotation.Y * rotation.Y + rotation.Z * rotation.Z + rotation.W * rotation.W); }
	static float Dot(Quaternion A, Quaternion B) { return A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W; }
	//
	void Scale(float amount) { JMP_THIS(0x645CD0); }
	void Set(float x = 0.0, float y = 0.0, float z = 0.0, float w = 1.0) { JMP_THIS(0x645C50); }

	inline void Make_Identity() { Set(); };
	inline float Length2() const { return (X*X + Y*Y + Z*Z + W*W); }
	inline float Length() const { return Math::sqrt(Length2()); }

	Quaternion* Func_645D60(Quaternion* B) { JMP_THIS(0x645D60); }

	static Quaternion* __fastcall Multiply(Quaternion* ret, Quaternion* A, Quaternion* B) { JMP_STD(0x645ED0); }
	static Quaternion Multiply(Quaternion& A, Quaternion& B)
	{
		Quaternion buffer;
		Multiply(&buffer, &A, &B);
		return buffer;
	}

	static Quaternion* __fastcall Func_646040(Quaternion* ret, Quaternion* A, Quaternion* B) { JMP_STD(0x646040); }
	static Quaternion Func_646040(Quaternion& A, Quaternion& B)
	{
		Quaternion buffer;
		Func_646040(&buffer, &A, &B);
		return buffer;
	}

	static Quaternion* __fastcall Inverse(Quaternion* ret, Quaternion* A) { JMP_STD(0x6460C0); }
	static Quaternion Inverse(Quaternion& A)
	{
		Quaternion buffer;
		Inverse(&buffer, &A);
		return buffer;

	}

	static Quaternion* __fastcall Conjugate(Quaternion* ret, Quaternion* A) { JMP_STD(0x646110); }
	static Quaternion Conjugate(Quaternion& A)
	{
		Quaternion buffer;
		Conjugate(&buffer, &A);
		return buffer;
	}

	static Quaternion* __fastcall Trackball(Quaternion* ret, float x0, float y0, float x1, float y1, float radius) { JMP_STD(0x646160); }
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
	static Quaternion FromAxis(Vector3D<float>& vec, float phi)
	{
		Quaternion buffer;
		FromAxis(&buffer, &vec, phi);
		return buffer;
	}

	static Quaternion* __fastcall FromMatrix(Quaternion* ret, Matrix3D* Mtx) { JMP_STD(0x00646730); }
	static Quaternion FromMatrix(Matrix3D& Mtx)
	{
		Quaternion buffer;
		FromMatrix(&buffer, &Mtx);
		return buffer;
	}

	static Matrix3D* __fastcall ToMatrix(Matrix3D* mat, Quaternion* A) { JMP_STD(0x646980); }
	static Matrix3D ToMatrix(Quaternion& A)
	{
		Matrix3D buffer;
		ToMatrix(&buffer, &A);
		return buffer;
	}

	Quaternion& operator/=(const float rhs)
	{
		X /= rhs;
		Y /= rhs;
		Z /= rhs;
		W /= rhs;
		return *this;
	}

	Quaternion& operator+=(const float rhs)
	{
		X += rhs;
		Y += rhs;
		Z += rhs;
		W += rhs;
		return *this;
	}

	Quaternion& operator-=(const float rhs)
	{
		X -= rhs;
		Y -= rhs;
		Z -= rhs;
		W -= rhs;
		return *this;
	}

	Quaternion& operator*=(const float rhs)
	{
		X *= rhs;
		Y *= rhs;
		Z *= rhs;
		W *= rhs;
		return *this;
	}

	Quaternion& operator+=(const Quaternion B)
	{
		X += B.X;
		Y += B.Y;
		Z += B.Z;
		W += B.W;
		return *this;
	}

	Quaternion& operator-=(const Quaternion B)
	{
		X -= B.X;
		Y -= B.Y;
		Z -= B.Z;
		W -= B.W;
		return *this;
	}

	Quaternion& operator*=(const Quaternion B)
	{
		Quaternion buffer;

		buffer.W = W * B.W - X * B.X - Y * B.Y - Z * B.Z;
		buffer.X = X * B.W + W * B.X + Y * B.Z - Z * B.Y;
		buffer.Y = W * B.Y - X * B.Z + Y * B.W + Z * B.X;
		buffer.Z = W * B.Z + X * B.Y - Y * B.X + Z * B.W;

		*this = buffer;
		return *this;
	}

	bool operator==(const Quaternion B)
	{
		return
			X == B.X &&
			Y == B.Y &&
			Z == B.Z &&
			W == B.W;
	}

	bool operator!=(const Quaternion B) { return !(*this == B); }

	Quaternion operator* (const Quaternion &B)
	{
		Quaternion buffer;

		buffer.W = W * B.W - X * B.X - Y * B.Y - Z * B.Z;
		buffer.X = W * B.X + X * B.W + Y * B.Z - Z * B.Y;
		buffer.Y = W * B.Y + Y * B.W + Z * B.X - X * B.Z;
		buffer.Z = W * B.Z + Z * B.W + X * B.Y - Y * B.X;

		return(buffer);
	}

	Quaternion operator* (float scl)
	{
		Quaternion buffer = *this;
		return Quaternion(scl*buffer[0], scl*buffer[1], scl*buffer[2], scl*buffer[3]);
	}

	Quaternion operator/ (const Quaternion& b)
	{
		return (*this) * Inverse(b);
	}

	Quaternion operator- (const Quaternion& b)
	{
		Quaternion a = *this;
		return Quaternion(a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]);
	}

	Quaternion operator+ (const Quaternion& b)
	{
		Quaternion a = *this;
		return Quaternion(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);
	}

	float & operator[](int i) { JMP_THIS(0x645D00); }
	const float & operator[](int i) const { JMP_THIS(0x645D10); }
	Quaternion operator-() const { return Quaternion(-X, -Y, -Z, -W); }
	Quaternion operator+() const { return *this; }

	float Angle(Quaternion B) { double dot = Dot(*this, B); return static_cast<float>(Math::acos(fmin(fabs(dot), 1)) * 2); }

	Quaternion FromEuler(float x, float y, float z)
	{
		float cx = Math::cos(x * 0.5f);
		float cy = Math::cos(y * 0.5f);
		float cz = Math::cos(z * 0.5f);
		float sx = Math::cos(x * 0.5f);
		float sy = Math::cos(y * 0.5f);
		float sz = Math::cos(z * 0.5f);

		Quaternion buffer;

		buffer.X = cx * sy * sz + cy * cz * sx;
		buffer.Y = cx * cz * sy - cy * sx * sz;
		buffer.Z = cx * cy * sz - cz * sx * sy;
		buffer.W = sx * sy * sz + cx * cy * cz;

		return buffer;
	}

	Quaternion Inverse(Quaternion rotation) { float n = Norm(rotation); return Conjugate(rotation) /= (n * n); }

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
			float n4 = Math::acos(n3);
			float n5 = 1.0f / Math::sin(n4);
			n2 = Math::sin((1 - alpha) * n4) * n5;
			n1 = flag ? -Math::sin(alpha * n4) * n5 : Math::sin(alpha * n4) * n5;
		}
		Quaternion buffer;
		auto A = *this;

		buffer.X = (n2 * A.X) + (n1 * B.X);
		buffer.Y = (n2 * A.Y) + (n1 * B.Y);
		buffer.Z = (n2 * A.Z) + (n1 * B.Z);
		buffer.W = (n2 * A.W) + (n1 * B.W);

		return Normalized(buffer);
	}

	void FromDouble(double *m)
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

	float X;
	float Y;
	float Z;
	float W; // the real part
};
