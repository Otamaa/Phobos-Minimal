#pragma once

#include <bitset>

#include <GeneralDefinitions.h>
#include <TacticalClass.h>
#include <ObjectClass.h>
#include <BulletClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <Utilities/Savegame.h>

template<typename T>
static FORCEDINLINE  Vector3D<T> ToVector3D(CoordStruct& vector)
{
	return { static_cast<T>(vector.X), static_cast<T>(vector.Y), static_cast<T>(vector.Z) };
}


template<typename T, typename T2>
static FORCEDINLINE  Vector3D<T> ToVector3D(Vector3D<T2>& vector)
{
	return { static_cast<T>(vector.X), static_cast<T>(vector.Y), static_cast<T>(vector.Z) };
}

template<typename T>
static FORCEDINLINE CoordStruct ToCoordStruct(Vector3D<T>& vector)
{
	return { static_cast<int>(vector.X), static_cast<int>(vector.Y), static_cast<int>(vector.Z) };
}

template<typename T>
static FORCEDINLINE  Vector3D<double> ToVelocity(Vector3D<T>& vector)
{
	return ToVector3D<double>(vector);
}

static FORCEDINLINE VelocityClass ToVelocityClass(CoordStruct& vector) {
	return { static_cast<double>(vector.X), static_cast<double>(vector.Y), static_cast<double>(vector.Z) };
}

static FORCEDINLINE  Vector3D<double> ToVelocity(CoordStruct& vector)
{
	return ToVector3D<double>(vector);
}

static FORCEDINLINE  CoordStruct ToCoords(Point2D point)
{
	return TacticalClass::Instance->ClientToCoords(point);
}

static FORCEDINLINE  Point2D ToClientPos(CoordStruct& coords)
{
	return TacticalClass::Instance->CoordsToClient(coords);
}

static FORCEDINLINE  Point2D CoordsToScreen(CoordStruct coords)
{
	return TacticalClass::Instance->CoordsToScreen(coords);
}

static OPTIONALINLINE  bool CastToBullet(AbstractClass* pTarget, BulletClass*& pBullet)
{
	pBullet = cast_to<BulletClass*>(pTarget);
	return pBullet != nullptr;
	/*
	if (pTarget)
	{
		switch (pTarget->WhatAmI())
		{
		case AbstractType::Bullet:
			pBullet = flag_cast_to<BulletClass*>(pTarget);
			return pBullet != nullptr;
		default:
			return false;
		}
	}
	return false;
	*/
}

static FORCEDINLINE  bool CastToBullet(ObjectClass* pObject, BulletClass*& pBullet)
{
	return CastToBullet(static_cast<AbstractClass*>(pObject), pBullet);
}

static OPTIONALINLINE  bool CastToTechno(AbstractClass* pTarget, TechnoClass*& pTechno)
{
	pTechno = flag_cast_to<TechnoClass*>(pTarget);
	return pTechno != nullptr;
	/*
	if (pTarget)
	{
		switch (pTarget->WhatAmI())
		{
		case AbstractType::Building:
		case AbstractType::Unit:
		case AbstractType::Infantry:
		case AbstractType::Aircraft:
			pTechno = flag_cast_to<TechnoClass*>(pTarget);
			return pTechno != nullptr;
		default:
			return false;
		}
	}
	return false;
	*/
}

static FORCEDINLINE  bool CastToTechno(ObjectClass* pObject, TechnoClass*& pTechno)
{
	return CastToTechno(static_cast<AbstractClass*>(pObject), pTechno);
}

static OPTIONALINLINE bool CastToFoot(TechnoClass* pTechno, FootClass*& pFoot)
{
	if (pTechno->AbstractFlags & AbstractFlags::Foot)
	{
		pFoot = static_cast<FootClass*>(pTechno);
		return pFoot != nullptr;
	}
	return false;
}

static OPTIONALINLINE  ColorStruct ToColorAdd(ColorStruct color)
{
	BYTE B = color.B >> 3;
	BYTE G = color.G >> 2;
	BYTE R = color.R >> 3;
	return ColorStruct{ R, G, B };
}

static OPTIONALINLINE unsigned int Add2RGB565(ColorStruct colorAdd)
{
	// 转2进制字符串
	std::bitset<5> R2(colorAdd.R);
	std::bitset<6> G2(colorAdd.G);
	std::bitset<5> B2(colorAdd.B);
	// 拼接字符串
	std::string C = R2.to_string();
	C += G2.to_string();
	C += B2.to_string();
	std::bitset<16> C2(C);
	return static_cast<unsigned int>(C2.to_ulong());
}

static OPTIONALINLINE unsigned int GetBright(unsigned int bright, float multi)
{
	double b = bright;
	if (multi != 1.0f)
	{
		b *= multi;
		if (b < 0)
		{
			b = 0;
		}
		else if (b > 2000)
		{
			b = 2000;
		}
	}
	return static_cast<unsigned int>(b);
}
