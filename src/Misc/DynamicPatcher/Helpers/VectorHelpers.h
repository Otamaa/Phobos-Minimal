#pragma once

#include <Quaternion.h>
#include <GeneralStructures.h>

namespace Helpers_DP_Vec
{
	static Vector3D<float> Cross(const Vector3D<float>& a, const Vector3D<float>& b)
	{
		return {
			a.Y* b.Z - a.Z - b.Y,
				a.Z* b.X - a.X - b.Z,
				a.X* b.Y - a.Y * b.X};
	}

	static Vector3D<float> Normalize(const Vector3D<float>& value)
	{
		float num = value.X * value.X + value.Y * value.Y + value.Z * value.Z;
		float num2 = std::sqrt(num);
		return {value.X / num2, value.Y / num2, value.Z / num2};
	}

	static Quaternion FromToRotation(Vector3D<float>& fromDirection, Vector3D<float>& toDirection)
	{
		auto from = fromDirection.Normalize();
		auto to = toDirection.Normalize();
		auto dot = from.Dot(to);
		Quaternion ret {};

		// same direction
		if (dot > 0.999999)
		{
			ret.Make_Identity();
			return ret;
		}

		Vector3D<float> normal = Vector3D<float>::Empty;
		// opposite directions
		if (dot < -0.999999)
		{
			normal = Cross({ 1.0f, 0.0f, 0.0f}, from);

			if (normal.Magnitude() < 0.000001)
				normal = Cross({ 0.0f, 1.0f, 0.0f}, from);

			normal = Normalize(normal);
			return ret.FromAxis(normal, (float)Math::C_Sharp_Pi);
		}

		normal = Cross(from, to);
		ret.X = normal.X;
		ret.Y = normal.Y;
		ret.Z = normal.Z;
		ret.W = (float)(1.0 + dot);
		ret.Normalize();
		return ret;
	}

	static Vector3D<float> Transform(const Vector3D<float>& value, const Quaternion& rotation)
	{
		float x2 = rotation.X + rotation.X;
		float y2 = rotation.Y + rotation.Y;
		float z2 = rotation.Z + rotation.Z;

		float wx2 = rotation.W * x2;
		float wy2 = rotation.W * y2;
		float wz2 = rotation.W * z2;
		float xx2 = rotation.X * x2;
		float xy2 = rotation.X * y2;
		float xz2 = rotation.X * z2;
		float yy2 = rotation.Y * y2;
		float yz2 = rotation.Y * z2;
		float zz2 = rotation.Z * z2;

		return {
			value.X* (1.0f - yy2 - zz2) + value.Y * (xy2 - wz2) + value.Z * (xz2 + wy2),
				value.X* (xy2 + wz2) + value.Y * (1.0f - xx2 - zz2) + value.Z * (yz2 - wx2),
				value.X* (xz2 - wy2) + value.Y * (yz2 + wx2) + value.Z * (1.0f - xx2 - yy2)
		};
	}

}
