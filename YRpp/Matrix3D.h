#pragma once

#include <YRPPCore.h>
#include <GeneralStructures.h>
#include <Helpers/CompileTime.h>
#include <Quaternion.h>

struct CompileTimeMatrix3D
{
	constexpr CompileTimeMatrix3D() noexcept
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

	constexpr CompileTimeMatrix3D(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23) noexcept
	{
		row[0][0] = m00; row[0][1] = m01; row[0][2] = m02; row[0][3] = m03;
		row[1][0] = m10; row[1][1] = m11; row[1][2] = m12; row[1][3] = m13;
		row[2][0] = m20; row[2][1] = m21; row[2][2] = m22; row[2][3] = m23;
	}

	constexpr CompileTimeMatrix3D(
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
	//static constexpr reference<Matrix3D, 0xB44318> VoxelDefaultMatrix {};
	//static constexpr reference<Matrix3D, 0xB45188, 21> VoxelRampMatrix {};

	//Constructor

	Matrix3D() noexcept
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
	Matrix3D(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23) noexcept
	{
		row[0][0] = m00; row[0][1] = m01; row[0][2] = m02; row[0][3] = m03;
		row[1][0] = m10; row[1][1] = m11; row[1][2] = m12; row[1][3] = m13;
		row[2][0] = m20; row[2][1] = m21; row[2][2] = m22; row[2][3] = m23;
	}

	// column vector ctor
	Matrix3D(
		Vector3D<float> const &x,
		Vector3D<float> const &y,
		Vector3D<float> const &z,
		Vector3D<float> const &pos) noexcept
	{
		row[0][0] = x.X; row[0][1] = y.X; row[0][2] = z.X; row[0][3] = pos.X;
		row[1][0] = x.Y; row[1][1] = y.Y; row[1][2] = z.Y; row[1][3] = pos.Y;
		row[2][0] = x.Z; row[2][1] = y.Z; row[2][2] = z.Z; row[2][3] = pos.Z;
		// JMP_THIS(0x5AE690);
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
		double v3 = angle;
		float c = Math::cos(angle);
		double s = Math::sin(v3);
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

	static Matrix3D GetIdentity()
	{
		Matrix3D mtx { };
		mtx.MakeIdentity();
		return mtx;
	}

	// copy ctor
	Matrix3D(const Matrix3D& nAnother) {
		memcpy(this, &nAnother, sizeof(Matrix3D));
		// JMP_THIS(0x5AE610);
	}

	//Matrix3D(Matrix3D&& nAnother) = default;

	//Vector4D<float> &operator [] (int i) { return Row[i]; }
	//const Vector4D<float> &operator [] (int i) const { return Row[i]; }

	Matrix3D& operator=(Matrix3D&& nAnother) = default;
	Matrix3D& operator=(const Matrix3D& nAnother)
	{
		memcpy(this, &nAnother, sizeof(Matrix3D));
		return *this;
	}

	Matrix3D operator*(const Matrix3D& nAnother) const
	{
		Matrix3D ret;
		MatrixMultiply(&ret, this, &nAnother);
		return ret;
	}

	void operator*=(const Matrix3D& nAnother)
	{
		MatrixMultiply(this, this, &nAnother);
	}

	Vector3D<float> operator*(const Vector3D<float>& point) const
	{
		Vector3D<float> ret;
		MatrixMultiply(&ret, this, &point);
		return ret;
	}

	// Non virtual
	static Matrix3D* __fastcall TransposeMatrix(Matrix3D* buffer, const Matrix3D* mat) { JMP_STD(0x5AFC20); }
	static Matrix3D TransposeMatrix(const Matrix3D& A)
	{
		Matrix3D v7; // [esp+8h] [ebp-30h] BYREF

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

	void Transpose()
	{
		*this = TransposeMatrix(*this);
	}

	static Matrix3D* __fastcall FromQuaternion(Matrix3D* mat, const Quaternion* q) { JMP_STD(0x646980); }
	static Matrix3D FromQuaternion(const Quaternion& q)
	{
		double v2; // st7
		double v4; // st6
		double v5; // st5
		double v6; // st4
		double v7; // st7
		double v8; // st6
		float v9; // [esp+8h] [ebp-40h]
		float v10; // [esp+Ch] [ebp-3Ch]
		float v11; // [esp+10h] [ebp-38h]
		float v12; // [esp+14h] [ebp-34h]
		Matrix3D mtx; // [esp+18h] [ebp-30h] BYREF

		v2 = q.Z * q.Z;
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
		mtx.row[2][2] = static_cast<float>(1.0 - (v11 + v12 + v11 + v12));
		return mtx;
	}

	void ApplyQuaternion(const Quaternion& q)
	{
		*this = FromQuaternion(q) * *this;
	}

	void AdjustTranslation(const Vector3D<float> &t) { Row[0][3] += t[0]; Row[1][3] += t[1]; Row[2][3] += t[2]; }
	void AdjustXTranslation(float x) { Row[0][3] += x; }
	void AdjustYTranslation(float y) { Row[1][3] += y; }
	void AdjustZTranslation(float z) { Row[2][3] += z; }

	Vector3D<float> GetTranslation() const { return { Row[0][3], Row[1][3], Row[2][3] }; }
	void GetTranslation(Vector3D<float>*set) const { set->X = Row[0][3]; set->Y = Row[1][3]; set->Z = Row[2][3]; }
	void SetTranslation(const Vector3D<float> &t) { Row[0][3] = t[0]; Row[1][3] = t[1]; Row[2][3] = t[2]; }

	float GetXTranslation() const { return Row[0][3]; }
	float GetYTranslation() const { return Row[1][3]; }
	float GetZTranslation() const { return Row[2][3]; }

	void MakeIdentity() {

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
	void Translate(float x, float y, float z)
	{
		this->row[0][3] = y * this->row[0][1] + z * this->row[0][2] + x * this->row[0][0] + this->row[0][3];
		this->row[1][3] = x * this->row[1][0] + y * this->row[1][1] + z * this->row[1][2] + this->row[1][3];
		this->row[2][3] = x * this->row[2][0] + y * this->row[2][1] + z * this->row[2][2] + this->row[2][3];
	}

	//void Translate(Vector3D<float> const& vec) const { JMP_THIS(0x5AE8F0); }
	void Translate(Vector3D<float> const& t)
	{
		double v3; // st7
		double v4; // st7
		double v5; // st6
		double v6; // st7
		double v7; // st6
		float v8; // [esp+0h] [ebp-4h]
		float v9; // [esp+0h] [ebp-4h]
		float a2a; // [esp+8h] [ebp+4h]
		float a2b; // [esp+8h] [ebp+4h]

		v3 = t.X;
		a2a = static_cast<float>(v3 * this->row[0][0] + this->row[0][3]);
		this->row[0][3] = a2a;
		v8 = static_cast<float>(v3 * this->row[1][0] + this->row[1][3]);
		this->row[1][3] = v8;
		v4 = v3 * this->row[2][0] + this->row[2][3];
		this->row[2][3] = static_cast<float>(v4);
		v5 = t.Y;
		a2b = static_cast<float>(v5 * this->row[0][1] + a2a);
		this->row[0][3] = a2b;
		v9 = static_cast<float>(v5 * this->row[1][1] + v8);
		this->row[1][3] = v9;
		v6 = v4 + v5 * this->row[2][1];
		this->row[2][3] = static_cast<float>(v6);
		v7 = t.Z;
		this->row[0][3] = static_cast<float>(v7 * this->row[0][2] + a2b);
		this->row[1][3] = static_cast<float>(v7 * this->row[1][2] + v9);
		this->row[2][3] = static_cast<float>(v7 * this->row[2][2] + v6);
	}

	//void TranslateX(float x) const { JMP_THIS(0x5AE980); }
	void TranslateX(float x)
	{
		this->row[0][3] = x * this->row[0][0] + this->Row[0].W;
		this->row[1][3] = x * this->Row[1].X + this->Row[1].W;
		this->row[2][3] = x * this->Row[2].X + this->Row[2].W;
	}

	//void TranslateY(float y) const { JMP_THIS(0x5AE9B0); }
	void TranslateY(float y)
	{
		this->row[0][3] = y * this->row[0][1] + this->Row[0].W;
		this->row[1][3] = y * this->row[1][1] + this->Row[1].W;
		this->row[2][3] = y * this->row[2][1] + this->Row[2].W;
	}

	//void TranslateZ(float z) const { JMP_THIS(0x5AE9E0); }
	void TranslateZ(float z)
	{
		this->row[0][3] = z * this->row[0][2] + this->row[0][3];
		this->row[1][3] = z * this->row[1][2] + this->row[1][3];
		this->row[2][3] = z * this->row[2][2] + this->row[2][3];
	}

	//void Scale(float factor) const { JMP_THIS(0x5AEA10); }
	void Scale(float factor)
	{
		this->row[0][0] = factor * this->row[0][0];
		this->row[1][0] = factor * this->row[1][0];
		this->row[2][0] = factor * this->row[2][0];
		this->row[0][1] = factor * this->row[0][1];
		this->row[1][1] = factor * this->row[1][1];
		this->row[2][1] = factor * this->row[2][1];
		this->row[0][2] = factor * this->row[0][2];
		this->row[1][2] = factor * this->row[1][2];
		this->row[2][2] = factor * this->row[2][2];
	}

	//void Scale(float x, float y, float z) const { JMP_THIS(0x5AEA70); }
	void Scale(float x, float y, float z)
	{
		this->row[0][0] = x * this->row[0][0];
		this->row[1][0] = x * this->Row[1].X;
		this->row[2][0] = x * this->row[2][0];
		this->row[0][1] = y * this->row[0][1];
		this->row[1][1] = y * this->row[1][1];
		this->row[2][1] = y * this->row[2][1];
		this->row[0][2] = z * this->row[0][2];
		this->row[1][2] = z * this->row[1][2];
		this->row[2][2] = z * this->row[2][2];
	}

	//void ScaleX(float factor) const { JMP_THIS(0x5AEAD0); }
	void ScaleX(float factor)
	{
		this->row[0][0] = factor * this->row[0][0];
		this->row[1][0] = factor * this->row[1][0];
		this->row[2][0] = factor * this->row[2][0];
	}

	//void ScaleY(float factor) const { JMP_THIS(0x5AEAF0); }
	void ScaleY(float factor)
	{
		this->row[0][1] = factor * this->row[0][1];
		this->row[1][1] = factor * this->row[1][1];
		this->row[2][1] = factor * this->row[2][1];
	}

	//void ScaleZ(float factor) const { JMP_THIS(0x5AEB20); }
	void ScaleZ(float factor)
	{
		this->row[0][2] = factor * this->Row[0].Z;
		this->row[1][2] = factor * this->Row[1].Z;
		this->row[2][2] = factor * this->Row[2].Z;
	}

	//void ShearYZ(float y, float z) const { JMP_THIS(0x5AEB50); }
	void ShearYZ(float y, float z)
	{
		this->row[0][0] = y * this->row[0][1] + z * this->row[0][2] + this->row[0][0];
		this->row[1][0] = y * this->row[1][1] + z * this->row[1][2] + this->row[1][0];
		this->row[2][0] = y * this->row[2][1] + z * this->row[2][2] + this->row[2][0];
	}

	//void ShearXY(float x, float y) const { JMP_THIS(0x5AEBA0); }
	void ShearXY(float x, float y)
	{
		this->row[0][2] = y * this->row[0][1] + x * this->row[0][0] + this->row[0][2];
		this->row[1][2] = x * this->row[1][0] + y * this->row[1][1] + this->row[1][2];
		this->row[2][2] = x * this->row[2][0] + y * this->row[2][1] + this->row[2][2];
	}

	//void ShearXZ(float x, float z) const { JMP_THIS(0x5AEBF0); }
	void ShearXZ(float x, float z)
	{
		this->row[0][1] = z * this->row[0][2] + x * this->row[0][0] + this->row[0][1];
		this->row[1][1] = x * this->row[1][0] + z * this->row[1][2] + this->row[1][1];
		this->row[2][1] = x * this->row[2][0] + z * this->row[2][2] + this->row[2][1];
	}

	//void PreRotateX(float theta) const { JMP_THIS(0x5AEC40); }
	void PreRotateX(float theta)
	{
		double v2 = theta;
		float s = Math::sin(theta);
		double c = Math::cos(v2);
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
		float s = Math::sin(theta);
		double c = Math::cos(theta);
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
		double v2; // kr00_8
		double c; // st7
		double sn; // st6
		double row_0_0; // st4
		float row_1_0; // [esp+0h] [ebp-24h]
		float row_1_1; // [esp+8h] [ebp-1Ch]
		float row_0_1; // [esp+Ch] [ebp-18h]
		float row_1_2; // [esp+10h] [ebp-14h]
		float row_0_2; // [esp+14h] [ebp-10h]
		float row_1_3; // [esp+18h] [ebp-Ch]
		float row_0_3; // [esp+1Ch] [ebp-8h]
		float s; // [esp+28h] [ebp+4h]

		v2 = theta;
		s = Math::sin(theta);
		c = Math::cos(v2);
		row_0_1 = this->row[0][1];
		sn = -s;
		row_1_0 = this->row[1][0];
		row_0_2 = this->row[0][2];
		row_0_0 = this->Row[0].X;
		row_1_1 = this->row[1][1];
		row_0_3 = this->row[0][3];
		row_1_2 = this->row[1][2];
		row_1_3 = this->row[1][3];
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
		double v2 = theta;
		float s = Math::sin(theta);
		double c = Math::cos(v2);
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
		double v2; // kr00_8
		double c; // st7
		double tmp1; // st6 MAPDST
		double tmp2; // st5 MAPDST
		float s; // [esp+Ch] [ebp+4h]

		v2 = theta;
		s = Math::sin(theta);
		c = Math::cos(v2);
		tmp1 = this->row[0][0];
		tmp2 = this->row[0][2];
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
		double v2 = theta;
		float c = Math::cos(theta);
		double s = Math::sin(v2);
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
		double tmp1; // st7 MAPDST
		double tmp2; // st6 MAPDST

		tmp1 = this->row[0][0];
		tmp2 = this->row[0][1];
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

	//float GetXVal() const { JMP_THIS(0x5AF2C0); }
	float GetXVal() {
		Vector3D<float> ret_ = MatrixMultiply(this, Vector3D<float>::Empty);
		return ret_.X;
	}

	//float GetYVal() const  { JMP_THIS(0x5AF310); }
	float GetYVal()
	{
		Vector3D<float> ret_ = MatrixMultiply(this, Vector3D<float>::Empty);
		return ret_.Y;
	}

	//float GetZVal() const { JMP_THIS(0x5AF360); }
	float GetZVal()
	{
		Vector3D<float> ret_ = MatrixMultiply(this, Vector3D<float>::Empty);
		return ret_.Z;
	}

	//float GetXRotation() const  { JMP_THIS(0x5AF3B0); }
	float GetXRotation() {
		Vector3D<float> ret_ = MatrixMultiply(this, { 0.0f , 1.0f , 0.0f });
		return (float)Math::atan2((double)ret_.Z , (double)ret_.Y);
	}

	//float GetYRotation() const  { JMP_THIS(0x5AF410); }
	float GetYRotation() {
		Vector3D<float> ret_ = MatrixMultiply(this, { 0.0f , 0.0f , 1.0f });
		return (float)Math::atan2((double)ret_.X, (double)ret_.Z);
	}

	//float GetZRotation() const { JMP_THIS(0x5AF470); }
	float GetZRotation()
	{
		Vector3D<float> ret_ = MatrixMultiply(this, { 1.0f , 0.0f , 0.0f });
		return (float)Math::atan2((double)ret_.Y, (double)ret_.X);
	}

	Vector3D<float>* RotateVector(Vector3D<float>* ret, Vector3D<float>* rotate) const { JMP_THIS(0x5AF4D0); }
	Vector3D<float> RotateVector(const Vector3D<float>& rotate)
	{
		return  {
			this->Row[0].Y* rotate.Y + this->Row[0].Z * rotate.Z + rotate.X * this->Row[0].X ,
			this->Row[1].Y* rotate.Y + this->Row[1].X * rotate.X + this->Row[1].Z * rotate.Z ,
			this->Row[2].Y* rotate.Y + this->Row[2].X * rotate.X + this->Row[2].Z * rotate.Z
		};
	}

	void LookAt1(Vector3D<float>* p, Vector3D<float>* t, float roll) const { JMP_THIS(0x5AF550) };
	void LookAt1(const Vector3D<float>& p, const Vector3D<float>& t, float roll)
	{
		double dz; // st7
		double len2; // st7
		double v9; // st7
		double v10; // st7
		float sinp_neg; // [esp+0h] [ebp-28h]
		float v12; // [esp+10h] [ebp-18h]
		float _dx; // [esp+10h] [ebp-18h]
		float v14; // [esp+14h] [ebp-14h]
		float v15; // [esp+14h] [ebp-14h]
		float v16; // [esp+14h] [ebp-14h]
		float cosp; // [esp+18h] [ebp-10h]
		float v18; // [esp+18h] [ebp-10h]
		float siny; // [esp+1Ch] [ebp-Ch]
		float v20; // [esp+1Ch] [ebp-Ch]
		float sinp; // [esp+20h] [ebp-8h]
		float v22; // [esp+24h] [ebp-4h]
		float len1; // [esp+2Ch] [ebp+4h]
		float cosy; // [esp+2Ch] [ebp+4h]
		float dy; // [esp+30h] [ebp+8h]

		v12 = t.X - p.X;
		dy = t.Y - p.Y;
		dz = t.Z - p.Z;
		v14 = static_cast<float>(dz);
		siny = static_cast<float>(dz * v14);
		cosp = v12 * v12;
		len1 = Math::sqrt(dy * dy + cosp + siny);
		len2 = Math::sqrt(cosp + siny);
		if (len1 == 0.0f)
		{
			sinp = 0.0f;
			v18 = 1.0f;
		}
		else
		{
			sinp = static_cast<float>(dy / len1);
			v18 = static_cast<float>(len2 / len1);
		}
		if (len2 == 0.0)
		{
			v20 = 0.0f;
			cosy = 1.0f;
		}
		else
		{
			v20 = static_cast<float>(v12 / len2);
			cosy = static_cast<float>(v14 / len2);
		}

		this->Row[0].X = 1.0;// Make_Identity
		this->Row[0].Y = 0.0;
		this->Row[0].Z = 0.0;
		this->Row[0].W = 0.0;
		this->Row[1].X = 0.0;
		this->Row[1].Y = 1.0;
		this->Row[1].Z = 0.0;
		this->Row[1].W = 0.0;
		this->Row[2].X = 0.0;
		this->Row[2].Y = 0.0;
		this->Row[2].Z = 1.0;
		this->Row[2].W = 0.0;
		v9 = p.X;
		this->Row[0].W = p.X;
		this->Row[1].W = 0.0;// Translate
		this->Row[2].W = 0.0;
		v10 = v9 + 0.0;
		v15 = p.Y;
		this->Row[0].W = static_cast<float>(v10);
		v16 = v15 + this->Row[1].W;
		this->Row[1].W = v16;
		_dx = 0.0 + 0.0;
		this->Row[2].W = _dx;
		v22 = p.Z;
		this->Row[0].W = static_cast<float>(v10 + 0.0);
		this->Row[1].W = static_cast<float>(v16 + 0.0);
		this->Row[2].W = _dx + v22;
		this->RotateY(v20, cosy);
		sinp_neg = -sinp;
		this->RotateX(sinp_neg, v18);
		this->RotateZ(roll);
	}

	void LookAt2(Vector3D<float>* p, Vector3D<float>* t, float roll) const { JMP_THIS(0x5AF710); }
	void LookAt2(const Vector3D<float>& p, const Vector3D<float>& t, float roll)
	{
		double v5; // st7
		double x; // st7
		double rolls; // st7
		double v8; // st6
		double len; // st7
		float v11; // edx
		float v12; // edx
		float yaw; // [esp+20h] [ebp-50h]
		float yaws; // [esp+20h] [ebp-50h]
		float yawc; // [esp+24h] [ebp-4Ch]
		float pitch; // [esp+28h] [ebp-48h]
		float v17; // [esp+28h] [ebp-48h]
		float v18; // [esp+28h] [ebp-48h]
		float v19; // [esp+2Ch] [ebp-44h]
		float v20; // [esp+30h] [ebp-40h]
		float v21; // [esp+3Ch] [ebp-34h]
		float v22; // [esp+40h] [ebp-30h]
		float pitchs; // [esp+44h] [ebp-2Ch]
		float v24; // [esp+48h] [ebp-28h]
		float v27; // [esp+50h] [ebp-20h]
		float _dx; // [esp+58h] [ebp-18h] MAPDST
		float _dy; // [esp+5Ch] [ebp-14h] MAPDST
		float _dy_4; // [esp+5Ch] [ebp-14h]
		float v31; // [esp+60h] [ebp-10h] MAPDST
		float _dz; // [esp+60h] [ebp-10h]
		float v33; // [esp+64h] [ebp-Ch]
		float v34; // [esp+68h] [ebp-8h]
		float v35; // [esp+6Ch] [ebp-4h]

		_dx = t.X - p.X;
		_dy = t.Y - p.Y;
		v31 = t.Z - p.Z;
		if (_dy == 0.0 && _dx == 0.0)
		{
			yaw = 0.0;
		}
		else
		{
			yaw = Math::atan2(_dy, _dx);
		}
		v22 = _dy * _dy;
		v5 = _dx * _dx;
		v21 = static_cast<float>(v5);
		x = Math::sqrt(v5 + v22);
		pitch = Math::atan2(-v31, static_cast<float>(x));
		yawc = Math::cos(yaw);
		yaws = Math::sin(yaw);
		v24 = Math::cos(pitch);
		pitchs = Math::sin(pitch);
		v17 = Math::cos(roll);
		rolls = Math::sin(roll);
		v8 = v17 * pitchs;
		v33 = static_cast<float>(v8 * yawc + rolls * yaws);
		v34 = static_cast<float>(v8 * yaws - rolls * yawc);
		v35 = v17 * v24;
		len = Math::sqrt(v31 * v31 + v21 + v22);
		if (len != 0.0)
		{
			v18 = static_cast<float>(_dx / len);
			_dx = v18;
			v19 = static_cast<float>(_dy / len);
			_dy = v19;
			v20 = static_cast<float>(v31 / len);
			v31 = v20;
		}
		v27 = v31;
		this->row[0][1] = v33;
		this->row[0][2] = _dx;
		_dy_4 = v31 * v33 - _dx * v35;
		_dz = _dx * v34 - _dy * v33;
		this->row[0][0] = _dy * v35 - v31 * v34;
		v11 = p.X;
		this->row[1][0] = _dy_4;
		this->row[0][3] = v11;
		this->row[1][2] = _dy;
		this->row[1][1] = v34;
		v12 = p.Y;
		this->row[2][0] = _dz;
		this->row[1][3] = v12;
		this->row[2][2] = v27;
		this->row[2][1] = v35;
		this->row[2][3] = p.Z;
	}

	static Matrix3D* __fastcall MatrixMultiply(Matrix3D* ret, const Matrix3D* A, const Matrix3D* B) { JMP_STD(0x5AF980); }
	static Matrix3D MatrixMultiply(const Matrix3D& A, const Matrix3D& B)
	{
		double a_row_00; // st7
		double a_row_01; // st6
		double a_row_02; // st5
		double b_row_00; // st4
		double b_row_10; // st3
		double b_row_20; // st2
		float b_row_13; // esi
		float a_row_12; // [esp+8h] [ebp-68h]
		float a_row_11; // [esp+Ch] [ebp-64h]
		float a_row_10; // [esp+10h] [ebp-60h]
		float a_row_22; // [esp+14h] [ebp-5Ch]
		float a_row_21; // [esp+18h] [ebp-58h]
		float a_row_20; // [esp+1Ch] [ebp-54h]
		float b_row_11; // [esp+20h] [ebp-50h]
		float b_row_01; // [esp+24h] [ebp-4Ch]
		float b_row_22; // [esp+28h] [ebp-48h]
		float b_row_12; // [esp+2Ch] [ebp-44h]
		float b_row_02; // [esp+30h] [ebp-40h]
		float b_row_23; // [esp+34h] [ebp-3Ch]
		float b_row_03; // [esp+3Ch] [ebp-34h]
		Matrix3D C; // [esp+40h] [ebp-30h] BYREF
		float b_row_21; // [esp+74h] [ebp+4h]

		a_row_10 = A.row[1][0];
		a_row_11 = A.row[1][1];
		a_row_00 = A.row[0][0];
		a_row_12 = A.row[1][2];
		a_row_01 = A.row[0][1];
		a_row_02 = A.row[0][2];
		a_row_20 = A.row[2][0];
		a_row_21 = A.row[2][1];
		a_row_22 = A.row[2][2];
		b_row_00 = B.row[0][0];
		b_row_10 = B.row[1][0];
		b_row_20 = B.row[2][0];
		b_row_01 = B.row[0][1];
		b_row_02 = B.row[0][2];
		b_row_03 = B.row[0][3];
		b_row_11 = B.row[1][1];
		b_row_12 = B.row[1][2];
		b_row_13 = B.row[1][3];
		C.row[0][0] = static_cast<float>(b_row_20 * a_row_02 + b_row_10 * a_row_01 + b_row_00 * a_row_00);
		b_row_21 = B.row[2][1];
		b_row_22 = B.row[2][2];
		b_row_23 = B.row[2][3];
		C.row[1][0] = static_cast<float>(b_row_20 * a_row_12 + b_row_10 * a_row_11 + b_row_00 * a_row_10);
		C.row[2][0] = static_cast<float>(b_row_20 * a_row_22 + b_row_10 * a_row_21 + b_row_00 * a_row_20);
		C.row[0][1] = static_cast<float>(b_row_21 * a_row_02 + b_row_11 * a_row_01 + b_row_01 * a_row_00);
		C.row[1][1] = static_cast<float>(b_row_21 * a_row_12 + b_row_11 * a_row_11 + b_row_01 * a_row_10);
		C.row[2][1] = static_cast<float>(b_row_21 * a_row_22 + b_row_11 * a_row_21 + b_row_01 * a_row_20);
		C.row[0][2] = static_cast<float>(b_row_22 * a_row_02 + b_row_12 * a_row_01 + b_row_02 * a_row_00);
		C.row[1][2] = static_cast<float>(b_row_22 * a_row_12 + b_row_12 * a_row_11 + b_row_02 * a_row_10);
		C.row[2][2] = static_cast<float>(b_row_22 * a_row_22 + b_row_12 * a_row_21 + b_row_02 * a_row_20);
		C.row[0][3] = static_cast<float>(b_row_23 * a_row_02 + b_row_13 * a_row_01 + b_row_03 * a_row_00 + A.row[0][3]);
		C.row[1][3] = static_cast<float>(b_row_23 * a_row_12 + b_row_13 * a_row_11 + b_row_03 * a_row_10 + A.row[1][3]);
		C.row[2][3] = static_cast<float>(b_row_23 * a_row_22 + b_row_13 * a_row_21 + b_row_03 * a_row_20 + A.row[2][3]);
		return C;
	}

	static Vector3D<float>* __fastcall MatrixMultiply(Vector3D<float>* ret, const Matrix3D* mat, const Vector3D<float>* vec) { JMP_STD(0x5AFB80); }
	static Vector3D<float> MatrixMultiply(const Matrix3D& mat, const Vector3D<float>& vect)
	{
		return {
			mat.Row[0].Z * vect.Z + mat.Row[0].Y * vect.Y + mat.Row[0].X * vect.X + mat.Row[0].W ,
			mat.Row[1].X * vect.X + mat.Row[1].Z * vect.Z + mat.Row[1].Y * vect.Y + mat.Row[1].W ,
			mat.Row[2].X * vect.X + mat.Row[2].Z * vect.Z + mat.Row[2].Y * vect.Y + mat.Row[2].W
		};
	}

	static Vector3D<float> MatrixMultiply(Matrix3D* mtx, const Vector3D<float>& vect)
	{
		return {
			mtx->Row[0].Z * vect.Z + mtx->Row[0].Y * vect.Y + mtx->Row[0].X * vect.X + mtx->Row[0].W ,
			mtx->Row[1].X * vect.X + mtx->Row[1].Z * vect.Z + mtx->Row[1].Y * vect.Y + mtx->Row[1].W ,
			mtx->Row[2].X * vect.X + mtx->Row[2].Z * vect.Z + mtx->Row[2].Y * vect.Y + mtx->Row[2].W
		};
	}

	static Vector3D<float> InverseRotateVector(const Matrix3D &tm, const Vector3D<float> &in)
	{
		return {
			(tm.row[0][0] * in.X + tm.row[1][0] * in.Y + tm.row[2][0] * in.Z) ,
			(tm.row[0][1] * in.X + tm.row[1][1] * in.Y + tm.row[2][1] * in.Z) ,
			(tm.row[0][2] * in.X + tm.row[1][2] * in.Y + tm.row[2][2] * in.Z)
		};
	}

	static Vector3D<float> InverseTransformVector(const Matrix3D &tm, const Vector3D<float> &in)
	{
		Vector3D<float> diff{ in.X - tm.row[0][3],  in.Y - tm.row[1][3], in.Z - tm.row[2][3] };
		return InverseRotateVector(tm, diff);
	}

	static Vector3D<float> TransformVector(const Matrix3D &tm, const Vector3D<float> &in)
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

	/*void RotateX90()
	{
		row[0][0] = 1.0; row[0][1] = 0.0; row[0][2] = 0.0; row[0][3] = 0.0;
		row[1][0] = 0.0; row[1][1] = 0.0; row[1][2] = -1.0; row[1][3] = 0.0;
		row[2][0] = 0.0; row[2][1] = 1.0; row[2][2] = 0.0; row[2][3] = 0.0;
	}

	void RotateX180()
	{
		row[0][0] = 1.0; row[0][1] = 0.0; row[0][2] = 0.0; row[0][3] = 0.0;
		row[1][0] = 0.0; row[1][1] = -1.0; row[1][2] = 0.0; row[1][3] = 0.0;
		row[2][0] = 0.0; row[2][1] = 0.0; row[2][2] = -1.0; row[2][3] = 0.0;
	};

	void RotateX270()
	{
		row[0][0] = 1.0; row[0][1] = 0.0; row[0][2] = 0.0; row[0][3] = 0.0;
		row[1][0] = 0.0; row[1][1] = 0.0; row[1][2] = 1.0; row[1][3] = 0.0;
		row[2][0] = 0.0; row[2][1] = -1.0; row[2][2] = 0.0; row[2][3] = 0.0;
	};

	void RotateY90()
	{
		row[0][0] = 0.0; row[0][1] = 0.0; row[0][2] = 1.0; row[0][3] = 0.0;
		row[1][0] = 0.0; row[1][1] = 1.0; row[1][2] = 0.0; row[1][3] = 0.0;
		row[2][0] = -1.0; row[2][1] = 0.0; row[2][2] = 0.0; row[2][3] = 0.0;
	};

	void RotateY180()
	{
		row[0][0] = -1.0; row[0][1] = 0.0; row[0][2] = 0.0; row[0][3] = 0.0;
		row[1][0] = 0.0; row[1][1] = 1.0; row[1][2] = 0.0; row[1][3] = 0.0;
		row[2][0] = 0.0; row[2][1] = 0.0; row[2][2] = -1.0; row[2][3] = 0.0;
	};

	void RotateY270()
	{
		row[0][0] = 0.0; row[0][1] = 0.0; row[0][2] = -1.0; row[0][3] = 0.0;
		row[1][0] = 0.0; row[1][1] = 1.0; row[1][2] = 0.0; row[1][3] = 0.0;
		row[2][0] = 1.0; row[2][1] = 0.0; row[2][2] = 0.0; row[2][3] = 0.0;
	};

	void RotateZ90()
	{
		row[0][0] = 0.0; row[0][1] = -1.0; row[0][2] = 0.0; row[0][3] = 0.0;
		row[1][0] = 1.0; row[1][1] = 0.0; row[1][2] = 0.0; row[1][3] = 0.0;
		row[2][0] = 0.0; row[2][1] = 0.0; row[2][2] = 1.0; row[2][3] = 0.0;
	};

	void RotateZ180()
	{
		row[0][0] = -1.0; row[0][1] = 0.0; row[0][2] = 0.0; row[0][3] = 0.0;
		row[1][0] = 0.0; row[1][1] = -1.0; row[1][2] = 0.0; row[1][3] = 0.0;
		row[2][0] = 0.0; row[2][1] = 0.0; row[2][2] = 1.0; row[2][3] = 0.0;
	};

	void RotateZ270()
	{
		row[0][0] = 0.0; row[0][1] = 1.0; row[0][2] = 0.0; row[0][3] = 0.0;
		row[1][0] = -1.0; row[1][1] = 0.0; row[1][2] = 0.0; row[1][3] = 0.0;
		row[2][0] = 0.0; row[2][1] = 0.0; row[2][2] = 1.0; row[2][3] = 0.0;
	};
	*/

//Properties
public:
	union
	{
		Vector4D<float> Row[3];
		float row[3][4];
		float Data[12];
	};
};

static_assert(sizeof(Matrix3D) == 0x30u);
