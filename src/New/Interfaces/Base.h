#pragma once

#include <Locomotor/LocomotionClass.h>

#define DEFINE_PIGGYLOCO(name , clsid) \
	static COMPILETIMEEVAL LocoIdent name##_data = { L#name, #name, L#clsid, #clsid }; \
    class DECLSPEC_UUID(#clsid) name##LocomotionClass : public LocomotionClass, public IPiggyback

#define DEFINE_LOCO(name, clsid) \
    static COMPILETIMEEVAL LocoIdent name##_data = { L#name, #name, L#clsid, #clsid }; \
    class DECLSPEC_UUID(#clsid) name##LocomotionClass : public LocomotionClass