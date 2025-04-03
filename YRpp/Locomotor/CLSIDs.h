#pragma once

#include <Helpers/CompileTime.h>

class CLSIDs
{
public:
#define LOCO_CLSID(_name,_addrs) \
	static COMPILETIMEEVAL reference<CLSID const, _addrs> const _name {};

	LOCO_CLSID(Drive, 0x7E9A30u)
	LOCO_CLSID(Jumpjet, 0x7E9AC0u)
	LOCO_CLSID(Hover, 0x7E9A40u)
	LOCO_CLSID(Rocket, 0x7E9AD0u)
	LOCO_CLSID(Tunnel, 0x7E9A50u)
	LOCO_CLSID(Walk, 0x7E9A60u)
	LOCO_CLSID(DropPod, 0x7E9A70u)
	LOCO_CLSID(Fly, 0x7E9A80u)
	LOCO_CLSID(Teleport, 0x7E9A90u)
	LOCO_CLSID(Mech, 0x7E9AA0u)
	LOCO_CLSID(Ship, 0x7E9AB0u)

#undef LOCO_CLSID

};
