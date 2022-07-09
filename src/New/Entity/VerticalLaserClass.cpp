#include "VerticalLaserClass.h"

#include <MapClass.h>
#include <WeaponTypeClass.h>
#include <TechnoClass.h>
#include <FootClass.h>

#include <LaserDrawClass.h>
#include <BulletClass.h>

#include <Ext/BulletType/Body.h>

DynamicVectorClass<VerticalLaserClass*> VerticalLaserClass::Array;

VerticalLaserClass::VerticalLaserClass() : Expired { false }
, count { 0 } , angle { 0 }, cur_frames { 0 }, frames_max { 0 }
, radius { 0.0 }, Weapon { nullptr }, From { CoordStruct::Empty }, Height { 0 }
, radius_decrement { 0 }
{ }

VerticalLaserClass::VerticalLaserClass(WeaponTypeClass* Weapon, CoordStruct From, int Height) : Expired { false }
 ,count { 5 } , angle { 0 }, cur_frames { 0 }, frames_max { 300 }
, radius { 1024.0 }, Weapon { Weapon }, From { From }, Height { Height }
, radius_decrement { 11 }
{ VerticalLaserClass::Array.AddItem(this); }

void VerticalLaserClass::AI(int start, int count)
{
	const int increasement = 360 / count;
	CoordStruct from = From;
	from.Z += 5000;

	for (int i = 0; i < count; i++)
	{
		const CoordStruct to = From + GetCoords(start, i , increasement);
		Draw(from, to);

		if (cur_frames > frames_max)
			DealDamage(to);
		else
			cur_frames++;
	}
}

CoordStruct VerticalLaserClass::GetCoords(int start, int i, int increase)
{
	const double x = radius * Math::cos((start + i * increase) * Math::C_Sharp_Pi / 180);
	const double y = radius * Math::sin((start + i * increase) * Math::C_Sharp_Pi / 180);
	return  { static_cast<int>(x), static_cast<int>(y), -Height };
}

void  VerticalLaserClass::DealDamage(const CoordStruct& to)
{
	auto const pWeaponExt = BulletTypeExt::ExtMap.Find(Weapon->Projectile);
	if (const auto pBullet = pWeaponExt->CreateBullet(Map[to], nullptr, Weapon))
	{
		pBullet->Limbo();
		pBullet->SetLocation(to);
		pBullet->Explode(true);
		pBullet->UnInit();
	}
}

void VerticalLaserClass::Draw(const CoordStruct& from, const CoordStruct& to)
{
	if (Weapon->IsLaser)
	{
		if (auto pLaser = GameCreate<LaserDrawClass>(from, to, Weapon->LaserInnerColor,
			Weapon->LaserOuterColor,
			Weapon->LaserOuterSpread,
			//Weapon->LaserDuration
			8
			)
		)
		{
			pLaser->Thickness = 10;
			pLaser->IsHouseColor = Weapon->IsHouseColor;
		}
	}
}

void VerticalLaserClass::AI()
{
	AI(angle, count);
	angle = (angle + 4) % 360;
	radius -= radius_decrement;

	if (radius < 0)
	{
		Reset();
	}
}

void VerticalLaserClass::Clear()
{
	for (auto const& pData : Array)
	{
		GameDelete(pData);
	}

	Array.Clear();
}

void VerticalLaserClass::Draw_All()
{
	if (Array.Count <= 0)
		return;

	for (int i = Array.Count - 1; i >= 0; --i)
	{
		VerticalLaserClass* ebolt = Array[i];

		if (!ebolt)
		{
			continue;
		}

		ebolt->AI();

		if (ebolt->Expired)
		{
			Array.RemoveItem(i);
			GameDelete(ebolt);
		}
	}
}
