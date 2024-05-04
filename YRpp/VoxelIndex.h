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

struct MainVoxelIndexKey
{
	MainVoxelIndexKey() { *reinterpret_cast<int*>(this) = 0; }
	~MainVoxelIndexKey() = default;

	int Get_Index_ID() const
	{
		return *reinterpret_cast<const int*>(this);
	}

	bool Is_Valid_Key() const
	{
		return Get_Index_ID() != -1;
	}

public:
	unsigned FrameIndex : 5;
	unsigned RampType : 5;
private:
	unsigned bitfield_10 : 6;
public:
	unsigned UseAuxVoxel : 1; // !(!pUnit->Type->NoSpawnAlt || pUnit->SpawnManager->Draw_State())
private:
	unsigned bitfield_17 : 15;
};

struct TurretWeaponVoxelIndexKey
{
	TurretWeaponVoxelIndexKey() { *reinterpret_cast<int*>(this) = 0; }
	~TurretWeaponVoxelIndexKey() = default;

	int Get_Index_ID() const
	{
		return *reinterpret_cast<const int*>(this);
	}

	bool Is_Valid_Key() const
	{
		return Get_Index_ID() != -1;
	}

public:
	unsigned Facing : 5;
	unsigned HasTurretOffset : 5;
private:
	unsigned bitfield_10 : 6;
public:
	unsigned FrameIndex : 8;
	unsigned TurretWeaponIndex : 8;
};

struct ShadowVoxelIndexKey
{
public:
	unsigned Data : 32;
};

struct TurretBarrelVoxelIndexKey
{
	TurretBarrelVoxelIndexKey() { *reinterpret_cast<int*>(this) = 0; }
	~TurretBarrelVoxelIndexKey() = default;

	int Get_Index_ID() const
	{
		return *reinterpret_cast<const int*>(this);
	}

	bool Is_Valid_Key() const
	{
		return Get_Index_ID() != -1;
	}

public:
	unsigned Facing : 5;
	unsigned HasTurretOffset : 5;
private:
	unsigned bitfield_10 : 6;
public:
	unsigned FrameIndex : 8;
private:
	unsigned bitfield_24 : 8;
};

struct ReservedVoxelIndexKey
{
private:
	unsigned bitfield_1 : 10;
public:
	unsigned ReservedIndex : 6;
private:
	unsigned bitfield_16 : 16;
};

union VoxelIndexKey
{
	int Get_Index_ID() const {
		return *reinterpret_cast<const int*>(this);
	}

	bool Is_Valid_Key() const {
		return Get_Index_ID() != -1;
	}

	void Invalidate() {
		*reinterpret_cast<int*>(this) = -1;
	}

	MainVoxelIndexKey MainVoxel;
	TurretWeaponVoxelIndexKey TurretWeapon;
	ShadowVoxelIndexKey Shadow;
	TurretBarrelVoxelIndexKey TurretBarrel;
	ReservedVoxelIndexKey Reserved;
};

static_assert(sizeof(VoxelIndexKey) == sizeof(int));