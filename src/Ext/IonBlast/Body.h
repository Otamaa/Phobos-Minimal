#pragma once
#include <IonBlastClass.h>
#include <Point2D.h>
#include <Utilities/Savegame.h>

class Surface;
class NOVTABLE FakeIonBlastClass final : public IonBlastClass
{
public:

	static COMPILETIMEEVAL reference<bool, 0xAA014C> IonBlastClass_inited {};
	static COMPILETIMEEVAL reference<Surface*, 0xA9FFC8, 80u> IonBlastClass_Surfaces {};
	static COMPILETIMEEVAL reference<int, 0xA9FAE8, 289u> ionblast_A9FAE8 {};
	static COMPILETIMEEVAL reference<int, 0xAA0150> IonBlastPitch {};

	COMPILETIMEEVAL static Point2D __fastcall IonBlastData_53D8E0(int number)
	{
		int v2 = number - 1;
		Point2D result {};
		int v4 = 1;

		if (v2 >= 8)
		{
			for (int i = 8; i <= v2; i += 8)
			{
				v2 -= i;
				++v4;
			}
		}

		if (v2 >= 2 * v4 + 1)
		{
			if (v2 >= 4 * v4 + 1)
			{
				if (v2 >= 6 * v4 + 1)
				{
					result.Y = -v4;
					result.X = v2 - 7 * v4;
				}
				else
				{
					result.Y = 5 * v4 - v2;
					result.X = -v4;
				}
			}
			else
			{
				result.X = 3 * v4 - v2;
				result.Y = v4;
			}
		}
		else
		{
			result.X = v4;
			result.Y = v2 - v4;
		}

		return result;
	}

	COMPILETIMEEVAL static int __stdcall IonBlastData_53D960(int a1, int a2)
	{
		// 0x53D96A: test esi,esi / 0x53D96E: test edi,edi — early-out for origin
		if (a1 == 0 && a2 == 0)
			return 0;

		// CDQ/XOR/SUB abs pattern on both args
		const int absA2 = Math::abs(a2); // ECX
		const int absA1 = Math::abs(a1); // EAX
		const int chebyshevMax = (absA1 > absA2) ? absA1 : absA2; // ECX = max

		// Ring base accumulator: EAX=1, loop EAX += EDX*8 while EDX < ECX
		int ringBase = 1;
		for (int i = 1; i < chebyshevMax; ++i)
			ringBase += 8 * i;

		// loc_53D9A2 — a1 == chebyshevMax  (ESI == ECX)
		// add edi,ecx / add eax,edi
		if (a1 == chebyshevMax)
			return chebyshevMax + a2 + ringBase;

		// loc_53D9AF — a2 == chebyshevMax  (EDI == ECX)
		// lea ecx,[ecx+ecx*2] / sub ecx,esi / add eax,ecx
		if (a2 == chebyshevMax)
			return 3 * chebyshevMax - a1 + ringBase;

		// loc_53D9BF — a1 == -chebyshevMax  (cmp ESI, neg ECX)
		// lea ecx,[ecx+ecx*4] / sub ecx,edi / add eax,ecx
		if (a1 == -chebyshevMax)
			return 5 * chebyshevMax - a2 + ringBase;

		// loc_53D9D3 — default
		// lea edx,ds:0[ecx*8] / sub edx,ecx / add edx,esi / add eax,edx
		return 7 * chebyshevMax + a1 + ringBase;
	}

	static void __fastcall DestroySurfaces();
	static void __fastcall InitOneTime();
	static void __fastcall _DrawAll();

	void _AI();
	void _Draw();

	bool Load(PhobosStreamReader& Stm)
	{
		return 
			Stm
			.Process(Location)
			.Process(Lifetime)
			.Process(DisableIonBeam)
			;
	}

	bool Save(PhobosStreamWriter& Stm)
	{
		return
			Stm
			.Process(Location)
			.Process(Lifetime)
			.Process(DisableIonBeam)
			;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static void Clear();
};
static_assert(sizeof(FakeIonBlastClass) == sizeof(IonBlastClass), "Size Missmatch !");