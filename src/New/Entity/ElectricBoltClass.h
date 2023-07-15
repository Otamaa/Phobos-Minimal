
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

struct BoltData
{
	int count;
	std::vector<ColorStruct> ColorData;
	std::vector<bool> Disabled;
};

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

static constexpr ColorStruct DefaultColor[EBOLT_DEFAULT_SEGMENT_LINES] {
	{ 255,255,255 } , { 82,81,255 } , { 82,81,255 }

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
		LineSegmentCount { EBOLT_DEFAULT_LINE_SEGEMENTS },
		LineDrawList {},
		DrawFrame {-1},
		Random { 0 },
		Data {}
	{ 
		Data.count = EBOLT_DEFAULT_SEGMENT_LINES;
		for (int i = 0; i < EBOLT_DEFAULT_SEGMENT_LINES; ++i)
			Data.ColorData.push_back(DefaultColor[i]);
	}

	ElectricBoltClass(CoordStruct const& start, CoordStruct const& end, const BoltData& nData , int z_adjust) :
		StartCoord { start },
		EndCoord { end },
		ZAdjust { z_adjust },
		Deviation { EBOLT_DEFAULT_DEVIATION },
		Lifetime { EBOLT_DEFAULT_LIFETIME },
		IterationCount { EBOLT_DEFAULT_INTERATIONS },
		LineSegmentCount { EBOLT_DEFAULT_LINE_SEGEMENTS },
		LineDrawList {},
		DrawFrame { -1 }, 
		Random { 0 },
		Data { nData }
	{ }

	ElectricBoltClass
		(CoordStruct const& start, 
		 CoordStruct const& end, 
		 ColorStruct const& col1, 
		 ColorStruct const& col2, 
		 ColorStruct const& col3, 
		 bool col1_disable,
		 bool col2_disable,
		 bool col3_disable,
		 int z_adjust
		) :
		StartCoord { start },
		EndCoord { end },
		ZAdjust { z_adjust },
		Deviation { EBOLT_DEFAULT_DEVIATION },
		Lifetime { EBOLT_DEFAULT_LIFETIME },
		IterationCount { EBOLT_DEFAULT_INTERATIONS },
		LineSegmentCount { EBOLT_DEFAULT_LINE_SEGEMENTS },
		LineDrawList {},
		DrawFrame { -1 },
		Random { 0 },
		Data {}
	{
		Data.count = EBOLT_DEFAULT_SEGMENT_LINES;
		Data.ColorData.resize(EBOLT_DEFAULT_SEGMENT_LINES);
		Data.Disabled.resize(EBOLT_DEFAULT_SEGMENT_LINES);
		Data.ColorData[0] = col1;
		Data.ColorData[1] = col2;
		Data.ColorData[2] = col3;
		Data.Disabled[0] = col1_disable;
		Data.Disabled[1] = col2_disable;
		Data.Disabled[2] = col3_disable;
	}

	~ElectricBoltClass() {
		Clear();
	}

	void Draw_It();
	static void Create(CoordStruct const& start, CoordStruct const& end, 
		ColorStruct const& col1, ColorStruct const& col2, ColorStruct const& col3, 
		bool col1_disable = false,  bool col2_disable = false, bool col3_disable = false,
		int z_adjust = 0, ParticleSystemTypeClass* pSys = nullptr, bool particleSysCoordFlip = false);
	static void Create(CoordStruct const& start, CoordStruct const& end, const BoltData& nData, int z_adjust, ParticleSystemTypeClass* pSys = nullptr, bool particleSysCoordFlip = false);

	void Flag_To_Delete() { Lifetime = 0; }

	static inline auto Distance(const CoordStruct& coord1, const CoordStruct& coord2)
	{
		 CoordStruct coord = coord1 - coord2;
		 return int(Math::sqrt(
			 static_cast<double>(coord.X) * static_cast<double>(coord.X) +
			 static_cast<double>(coord.Y) * static_cast<double>(coord.Y) +
			 static_cast<double>(coord.Z) * static_cast<double>(coord.Z)
		 ));
	}

	static inline int Sim_Random_Pick(int a, int b) {
		return Random2Class::NonCriticalRandomNumber()(a, b);
	}

	ElectricBoltClass(const ElectricBoltClass& other) = default;
	ElectricBoltClass& operator=(const ElectricBoltClass& other) = default;

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

	int Random;

	BoltData Data;
};

struct ElectricBoltManager
{
	static std::vector<ElectricBoltClass> ElectricBoltArray;

	static void Draw_All();
	static void Clear();
};
