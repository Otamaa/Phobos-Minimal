#pragma once

#include <Locomotor/LocomotionClass.h>
#include <Phobos.version.h>
#include <vector>

struct LocoIdent
{
	wchar_t* w_name;
	const char* s_name;
	wchar_t* w_CLSID;
	const char* s_CLSID;
};

#define DEFINE_PIGGYLOCO(name , clsid) \
static constexpr LocoIdent name##_data = {_WSTR_(name) ,_STR(name) , _WSTR_(clsid) , _STR(clsid)}; \
class DECLSPEC_UUID(_STR(clsid)) ##name##LocomotionClass : public LocomotionClass, public IPiggyback

#define DEFINE_LOCO(name , clsid) \
static constexpr LocoIdent name##_data = {_WSTR_(name) ,_STR(name) , _WSTR_(clsid) , _STR(clsid)}; \
class DECLSPEC_UUID(_STR(clsid)) ##name##LocomotionClass : public LocomotionClass