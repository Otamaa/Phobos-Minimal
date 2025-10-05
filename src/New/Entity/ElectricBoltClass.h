
#pragma once

#ifdef _Enable

#include <CoordStruct.h>
#include <ColorStruct.h>
#include <GeneralStructures.h>
#include <ScenarioClass.h>

#include <Unsorted.h>
#include <Utilities/SavegameDef.h>

class TechnoClass;
class WeaponTypeClass;

// Constants
constexpr float EBOLT_DEFAULT_DEVIATION = 1.0f;
constexpr int EBOLT_DEFAULT_ITERATIONS = 1;
constexpr int EBOLT_DEFAULT_LINE_SEGMENTS = 8;
constexpr int EBOLT_DEFAULT_SEGMENT_LINES = 3;
constexpr int EBOLT_DEFAULT_LIFETIME = 17;
constexpr int EBOLT_MAX_LIFETIME = 60;
constexpr ColorStruct EBOLT_DEFAULT_COLOR_1 { 255, 255, 255 }; // White
constexpr ColorStruct EBOLT_DEFAULT_COLOR_2 { 82, 81, 255 };   // Dark Blue
constexpr ColorStruct EBOLT_DEFAULT_COLOR_3 { 82, 81, 255 };   // Dark Blue

class EBoltClass
{
public:
	// Data structures
	struct LineDrawData
	{
		CoordStruct start {};
		CoordStruct end {};
		ColorStruct color {};
		int start_z { 0 };
		int end_z { 0 };

		auto operator<=>(const LineDrawData&) const = default;
		bool operator==(const LineDrawData&) const = default;
	};

	struct EBoltPlotData
	{
		std::array<CoordStruct, EBOLT_DEFAULT_SEGMENT_LINES> start_coords {};
		std::array<CoordStruct, EBOLT_DEFAULT_SEGMENT_LINES> end_coords {};
		int distance { 0 };
		int deviation { 0 };
		int start_z { 0 };
		int end_z { 0 };

		auto operator<=>(const EBoltPlotData&) const = default;
		bool operator==(const EBoltPlotData&) const = default;
	};

public:
	EBoltClass();
	~EBoltClass();

	// Core functionality
	void draw();
	void create(const CoordStruct& start, const CoordStruct& end, int z_adjust);
	void flag_to_delete() noexcept { lifetime_ = 0; }

	// Property access
	[[nodiscard]] CoordStruct source_coord() const;
	[[nodiscard]] bool is_expired() const noexcept { return lifetime_ <= 0; }
	[[nodiscard]] bool has_source() const noexcept { return source_ != nullptr; }

	// Configuration
	void set_properties(TechnoClass* techno, const WeaponTypeClass* weapon, WeaponSlotType slot);

	// Simple Save/Load - only 4 functions
	HRESULT Save(LPSTREAM stream) const;
	HRESULT Load(LPSTREAM stream);
	static HRESULT Save_Array(LPSTREAM stream);
	static HRESULT Load_Array(LPSTREAM stream);

	// Static management functions
	static void draw_all();
	static void clear_all();
	static size_t active_count() noexcept;

private:
	void clear();
	void add_plot_line(const CoordStruct& start, const CoordStruct& end, const ColorStruct& line_color,
					  int start_z, int end_z);
	void plot_bolt(const CoordStruct& start, const CoordStruct& end);
	void draw_bolts();
	void update_lifetime() noexcept { --lifetime_; }
	bool should_draw_this_frame() const noexcept { return draw_frame_ == Unsorted::CurrentFrame; }
	void update_source_coord();

	// Stream I/O helpers
	static HRESULT write_to_stream(LPSTREAM stream, const void* data, ULONG size);
	static HRESULT read_from_stream(LPSTREAM stream, void* data, ULONG size);

	// Serialization version for compatibility
	static constexpr uint32_t SERIALIZATION_VERSION = 1;

private:
	// Core state (all directly serializable)
	CoordStruct start_coord_ {};
	CoordStruct end_coord_ {};
	int z_adjust_ { 0 };
	float deviation_ { EBOLT_DEFAULT_DEVIATION };
	int lifetime_ { EBOLT_DEFAULT_LIFETIME };
	int iteration_count_ { EBOLT_DEFAULT_ITERATIONS };
	ColorStruct line_color1_ { EBOLT_DEFAULT_COLOR_1 };
	ColorStruct line_color2_ { EBOLT_DEFAULT_COLOR_2 };
	ColorStruct line_color3_ { EBOLT_DEFAULT_COLOR_3 };
	int line_segment_count_ { EBOLT_DEFAULT_LINE_SEGMENTS };
	int draw_frame_ { -1 };
	int weapon_slot_ { 0 };

	// Runtime state (not serialized)
	TechnoClass* source_ { nullptr };  // Raw pointer for game engine compatibility
	const WeaponTypeClass* weapon_ { nullptr };
	std::vector<LineDrawData> line_draw_list_;

	// Static management
	static std::vector<std::unique_ptr<EBoltClass>> active_bolts_;
	static void register_bolt(std::unique_ptr<EBoltClass> bolt);
	static void cleanup_expired_bolts();
};

// Factory functions for better memory management
[[nodiscard]] std::unique_ptr<EBoltClass> create_ebolt();
[[nodiscard]] EBoltClass* create_ebolt_raw(); // Returns raw pointer for C-style APIs
#endif