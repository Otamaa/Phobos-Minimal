#pragma once

#include <unknwn.h>

DECLARE_INTERFACE_IID_(IPowerEvents, IUnknown, "56272740-89BB-11D1-B707-00A024DDAFD1")
{
	/**
	 *  Triggered when power first becomes sufficient.
	 */
	STDMETHOD_(LONG, Power_Activated)() PURE;

	/**
	 *  Triggered when power first becomes insufficient.
	 */
	STDMETHOD_(LONG, Power_Lost)() PURE;
};
