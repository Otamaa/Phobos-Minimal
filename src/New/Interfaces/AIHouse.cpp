/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AIHOUSE.CPP
 *
 *  @author        CCHyper
 *
 *  @brief
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#include "AIHouse.h"
#include <HouseClass.h>
#include <Utilities/Debug.h>


HRESULT AIHouse::QueryInterface(const CLSID& riid, void** ppvObject)
{
	return S_OK;
}


ULONG AIHouse::AddRef()
{
	return S_OK;
}


ULONG AIHouse::Release()
{
	return S_OK;
}


// Retrieves the class identifier (CLSID) of the object.
/*HRESULT AIHouse::GetClassID(CLSID *lpClassID)
{
	DEBUG_INFO("Enter");
	if (lpClassID != nullptr) {
		*lpClassID = __uuidof(this);
		return S_OK;
	}
	return E_POINTER;
}*/


HRESULT AIHouse::Link_House(IHouse* house)
{
	return S_OK;
}


HRESULT AIHouse::AI(long* framedelay)
{
	return S_OK;
}


HRESULT AIMeade::Link_House(IHouse* house)
{
	House = house;

	return S_OK;
}


HRESULT AIMeade::AI(long* framedelay)
{
	Debug::Log("AIMeade::AI(enter)\n");

	if (House == nullptr)
	{
		return S_OK;
	}

	char buffer[256];
	std::snprintf(buffer, sizeof(buffer),
		"Name: %S, ID: %d\n"
		"framedelay: %d\n"
		"AvilMoney: %d, AvilStorage: %d\n"
		"PowerOutput: %d, PowerDrain: %d\n",
		House->Name(), House->ID_Number(),
		*framedelay,
		House->Available_Money(), House->Available_Storage(),
		House->Power_Output(), House->Power_Drain()
	);
	Debug::Log(buffer);

	return S_OK;
}


HRESULT AIJackson::Link_House(IHouse* house)
{
	House = house;

	return S_OK;
}


HRESULT AIJackson::AI(long* framedelay)
{
	Debug::Log("AIJackson::AI(enter)\n");

	if (House == nullptr)
	{
		return S_OK;
	}

	char buffer[256];
	std::snprintf(buffer, sizeof(buffer),
		"Name: %S, ID: %d\n"
		"framedelay: %d\n"
		"AvilMoney: %d, AvilStorage: %d\n"
		"PowerOutput: %d, PowerDrain: %d\n",
		House->Name(), House->ID_Number(),
		*framedelay,
		House->Available_Money(), House->Available_Storage(),
		House->Power_Output(), House->Power_Drain()
	);
	Debug::Log(buffer);

	return S_OK;
}


HRESULT AIGrant::Link_House(IHouse* house)
{
	House = house;

	return S_OK;
}


HRESULT AIGrant::AI(long* framedelay)
{
	Debug::Log("AIGrant::AI(enter)\n");

	if (House == nullptr)
	{
		return S_OK;
	}

	char buffer[256];
	std::snprintf(buffer, sizeof(buffer),
		"Name: %S, ID: %d\n"
		"framedelay: %d\n"
		"AvilMoney: %d, AvilStorage: %d\n"
		"PowerOutput: %d, PowerDrain: %d\n",
		House->Name(), House->ID_Number(),
		*framedelay,
		House->Available_Money(), House->Available_Storage(),
		House->Power_Output(), House->Power_Drain()
	);
	Debug::Log(buffer);

	return S_OK;
}


HRESULT AIHooker::Link_House(IHouse* house)
{
	House = house;

	return S_OK;
}


LONG AIHooker::AI(long* framedelay)
{
	Debug::Log("AIHooker::AI(enter)\n");

	if (House == nullptr)
	{
		return S_OK;
	}

	char buffer[256];
	std::snprintf(buffer, sizeof(buffer),
		"Name: %S, ID: %d\n"
		"framedelay: %d\n"
		"AvilMoney: %d, AvilStorage: %d\n"
		"PowerOutput: %d, PowerDrain: %d\n",
		House->Name(), House->ID_Number(),
		*framedelay,
		House->Available_Money(), House->Available_Storage(),
		House->Power_Output(), House->Power_Drain()
	);
	Debug::Log(buffer);

	return S_OK;
}