#pragma once
enum class LocoType
{
	None = 0,
	Drive = 1,
	Hover = 2,
	Tunnel = 3,
	Walk = 4,
	Droppod = 5,
	Fly = 6,
	Teleport = 7,
	Mech = 8,
	Ship = 9,
	Jumpjet = 10,
	Rocket = 11
};

enum class BulletType
{
	UNKNOWN = 0,
	INVISO = 1,
	ARCING = 2,
	MISSILE = 3,
	ROCKET = 4,
	NOROT = 5,
	BOMB = 6
};

enum class SubjectToGroundType
{
	AUTO = 0,
	YES = 1,
	NO = 2
};
