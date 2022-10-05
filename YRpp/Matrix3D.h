#pragma once

#include <YRPPCore.h>
#include <GeneralStructures.h>
#include <Helpers/CompileTime.h>
#include <Quaternion.h>

class Matrix3D
{
public:
	static constexpr reference<Matrix3D, 0xB44318> VoxelDefaultMatrix {};
	static constexpr reference<Matrix3D, 0xB45188, 21> VoxelRampMatrix {};

	//Constructor

	Matrix3D() = default;

	// plain floats ctor
	Matrix3D(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23)
	{
		row[0][0] = m00; row[0][1] = m01; row[0][2] = m02; row[0][3] = m03;
		row[1][0] = m10; row[1][1] = m11; row[1][2] = m12; row[1][3] = m13;
		row[2][0] = m20; row[2][1] = m21; row[2][2] = m22; row[2][3] = m23;
		// JMP_THIS(0x5AE630);
	}

	// column vector ctor
	Matrix3D(
		Vector3D<float> const &x,
		Vector3D<float> const &y,
		Vector3D<float> const &z,
		Vector3D<float> const &pos)
	{
		row[0][0] = x.X; row[0][1] = y.X; row[0][2] = z.X; row[0][3] = pos.X;
		row[1][0] = x.Y; row[1][1] = y.Y; row[1][2] = z.Y; row[1][3] = pos.Y;
		row[2][0] = x.Z; row[2][1] = y.Z; row[2][2] = z.Z; row[2][3] = pos.Z;
		// JMP_THIS(0x5AE690);
	}

	// some other rotation ctor?
	Matrix3D(float rotate_z, float rotate_x) { JMP_THIS(0x5AE6F0); }

	// rotation ctor
	Matrix3D(const Vector3D<float>* axis, float angle) { JMP_THIS(0x5AE750); }

	// copy ctor
	Matrix3D(const Matrix3D& another) {
		memcpy(this, &another, sizeof(Matrix3D));
		// JMP_THIS(0x5AE610);
	}

	Matrix3D(Matrix3D&& another) = default;

	Vector4D<float> &operator [] (int i) { return Row[i]; }
	const Vector4D<float> &operator [] (int i) const { return Row[i]; }

	//inline Matrix3D &operator = (const Matrix3D &m)
	//{
	//	Row[0] = m.Row[0];
	//	Row[1] = m.Row[1];
	//	Row[2] = m.Row[2];
	//	return *this;
	//}

	Matrix3D& operator=(Matrix3D&& another) = default;
	Matrix3D &operator=(const Matrix3D& another)
	{
		memcpy(this, &another, sizeof(Matrix3D));
		return *this;
	}

	Matrix3D &operator*(const Matrix3D &another) const
	{
		Matrix3D ret;
		MatrixMultiply(&ret, this, &another);
		return ret;
	}

	void operator*=(const Matrix3D& another)
	{
		MatrixMultiply(this, this, &another);
	}

	Vector3D<float> operator*(const Vector3D<float>& point) const
	{
		Vector3D<float> ret;
		MatrixMultiply(&ret, this, &point);
		return ret;
	}

	// Non virtual
	static Matrix3D* __fastcall TransposeMatrix(Matrix3D* buffer, const Matrix3D* mat) { JMP_STD(0x5AFC20); }

	static Matrix3D TransposeMatrix(const Matrix3D& mat)
	{
		Matrix3D buffer;
		TransposeMatrix(&buffer, &mat);
		return buffer;
	}

	void Transpose()
	{
		*this = TransposeMatrix(*this);
	}

	static Matrix3D* __fastcall FromQuaternion(Matrix3D* mat, const Quaternion* q) { JMP_STD(0x646980); }
	static Matrix3D FromQuaternion(const Quaternion& q)
	{
		Matrix3D buffer;
		FromQuaternion(&buffer, &q);
		return buffer;
	}

	void ApplyQuaternion(const Quaternion& q)
	{
		*this = FromQuaternion(q) * *this;
	}

	inline void Adjust_Translation(const Vector3D<float> &t) { Row[0][3] += t[0]; Row[1][3] += t[1]; Row[2][3] += t[2]; }
	inline void Adjust_X_Translation(float x) { Row[0][3] += x; }
	inline void Adjust_Y_Translation(float y) { Row[1][3] += y; }
	inline void Adjust_Z_Translation(float z) { Row[2][3] += z; }

	Vector3D<float> Get_Translation() const { return Vector3D<float>{ Row[0][3], Row[1][3], Row[2][3] }; }
	void Get_Translation(Vector3D<float> *set) const { set->X = Row[0][3]; set->Y = Row[1][3]; set->Z = Row[2][3]; }
	void Set_Translation(const Vector3D<float> &t) { Row[0][3] = t[0]; Row[1][3] = t[1]; Row[2][3] = t[2]; }

	inline float Get_X_Translation(void) const { return Row[0][3]; }
	inline float Get_Y_Translation(void) const { return Row[1][3]; }
	inline float Get_Z_Translation(void) const { return Row[2][3]; }

	void MakeIdentity() const { JMP_THIS(0x5AE860); } // 1-matrix
	void Translate(float x, float y, float z) const { JMP_THIS(0x5AE890); }
	void Translate(Vector3D<float> const& vec) { JMP_THIS(0x5AE8F0); }
	void TranslateX(float x) { JMP_THIS(0x5AE980); }
	void TranslateY(float y) { JMP_THIS(0x5AE9B0); }
	void TranslateZ(float z) { JMP_THIS(0x5AE9E0); }
	void Scale(float factor) { JMP_THIS(0x5AEA10); }
	void Scale(float x, float y, float z) { JMP_THIS(0x5AEA70); }
	void ScaleX(float factor) { JMP_THIS(0x5AEAD0); }
	void ScaleY(float factor) { JMP_THIS(0x5AEAF0); }
	void ScaleZ(float factor) { JMP_THIS(0x5AEB20); }
	void ShearYZ(float y, float z) { JMP_THIS(0x5AEB50); }
	void ShearXY(float x, float y) { JMP_THIS(0x5AEBA0); }
	void Shear_YZ(float y, float z)
	{
		this->row[0][0] = y * this->row[0][1] + z * this->row[0][2] + this->row[0][0];
		this->row[1][0] = y * this->row[1][1] + z * this->row[1][2] + this->row[1][0];
		this->row[2][0] = y * this->row[2][1] + z * this->row[2][2] + this->row[2][0];
	}

	void Shear_XY(float x, float y)
	{
		this->row[0][2] = x * this->row[0][0] + y * this->row[0][1] + this->row[0][2];
		this->row[1][2] = x * this->row[1][0] + y * this->row[1][1] + this->row[1][2];
		this->row[2][2] = x * this->row[2][0] + y * this->row[2][1] + this->row[2][2];
	}
	void ShearXZ(float x, float z) { JMP_THIS(0x5AEBF0); }
	void PreRotateX(float theta) { JMP_THIS(0x5AEC40); }
	void PreRotateY(float theta) { JMP_THIS(0x5AED50); }
	void PreRotateZ(float theta) { JMP_THIS(0x5AEE50); }
	void RotateX(float theta) { JMP_THIS(0x5AEF60); }
	void RotateX(float Sin, float Cos) { JMP_THIS(0x5AF000); }
	void RotateY(float theta) { JMP_THIS(0x5AF080); }
	void RotateY(float Sin, float Cos) { JMP_THIS(0x5AF120); }
	void RotateZ(float theta) const { JMP_THIS(0x5AF1A0); }
	void RotateZ(float Sin, float Cos) { JMP_THIS(0x5AF240); }
	float GetXVal() { JMP_THIS(0x5AF2C0); }
	float GetYVal() { JMP_THIS(0x5AF310); }
	float GetZVal() { JMP_THIS(0x5AF360); }
	float GetXRotation() { JMP_THIS(0x5AF3B0); }
	float GetYRotation() { JMP_THIS(0x5AF410); }
	float GetZRotation() const { JMP_THIS(0x5AF470); }
	Vector3D<float>* RotateVector(Vector3D<float>* ret, Vector3D<float>* rotate) { JMP_THIS(0x5AF4D0); }
	Vector3D<float> RotateVector(Vector3D<float>& rotate)
	{
		Vector3D<float> buffer;
		RotateVector(&buffer, &rotate);
		return buffer;
	}
	void LookAt1(Vector3D<float>& p, Vector3D<float>& t, float roll) { JMP_THIS(0x5AF550); }
	void LookAt2(Vector3D<float>& p, Vector3D<float>& t, float roll) { JMP_THIS(0x5AF710); }

	static Matrix3D* __fastcall MatrixMultiply(Matrix3D* ret, const Matrix3D* A, const Matrix3D* B) { JMP_STD(0x5AF980); }
	static Matrix3D MatrixMultiply(const Matrix3D& A, const Matrix3D& B)
	{
		Matrix3D buffer;
		MatrixMultiply(&buffer, &A, &B);
		return buffer;
	}

	static Vector3D<float>* __fastcall MatrixMultiply(Vector3D<float>* ret, const Matrix3D* mat, const Vector3D<float>* vec) { JMP_STD(0x5AFB80); }
	static Vector3D<float> MatrixMultiply(const Matrix3D& mat, const Vector3D<float>& vec)
	{
		Vector3D<float> buffer;
		MatrixMultiply(&buffer, &mat, &vec);
		return buffer;
	}

	static Vector3D<float> *Inverse_Rotate_Vector(const Matrix3D &tm, const Vector3D<float> &in)
	{
		Vector3D<float> *v = (Vector3D<float> *) & in;
		Vector3D<float> out = { 0.0f ,0.0f ,0.0f };

		out.X = (tm[0][0] * v->X + tm[1][0] * v->Y + tm[2][0] * v->Z);
		out.Y = (tm[0][1] * v->X + tm[1][1] * v->Y + tm[2][1] * v->Z);
		out.Z = (tm[0][2] * v->X + tm[1][2] * v->Y + tm[2][2] * v->Z);

		return &out;
	}

	static Vector3D<float> *Inverse_Transform_Vector(const Matrix3D &tm, const Vector3D<float> &in)
	{
		Vector3D<float> *v = (Vector3D<float> *) & in;
		Vector3D<float> out = { 0.0f ,0.0f ,0.0f };
		Vector3D<float> diff{ v->X - tm[0][3], v->Y - tm[1][3], v->Z - tm[2][3] };

		return Inverse_Rotate_Vector(tm, diff);
	}

	static Vector3D<float> *Rotate_Vector(const Matrix3D &tm, const Vector3D<float> &in)
	{
		Vector3D<float> *v = (Vector3D<float> *) & in;
		Vector3D<float> out = { 0.0f ,0.0f ,0.0f };

		out.X = (tm[0][0] * v->X + tm[0][1] * v->Y + tm[0][2] * v->Z);
		out.Y = (tm[1][0] * v->X + tm[1][1] * v->Y + tm[1][2] * v->Z);
		out.Z = (tm[2][0] * v->X + tm[2][1] * v->Y + tm[2][2] * v->Z);

		return &out;
	}

	static Vector3D<float> *Transform_Vector(const Matrix3D &tm, const Vector3D<float> &in)
	{
		Vector3D<float> *v = (Vector3D<float> *) & in;
		Vector3D<float> out = { 0.0f ,0.0f ,0.0f };

		out.X = (tm[0][0] * v->X + tm[0][1] * v->Y + tm[0][2] * v->Z + tm[0][3]);
		out.Y = (tm[1][0] * v->X + tm[1][1] * v->Y + tm[1][2] * v->Z + tm[1][3]);
		out.Z = (tm[2][0] * v->X + tm[2][1] * v->Y + tm[2][2] * v->Z + tm[2][3]);

		return &out;
	}

	static Quaternion* __fastcall FromMatrix(Quaternion* ret, Matrix3D* Mtx) { JMP_STD(0x00646730); }
	static Quaternion FromMatrix(Matrix3D& Mtx)
	{
		Quaternion buffer;
		FromMatrix(&buffer, &Mtx);
		return buffer;
	}

	static const Matrix3D& RotateX90()
	{
		return Matrix3D(
			1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, -1.0, 0.0,
			0.0, 1.0, 0.0, 0.0
		);
	};
	static const Matrix3D& Identity()
	{
		return Matrix3D(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0
		);
	};

	static const Matrix3D& RotateX180()
	{
		return Matrix3D(
			1.0, 0.0, 0.0, 0.0,
			0.0, -1.0, 0.0, 0.0,
			0.0, 0.0, -1.0, 0.0
		);
	};

	static const Matrix3D& RotateX270()
	{
		return Matrix3D(
			1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, -1.0, 0.0, 0.0
		);
	};

	static const Matrix3D& RotateY90()
	{
		return Matrix3D(
			0.0, 0.0, 1.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			-1.0, 0.0, 0.0, 0.0
		);
	};

	static const Matrix3D& RotateY180()
	{
		return Matrix3D(
			-1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, -1.0, 0.0
		);
	};

	static const Matrix3D& RotateY270()
	{
		return Matrix3D(
			0.0, 0.0, -1.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			1.0, 0.0, 0.0, 0.0
		);
	};

	static const Matrix3D& RotateZ90()
	{
		return Matrix3D(
			0.0, -1.0, 0.0, 0.0,
			1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0
		);
	};

	static const Matrix3D& RotateZ180()
	{
		return Matrix3D(
			-1.0, 0.0, 0.0, 0.0,
			0.0, -1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0
		);
	};

	static const Matrix3D& RotateZ270()
	{
		return Matrix3D(
			0.0, 1.0, 0.0, 0.0,
			-1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0
		);
	};

	//Properties
public:
	union
	{
		Vector4D<float> Row[3];
		float row[3][4];
		float Data[12];
	};
};

//static_assert(sizeof(Matrix3D) == 0x30u);
