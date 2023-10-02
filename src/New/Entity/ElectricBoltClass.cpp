#include "ElectricBoltClass.h"

#include <Base/Always.h>
#include <Utilities/TemplateDef.h>
#include <TacticalClass.h>
#include <Drawing.h>
#include <Memory.h>
#include <Unsorted.h>
#include <RulesClass.h>

std::vector<ElectricBoltClass> ElectricBoltManager::ElectricBoltArray;

void ElectricBoltClass::Clear()
{
	LineDrawList.clear();
}

void ElectricBoltClass::Draw_It()
{
	if (DrawFrame == Unsorted::CurrentFrame) {
		/**
		 *  This is our draw frame, so draw!
		 */
		if (LineDrawList.size()) {
			Draw_Bolts();
		}

	} else {

		/**
		 *  Clear previous lines, we are about to plot a new set.
		 */
		LineDrawList.clear();

		for (int i = 0; i < IterationCount; ++i) {
			if (Lifetime) {
				Point2D pixel_start {};
				Point2D pixel_end {};

				TacticalClass::Instance->CoordsToClient(&StartCoord,&pixel_start);
				TacticalClass::Instance->CoordsToClient(&EndCoord,&pixel_end);

				if (Game::Clip_Line(&pixel_start, &pixel_end, &Drawing::SurfaceDimensions_Hidden()))
					Plot_Bolt(StartCoord, EndCoord);
			}
		}

		/**
		 *  Draw the initial set of lines.
		 */
		if (LineDrawList.size()) {
			Draw_Bolts();

			++Random;
		}

		/**
		 *  Update the lifetime.
		 */
		--Lifetime;

		DrawFrame = Unsorted::CurrentFrame;
	}
}

void ElectricBoltClass::Create(CoordStruct const& start, CoordStruct const& end, const BoltData& nData, int z_adjust, ParticleSystemTypeClass* pSys , bool particleSysCoordFlip)
{

	ElectricBoltManager::ElectricBoltArray.emplace_back(start, end, nData, z_adjust);

	/**
	*  Spawn a spark particle at the destination of the electric bolt.
	*/
	if (pSys)
	{
		auto nLoc = particleSysCoordFlip ? start : end;
		if (pSys->BehavesLike == BehavesLike::Smoke)
			nLoc.Z += 100;

		GameCreate<ParticleSystemClass>(pSys, nLoc);
	}
}

void ElectricBoltClass::Create(CoordStruct const& start, CoordStruct const& end,
	ColorStruct const& col1, ColorStruct const& col2, ColorStruct const& col3,
	bool col1_disable , bool col2_disable , bool col3_disable ,
	int z_adjust ,ParticleSystemTypeClass* pSys , bool particleSysCoordFlip)
{
	ElectricBoltManager::ElectricBoltArray.emplace_back(start, end, col1, col2, col3 ,
		col1_disable , col2_disable , col3_disable , z_adjust);

	/**
	 *  Spawn a spark particle at the destination of the electric bolt.
	 */
	if(pSys){
		auto nLoc = particleSysCoordFlip ? start : end;
		if(pSys->BehavesLike == BehavesLike::Smoke)
			nLoc.Z += 100;

		GameCreate<ParticleSystemClass>(pSys, nLoc);
	}
}

void ElectricBoltClass::Plot_Bolt(CoordStruct& start, CoordStruct& end)
{
	struct EBoltPlotStruct
	{
		std::vector<CoordStruct> StartCoords;
		std::vector<CoordStruct> EndCoords;
		int Distance;
		int Deviation;
		int StartZ;
		int EndZ;

		EBoltPlotStruct(int count) :
			StartCoords { size_t(count) ,CoordStruct::Empty}
			, EndCoords { size_t(count) ,CoordStruct::Empty}
			, Distance {}
			, Deviation {}
			, StartZ {}
			, EndZ {}
		{ }

		EBoltPlotStruct() = default;
		~EBoltPlotStruct() = default;

		bool operator==(const EBoltPlotStruct& that) const { return std::memcmp(this, &that, sizeof(EBoltPlotStruct)) == 0; }
		bool operator!=(const EBoltPlotStruct& that) const { return std::memcmp(this, &that, sizeof(EBoltPlotStruct)) != 0; }
	};

	int BoltCount = Data.count;
	int SEGEMENT_COORDS_SIZE = sizeof(CoordStruct) * BoltCount;

	/**
	 *  Check to make sure there is actual distance between the two coords.
	 */

	if (int distance = Distance(start, end))
	{
		std::vector<EBoltPlotStruct> ebolt_plots(BoltCount, EBoltPlotStruct(BoltCount));
		std::vector<CoordStruct> start_coords(BoltCount, start);
		std::vector<CoordStruct> end_coords(BoltCount, end);
		std::vector<CoordStruct> working_coords(BoltCount, CoordStruct::Empty);

		int deviation_values[6] {};

		bool init_deviation_values = true;
		int plot_index = 0;

		int line_start_z = ZAdjust;
		int line_end_z = 0;
		int dist_a = (102 * distance / Unsorted::LeptonsPerCell);

		/**
		 *  Max distance from line center, with "Deviation" as delta.
		 */
		int desired_deviation = 23;
		int line_deviation = static_cast<int>((desired_deviation * Deviation) * distance / Unsorted::LeptonsPerCell);

		while (true)
		{

			while (distance > (Unsorted::LeptonsPerCell / 4) && plot_index < static_cast<int>(ebolt_plots.size()))
			{

				for (int i = 0; i < BoltCount; ++i)
				{
					working_coords[i].X = (end_coords[i].X + start_coords[i].X) / 2;
					working_coords[i].Y = (end_coords[i].Y + start_coords[i].Y) / 2;
					working_coords[i].Z = (end_coords[i].Z + start_coords[i].Z) / 2;
				}

				/**
				 *  Initialises the line deviation values.
				 */
				if (init_deviation_values)
				{

					for (int i = 0; i < ARRAY_SIZE(deviation_values); ++i)
					{
						deviation_values[i] = static_cast<int>(Math::sin(static_cast<double>(this->Random * Math::Pi / static_cast<double>(i + 7)) * static_cast<double>(line_deviation)));
					}

					for (int i = 0; i < BoltCount; ++i)
					{
						working_coords[i].X += deviation_values[0] + deviation_values[3];
						working_coords[i].Y += deviation_values[1] + deviation_values[5];
						working_coords[i].Z += (deviation_values[2] + deviation_values[4] + 2 * line_deviation) / 2;
					}

					init_deviation_values = false;
				}

				if (distance <= (Unsorted::LeptonsPerCell / 2))
				{
					working_coords[0].X += 2 * line_deviation * Sim_Random_Pick(-1, 1);
					working_coords[0].Y += 2 * line_deviation * Sim_Random_Pick(-1, 1);
					working_coords[0].Z += 2 * line_deviation * Sim_Random_Pick(-1, 1);
				}
				else
				{
					working_coords[0].X += Sim_Random_Pick(-line_deviation, line_deviation);
					working_coords[0].Y += Sim_Random_Pick(-line_deviation, line_deviation);
					working_coords[0].Z += Sim_Random_Pick(-line_deviation, line_deviation);
				}

				if (distance > dist_a)
				{
					for (int i = 1; i < BoltCount; ++i)
					{
						working_coords[i].X = working_coords[0].X + (Sim_Random_Pick(-line_deviation, line_deviation) / 2);
						working_coords[i].Y = working_coords[0].Y + (Sim_Random_Pick(-line_deviation, line_deviation) / 2);
						working_coords[i].Z = working_coords[0].Z + (Sim_Random_Pick(-line_deviation, line_deviation) / 2);
					}
				}
				else
				{
					for (int i = 1; i < BoltCount; ++i)
					{
						working_coords[i].X += Sim_Random_Pick(-line_deviation, line_deviation);
						working_coords[i].Y += Sim_Random_Pick(-line_deviation, line_deviation);
						working_coords[i].Z += Sim_Random_Pick(-line_deviation, line_deviation);
					}
				}

				line_deviation /= 2;
				distance /= 2;

				EBoltPlotStruct& plot = ebolt_plots[plot_index];

				std::memcpy(plot.StartCoords.data(), working_coords.data(), SEGEMENT_COORDS_SIZE);
				std::memcpy(plot.EndCoords.data(), end_coords.data(), SEGEMENT_COORDS_SIZE);
				std::memcpy(end_coords.data(), working_coords.data(), SEGEMENT_COORDS_SIZE);

				plot.Distance = distance;
				plot.Deviation = line_deviation;
				plot.StartZ = (line_end_z + line_start_z) / 2;
				plot.EndZ = line_end_z;

				line_end_z = (line_end_z + line_start_z) / 2;

				++plot_index;
			}

			/**
			 *  Add the line segments to the draw list.
			 */

			// plot-line was in 1 - 2 - 0 , order
			// there is may error on disable tag
			for(int i = 0; i < BoltCount; ++i){
				if(!Data.Disabled[i])
					Add_Plot_Line(start_coords[i], end_coords[i], Data.ColorData[i], line_start_z, line_end_z);
			}

			if (--plot_index < 0) {
				break;
			}

			EBoltPlotStruct& plot = ebolt_plots[plot_index];

			distance = plot.Distance;
			line_deviation = plot.Deviation;
			line_start_z = plot.StartZ;
			line_end_z = plot.EndZ;

			std::memcpy(start_coords.data(), plot.StartCoords.data(), SEGEMENT_COORDS_SIZE);
			std::memcpy(end_coords.data(), plot.EndCoords.data(), SEGEMENT_COORDS_SIZE);
		}
	}
}

void ElectricBoltClass::Draw_Bolts()
{
	for (int i = 0; i < (int)LineDrawList.size(); ++i)
	{
		LineDrawDataStruct& data = LineDrawList[i];

		Point2D start_pixel {};
		Point2D end_pixel {};

		TacticalClass::Instance->CoordsToClient(&data.Start,&start_pixel);
		TacticalClass::Instance->CoordsToClient(&data.End,&end_pixel);

		RectangleStruct nRect = DSurface::ViewBounds();

		int start_z = data.StartZ - Game::AdjustHeight(data.Start.Z) - 2;
		int end_z = data.EndZ - Game::AdjustHeight(data.End.Z) - 2;

		unsigned color = DSurface::RGBA_To_Pixel(data.Color.R, data.Color.G, data.Color.B);

		DSurface::Composite->DrawLineColor_AZ(nRect, start_pixel, end_pixel, static_cast<COLORREF>(color), start_z, end_z, false);
	}
}

void ElectricBoltManager::Clear()
{
	ElectricBoltArray.clear();
}

void ElectricBoltManager::Draw_All()
{
	if (ElectricBoltArray.empty())
		return;

	for (size_t i = 0; i < ElectricBoltArray.size(); ++i) {
		auto& pBolt = ElectricBoltArray[i];
		if (pBolt.Lifetime <= 0) {
			ElectricBoltArray.erase(ElectricBoltArray.begin() + i);
		} else {
			pBolt.Draw_It();
		}
	}
}
