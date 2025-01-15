#pragma once

#include <YRPPCore.h>
#include <GeneralStructures.h>
#include <Helpers/CompileTime.h>
#include <Quaternion.h>

struct CompileTimeMatrix3D
{
	COMPILETIMEEVAL CompileTimeMatrix3D() noexcept
	{
		this->row[0][0] = 1.0;
		this->row[0][1] = 0.0;
		this->row[0][2] = 0.0;
		this->row[0][3] = 0.0;
		this->row[1][0] = 0.0;
		this->row[1][1] = 1.0;
		this->row[1][2] = 0.0;
		this->row[1][3] = 0.0;
		this->row[2][0] = 0.0;
		this->row[2][1] = 0.0;
		this->row[2][2] = 1.0;
		this->row[2][3] = 0.0;
	}

	COMPILETIMEEVAL CompileTimeMatrix3D(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23) noexcept
	{
		row[0][0] = m00; row[0][1] = m01; row[0][2] = m02; row[0][3] = m03;
		row[1][0] = m10; row[1][1] = m11; row[1][2] = m12; row[1][3] = m13;
		row[2][0] = m20; row[2][1] = m21; row[2][2] = m22; row[2][3] = m23;
	}

	COMPILETIMEEVAL CompileTimeMatrix3D(
	Vector3D<float> const& x,
	Vector3D<float> const& y,
	Vector3D<float> const& z,
	Vector3D<float> const& pos) noexcept
	{
		row[0][0] = x.X; row[0][1] = y.X; row[0][2] = z.X; row[0][3] = pos.X;
		row[1][0] = x.Y; row[1][1] = y.Y; row[1][2] = z.Y; row[1][3] = pos.Y;
		row[2][0] = x.Z; row[2][1] = y.Z; row[2][2] = z.Z; row[2][3] = pos.Z;
	}

public:
	union
	{
		Vector4D<float> Row[3];
		float row[3][4];
		float Data[12];
	};
};

class Matrix3D
{
public:
	//static COMPILETIMEEVAL reference<Matrix3D, 0xB44318> VoxelDefaultMatrix {};
	//static COMPILETIMEEVAL reference<Matrix3D, 0xB45188, 21> VoxelRampMatrix {};

	//Constructor

	COMPILETIMEEVAL Matrix3D() noexcept
	{
		this->row[0][0] = 0.0;
		this->row[0][1] = 0.0;
		this->row[0][2] = 0.0;
		this->row[0][3] = 0.0;
		this->row[1][0] = 0.0;
		this->row[1][1] = 0.0;
		this->row[1][2] = 0.0;
		this->row[1][3] = 0.0;
		this->row[2][0] = 0.0;
		this->row[2][1] = 0.0;
		this->row[2][2] = 0.0;
		this->row[2][3] = 0.0;
	}

	// plain floats ctor
	COMPILETIMEEVAL Matrix3D(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23) noexcept
	{
		row[0][0] = m00; row[0][1] = m01; row[0][2] = m02; row[0][3] = m03;
		row[1][0] = m10; row[1][1] = m11; row[1][2] = m12; row[1][3] = m13;
		row[2][0] = m20; row[2][1] = m21; row[2][2] = m22; row[2][3] = m23;
	}

	// column vector ctor
	COMPILETIMEEVAL Matrix3D(
		Vector3D<float> const &x,
		Vector3D<float> const &y,
		Vector3D<float> const &z,
		Vector3D<float> const &pos) noexcept
	{
		row[0][0] = x.X; row[0][1] = y.X; row[0][2] = z.X; row[0][3] = pos.X;
		row[1][0] = x.Y; row[1][1] = y.Y; row[1][2] = z.Y; row[1][3] = pos.Y;
		row[2][0] = x.Z; row[2][1] = y.Z; row[2][2] = z.Z; row[2][3] = pos.Z;
		//JMP_THIS(0x5AE690);
	}

	// some other rotation ctor?
	Matrix3D(float rotate_z, float rotate_x) noexcept
	{
		//JMP_THIS(0x5AE6F0);
		this->row[0][1] = 0.0;
		this->row[0][2] = 0.0;
		this->row[0][3] = 0.0;
		this->row[1][0] = 0.0;
		this->row[1][2] = 0.0;
		this->row[1][3] = 0.0;
		this->row[2][0] = 0.0;
		this->row[2][1] = 0.0;
		this->row[2][3] = 0.0;
		this->row[0][0] = 1.0;
		this->row[1][1] = 1.0;// inlined ctor
		this->row[2][2] = 1.0;
		this->RotateZ(rotate_z);
		this->RotateX(rotate_x);
		float theta = -rotate_z;
		this->RotateZ(theta);
	}

	// rotation ctor
	Matrix3D(const Vector3D<float>* axis, float angle) noexcept
	{
		//JMP_THIS(0x5AE750);
		float c = Math::cos((double)angle);
		double s = Math::sin((double)angle);
		double v7 = axis->X * axis->X;
		this->row[0][0] = static_cast<float>((1.0 - v7) * c + v7);
		double v8 = 1.0 - c;
		this->row[0][1] = static_cast<float>(axis->X * axis->Y * v8 - s * axis->Z);
		double v9 = axis->Z * axis->X * v8;
		double v10 = s * axis->Y;
		this->row[0][3] = 0.0f;
		this->row[0][2] = static_cast<float>(v9 + v10);
		this->row[1][0] = static_cast<float>(axis->X * axis->Y * v8 + s * axis->Z);
		double v11 = axis->Y * axis->Y;
		this->row[1][1] = static_cast<float>((1.0 - v11) * c + v11);
		double v12 = axis->Z * axis->Y * v8;
		double v13 = s * axis->X;
		this->row[1][3] = 0.0f;
		this->row[1][2] = static_cast<float>(v12 - v13);
		this->row[2][0] = static_cast<float>(axis->Z * axis->X * v8 - s * axis->Y);
		this->row[2][1] = static_cast<float>(axis->Z * axis->Y * v8 + s * axis->X);
		double v14 = axis->Z * axis->Z;
		this->row[2][3] = 0.0f;
		this->row[2][2] = static_cast<float>((1.0 - v14) * c + v14);
	}

	static FORCEDINLINE Matrix3D GetIdentity() {
		Matrix3D mtx {};
		mtx.MakeIdentity();
		return mtx;
	}

	// copy ctor
	Matrix3D(const Matrix3D& nAnother) noexcept = default;

	//{
	//	memcpy(this, &nAnother, sizeof(Matrix3D));
		// JMP_THIS(0x5AE610);
	//}

	//Matrix3D(Matrix3D&& nAnother) = default;

	//Vector4D<float> &operator [] (int i) { return Row[i]; }
	//const Vector4D<float> &operator [] (int i) const { return Row[i]; }

	Matrix3D& operator=(Matrix3D&& nAnother)  noexcept = default;
	Matrix3D& operator=(const Matrix3D& nAnother) noexcept = default;

	COMPILETIMEEVAL Matrix3D operator*(const Matrix3D& nAnother) const
	{
		Matrix3D C {}; // [esp+40h] [ebp-30h] BYREF
		const Matrix3D C_another = nAnother;

		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				C.row[i][j] =
				this->row[i][0] * C_another.row[0][j] +
				this->row[i][1] * C_another.row[1][j] +
				this->row[i][2] * C_another.row[2][j];
			}

			C.row[i][3] =
				this->row[i][0] * C_another.row[0][3] +
				this->row[i][1] * C_another.row[1][3] +
				this->row[i][2] * C_another.row[2][3] +
				this->row[i][3];
		}

		return C;
	}

	COMPILETIMEEVAL void operator*=(const Matrix3D& nAnother)
	{
		*this = *this * nAnother;
	}

	COMPILETIMEEVAL Vector3D<float> operator*(const Vector3D<float>& point) const
	{
		return RotateVector(point) + GetTranslation();
	}

	// Non virtual
	//static Matrix3D* __fastcall TransposeMatrix(Matrix3D* buffer, const Matrix3D* mat) { JMP_STD(0x5AFC20); }
	COMPILETIMEEVAL static Matrix3D TransposeMatrix(const Matrix3D& A)
	{
		Matrix3D v7 {}; // [esp+8h] [ebp-30h] BYREF
		//TransposeMatrix(&v7, &A);
		v7.row[0][0] = A.row[0][0];
		double v3 = v7.row[0][0] * A.row[0][3];
		v7.row[0][1] = A.row[1][0];
		v7.row[0][2] = A.row[2][0];
		double v4 = v7.row[0][2] * A.row[2][3];
		v7.row[1][0] = A.row[0][1];
		v7.row[1][1] = A.row[1][1];
		double v5 = v3 + v4;
		double v6 = v7.row[0][1] * A.row[1][3];
		v7.row[1][2] = A.row[2][1];
		v7.row[2][0] = A.row[0][2];
		v7.row[2][1] = A.row[1][2];
		v7.row[2][2] = A.row[2][2];
		v7.row[0][3] = static_cast<float>(-(v5 + v6));
		v7.row[1][3] = static_cast<float>(-(v7.row[1][0] * A.row[0][3] + v7.row[1][2] * A.row[2][3] + v7.row[1][1] * A.row[1][3]));
		v7.row[2][3] = static_cast<float>(-(v7.row[2][0] * A.row[0][3] + v7.row[2][2] * A.row[2][3] + v7.row[2][1] * A.row[1][3]));
		return v7;
	}

	COMPILETIMEEVAL void Transpose()
	{
		*this = TransposeMatrix(*this);
	}

	static Matrix3D* __fastcall FromQuaternion(Matrix3D* mat, const Quaternion* q) { JMP_STD(0x646980); }
	static Matrix3D FromQuaternion(const Quaternion& q)
	{
		//double v2; // st7
		//double v4; // st6
		//double v5; // st5
		//double v6; // st4
		//double v7; // st7
		//double v8; // st6
		//float v9; // [esp+8h] [ebp-40h]
		//float v10; // [esp+Ch] [ebp-3Ch]
		//float v11; // [esp+10h] [ebp-38h]
		//float v12; // [esp+14h] [ebp-34h]
		Matrix3D mtx; // [esp+18h] [ebp-30h] BYREF
		FromQuaternion(&mtx, &q);
		/*v2 = q.Z * q.Z;
		v12 = q.Y * q.Y;
		mtx.row[0][0] = static_cast<float>(1.0 - (v12 + v2 + v12 + v2));
		v4 = q.Y * q.X;
		v5 = q.W * q.Z;
		mtx.row[0][1] = static_cast<float>(v4 - v5 + v4 - v5);
		v10 = q.W * q.Y;
		v6 = q.X * q.Z;
		v9 = static_cast<float>(v6);
		mtx.row[0][2] = static_cast<float>(v6 + v10 + v6 + v10);
		mtx.row[1][0] = static_cast<float>(v5 + v4 + v5 + v4);
		v11 = q.X * q.X;
		mtx.row[1][1] = static_cast<float>(1.0 - (v11 + v2 + v11 + v2));
		v7 = q.Y * q.Z;
		v8 = q.W * q.X;
		mtx.row[1][2] = static_cast<float>(v7 - v8 + v7 - v8);
		mtx.row[2][0] = static_cast<float>(v9 - v10 + v9 - v10);
		mtx.row[2][3] = 0.0f;
		mtx.row[1][3] = 0.0f;
		mtx.row[0][3] = 0.0f;
		mtx.row[2][1] = static_cast<float>(v8 + v7 + v8 + v7);
		mtx.row[2][2] = static_cast<float>(1.0 - (v11 + v12 + v11 + v12));*/
		return mtx;
	}

	void ApplyQuaternion(const Quaternion& q)
	{
		*this = FromQuaternion(q) * *this;
	}

	COMPILETIMEEVAL void AdjustTranslation(const Vector3D<float> &t) { Row[0][3] += t[0]; Row[1][3] += t[1]; Row[2][3] += t[2]; }
	COMPILETIMEEVAL void AdjustXTranslation(float x) { Row[0][3] += x; }
	COMPILETIMEEVAL void AdjustYTranslation(float y) { Row[1][3] += y; }
	COMPILETIMEEVAL void AdjustZTranslation(float z) { Row[2][3] += z; }

	COMPILETIMEEVAL Vector3D<float> GetTranslation() const { return { Row[0][3], Row[1][3], Row[2][3] }; }
	COMPILETIMEEVAL void GetTranslation(Vector3D<float>*set) const { set->X = Row[0][3]; set->Y = Row[1][3]; set->Z = Row[2][3]; }
	COMPILETIMEEVAL void SetTranslation(const Vector3D<float> &t) { Row[0][3] = t[0]; Row[1][3] = t[1]; Row[2][3] = t[2]; }

	COMPILETIMEEVAL float GetXTranslation() const { return Row[0][3]; }
	COMPILETIMEEVAL float GetYTranslation() const { return Row[1][3]; }
	COMPILETIMEEVAL float GetZTranslation() const { return Row[2][3]; }

	COMPILETIMEEVAL void FORCEDINLINE MakeIdentity() {
		this->row[0][0] = 1.0;
		this->row[0][1] = 0.0;
		this->row[0][2] = 0.0;
		this->row[0][3] = 0.0;
		this->row[1][0] = 0.0;
		this->row[1][1] = 1.0;
		this->row[1][2] = 0.0;
		this->row[1][3] = 0.0;
		this->row[2][0] = 0.0;
		this->row[2][1] = 0.0;
		this->row[2][2] = 1.0;
		this->row[2][3] = 0.0;
	} // 1-matrix

	//void Translate(float x, float y, float z) const { JMP_THIS(0x5AE890); }
	COMPILETIMEEVAL void Translate(float x, float y, float z)
	{
		this->row[0][3] = y * this->row[0][1] + z * this->row[0][2] + x * this->row[0][0] + this->row[0][3];
		this->row[1][3] = x * this->row[1][0] + y * this->row[1][1] + z * this->row[1][2] + this->row[1][3];
		this->row[2][3] = x * this->row[2][0] + y * this->row[2][1] + z * this->row[2][2] + this->row[2][3];
	}

	//void Translate(Vector3D<float> const& vec) const { JMP_THIS(0x5AE8F0); }
	COMPILETIMEEVAL void Translate(Vector3D<float> const& t)
	{
		double v3 = t.X;
		double v4 = v3 * this->row[2][0] + this->row[2][3];
		double v5 = t.Y;
		double v6 = v4 + v5 * this->row[2][1];
		double v7 = t.Z;
		float v8 = static_cast<float>(v3 * this->row[1][0] + this->row[1][3]);
		float v9 = static_cast<float>(v5 * this->row[1][1] + v8);
		float a2a = static_cast<float>(v3 * this->row[0][0] + this->row[0][3]);
		float a2b = static_cast<float>(v5 * this->row[0][1] + a2a);

		this->row[0][3] = a2a;
		this->row[1][3] = v8;
		this->row[2][3] = static_cast<float>(v4);
		this->row[0][3] = a2b;
		this->row[1][3] = v9;
		this->row[2][3] = static_cast<float>(v6);
		this->row[0][3] = static_cast<float>(v7 * this->row[0][2] + a2b);
		this->row[1][3] = static_cast<float>(v7 * this->row[1][2] + v9);
		this->row[2][3] = static_cast<float>(v7 * this->row[2][2] + v6);
	}

	COMPILETIMEEVAL void TranslateX(float x) //{ JMP_THIS(0x5AE980); }
	{
		for (int i = 0; i < 3; i++)
			row[i][3] += x * row[i][0];
	}

	COMPILETIMEEVAL void TranslateY(float y) //{ JMP_THIS(0x5AE9B0); }
	{
		for (int i = 0; i < 3; i++)
			row[i][3] += y * row[i][1];
	}

	COMPILETIMEEVAL void TranslateZ(float z) //{ JMP_THIS(0x5AE9E0); }
	{
		for (int i = 0; i < 3; i++)
			row[i][3] += z * row[i][2];
	}

	COMPILETIMEEVAL void Scale(float factor) //{ JMP_THIS(0x5AEA10); }
	{
		for (float& i : Data)
			i *= factor;
	}

	COMPILETIMEEVAL void Scale(float x, float y, float z) //{ JMP_THIS(0x5AEA70); }
	{
		ScaleX(x);
		ScaleY(y);
		ScaleZ(z);
	}

	COMPILETIMEEVAL void ScaleX(float factor) //{ JMP_THIS(0x5AEAD0); }
	{
		for (int i = 0; i < 3; i++)
			row[i][0] *= factor;
	}

	COMPILETIMEEVAL void ScaleY(float factor) //{ JMP_THIS(0x5AEAF0); }
	{
		for (int i = 0; i < 3; i++)
			row[i][1] *= factor;
	}

	COMPILETIMEEVAL void ScaleZ(float factor) //{ JMP_THIS(0x5AEB20); }
	{
		for (int i = 0; i < 3; i++)
			row[i][2] *= factor;
	}

	//void ShearYZ(float y, float z) const { JMP_THIS(0x5AEB50); }
	COMPILETIMEEVAL void ShearYZ(float y, float z)
	{
		this->row[0][0] = y * this->row[0][1] + z * this->row[0][2] + this->row[0][0];
		this->row[1][0] = y * this->row[1][1] + z * this->row[1][2] + this->row[1][0];
		this->row[2][0] = y * this->row[2][1] + z * this->row[2][2] + this->row[2][0];
	}

	//void ShearXY(float x, float y) const { JMP_THIS(0x5AEBA0); }
	COMPILETIMEEVAL void ShearXY(float x, float y)
	{
		this->row[0][2] = y * this->row[0][1] + x * this->row[0][0] + this->row[0][2];
		this->row[1][2] = x * this->row[1][0] + y * this->row[1][1] + this->row[1][2];
		this->row[2][2] = x * this->row[2][0] + y * this->row[2][1] + this->row[2][2];
	}

	//void ShearXZ(float x, float z) const { JMP_THIS(0x5AEBF0); }
	COMPILETIMEEVAL void ShearXZ(float x, float z)
	{
		this->row[0][1] = z * this->row[0][2] + x * this->row[0][0] + this->row[0][1];
		this->row[1][1] = x * this->row[1][0] + z * this->row[1][2] + this->row[1][1];
		this->row[2][1] = x * this->row[2][0] + z * this->row[2][2] + this->row[2][1];
	}

	//void PreRotateX(float theta) const { JMP_THIS(0x5AEC40); }
	void PreRotateX(float theta)
	{
		float s = Math::sin((double)theta);
		double c = (double)Math::cos((double)theta);
		float row_1_1 = this->row[1][1];
		double sn = -s;
		float row_2_0 = this->row[2][0];
		float row_1_2 = this->row[1][2];
		double row_1_0 = this->row[1][0];
		float row_2_1 = this->row[2][1];
		float row_1_3 = this->row[1][3];
		float row_2_2 = this->row[2][2];
		float row_2_3 = this->row[2][3];
		this->row[1][0] = static_cast<float>(row_2_0 * sn + row_1_0 * c);
		this->row[1][1] = static_cast<float>(row_2_1 * sn + row_1_1 * c);
		this->row[1][2] = static_cast<float>(row_2_2 * sn + row_1_2 * c);
		this->row[1][3] = static_cast<float>(row_2_3 * sn + row_1_3 * c);
		this->row[2][0] = static_cast<float>(row_2_0 * c + row_1_0 * s);
		this->row[2][1] = static_cast<float>(row_2_1 * c + row_1_1 * s);
		this->row[2][2] = static_cast<float>(row_2_2 * c + row_1_2 * s);
		this->row[2][3] = static_cast<float>(row_2_3 * c + row_1_3 * s);
	}

	//void PreRotateY(float theta) const { JMP_THIS(0x5AED50); }
	void PreRotateY(float theta)
	{
		float s = Math::sin((double)theta);
		double c = Math::cos((double)theta);
		double sn = -s;
		double row_0_0 = this->row[0][0];
		double row_2_0 = this->row[2][0];
		float row_0_1 = this->row[0][1];
		float row_2_1 = this->row[2][1];
		float row_0_2 = this->row[0][2];
		float row_2_2 = this->row[2][2];
		float row_0_3 = this->row[0][3];
		this->row[0][0] = static_cast<float>(row_2_0 * s + row_0_0 * c);
		float row_2_3 = this->row[2][3];
		this->row[0][1] = static_cast<float>(row_2_1 * s + row_0_1 * c);
		this->row[0][2] = static_cast<float>(row_2_2 * s + row_0_2 * c);
		this->row[0][3] = static_cast<float>(row_2_3 * s + row_0_3 * c);
		this->row[2][0] = static_cast<float>(row_0_0 * sn + row_2_0 * c);
		this->row[2][1] = static_cast<float>(row_0_1 * sn + row_2_1 * c);
		this->row[2][2] = static_cast<float>(row_0_2 * sn + row_2_2 * c);
		this->row[2][3] = static_cast<float>(row_0_3 * sn + row_2_3 * c);
	}

	//void PreRotateZ(float theta) const { JMP_THIS(0x5AEE50); }
	void PreRotateZ(float theta)
	{
		double c = (double)Math::cos((double)theta);
		float s = Math::sin((double)theta);
		double sn = -s;
		double row_0_0 = this->Row[0].X;
		float row_1_0 = this->row[1][0];
		float row_1_1 = this->row[1][1];
		float row_0_1 = this->row[0][1];
		float row_1_2 = this->row[1][2];
		float row_0_2 = this->row[0][2];
		float row_1_3 = this->row[1][3];
		float row_0_3 = this->row[0][3];

		this->row[0][0] = static_cast<float>(row_1_0 * sn + row_0_0 * c);
		this->row[0][1] = static_cast<float>(row_1_1 * sn + row_0_1 * c);
		this->row[0][2] = static_cast<float>(row_1_2 * sn + row_0_2 * c);
		this->row[0][3] = static_cast<float>(row_1_3 * sn + row_0_3 * c);
		this->row[1][0] = static_cast<float>(row_1_0 * c + row_0_0 * s);
		this->row[1][1] = static_cast<float>(row_1_1 * c + row_0_1 * s);
		this->row[1][2] = static_cast<float>(row_1_2 * c + row_0_2 * s);
		this->row[1][3] = static_cast<float>(row_1_3 * c + row_0_3 * s);
	}

	//void RotateX(float theta) const { JMP_THIS(0x5AEF60); }
	void RotateX(float theta) {
		float s = Math::sin((double)theta);
		double c = Math::cos((double)theta);
		double tmp1 = this->row[0][1];
		double tmp2 = this->row[0][2];
		this->row[0][1] = static_cast<float>(tmp1 * c + tmp2 * s);
		this->row[0][2] = static_cast<float>(tmp2 * c - tmp1 * s);
		tmp1 = this->row[1][1];
		tmp2 = this->row[1][2];
		this->row[1][1] = static_cast<float>(tmp1 * c + tmp2 * s);
		this->row[1][2] = static_cast<float>(tmp2 * c - tmp1 * s);
		tmp1 = this->row[2][1];
		tmp2 = this->row[2][2];
		this->row[2][1] = static_cast<float>(tmp1 * c + tmp2 * s);
		this->row[2][2] = static_cast<float>(tmp2 * c - tmp1 * s);
	}

	//void RotateX(float Sin, float Cos) const { JMP_THIS(0x5AF000); }
	void RotateX(float s, float c)
	{
		double tmp1 = this->row[0][1];
		double tmp2 = this->row[0][2];
		this->row[0][1] = static_cast<float>(tmp2 * s + tmp1 * c);
		this->row[0][2] = static_cast<float>(tmp2 * c - tmp1 * s);
		tmp1 = this->row[1][1];
		tmp2 = this->row[1][2];
		this->row[1][1] = static_cast<float>(tmp2 * s + tmp1 * c);
		this->row[1][2] = static_cast<float>(tmp2 * c - tmp1 * s);
		tmp1 = this->row[2][1];
		tmp2 = this->row[2][2];
		this->row[2][1] = static_cast<float>(tmp2 * s + tmp1 * c);
		this->row[2][2] = static_cast<float>(tmp2 * c - tmp1 * s);
	}

	//void RotateY(float theta) const { JMP_THIS(0x5AF080); }
	void RotateY(float theta)
	{
		double c = (double)Math::cos((double)theta);
		double tmp1 = this->row[0][0];
		double tmp2 = this->row[0][2];
		float s = Math::sin((double)theta);
		this->row[0][0] = static_cast<float>(tmp1 * c - tmp2 * s);
		this->row[0][2] = static_cast<float>(tmp2 * c + tmp1 * s);
		tmp1 = this->row[1][0];
		tmp2 = this->row[1][2];
		this->row[1][0] = static_cast<float>(tmp1 * c - tmp2 * s);
		this->row[1][2] = static_cast<float>(tmp2 * c + tmp1 * s);
		tmp1 = this->row[2][0];
		tmp2 = this->row[2][2];
		this->row[2][0] = static_cast<float>(tmp1 * c - tmp2 * s);
		this->row[2][2] = static_cast<float>(tmp2 * c + tmp1 * s);
	}

	//void RotateY(float Sin, float Cos) const { JMP_THIS(0x5AF120); }
	void RotateY(float s, float c)
	{
		double tmp1 = this->row[0][0];
		double tmp2 = this->row[0][2];
		this->row[0][0] = static_cast<float>(tmp1 * c - tmp2 * s);
		this->row[0][2] = static_cast<float>(tmp1 * s + tmp2 * c);
		tmp1 = this->row[1][0];
		tmp2 = this->row[1][2];
		this->row[1][0] = static_cast<float>(tmp1 * c - tmp2 * s);
		this->row[1][2] = static_cast<float>(tmp1 * s + tmp2 * c);
		tmp1 = this->row[2][0];
		tmp2 = this->row[2][2];
		this->row[2][0] = static_cast<float>(tmp1 * c - tmp2 * s);
		this->row[2][2] = static_cast<float>(tmp1 * s + tmp2 * c);
	}

	//void RotateZ(float theta) const { JMP_THIS(0x5AF1A0); }
	void RotateZ(float theta) {
		float c = Math::cos((double)theta);
		double s = (double)Math::sin((double)theta);
		double tmp1 = this->row[0][0];
		double tmp2 = this->row[0][1];
		this->row[0][0] = static_cast<float>(tmp2 * s + tmp1 * c);
		this->row[0][1] = static_cast<float>(tmp2 * c - tmp1 * s);
		tmp1 = this->row[1][0];
		tmp2 = this->row[1][1];
		this->row[1][0] = static_cast<float>(tmp2 * s + tmp1 * c);
		this->row[1][1] = static_cast<float>(tmp2 * c - tmp1 * s);
		tmp1 = this->row[2][0];
		tmp2 = this->row[2][1];
		this->row[2][0] = static_cast<float>(tmp2 * s + tmp1 * c);
		this->row[2][1] = static_cast<float>(tmp2 * c - tmp1 * s);
	}

	//void RotateZ(float Sin, float Cos) const { JMP_THIS(0x5AF240); }
	void RotateZ(float s, float c)
	{
		double tmp1 = this->row[0][0];
		double tmp2 = this->row[0][1];
		this->row[0][0] = static_cast<float>(tmp2 * s + tmp1 * c);
		this->row[0][1] = static_cast<float>(tmp2 * c - tmp1 * s);
		tmp1 = this->row[1][0];
		tmp2 = this->row[1][1];
		this->row[1][0] = static_cast<float>(tmp2 * s + tmp1 * c);
		this->row[1][1] = static_cast<float>(tmp2 * c - tmp1 * s);
		tmp1 = this->row[2][0];
		tmp2 = this->row[2][1];
		this->row[2][0] = static_cast<float>(tmp2 * s + tmp1 * c);
		this->row[2][1] = static_cast<float>(tmp2 * c - tmp1 * s);
	}

	COMPILETIMEEVAL float GetXVal() //{ JMP_THIS(0x5AF2C0); }
	{
		Vector3D<float> ret_ {};
		MatrixMultiply(&ret_, this, &Vector3D<float>::Empty);
		return ret_.X;
	}

	//float GetYVal() const  { JMP_THIS(0x5AF310); }
	COMPILETIMEEVAL float GetYVal()
	{
		Vector3D<float> ret_ {};
		MatrixMultiply(&ret_, this, &Vector3D<float>::Empty);
		return ret_.Y;
	}

	//float GetZVal() const { JMP_THIS(0x5AF360); }
	COMPILETIMEEVAL float GetZVal()
	{
		Vector3D<float> ret_ {};
		MatrixMultiply(&ret_, this, &Vector3D<float>::Empty);
		return ret_.Z;
	}

	//float GetXRotation() const  { JMP_THIS(0x5AF3B0); }
	float GetXRotation() {
		Vector3D<float> rot_ { 0.0f , 1.0f , 0.0f };
		Vector3D<float> ret_ {};
		MatrixMultiply(&ret_, this, &rot_);
		return (float)Math::atan2((double)ret_.Z , (double)ret_.Y);
	}

	//float GetYRotation() const  { JMP_THIS(0x5AF410); }
	float GetYRotation() {
		Vector3D<float> rot_ { 0.0f , 0.0f , 1.0f };
		Vector3D<float> ret_ {};
		MatrixMultiply(&ret_, this, &rot_);
		return (float)Math::atan2((double)ret_.X, (double)ret_.Z);
	}

	//float GetZRotation() const { JMP_THIS(0x5AF470); }
	float GetZRotation()
	{
		Vector3D<float> rot_ { 1.0f , 0.0f , 0.0f };
		Vector3D<float> ret_ {};
		MatrixMultiply(&ret_, this, &rot_);
		return (float)Math::atan2((double)ret_.Y, (double)ret_.X);
	}

	//Vector3D<float>* __RotateVector(Vector3D<float>* ret, Vector3D<float>* rotate) const { JMP_THIS(0x5AF4D0); }
	COMPILETIMEEVAL Vector3D<float> RotateVector(const Vector3D<float>& rotate) const {
		return {
				row[0][0] * rotate.X + row[0][1] * rotate.Y + row[0][2] * rotate.Z,
				row[1][0] * rotate.X + row[1][1] * rotate.Y + row[1][2] * rotate.Z,
				row[2][0] * rotate.X + row[2][1] * rotate.Y + row[2][2] * rotate.Z,
		};
	}

	void LookAt1(Vector3D<float>& p, Vector3D<float>& t, float roll) { JMP_THIS(0x5AF550); }
	void LookAt2(Vector3D<float>& p, Vector3D<float>& t, float roll) { JMP_THIS(0x5AF710); }

	//static Matrix3D* __fastcall MatrixMultiply__(Matrix3D* ret, const Matrix3D* A, const Matrix3D* B) { JMP_STD(0x5AF980); }
	COMPILETIMEEVAL static Matrix3D MatrixMultiply__(const Matrix3D& A, const Matrix3D& B)
	{
		//Matrix3D buffer;
		//MatrixMultiply__(&buffer, &A, &B);
		return A * B;
	}

	COMPILETIMEEVAL static Vector3D<float>* //__fastcall
			MatrixMultiply(Vector3D<float>* vecret, const Matrix3D* mat, const Vector3D<float>* vec) {
		//JMP_FAST(0x5AFB80);
   		 vecret->X = mat->row[0][2] * vec->Z + mat->row[0][1] * vec->Y + mat->Row[0][0] * vec->X + mat->row[0][3];
    	 vecret->Y = mat->row[1][0] * vec->X + mat->row[1][2] * vec->Z + mat->row[1][1] * vec->Y + mat->row[1][3];
   		 return vecret;
	}

	COMPILETIMEEVAL static Vector3D<float> InverseRotateVector(const Matrix3D &tm, const Vector3D<float> &in)
	{
		return {
			(tm.row[0][0] * in.X + tm.row[1][0] * in.Y + tm.row[2][0] * in.Z) ,
			(tm.row[0][1] * in.X + tm.row[1][1] * in.Y + tm.row[2][1] * in.Z) ,
			(tm.row[0][2] * in.X + tm.row[1][2] * in.Y + tm.row[2][2] * in.Z)
		};
	}

	COMPILETIMEEVAL static Vector3D<float> InverseTransformVector(const Matrix3D &tm, const Vector3D<float> &in)
	{
		Vector3D<float> diff{ in.X - tm.row[0][3],  in.Y - tm.row[1][3], in.Z - tm.row[2][3] };
		return InverseRotateVector(tm, diff);
	}

	COMPILETIMEEVAL static Vector3D<float> TransformVector(const Matrix3D &tm, const Vector3D<float> &in)
	{
		return {
			(tm.row[0][0] * in.X + tm.row[0][1] * in.Y + tm.row[0][2] * in.Z + tm.row[0][3]) ,
			(tm.row[1][0] * in.X + tm.row[1][1] * in.Y + tm.row[1][2] * in.Z + tm.row[1][3]) ,
			(tm.row[2][0] * in.X + tm.row[2][1] * in.Y + tm.row[2][2] * in.Z + tm.row[2][3])
		};
	}

	static Quaternion* __fastcall FromMatrix(Quaternion* ret, Matrix3D* Mtx) { JMP_STD(0x00646730); }
	// Idk : Otamaa
	static Quaternion FromMatrix(Matrix3D& Mtx)
	{
		Quaternion buffer;
		FromMatrix(&buffer, &Mtx);
		return buffer;
	}

//Properties
public:
	union
	{
		Vector4D<float> Row[3];
		float row[3][4];
		float Data[12] = {};
	};
};

static_assert(sizeof(Matrix3D) == 0x30u);
