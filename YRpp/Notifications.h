#pragma once

#include <ArrayClasses.h>
#include <Helpers/CompileTime.h>

class AbstractClass;

// encapsulates a bunch of vectors that are used for announcing invalid pointers.
// if an AbstractClass is contained in an list it will be notified through
// PointerExpired whenever an object of that type expires.
class PointerExpiredNotification {
public:
	static COMPILETIMEEVAL reference<PointerExpiredNotification, 0xB0F720u> const NotifyInvalidObject{}; // Object class hierarchy
	static COMPILETIMEEVAL reference<PointerExpiredNotification, 0xB0F670u> const NotifyInvalidType{}; // AbstractType class hierarchy
	static COMPILETIMEEVAL reference<PointerExpiredNotification, 0xB0F5B8u> const NotifyInvalidAnim{}; // AnimClass
	static COMPILETIMEEVAL reference<PointerExpiredNotification, 0xB0F6C8u> const NotifyInvalidHouse{}; // HouseClass
	static COMPILETIMEEVAL reference<PointerExpiredNotification, 0xB0F618u> const NotifyInvalidTag{}; // TagClass
	static COMPILETIMEEVAL reference<PointerExpiredNotification, 0xB0F708u> const NotifyInvalidTrigger{}; // TriggerClass
	static COMPILETIMEEVAL reference<PointerExpiredNotification, 0xB0F640u> const NotifyInvalidFactory{}; // FactoryClass
	static COMPILETIMEEVAL reference<PointerExpiredNotification, 0xB0F5F0u> const NotifyInvalidWaypoint{}; // WaypointClass
	static COMPILETIMEEVAL reference<PointerExpiredNotification, 0xB0F5D8u> const NotifyInvalidTeam{}; // TeamClass
	static COMPILETIMEEVAL reference<PointerExpiredNotification, 0xB0F6F0u> const NotifyInvalidNeuron{}; // NeuronClass
	static COMPILETIMEEVAL reference<PointerExpiredNotification, 0xB0F658u> const NotifyInvalidActionOrEvent{}; // ActionClass and EventClass

	OPTIONALINLINE bool Add(AbstractClass* object) {
		// add only if doesn't exist
		return this->Array.AddUnique(object);
	}

	OPTIONALINLINE bool Remove(AbstractClass* object) {
		return this->Array.Remove(object);
	}

	DynamicVectorClass<AbstractClass*> Array;
};
