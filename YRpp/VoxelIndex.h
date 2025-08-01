#pragma once
#include <FileFormats/VXL.h>

struct VoxelCacheStruct
{
	short X;
	short Y;
	unsigned short Width;
	unsigned short Height;
	void* Buffer;
};

/*
----------------------------------------------------------------------------------------------------
> Jul 13, 2025 - CrimRecya
> How does WW generate a VoxelIndexKey :
----------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------
> Unit:
>
> 0x73B59D :  key = Game::bVPLRead - 1;
> ···· ····  ···· ····  ···· ····  ···· ····
>
> 0x73B5CB :  loco->Draw_Matrix(&mtx, &key);
> Drive / Ship / Teleport / Hover(!Powered) / Jumpjet(!State) / Tunnel(!State) ···· ····  ···· ····  ···· ·rrr  rrrb bbbb
> Hover(Powered) / Jumpjet(State) / DropPod / Mech / Walk                    ···· ····  ···· ····  ···· ·000  000b bbbb
> Tunnel(State)                                                      ···· ····  ···· ····  ···b bbbb  00dd dddd
> Fly                                                                ···· ····  ···· ····  ···· ····  bbbb bplr
> Rocket                                                             ···· ····  ···· ····  ···· ····  ·fib bbbb
>
> 0x73B607 :  key = (((this->WalkedFramesSoFar % Type->MainVoxel.HVA->FrameCount) & 0x1F) | (key << 5));
> Drive / Ship / Teleport / Hover(!Powered) / Jumpjet(!State) / Tunnel(!State) ···· ····  ···· ····  rrrr  rrbb  bbbf ffff
> Hover(Powered) / Jumpjet(State) / DropPod / Mech / Walk                    ···· ····  ···· ····  0000  00bb  bbbf ffff
> Tunnel(State)                                                      ···· ····  ···· ··bb  bbb0  0ddd  dddf ffff
> Fly                                                                ···· ····  ···· ····  ···b  bbbb  plrf ffff
> Rocket                                                             ···· ····  ···· ····  ····  fibb  bbbf ffff
>
> 0x73B6AA :  key = key | (1 << 16);  (if (this->Type->NoSpawnAlt && !this->SpawnManager->CountDockedSpawns()))
> Drive / Ship / Teleport / Hover(!Powered) / Jumpjet(!State) / Tunnel(!State) ···· ····  ···· ···a  rrrr  rrbb  bbbf ffff
> Hover(Powered) / Jumpjet(State) / DropPod / Mech / Walk                    ···· ····  ···· ···a  0000  00bb  bbbf ffff
> Tunnel(State)                                                      ···· ····  ···· ··b ? bbb0  0ddd  dddf ffff
> Fly                                                                ···· ····  ···· ···a  ···b  bbbb  plrf ffff
> Rocket                                                             ···· ····  ···· ···a  ····  fibb  bbbf ffff
>
> Obtain main result
>
> Draw main
>
> 0x73B778 :  key = (this->SecondaryFacing.Current().GetFacing<32>() | (key & ~0x1F));
> Drive / Ship / Teleport / Hover(!Powered) / Jumpjet(!State) / Tunnel(!State) ···· ····  ···· ····  rrrr  rrbb  bbbt tttt
> Hover(Powered) / Jumpjet(State) / DropPod / Mech / Walk                    ···· ····  ···· ····  0000  00bb  bbbt tttt
> Tunnel(State)                                                      ···· ····  ···· ··bb  bbb0  0ddd  dddt tttt
> Fly                                                                ···· ····  ···· ····  ···b  bbbb  plrt tttt
> Rocket                                                             ···· ····  ···· ····  ····  fibb  bbbt tttt
>
> 0x73B78A :  key = key & ~0x3E0;  (if (!this->Type->TurretOffset))
> Drive / Ship / Teleport / Hover(!Powered) / Jumpjet(!State) / Tunnel(!State) ···· ····  ···· ····  rrrr  rr00  000t tttt
> Hover(Powered) / Jumpjet(State) / DropPod / Mech / Walk                    ···· ····  ···· ····  0000  0000  000t tttt
> Tunnel(State)                                                      ···· ····  ···· ··bb  bbb0  0d ? ? ? ? ? t tttt
> Fly                                                                ···· ····  ···· ····  ···b  bb ? ? ? ? ? t tttt
> Rocket                                                             ···· ····  ···· ····  ····  fi00  000t tttt
>
> 0x73B79F :  key = key | ((this->TurretAnimFrame % Type->TurretVoxel.HVA->FrameCount) << 16);
> Drive / Ship / Teleport / Hover(!Powered) / Jumpjet(!State) / Tunnel(!State) ···· ····  iiii iiii  rrrr  rrbb  bbbt tttt
> Hover(Powered) / Jumpjet(State) / DropPod / Mech / Walk                    ···· ····  iiii iiii  0000  00bb  bbbt tttt
> Tunnel(State)                                                      ···· ····  iiii ii ? ? bbb0  0ddd  dddt tttt
> Fly                                                                ···· ····  iiii iiii  ···b  bbbb  plrt tttt
> Rocket                                                             ···· ····  iiii iiii  ····  fibb  bbbt tttt
>
> Draw barrel if no turret
>
> 0x73B917 :  key = key | (this->CurrentTurretNumber << 24);
> Drive / Ship / Teleport / Hover(!Powered) / Jumpjet(!State) / Tunnel(!State) cccc cccc  iiii iiii  rrrr  rrbb  bbbt tttt
> Hover(Powered) / Jumpjet(State) / DropPod / Mech / Walk                    cccc cccc  iiii iiii  0000  00bb  bbbt tttt
> Tunnel(State)                                                      cccc cccc  iiii ii ? ? bbb0  0ddd  dddt tttt
> Fly                                                                cccc cccc  iiii iiii  ···b  bbbb  plrt tttt
> Rocket                                                             cccc cccc  iiii iiii  ····  fibb  bbbt tttt
>
> Obtain minor result
>
> Draw minor
>
> ... Then shadow ...
----------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------
> Aircraft:
>
> 0x41470F :  key = 0;
> ···· ····  ···· ····  ···· ····  ···· ····
>
> ... Draw Shadow ...
>
> 0x414943 :  key = ((this->WalkedFramesSoFar % Type->MainVoxel.HVA->FrameCount) & 0x1F);
> ···· ····  ···· ····  ···· ····  ···f ffff
>
> 0x414969 :  loco->Draw_Matrix(&mtx, &key);
> Drive / Ship / Teleport / Hover(!Powered) / Jumpjet(!State) / Tunnel(!State) ···· ····  ···· ····  ffff frrr  rrrb bbbb
> Hover(Powered) / Jumpjet(State) / DropPod / Mech / Walk                    ···· ····  ···· ····  ffff f000  000b bbbb
> Tunnel(State)                                                      ···· ····  ···· ··ff  fffb bbbb  00dd dddd
> Fly                                                                ···· ····  ···· ····  ···· ····  bbb ? ? ? ? ?
> Rocket                                                             ···· ····  ···· ····  ···· ····  ·fi ? ? ? ? ?
>
> Obtain main result
>
> Draw main
----------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------
> Draw_Matrix:
>
> Drive / Ship / Teleport / Hover / Jumpjet / Tunnel(!State) / DropPod / Mech / Walk: <<= 6, +=, <<= 5, |=
>
> Tunnel(State) <<= 5, |=, <<= 8, |=
>
> Fly / Rocket:                                                               |=, |= (No shift)
>
> Return
----------------------------------------------------------------------------------------------------
*/

struct MainVoxelIndexKey
{
	/*
		pUnit->WalkedFramesSoFar % pType->MainVoxel.HVA->FrameCount
	*/
	unsigned FrameIndex : 5;

	/*
		pUnit->PrimaryFacing.Current().GetFacing<32>()
	*/
	unsigned BodyFacing : 5;

	/*
		Drive : Loco->Ramp
		Ship : Loco->Ramp
		Hover (!loco->Is_Powered()): pUnit->GetCell()->SlopeIndex
		Jumpjet (!loco->State): pUnit->GetCell()->SlopeIndex
		Tunnel (!loco->State): pUnit->GetCell()->SlopeIndex
		Teleport (Fixed) : pUnit->GetCell()->SlopeIndex
		DropPod / Mech / Walk : Not set
	*/
	unsigned RampType : 6;

	/*
		pUnit->Type->NoSpawnAlt && !pUnit->SpawnManager->CountDockedSpawns()
	*/
	unsigned AuxVoxel : 1;

	/*
		No use
	*/
	unsigned Reserved : 15;
};

struct MinorVoxelIndexKey
{
	/*
		pUnit->SecondaryFacing.Current().GetFacing<32>()
	*/
	unsigned TurretFacing : 5;

	/*
		Inherit from MainVoxelIndexKey

		Reset to 0 if TurretOffset=0
	*/
	unsigned BodyFacing : 5;

	/*
		Inherit from MainVoxelIndexKey
	*/
	unsigned RampType : 6;

	/*
		pUnit->TurretAnimFrame % pType->TurretVoxel.HVA->FrameCount

		Not set if have MainVoxelIndexKey.FrameIndex
	*/
	unsigned TurretFrameIndex : 8;

	/*
		pUnit->CurrentTurretNumber

		Not set if no turret but have barrel
	*/
	unsigned TurretWeaponIndex : 8;
};

struct AircraftVoxelIndexKey
{
	unsigned BodyFacing : 5;
	unsigned RampType : 6;
	unsigned FrameIndex : 5;
	unsigned Reserved : 16;
};

struct FlyVoxelIndexKey
{
	unsigned FrameIndex : 5;
	unsigned RollRight : 1;
	unsigned RollLeft : 1;
	unsigned PitchState : 1;
	unsigned BodyFacing : 5;
	unsigned Reserved : 19;
};

struct RocketVoxelIndexKey
{
	unsigned FrameIndex : 5;
	unsigned BodyFacing : 5;
	unsigned InitialPitch : 1;
	unsigned FinalPitch : 1;
	unsigned Reserved : 20;
};

struct TunnelVoxelIndexKey
{
	unsigned FrameIndex : 5;
	unsigned DigState : 6;
	unsigned Empty : 2;
	unsigned BodyFacing : 5;
	unsigned Reserved : 14;
};

struct ShadowVoxelIndexKey
{
public:
	unsigned Data : 32;
};

struct ReservedVoxelIndexKey
{
private:
	unsigned bitfield_0 : 10;
public:
	unsigned ReservedIndex : 6;
private:
	unsigned bitfield_16 : 16;
};

union VoxelIndexKey
{
	constexpr VoxelIndexKey(int val = 0) noexcept
	{
		Value = val;
	}

	constexpr operator int() const
	{
		return Value;
	}

	constexpr bool Is_Valid_Key() const
	{
		return Value != -1;
	}

	constexpr void Invalidate()
	{
		Value = -1;
	}

	MainVoxelIndexKey MainVoxel;  // (Unit) For body to use
	MinorVoxelIndexKey MinorVoxel;  // (Unit) For turret/barrel to use
	AircraftVoxelIndexKey AircraftVoxel;
	FlyVoxelIndexKey FlyVoxel; // (Unit)
	RocketVoxelIndexKey RocketVoxel; // (Unit)
	TunnelVoxelIndexKey UndergroundVoxel; // (Unit) loco->State != 0
	ShadowVoxelIndexKey Shadow;
	ReservedVoxelIndexKey Reserved;
	int Value;
};

static_assert(sizeof(VoxelIndexKey) == sizeof(int));
