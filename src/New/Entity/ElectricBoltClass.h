
#pragma once

#include <CoordStruct.h>
#include <ColorStruct.h>
#include <GeneralStructures.h>
#include <ScenarioClass.h>

#include <Unsorted.h>
#include <vector>

constexpr auto EBOLT_DEFAULT_DEVIATION  = 1.0f;
constexpr auto  EBOLT_DEFAULT_INTERATIONS = 1;
constexpr auto EBOLT_DEFAULT_LINE_SEGEMENTS = 8;
constexpr auto EBOLT_DEFAULT_SEGMENT_LINES = 3;
constexpr auto EBOLT_DEFAULT_LIFETIME = 17;
constexpr auto EBOLT_MAX_LIFETIME = 60;

struct LineDrawDataStruct
{
	CoordStruct Start;
	CoordStruct End;
	ColorStruct Color;
	int StartZ;
	int EndZ;

	bool operator==(const LineDrawDataStruct& that) const { return std::memcmp(this, &that, sizeof(LineDrawDataStruct)) == 0; }
	bool operator!=(const LineDrawDataStruct& that) const { return std::memcmp(this, &that, sizeof(LineDrawDataStruct)) != 0; }
};

class ParticleSystemTypeClass;
class ElectricBoltClass
{
public:
	ElectricBoltClass() :
		StartCoord {},
		EndCoord {},
		ZAdjust {0},
		Deviation { EBOLT_DEFAULT_DEVIATION },
		Lifetime { EBOLT_DEFAULT_LIFETIME },
		IterationCount { EBOLT_DEFAULT_INTERATIONS },
		LineColor1 { 255,255,255 },
		LineColor2 { 82,81,255 },
		LineColor3 { 82,81,255 },
		LineSegmentCount { EBOLT_DEFAULT_LINE_SEGEMENTS },
		LineDrawList {},
		DrawFrame {-1}
	{ }

	~ElectricBoltClass() {
		Clear();
	}

	void Draw_It();
	void Create(CoordStruct& start, CoordStruct& end, int z_adjust, ParticleSystemTypeClass* pSys = nullptr, bool particleSysCoordFlip = false);

	void Flag_To_Delete() { Lifetime = 0; }

	static inline auto Distance(const CoordStruct& coord1, const CoordStruct& coord2)
	{
		 CoordStruct coord = coord1 - coord2;
		 return Game::F2I(Math::sqrt(
			 static_cast<double>(coord.X) * static_cast<double>(coord.X) +
			 static_cast<double>(coord.Y) * static_cast<double>(coord.Y) +
			 static_cast<double>(coord.Z) * static_cast<double>(coord.Z)
		 ));
	}

	static inline int Sim_Random_Pick(int a, int b) {
		return Random2Class::NonCriticalRandomNumber()(a, b);
	}


private:
	void Clear();

	void Add_Plot_Line(CoordStruct& start, CoordStruct& end, ColorStruct& line_color, int start_z, int end_z) {
		LineDrawList.emplace_back(LineDrawDataStruct { start, end, line_color, start_z, end_z });
	}

	void Plot_Bolt(CoordStruct& start, CoordStruct& end);
	void Draw_Bolts();

public:
	/**
	 *  The start coordinate for this electric bolt.
	 */
	CoordStruct StartCoord;

	/**
	 *  The end coordinate for this electric bolt.
	 */
	CoordStruct EndCoord;

	/**
	 *  The initial z draw adjustment value.
	 */
	int ZAdjust;

	/**
	 *  The deviation distance. The higher this value is, the more "wild"
	 *  in variation the bolts will appear.
	 */
	float Deviation;

	/**
	 *  The lifetime that this electric bolt should stay around for.
	 */
	int Lifetime;

	/**
	 *  How many plot and draw iterations should we perform?
	 */
	int IterationCount;

	/**
	 *  Line segment colors, copied from the firing object's weapon on creation.
	 */
	ColorStruct LineColor1;
	ColorStruct LineColor2;
	ColorStruct LineColor3;

	/**
	 *  How many segment blocks this electric bolt is made up from.
	 */
	int LineSegmentCount;

	/**
	 *  The list of pending lines to draw.
	 */

	std::vector<LineDrawDataStruct> LineDrawList;

	/**
	 *  The frame in which we should draw on. This helps clamp the drawing
	 *  to the games internal frame tick.
	 */
	int DrawFrame;

};

struct ElectricBoltManager
{
	static std::vector<ElectricBoltClass*> ElectricBoltArray;

	static void Draw_All();
	static void Clear_All();
};
