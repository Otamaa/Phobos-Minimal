/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include "Main.h"
#include <Utilities/Macro.h>

DEFINE_HOOK(0x692DD8, ScrollClass_DisableEdgeScrolling1, 0x7)
{
	return SpawnerMain::GetMainConfigs()->DisableEdgeScrolling
		? 0x692E07
		: 0;
}

DEFINE_HOOK(0x692DFA, ScrollClass_DisableEdgeScrolling2, 0x5)
{
	return SpawnerMain::GetMainConfigs()->DisableEdgeScrolling
		? 0x692EA2
		: 0;
}

DEFINE_HOOK(0x692E34, ScrollClass_DisableEdgeScrolling3, 0x6)
{
	return SpawnerMain::GetMainConfigs()->DisableEdgeScrolling
		? 0x692E40
		: 0;
}
