#include "ElectricBoltClass.h"

#ifdef _Enable

#include <Base/Always.h>
#include <Utilities/TemplateDef.h>
#include <TacticalClass.h>
#include <Drawing.h>
#include <Memory.h>
#include <Unsorted.h>
#include <RulesClass.h>

#include <ParticleSystemClass.h>

/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EBOLT.CPP
 *
 *  @author        CCHyper, tomsons26 (Modernized)
 *
 *  @brief         Modern graphical electric bolts for weapons.
 *
 ******************************************************************************/
#include "ebolt.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "technoext.h"
#include "techno.h"
#include "particlesys.h"
#include "weapontype.h"
#include "weapontypeext.h"
#include "tactical.h"
#include "rgb.h"
#include "dsurface.h"
#include "rules.h"
#include "random.h"
#include "wwmath.h"
#include "clipline.h"
#include "extension.h"
#include "debughandler.h"
#include "asserthandler.h"

#include <algorithm>
#include <numeric>
#include <cmath>
#include <iterator>
#include <comdef.h>
#include <shlwapi.h>

 // Static member initialization
std::vector<std::unique_ptr<EBoltClass>> EBoltClass::active_bolts_;

/**
 * Default constructor
 */
EBoltClass::EBoltClass()
{
	line_draw_list_.reserve(64); // Reserve space for better performance
}

/**
 * Destructor
 */
EBoltClass::~EBoltClass()
{
	clear();
}

/**
 * Clears references and runtime state
 */
void EBoltClass::clear()
{
	if (source_)
	{
		if (auto* ext = Extension::Fetch(source_))
		{
			ext->ElectricBolt = nullptr;
		}
		source_ = nullptr;
	}
	weapon_ = nullptr;
	line_draw_list_.clear();
}

/**
 * Main drawing function - modernized with better logic flow
 */
void EBoltClass::draw()
{
	if (should_draw_this_frame())
	{
		// This is our draw frame, so draw existing lines
		if (!line_draw_list_.empty())
		{
			draw_bolts();
		}
	}
	else
	{
		// Clear previous lines and plot new ones
		line_draw_list_.clear();

		for (int i = 0; i < iteration_count_; ++i)
		{
			if (lifetime_ > 0)
			{
				Point2D pixel_start, pixel_end;
				TacticalMap->Coord_To_Pixel(start_coord_, pixel_start);
				TacticalMap->Coord_To_Pixel(end_coord_, pixel_end);

				if (Clip_Line(pixel_start, pixel_end, TacticalRect))
				{
					plot_bolt(start_coord_, end_coord_);
				}
			}
		}

		// Draw the initial set of lines
		if (!line_draw_list_.empty())
		{
			draw_bolts();
		}

		update_lifetime();
		draw_frame_ = Frame;
	}
}

/**
 * Creates and initializes the electric bolt
 */
void EBoltClass::create(const CoordStruct& start, const CoordStruct& end, int z_adjust)
{
	start_coord_ = start;
	end_coord_ = end;
	z_adjust_ = z_adjust;

	// Spawn spark particle at destination
	if (auto* particlesys = new ParticleSystemClass(Rule->DefaultSparkSystem, end))
	{
		// Particle system will manage itself
	}
}

/**
 * Gets the source coordinate of the electric bolt
 */
CoordStruct EBoltClass::source_coord() const
{
	if (source_ && source_->IsActive && !source_->IsInLimbo)
	{
		return source_->Fire_Coord(weapon_slot_);
	}
	return COORD_NONE;
}

/**
 * Sets properties from the firing weapon and source
 */
void EBoltClass::set_properties(TechnoClass* techno, const WeaponTypeClass* weapon, WeaponSlotType slot)
{
	if (!techno || !techno->IsActive || techno->IsInLimbo)
	{
		return;
	}

	source_ = techno;
	weapon_ = weapon;
	weapon_slot_ = slot;

	// Apply weapon-specific properties
	if (weapon)
	{
		if (auto* weapon_ext = Extension::Fetch(weapon))
		{
			line_color1_ = weapon_ext->ElectricBoltColor1;
			line_color2_ = weapon_ext->ElectricBoltColor2;
			line_color3_ = weapon_ext->ElectricBoltColor3;
			iteration_count_ = weapon_ext->ElectricBoltIterationCount;
			line_segment_count_ = weapon_ext->ElectricBoltSegmentCount;
			lifetime_ = std::clamp(weapon_ext->ElectricBoltLifetime, 0, EBOLT_MAX_LIFETIME);
			deviation_ = weapon_ext->ElectricBoltDeviation;
		}
	}
}

/**
 * Updates source coordinate if source exists
 */
void EBoltClass::update_source_coord()
{
	if (auto coord = source_coord(); coord != COORD_NONE)
	{
		start_coord_ = coord;
	}
}

/**
 * Adds a line to the drawing list
 */
void EBoltClass::add_plot_line(const CoordStruct& start, const CoordStruct& end, const ColorStruct& line_color,
							  int start_z, int end_z)
{
	line_draw_list_.emplace_back(LineDrawData { start, end, line_color, start_z, end_z });
}

/**
 * Modernized bolt plotting with improved algorithms
 */
void EBoltClass::plot_bolt(const CoordStruct& start, const CoordStruct& end)
{
	// Early exit for zero distance
	const int distance = Distance(start, end);
	if (distance == 0) return;

	// Pre-allocate vectors for better performance
	std::vector<EBoltPlotData> bolt_plots;
	bolt_plots.reserve(line_segment_count_);

	// Initialize coordinate arrays
	std::array<CoordStruct, EBOLT_DEFAULT_SEGMENT_LINES> start_coords;
	std::array<CoordStruct, EBOLT_DEFAULT_SEGMENT_LINES> end_coords;
	std::array<CoordStruct, EBOLT_DEFAULT_SEGMENT_LINES> working_coords;

	// Initialize all coordinates to start/end positions
	start_coords.fill(start);
	end_coords.fill(end);

	int current_distance = distance;
	int line_start_z = z_adjust_;
	int line_end_z = 0;

	// Calculate deviation parameters
	const int dist_threshold = (102 * distance) / CELL_LEPTON_W;
	const int desired_deviation = 23;
	int line_deviation = ((desired_deviation * deviation_) * distance) / CELL_LEPTON_W;

	// Generate deviation values using modern random
	std::array<int, 6> deviation_values;
	for (size_t i = 0; i < deviation_values.size(); ++i)
	{
		const double angle = Sim_Random_Pick(0, 256) * WWMATH_PI / static_cast<double>(i + 7);
		deviation_values[i] = static_cast<int>(WWMath::Sin(angle) * line_deviation);
	}

	int plot_index = 0;
	bool init_deviation_values = true;

	// Main plotting loop with modern iterator usage
	while (current_distance > (CELL_LEPTON_W / 4) && plot_index < line_segment_count_)
	{
		// Calculate midpoints
		for (auto& coord : working_coords)
		{
			const auto start_it = start_coords.begin() + (&coord - working_coords.data());
			const auto end_it = end_coords.begin() + (&coord - working_coords.data());

			coord.X = (end_it->X + start_it->X) / 2;
			coord.Y = (end_it->Y + start_it->Y) / 2;
			coord.Z = (end_it->Z + start_it->Z) / 2;
		}

		// Apply initial deviation values
		if (init_deviation_values)
		{
			for (auto& coord : working_coords)
			{
				coord.X += deviation_values[0] + deviation_values[3];
				coord.Y += deviation_values[1] + deviation_values[5];
				coord.Z += (deviation_values[2] + deviation_values[4] + 2 * line_deviation) / 2;
			}
			init_deviation_values = false;
		}

		// Apply random deviations
		if (current_distance <= (CELL_LEPTON_W / 2))
		{
			auto& first_coord = working_coords[0];
			first_coord.X += 2 * line_deviation * Sim_Random_Pick(-1, 1);
			first_coord.Y += 2 * line_deviation * Sim_Random_Pick(-1, 1);
			first_coord.Z += 2 * line_deviation * Sim_Random_Pick(-1, 1);
		}
		else
		{
			auto& first_coord = working_coords[0];
			first_coord.X += Sim_Random_Pick(-line_deviation, line_deviation);
			first_coord.Y += Sim_Random_Pick(-line_deviation, line_deviation);
			first_coord.Z += Sim_Random_Pick(-line_deviation, line_deviation);
		}

		// Apply variations to other coordinates
		const bool use_half_variation = (current_distance > dist_threshold);
		const int variation_divisor = use_half_variation ? 2 : 1;

		for (size_t i = 1; i < working_coords.size(); ++i)
		{
			auto& coord = working_coords[i];
			const auto& base_coord = working_coords[0];

			if (use_half_variation)
			{
				coord.X = base_coord.X + (Sim_Random_Pick(-line_deviation, line_deviation) / variation_divisor);
				coord.Y = base_coord.Y + (Sim_Random_Pick(-line_deviation, line_deviation) / variation_divisor);
				coord.Z = base_coord.Z + (Sim_Random_Pick(-line_deviation, line_deviation) / variation_divisor);
			}
			else
			{
				coord.X += Sim_Random_Pick(-line_deviation, line_deviation);
				coord.Y += Sim_Random_Pick(-line_deviation, line_deviation);
				coord.Z += Sim_Random_Pick(-line_deviation, line_deviation);
			}
		}

		// Store plot data
		EBoltPlotData plot_data;
		plot_data.start_coords = working_coords;
		plot_data.end_coords = end_coords;
		plot_data.distance = current_distance;
		plot_data.deviation = line_deviation;
		plot_data.start_z = (line_end_z + line_start_z) / 2;
		plot_data.end_z = line_end_z;

		bolt_plots.push_back(plot_data);

		// Update for next iteration
		end_coords = working_coords;
		line_deviation /= 2;
		current_distance /= 2;
		line_end_z = (line_end_z + line_start_z) / 2;
		++plot_index;
	}

	// Process plots in reverse order and add lines
	while (!bolt_plots.empty())
	{
		// Add the final segment lines
		add_plot_line(start_coords[1], end_coords[1], line_color2_, line_start_z, line_end_z);
		add_plot_line(start_coords[2], end_coords[2], line_color3_, line_start_z, line_end_z);
		add_plot_line(start_coords[0], end_coords[0], line_color1_, line_start_z, line_end_z);

		if (bolt_plots.empty()) break;

		// Get the last plot and remove it
		const auto& plot = bolt_plots.back();
		current_distance = plot.distance;
		line_deviation = plot.deviation;
		line_start_z = plot.start_z;
		line_end_z = plot.end_z;
		start_coords = plot.start_coords;
		end_coords = plot.end_coords;

		bolt_plots.pop_back();
	}
}

/**
 * Draws all pending bolts using modern algorithms
 */
void EBoltClass::draw_bolts()
{
	for (const auto& line_data : line_draw_list_)
	{
		Point2D start_pixel, end_pixel;

		TacticalMap->Coord_To_Pixel(line_data.start, start_pixel);
		TacticalMap->Coord_To_Pixel(line_data.end, end_pixel);

		const int start_z = line_data.start_z - TacticalMap->Z_Lepton_To_Pixel(line_data.start.Z) - 2;
		const int end_z = line_data.end_z - TacticalMap->Z_Lepton_To_Pixel(line_data.end.Z) - 2;

		const auto color = DSurface::RGB_To_Pixel(line_data.color.Red, line_data.color.Green, line_data.color.Blue);

		CompositeSurface->Draw_Line_entry_34(TacticalRect, start_pixel, end_pixel, color, start_z, end_z);
	}
}

/**
 * Helper function to safely write data to stream
 */
HRESULT EBoltClass::write_to_stream(LPSTREAM stream, const void* data, ULONG size)
{
	if (!stream || !data) return E_INVALIDARG;

	ULONG bytes_written = 0;
	HRESULT hr = stream->Write(data, size, &bytes_written);

	if (FAILED(hr)) return hr;
	if (bytes_written != size) return STG_E_WRITEFAULT;

	return S_OK;
}

/**
 * Helper function to safely read data from stream
 */
HRESULT EBoltClass::read_from_stream(LPSTREAM stream, void* data, ULONG size)
{
	if (!stream || !data) return E_INVALIDARG;

	ULONG bytes_read = 0;
	HRESULT hr = stream->Read(data, size, &bytes_read);

	if (FAILED(hr)) return hr;
	if (bytes_read != size) return STG_E_READFAULT;

	return S_OK;
}

/**
 * Save this bolt's data to stream - simple direct serialization
 */
HRESULT EBoltClass::Save(LPSTREAM stream) const
{
	if (!stream) return E_INVALIDARG;

	HRESULT hr;

	// Write version for future compatibility
	hr = write_to_stream(stream, &SERIALIZATION_VERSION, sizeof(SERIALIZATION_VERSION));
	if (FAILED(hr)) return hr;

	// Write all serializable members directly
	hr = write_to_stream(stream, &start_coord_, sizeof(start_coord_));
	if (FAILED(hr)) return hr;

	hr = write_to_stream(stream, &end_coord_, sizeof(end_coord_));
	if (FAILED(hr)) return hr;

	hr = write_to_stream(stream, &z_adjust_, sizeof(z_adjust_));
	if (FAILED(hr)) return hr;

	hr = write_to_stream(stream, &deviation_, sizeof(deviation_));
	if (FAILED(hr)) return hr;

	hr = write_to_stream(stream, &lifetime_, sizeof(lifetime_));
	if (FAILED(hr)) return hr;

	hr = write_to_stream(stream, &iteration_count_, sizeof(iteration_count_));
	if (FAILED(hr)) return hr;

	hr = write_to_stream(stream, &line_color1_, sizeof(line_color1_));
	if (FAILED(hr)) return hr;

	hr = write_to_stream(stream, &line_color2_, sizeof(line_color2_));
	if (FAILED(hr)) return hr;

	hr = write_to_stream(stream, &line_color3_, sizeof(line_color3_));
	if (FAILED(hr)) return hr;

	hr = write_to_stream(stream, &line_segment_count_, sizeof(line_segment_count_));
	if (FAILED(hr)) return hr;

	hr = write_to_stream(stream, &draw_frame_, sizeof(draw_frame_));
	if (FAILED(hr)) return hr;

	hr = write_to_stream(stream, &weapon_slot_, sizeof(weapon_slot_));
	if (FAILED(hr)) return hr;

	return S_OK;
}

/**
 * Load this bolt's data from stream - simple direct deserialization
 */
HRESULT EBoltClass::Load(LPSTREAM stream)
{
	if (!stream) return E_INVALIDARG;

	HRESULT hr;

	// Read and verify version
	uint32_t version;
	hr = read_from_stream(stream, &version, sizeof(version));
	if (FAILED(hr)) return hr;

	if (version != SERIALIZATION_VERSION)
	{
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}

	// Read all serializable members directly
	hr = read_from_stream(stream, &start_coord_, sizeof(start_coord_));
	if (FAILED(hr)) return hr;

	hr = read_from_stream(stream, &end_coord_, sizeof(end_coord_));
	if (FAILED(hr)) return hr;

	hr = read_from_stream(stream, &z_adjust_, sizeof(z_adjust_));
	if (FAILED(hr)) return hr;

	hr = read_from_stream(stream, &deviation_, sizeof(deviation_));
	if (FAILED(hr)) return hr;

	hr = read_from_stream(stream, &lifetime_, sizeof(lifetime_));
	if (FAILED(hr)) return hr;

	hr = read_from_stream(stream, &iteration_count_, sizeof(iteration_count_));
	if (FAILED(hr)) return hr;

	hr = read_from_stream(stream, &line_color1_, sizeof(line_color1_));
	if (FAILED(hr)) return hr;

	hr = read_from_stream(stream, &line_color2_, sizeof(line_color2_));
	if (FAILED(hr)) return hr;

	hr = read_from_stream(stream, &line_color3_, sizeof(line_color3_));
	if (FAILED(hr)) return hr;

	hr = read_from_stream(stream, &line_segment_count_, sizeof(line_segment_count_));
	if (FAILED(hr)) return hr;

	hr = read_from_stream(stream, &draw_frame_, sizeof(draw_frame_));
	if (FAILED(hr)) return hr;

	hr = read_from_stream(stream, &weapon_slot_, sizeof(weapon_slot_));
	if (FAILED(hr)) return hr;

	return S_OK;
}

/**
 * Save all active bolts to stream - array save
 */
HRESULT EBoltClass::Save_Array(LPSTREAM stream)
{
	if (!stream) return E_INVALIDARG;

	HRESULT hr;

	// Write file format header
	const uint32_t magic = 0x544C4F42; // "BOLT" in hex
	hr = write_to_stream(stream, &magic, sizeof(magic));
	if (FAILED(hr)) return hr;

	hr = write_to_stream(stream, &SERIALIZATION_VERSION, sizeof(SERIALIZATION_VERSION));
	if (FAILED(hr)) return hr;

	// Count valid bolts
	size_t valid_count = 0;
	for (const auto& bolt : active_bolts_)
	{
		if (bolt && !bolt->is_expired())
		{
			++valid_count;
		}
	}

	// Write count
	uint32_t count = static_cast<uint32_t>(valid_count);
	hr = write_to_stream(stream, &count, sizeof(count));
	if (FAILED(hr)) return hr;

	// Write each bolt
	for (const auto& bolt : active_bolts_)
	{
		if (bolt && !bolt->is_expired())
		{
			hr = bolt->Save(stream);
			if (FAILED(hr)) return hr;
		}
	}

	return S_OK;
}

/**
 * Load all bolts from stream - array load
 */
HRESULT EBoltClass::Load_Array(LPSTREAM stream)
{
	if (!stream) return E_INVALIDARG;

	HRESULT hr;

	// Read and verify magic header
	uint32_t magic;
	hr = read_from_stream(stream, &magic, sizeof(magic));
	if (FAILED(hr)) return hr;

	if (magic != 0x544C4F42)
	{ // "BOLT"
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}

	// Read and verify version
	uint32_t version;
	hr = read_from_stream(stream, &version, sizeof(version));
	if (FAILED(hr)) return hr;

	if (version != SERIALIZATION_VERSION)
	{
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}

	// Clear existing bolts
	clear_all();

	// Read count
	uint32_t count;
	hr = read_from_stream(stream, &count, sizeof(count));
	if (FAILED(hr)) return hr;

	// Sanity check on count
	if (count > 10000)
	{ // Reasonable upper limit
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}

	// Read and create bolts
	for (uint32_t i = 0; i < count; ++i)
	{
		auto bolt = std::make_unique<EBoltClass>();
		hr = bolt->Load(stream);
		if (FAILED(hr))
		{
			clear_all(); // Clean up on failure
			return hr;
		}

		// Add bolt to active list
		register_bolt(std::move(bolt));
	}

	return S_OK;
}

/**
 * Static management functions using modern container algorithms
 */
void EBoltClass::draw_all()
{
	// Clean up expired bolts first
	cleanup_expired_bolts();

	// Draw remaining bolts
	for (auto& bolt : active_bolts_)
	{
		if (!bolt) continue;

		// Check if source is still valid
		if (bolt->source_ && (!bolt->source_->IsActive || bolt->source_->IsInLimbo))
		{
			bolt->flag_to_delete();
			continue;
		}

		// Update source coordinate
		bolt->update_source_coord();

		// Draw the bolt
		bolt->draw();
	}

	// Final cleanup of expired bolts
	cleanup_expired_bolts();
}

void EBoltClass::clear_all()
{
	active_bolts_.clear();
}

void EBoltClass::cleanup_expired_bolts()
{
	active_bolts_.erase(
		std::ranges::remove_if(active_bolts_,
			[](const auto& bolt) { return !bolt || bolt->is_expired(); }),
		active_bolts_.end()
	);
}

void EBoltClass::register_bolt(std::unique_ptr<EBoltClass> bolt)
{
	if (bolt)
	{
		active_bolts_.push_back(std::move(bolt));
	}
}

size_t EBoltClass::active_count() noexcept
{
	return active_bolts_.size();
}

/**
 * Factory functions for better memory management
 */
std::unique_ptr<EBoltClass> create_ebolt()
{
	auto bolt = std::make_unique<EBoltClass>();
	auto* raw_ptr = bolt.get();
	EBoltClass::register_bolt(std::move(bolt));
	return std::unique_ptr<EBoltClass>(raw_ptr); // Return non-owning unique_ptr for interface compatibility
}

EBoltClass* create_ebolt_raw()
{
	auto bolt = std::make_unique<EBoltClass>();
	auto* raw_ptr = bolt.get();
	EBoltClass::register_bolt(std::move(bolt));
	return raw_ptr;
}
#endif