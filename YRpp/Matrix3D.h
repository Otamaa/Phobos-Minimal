#pragma once

#include <YRPPCore.h>
#include <GeneralStructures.h>
#include <Helpers/CompileTime.h>
#include <Quaternion.h>

struct CompileTimeMatrix3D
{
	COMPILETIMEEVAL CompileTimeMatrix3D() noexcept
	{
		this->Row[0][0] = 1.0;
		this->Row[0][1] = 0.0;
		this->Row[0][2] = 0.0;
		this->Row[0][3] = 0.0;
		this->Row[1][0] = 0.0;
		this->Row[1][1] = 1.0;
		this->Row[1][2] = 0.0;
		this->Row[1][3] = 0.0;
		this->Row[2][0] = 0.0;
		this->Row[2][1] = 0.0;
		this->Row[2][2] = 1.0;
		this->Row[2][3] = 0.0;
	}

	COMPILETIMEEVAL CompileTimeMatrix3D(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23) noexcept
	{
		Row[0][0] = m00; Row[0][1] = m01; Row[0][2] = m02; Row[0][3] = m03;
		Row[1][0] = m10; Row[1][1] = m11; Row[1][2] = m12; Row[1][3] = m13;
		Row[2][0] = m20; Row[2][1] = m21; Row[2][2] = m22; Row[2][3] = m23;
	}

	COMPILETIMEEVAL CompileTimeMatrix3D(
	Vector3D<float> const& x,
	Vector3D<float> const& y,
	Vector3D<float> const& z,
	Vector3D<float> const& pos) noexcept
	{
		Row[0][0] = x.X; Row[0][1] = y.X; Row[0][2] = z.X; Row[0][3] = pos.X;
		Row[1][0] = x.Y; Row[1][1] = y.Y; Row[1][2] = z.Y; Row[1][3] = pos.Y;
		Row[2][0] = x.Z; Row[2][1] = y.Z; Row[2][2] = z.Z; Row[2][3] = pos.Z;
	}

public:
	Vector4D<float> Row[3];
};

class Matrix3D
{
public:
	//static COMPILETIMEEVAL reference<Matrix3D, 0xB44318> VoxelDefaultMatrix {};
	//static COMPILETIMEEVAL reference<Matrix3D, 0xB45188, 21> VoxelRampMatrix {};

	//Constructor

	COMPILETIMEEVAL Matrix3D() noexcept
	{
		this->Row[0][0] = 0.0;
		this->Row[0][1] = 0.0;
		this->Row[0][2] = 0.0;
		this->Row[0][3] = 0.0;
		this->Row[1][0] = 0.0;
		this->Row[1][1] = 0.0;
		this->Row[1][2] = 0.0;
		this->Row[1][3] = 0.0;
		this->Row[2][0] = 0.0;
		this->Row[2][1] = 0.0;
		this->Row[2][2] = 0.0;
		this->Row[2][3] = 0.0;
	}

	// plain floats ctor
	COMPILETIMEEVAL Matrix3D(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23) noexcept
	{
		Row[0][0] = m00; Row[0][1] = m01; Row[0][2] = m02; Row[0][3] = m03;
		Row[1][0] = m10; Row[1][1] = m11; Row[1][2] = m12; Row[1][3] = m13;
		Row[2][0] = m20; Row[2][1] = m21; Row[2][2] = m22; Row[2][3] = m23;
	}

	// column vector ctor
	COMPILETIMEEVAL Matrix3D(
		Vector3D<float> const &x,
		Vector3D<float> const &y,
		Vector3D<float> const &z,
		Vector3D<float> const &pos) noexcept
	{
		Row[0][0] = x.X; Row[0][1] = y.X; Row[0][2] = z.X; Row[0][3] = pos.X;
		Row[1][0] = x.Y; Row[1][1] = y.Y; Row[1][2] = z.Y; Row[1][3] = pos.Y;
		Row[2][0] = x.Z; Row[2][1] = y.Z; Row[2][2] = z.Z; Row[2][3] = pos.Z;
		//JMP_THIS(0x5AE690);
	}

	// some other rotation ctor?
	Matrix3D(float rotate_z, float rotate_x) noexcept
	{
		//JMP_THIS(0x5AE6F0);
		this->Row[0][1] = 0.0;
		this->Row[0][2] = 0.0;
		this->Row[0][3] = 0.0;
		this->Row[1][0] = 0.0;
		this->Row[1][2] = 0.0;
		this->Row[1][3] = 0.0;
		this->Row[2][0] = 0.0;
		this->Row[2][1] = 0.0;
		this->Row[2][3] = 0.0;
		this->Row[0][0] = 1.0;
		this->Row[1][1] = 1.0;// inlined ctor
		this->Row[2][2] = 1.0;
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
		this->Row[0][0] = static_cast<float>((1.0 - v7) * c + v7);
		double v8 = 1.0 - c;
		this->Row[0][1] = static_cast<float>(axis->X * axis->Y * v8 - s * axis->Z);
		double v9 = axis->Z * axis->X * v8;
		double v10 = s * axis->Y;
		this->Row[0][3] = 0.0f;
		this->Row[0][2] = static_cast<float>(v9 + v10);
		this->Row[1][0] = static_cast<float>(axis->X * axis->Y * v8 + s * axis->Z);
		double v11 = axis->Y * axis->Y;
		this->Row[1][1] = static_cast<float>((1.0 - v11) * c + v11);
		double v12 = axis->Z * axis->Y * v8;
		double v13 = s * axis->X;
		this->Row[1][3] = 0.0f;
		this->Row[1][2] = static_cast<float>(v12 - v13);
		this->Row[2][0] = static_cast<float>(axis->Z * axis->X * v8 - s * axis->Y);
		this->Row[2][1] = static_cast<float>(axis->Z * axis->Y * v8 + s * axis->X);
		double v14 = axis->Z * axis->Z;
		this->Row[2][3] = 0.0f;
		this->Row[2][2] = static_cast<float>((1.0 - v14) * c + v14);
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
				C.Row[i][j] =
				this->Row[i][0] * C_another.Row[0][j] +
				this->Row[i][1] * C_another.Row[1][j] +
				this->Row[i][2] * C_another.Row[2][j];
			}

			C.Row[i][3] =
				this->Row[i][0] * C_another.Row[0][3] +
				this->Row[i][1] * C_another.Row[1][3] +
				this->Row[i][2] * C_another.Row[2][3] +
				this->Row[i][3];
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
	static Matrix3D* __fastcall TransposeMatrix(Matrix3D* buffer, const Matrix3D* mat) { JMP_STD(0x5AFC20); }
	static Matrix3D TransposeMatrix(const Matrix3D& A)
	{
		Matrix3D v7 {};
		TransposeMatrix(&v7, &A);
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
		mtx.Row[0][0] = static_cast<float>(1.0 - (v12 + v2 + v12 + v2));
		v4 = q.Y * q.X;
		v5 = q.W * q.Z;
		mtx.Row[0][1] = static_cast<float>(v4 - v5 + v4 - v5);
		v10 = q.W * q.Y;
		v6 = q.X * q.Z;
		v9 = static_cast<float>(v6);
		mtx.Row[0][2] = static_cast<float>(v6 + v10 + v6 + v10);
		mtx.Row[1][0] = static_cast<float>(v5 + v4 + v5 + v4);
		v11 = q.X * q.X;
		mtx.Row[1][1] = static_cast<float>(1.0 - (v11 + v2 + v11 + v2));
		v7 = q.Y * q.Z;
		v8 = q.W * q.X;
		mtx.Row[1][2] = static_cast<float>(v7 - v8 + v7 - v8);
		mtx.Row[2][0] = static_cast<float>(v9 - v10 + v9 - v10);
		mtx.Row[2][3] = 0.0f;
		mtx.Row[1][3] = 0.0f;
		mtx.Row[0][3] = 0.0f;
		mtx.Row[2][1] = static_cast<float>(v8 + v7 + v8 + v7);
		mtx.Row[2][2] = static_cast<float>(1.0 - (v11 + v12 + v11 + v12));*/
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
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 4; j++)
				Row[i][j] = i == j ? 1.f : 0.f;
	} // 1-matrix

	//void Translate(float x, float y, float z) const { JMP_THIS(0x5AE890); }
	COMPILETIMEEVAL void Translate(float x, float y, float z)
	{
		TranslateX(x);
		TranslateY(y);
		TranslateZ(z);
	}

	//void Translate(Vector3D<float> const& vec) const { JMP_THIS(0x5AE8F0); }
	COMPILETIMEEVAL void Translate(Vector3D<float> const& t)
	{
		Translate(t.X, t.Y, t.Z);
	}

	COMPILETIMEEVAL void TranslateX(float x) //{ JMP_THIS(0x5AE980); }
	{
		for (int i = 0; i < 3; i++)
			Row[i][3] += x * Row[i][0];
	}

	COMPILETIMEEVAL void TranslateY(float y) //{ JMP_THIS(0x5AE9B0); }
	{
		for (int i = 0; i < 3; i++)
			Row[i][3] += y * Row[i][1];
	}

	COMPILETIMEEVAL void TranslateZ(float z) //{ JMP_THIS(0x5AE9E0); }
	{
		for (int i = 0; i < 3; i++)
			Row[i][3] += z * Row[i][2];
	}

	COMPILETIMEEVAL void Scale(float factor) //{ JMP_THIS(0x5AEA10); }
	{
		for (int i = 0; i < 3; i++)
			Row[i] *= factor;
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
			Row[i][0] *= factor;
	}

	COMPILETIMEEVAL void ScaleY(float factor) //{ JMP_THIS(0x5AEAF0); }
	{
		for (int i = 0; i < 3; i++)
			Row[i][1] *= factor;
	}

	COMPILETIMEEVAL void ScaleZ(float factor) //{ JMP_THIS(0x5AEB20); }
	{
		for (int i = 0; i < 3; i++)
			Row[i][2] *= factor;
	}

	//void ShearYZ(float y, float z) const { JMP_THIS(0x5AEB50); }
	COMPILETIMEEVAL void ShearYZ(float y, float z)
	{
		this->Row[0][0] = y * this->Row[0][1] + z * this->Row[0][2] + this->Row[0][0];
		this->Row[1][0] = y * this->Row[1][1] + z * this->Row[1][2] + this->Row[1][0];
		this->Row[2][0] = y * this->Row[2][1] + z * this->Row[2][2] + this->Row[2][0];
	}

	//void ShearXY(float x, float y) const { JMP_THIS(0x5AEBA0); }
	COMPILETIMEEVAL void ShearXY(float x, float y)
	{
		this->Row[0][2] = y * this->Row[0][1] + x * this->Row[0][0] + this->Row[0][2];
		this->Row[1][2] = x * this->Row[1][0] + y * this->Row[1][1] + this->Row[1][2];
		this->Row[2][2] = x * this->Row[2][0] + y * this->Row[2][1] + this->Row[2][2];
	}

	//void ShearXZ(float x, float z) const { JMP_THIS(0x5AEBF0); }
	COMPILETIMEEVAL void ShearXZ(float x, float z)
	{
		this->Row[0][1] = z * this->Row[0][2] + x * this->Row[0][0] + this->Row[0][1];
		this->Row[1][1] = x * this->Row[1][0] + z * this->Row[1][2] + this->Row[1][1];
		this->Row[2][1] = x * this->Row[2][0] + z * this->Row[2][2] + this->Row[2][1];
	}

	void PreRotateX(float theta) const { JMP_THIS(0x5AEC40); }
	void PreRotateY(float theta) const { JMP_THIS(0x5AED50); }
	void PreRotateZ(float theta) const { JMP_THIS(0x5AEE50); }

	void RotateX(float theta) const { JMP_THIS(0x5AEF60); }
	void RotateX(float Sin, float Cos) const { JMP_THIS(0x5AF000); }
	void RotateY(float theta) const { JMP_THIS(0x5AF080); }

	void RotateY(float Sin, float Cos) const { JMP_THIS(0x5AF120); }
	void RotateZ(float theta) const { JMP_THIS(0x5AF1A0); }
	void RotateZ(float Sin, float Cos) const { JMP_THIS(0x5AF240); }

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

	Vector3D<float>* __RotateVector(Vector3D<float>* ret, Vector3D<float>* rotate) const { JMP_THIS(0x5AF4D0); }
	COMPILETIMEEVAL Vector3D<float> RotateVector(const Vector3D<float>& rotate) const {
		return {
				Row[0][0] * rotate.X + Row[0][1] * rotate.Y + Row[0][2] * rotate.Z,
				Row[1][0] * rotate.X + Row[1][1] * rotate.Y + Row[1][2] * rotate.Z,
				Row[2][0] * rotate.X + Row[2][1] * rotate.Y + Row[2][2] * rotate.Z,
		};
	}

	void LookAt1(Vector3D<float>& p, Vector3D<float>& t, float roll) { JMP_THIS(0x5AF550); }
	void LookAt2(Vector3D<float>& p, Vector3D<float>& t, float roll) { JMP_THIS(0x5AF710); }

	//static Matrix3D* __fastcall MatrixMultiply__(Matrix3D* ret, const Matrix3D* A, const Matrix3D* B) { JMP_STD(0x5AF980); }
	COMPILETIMEEVAL FORCEDINLINE static Matrix3D MatrixMultiply__(const Matrix3D& A, const Matrix3D& B)
	{
		//Matrix3D buffer;
		//MatrixMultiply__(&buffer, &A, &B);
		return A * B;
	}

	COMPILETIMEEVAL static Vector3D<float>* //__fastcall
			MatrixMultiply(Vector3D<float>* vecret, const Matrix3D* mat, const Vector3D<float>* vec) {
		//JMP_FAST(0x5AFB80);
		vecret->X = (mat->Row[0][0] * vec->X + mat->Row[0][1] * vec->Y + mat->Row[0][2] * vec->Z + mat->Row[0][3]);
		vecret->Y = (mat->Row[1][0] * vec->X + mat->Row[1][1] * vec->Y + mat->Row[1][2] * vec->Z + mat->Row[1][3]);
		vecret->Z = (mat->Row[2][0] * vec->X + mat->Row[2][1] * vec->Y + mat->Row[2][2] * vec->Z + mat->Row[2][3]);
   		 return vecret;
	}

	COMPILETIMEEVAL static Vector3D<float> InverseRotateVector(const Matrix3D &tm, const Vector3D<float> &in)
	{
		return {
			(tm.Row[0][0] * in.X + tm.Row[1][0] * in.Y + tm.Row[2][0] * in.Z) ,
			(tm.Row[0][1] * in.X + tm.Row[1][1] * in.Y + tm.Row[2][1] * in.Z) ,
			(tm.Row[0][2] * in.X + tm.Row[1][2] * in.Y + tm.Row[2][2] * in.Z)
		};
	}

	COMPILETIMEEVAL static Vector3D<float> InverseTransformVector(const Matrix3D &tm, const Vector3D<float> &in)
	{
		Vector3D<float> diff{ in.X - tm.Row[0][3],  in.Y - tm.Row[1][3], in.Z - tm.Row[2][3] };
		return InverseRotateVector(tm, diff);
	}

	COMPILETIMEEVAL static Vector3D<float> TransformVector(const Matrix3D &tm, const Vector3D<float> &in)
	{
		return {
			(tm.Row[0][0] * in.X + tm.Row[0][1] * in.Y + tm.Row[0][2] * in.Z + tm.Row[0][3]) ,
			(tm.Row[1][0] * in.X + tm.Row[1][1] * in.Y + tm.Row[1][2] * in.Z + tm.Row[1][3]) ,
			(tm.Row[2][0] * in.X + tm.Row[2][1] * in.Y + tm.Row[2][2] * in.Z + tm.Row[2][3])
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
	Vector4D<float> Row[3];
};

static_assert(sizeof(Matrix3D) == 0x30u);
