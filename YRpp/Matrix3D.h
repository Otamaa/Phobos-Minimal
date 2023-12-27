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
		//row[0][0] = x.X; row[0][1] = y.X; row[0][2] = z.X; row[0][3] = pos.X;
		//row[1][0] = x.Y; row[1][1] = y.Y; row[1][2] = z.Y; row[1][3] = pos.Y;
		//row[2][0] = x.Z; row[2][1] = y.Z; row[2][2] = z.Z; row[2][3] = pos.Z;
		JMP_THIS(0x5AE690);
	}

	// some other rotation ctor?
	Matrix3D(float rotate_z, float rotate_x) noexcept
	{
		JMP_THIS(0x5AE6F0);
		//this->row[0][1] = 0.0;
		//this->row[0][2] = 0.0;
		//this->row[0][3] = 0.0;
		//this->row[1][0] = 0.0;
		//this->row[1][2] = 0.0;
		//this->row[1][3] = 0.0;
		//this->row[2][0] = 0.0;
		//this->row[2][1] = 0.0;
		//this->row[2][3] = 0.0;
		//this->row[0][0] = 1.0;
		//this->row[1][1] = 1.0;// inlined ctor
		//this->row[2][2] = 1.0;
		//this->RotateZ(rotate_z);
		//this->RotateX(rotate_x);
		//float theta = -rotate_z;
		//this->RotateZ(theta);
	}

	// rotation ctor
	Matrix3D(const Vector3D<float>* axis, float angle) noexcept
	{
		JMP_THIS(0x5AE750);
		/*float c = Math::cos((double)angle);
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
		this->row[2][2] = static_cast<float>((1.0 - v14) * c + v14);*/
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
		TransposeMatrix(&v7, &A);
		//v7.row[0][0] = A.row[0][0];
		//double v3 = v7.row[0][0] * A.row[0][3];
		//v7.row[0][1] = A.row[1][0];
		//v7.row[0][2] = A.row[2][0];
		//double v4 = v7.row[0][2] * A.row[2][3];
		//v7.row[1][0] = A.row[0][1];
		//v7.row[1][1] = A.row[1][1];
		//double v5 = v3 + v4;
		//double v6 = v7.row[0][1] * A.row[1][3];
		//v7.row[1][2] = A.row[2][1];
		//v7.row[2][0] = A.row[0][2];
		//v7.row[2][1] = A.row[1][2];
		//v7.row[2][2] = A.row[2][2];
		//v7.row[0][3] = static_cast<float>(-(v5 + v6));
		//v7.row[1][3] = static_cast<float>(-(v7.row[1][0] * A.row[0][3] + v7.row[1][2] * A.row[2][3] + v7.row[1][1] * A.row[1][3]));
		//v7.row[2][3] = static_cast<float>(-(v7.row[2][0] * A.row[0][3] + v7.row[2][2] * A.row[2][3] + v7.row[2][1] * A.row[1][3]));
		return v7;
	}

	void Transpose()
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

	void Translate(float x, float y, float z) const { JMP_THIS(0x5AE890); }
	//void Translate(float x, float y, float z)
	//{
	//	this->row[0][3] = y * this->row[0][1] + z * this->row[0][2] + x * this->row[0][0] + this->row[0][3];
	//	this->row[1][3] = x * this->row[1][0] + y * this->row[1][1] + z * this->row[1][2] + this->row[1][3];
	//	this->row[2][3] = x * this->row[2][0] + y * this->row[2][1] + z * this->row[2][2] + this->row[2][3];
	//}

	void Translate(Vector3D<float> const& vec) const { JMP_THIS(0x5AE8F0); }
	//void Translate(Vector3D<float> const& t)
	//{
	//	double v3; // st7
	//	double v4; // st7
	//	double v5; // st6
	//	double v6; // st7
	//	double v7; // st6
	//	float v8; // [esp+0h] [ebp-4h]
	//	float v9; // [esp+0h] [ebp-4h]
	//	float a2a; // [esp+8h] [ebp+4h]
	//	float a2b; // [esp+8h] [ebp+4h]

	//	v3 = t.X;
	//	a2a = static_cast<float>(v3 * this->row[0][0] + this->row[0][3]);
	//	this->row[0][3] = a2a;
	//	v8 = static_cast<float>(v3 * this->row[1][0] + this->row[1][3]);
	//	this->row[1][3] = v8;
	//	v4 = v3 * this->row[2][0] + this->row[2][3];
	//	this->row[2][3] = static_cast<float>(v4);
	//	v5 = t.Y;
	//	a2b = static_cast<float>(v5 * this->row[0][1] + a2a);
	//	this->row[0][3] = a2b;
	//	v9 = static_cast<float>(v5 * this->row[1][1] + v8);
	//	this->row[1][3] = v9;
	//	v6 = v4 + v5 * this->row[2][1];
	//	this->row[2][3] = static_cast<float>(v6);
	//	v7 = t.Z;
	//	this->row[0][3] = static_cast<float>(v7 * this->row[0][2] + a2b);
	//	this->row[1][3] = static_cast<float>(v7 * this->row[1][2] + v9);
	//	this->row[2][3] = static_cast<float>(v7 * this->row[2][2] + v6);
	//}

	void TranslateX(float x) const { JMP_THIS(0x5AE980); }
	//void TranslateX(float x)
	//{
	//	this->row[0][3] = x * this->row[0][0] + this->Row[0].W;
	//	this->row[1][3] = x * this->Row[1].X + this->Row[1].W;
	//	this->row[2][3] = x * this->Row[2].X + this->Row[2].W;
	//}

	void TranslateY(float y) const { JMP_THIS(0x5AE9B0); }
	//void TranslateY(float y)
	//{
	//	this->row[0][3] = y * this->row[0][1] + this->Row[0].W;
	//	this->row[1][3] = y * this->row[1][1] + this->Row[1].W;
	//	this->row[2][3] = y * this->row[2][1] + this->Row[2].W;
	//}

	void TranslateZ(float z) const { JMP_THIS(0x5AE9E0); }
	//void TranslateZ(float z)
	//{
	//	this->row[0][3] = z * this->row[0][2] + this->row[0][3];
	//	this->row[1][3] = z * this->row[1][2] + this->row[1][3];
	//	this->row[2][3] = z * this->row[2][2] + this->row[2][3];
	//}

	void Scale(float factor) const { JMP_THIS(0x5AEA10); }
	//void Scale(float factor)
	//{
	//	this->row[0][0] = factor * this->row[0][0];
	//	this->row[1][0] = factor * this->row[1][0];
	//	this->row[2][0] = factor * this->row[2][0];
	//	this->row[0][1] = factor * this->row[0][1];
	//	this->row[1][1] = factor * this->row[1][1];
	//	this->row[2][1] = factor * this->row[2][1];
	//	this->row[0][2] = factor * this->row[0][2];
	//	this->row[1][2] = factor * this->row[1][2];
	//	this->row[2][2] = factor * this->row[2][2];
	//}

	void Scale(float x, float y, float z) const { JMP_THIS(0x5AEA70); }
	//void Scale(float x, float y, float z)
	//{
	//	this->row[0][0] = x * this->row[0][0];
	//	this->row[1][0] = x * this->Row[1].X;
	//	this->row[2][0] = x * this->row[2][0];
	//	this->row[0][1] = y * this->row[0][1];
	//	this->row[1][1] = y * this->row[1][1];
	//	this->row[2][1] = y * this->row[2][1];
	//	this->row[0][2] = z * this->row[0][2];
	//	this->row[1][2] = z * this->row[1][2];
	//	this->row[2][2] = z * this->row[2][2];
	//}

	void ScaleX(float factor) const { JMP_THIS(0x5AEAD0); }
	//void ScaleX(float factor)
	//{
	//	this->row[0][0] = factor * this->row[0][0];
	//	this->row[1][0] = factor * this->row[1][0];
	//	this->row[2][0] = factor * this->row[2][0];
	//}

	void ScaleY(float factor) const { JMP_THIS(0x5AEAF0); }
	//void ScaleY(float factor)
	//{
	//	this->row[0][1] = factor * this->row[0][1];
	//	this->row[1][1] = factor * this->row[1][1];
	//	this->row[2][1] = factor * this->row[2][1];
	//}

	void ScaleZ(float factor) const { JMP_THIS(0x5AEB20); }
	//void ScaleZ(float factor)
	//{
	//	this->row[0][2] = factor * this->Row[0].Z;
	//	this->row[1][2] = factor * this->Row[1].Z;
	//	this->row[2][2] = factor * this->Row[2].Z;
	//}

	void ShearYZ(float y, float z) const { JMP_THIS(0x5AEB50); }
	//void ShearYZ(float y, float z)
	//{
	//	this->row[0][0] = y * this->row[0][1] + z * this->row[0][2] + this->row[0][0];
	//	this->row[1][0] = y * this->row[1][1] + z * this->row[1][2] + this->row[1][0];
	//	this->row[2][0] = y * this->row[2][1] + z * this->row[2][2] + this->row[2][0];
	//}

	void ShearXY(float x, float y) const { JMP_THIS(0x5AEBA0); }
	//void ShearXY(float x, float y)
	//{
	//	this->row[0][2] = y * this->row[0][1] + x * this->row[0][0] + this->row[0][2];
	//	this->row[1][2] = x * this->row[1][0] + y * this->row[1][1] + this->row[1][2];
	//	this->row[2][2] = x * this->row[2][0] + y * this->row[2][1] + this->row[2][2];
	//}

	void ShearXZ(float x, float z) const { JMP_THIS(0x5AEBF0); }
	//void ShearXZ(float x, float z)
	//{
	//	this->row[0][1] = z * this->row[0][2] + x * this->row[0][0] + this->row[0][1];
	//	this->row[1][1] = x * this->row[1][0] + z * this->row[1][2] + this->row[1][1];
	//	this->row[2][1] = x * this->row[2][0] + z * this->row[2][2] + this->row[2][1];
	//}

	void PreRotateX(float theta) const { JMP_THIS(0x5AEC40); }
	/*void PreRotateX(float theta)
	{
		float s = Math::sin((double)theta);
		double c = Math::cos((double)theta);
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
	}*/

	void PreRotateY(float theta) const { JMP_THIS(0x5AED50); }
	/*void PreRotateY(float theta)
	{
		float s = Math::sin((double)theta);
		double c = (float)Math::cos((double)theta);
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
	}*/

	void PreRotateZ(float theta) const { JMP_THIS(0x5AEE50); }
	//void PreRotateZ(float theta)
	//{
	//	double v2; // kr00_8
	//	double c; // st7
	//	double sn; // st6
	//	double row_0_0; // st4
	//	float row_1_0; // [esp+0h] [ebp-24h]
	//	float row_1_1; // [esp+8h] [ebp-1Ch]
	//	float row_0_1; // [esp+Ch] [ebp-18h]
	//	float row_1_2; // [esp+10h] [ebp-14h]
	//	float row_0_2; // [esp+14h] [ebp-10h]
	//	float row_1_3; // [esp+18h] [ebp-Ch]
	//	float row_0_3; // [esp+1Ch] [ebp-8h]
	//	float s; // [esp+28h] [ebp+4h]
	//
	//	v2 = theta;
	//	s = Math::sin(theta);
	//	c = Math::cos(v2);
	//	row_0_1 = this->row[0][1];
	//	sn = -s;
	//	row_1_0 = this->row[1][0];
	//	row_0_2 = this->row[0][2];
	//	row_0_0 = this->Row[0].X;
	//	row_1_1 = this->row[1][1];
	//	row_0_3 = this->row[0][3];
	//	row_1_2 = this->row[1][2];
	//	row_1_3 = this->row[1][3];
	//	this->row[0][0] = static_cast<float>(row_1_0 * sn + row_0_0 * c);
	//	this->row[0][1] = static_cast<float>(row_1_1 * sn + row_0_1 * c);
	//	this->row[0][2] = static_cast<float>(row_1_2 * sn + row_0_2 * c);
	//	this->row[0][3] = static_cast<float>(row_1_3 * sn + row_0_3 * c);
	//	this->row[1][0] = static_cast<float>(row_1_0 * c + row_0_0 * s);
	//	this->row[1][1] = static_cast<float>(row_1_1 * c + row_0_1 * s);
	//	this->row[1][2] = static_cast<float>(row_1_2 * c + row_0_2 * s);
	//	this->row[1][3] = static_cast<float>(row_1_3 * c + row_0_3 * s);
	//}

	void RotateX(float theta) const { JMP_THIS(0x5AEF60); }
	/*void RotateX(float theta) {
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
	}*/

	void RotateX(float Sin, float Cos) const { JMP_THIS(0x5AF000); }
	/*void RotateX(float s, float c)
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
	}*/

	void RotateY(float theta) const { JMP_THIS(0x5AF080); }
	//void RotateY(float theta)
	//{
	//	double v2; // kr00_8
	//	double c; // st7
	//	double tmp1; // st6 MAPDST
	//	double tmp2; // st5 MAPDST
	//	float s; // [esp+Ch] [ebp+4h]
	//
	//	v2 = theta;
	//	s = Math::sin(theta);
	//	c = Math::cos(v2);
	//	tmp1 = this->row[0][0];
	//	tmp2 = this->row[0][2];
	//	this->row[0][0] = static_cast<float>(tmp1 * c - tmp2 * s);
	//	this->row[0][2] = static_cast<float>(tmp2 * c + tmp1 * s);
	//	tmp1 = this->row[1][0];
	//	tmp2 = this->row[1][2];
	//	this->row[1][0] = static_cast<float>(tmp1 * c - tmp2 * s);
	//	this->row[1][2] = static_cast<float>(tmp2 * c + tmp1 * s);
	//	tmp1 = this->row[2][0];
	//	tmp2 = this->row[2][2];
	//	this->row[2][0] = static_cast<float>(tmp1 * c - tmp2 * s);
	//	this->row[2][2] = static_cast<float>(tmp2 * c + tmp1 * s);
	//}

	void RotateY(float Sin, float Cos) const { JMP_THIS(0x5AF120); }
	/*void RotateY(float s, float c)
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
	}*/

	void RotateZ(float theta) const { JMP_THIS(0x5AF1A0); }
	/*void RotateZ(float theta) {
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
	}*/

	void RotateZ(float Sin, float Cos) const { JMP_THIS(0x5AF240); }
	//void RotateZ(float s, float c)
	//{
	//	double tmp1; // st7 MAPDST
	//	double tmp2; // st6 MAPDST
	//
	//	tmp1 = this->row[0][0];
	//	tmp2 = this->row[0][1];
	//	this->row[0][0] = static_cast<float>(tmp2 * s + tmp1 * c);
	//	this->row[0][1] = static_cast<float>(tmp2 * c - tmp1 * s);
	//	tmp1 = this->row[1][0];
	//	tmp2 = this->row[1][1];
	//	this->row[1][0] = static_cast<float>(tmp2 * s + tmp1 * c);
	//	this->row[1][1] = static_cast<float>(tmp2 * c - tmp1 * s);
	//	tmp1 = this->row[2][0];
	//	tmp2 = this->row[2][1];
	//	this->row[2][0] = static_cast<float>(tmp2 * s + tmp1 * c);
	//	this->row[2][1] = static_cast<float>(tmp2 * c - tmp1 * s);
	//}

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

	float GetXRotation() const  { JMP_THIS(0x5AF3B0); }
	//float GetXRotation() {
	//	Vector3D<float> ret_ = MatrixMultiply(this, { 0.0f , 1.0f , 0.0f });
	//	return (float)Math::atan2((double)ret_.Z , (double)ret_.Y);
	//}

	float GetYRotation() const  { JMP_THIS(0x5AF410); }
	//float GetYRotation() {
	//	Vector3D<float> ret_ = MatrixMultiply(this, { 0.0f , 0.0f , 1.0f });
	//	return (float)Math::atan2((double)ret_.X, (double)ret_.Z);
	//}

	float GetZRotation() const { JMP_THIS(0x5AF470); }
	//float GetZRotation()
	//{
	//	Vector3D<float> ret_ = MatrixMultiply(this, { 1.0f , 0.0f , 0.0f });
	//	return (float)Math::atan2((double)ret_.Y, (double)ret_.X);
	//}

	Vector3D<float>* RotateVector(Vector3D<float>* ret, Vector3D<float>* rotate) const { JMP_THIS(0x5AF4D0); }
	Vector3D<float> RotateVector(Vector3D<float>& rotate) {
		Vector3D<float> buffer;
		RotateVector(&buffer, &rotate);
		return buffer;
	}

	void LookAt1(Vector3D<float>* p, Vector3D<float>* t, float roll) const { JMP_THIS(0x5AF550) };
	void LookAt1(const Vector3D<float>& p, const Vector3D<float>& t, float roll) {
		JMP_THIS(0x5AF550);
	}

	void LookAt2(Vector3D<float>* p, Vector3D<float>* t, float roll) const { JMP_THIS(0x5AF710); }
	void LookAt2(const Vector3D<float>& p, const Vector3D<float>& t, float roll)
	{ JMP_THIS(0x5AF710); }

	static Matrix3D* __fastcall MatrixMultiply(Matrix3D* ret, const Matrix3D* A, const Matrix3D* B) { JMP_STD(0x5AF980); }
	static Matrix3D MatrixMultiply(const Matrix3D& A, const Matrix3D& B)
	{
		Matrix3D buffer;
		MatrixMultiply(&buffer, &A, &B);
		return buffer;

	}

	static Vector3D<float>* __fastcall MatrixMultiply(Vector3D<float>* ret, const Matrix3D* mat, const Vector3D<float>* vec) { JMP_STD(0x5AFB80); }
	static Vector3D<float> MatrixMultiply(const Matrix3D& mat, const Vector3D<float>& vect)
	{ 
		Vector3D<float> buffer;
		MatrixMultiply(&buffer, &mat, &vect);
		return buffer;
	}

	static Vector3D<float> MatrixMultiply(Matrix3D* mtx, const Vector3D<float>& vect)
	{
		Vector3D<float> buffer;
		MatrixMultiply(&buffer, mtx, &vect);
		return buffer;
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
