#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <CoordStruct.h>
#include <Point2D.h>
#include <YRMathVector.h>

#include <Utilities/VectorHelper.h>

// Forward declarations
class IStream;
class DSurface;
class AbstractClass;
class AnimTypeClass;
class TechnoClass;
class FootClass;
class WarheadTypeClass;
class HouseClass;

/**
 * Modern Ion Blast Effect System
 *
 * Features:
 * - Real-time wave calculation (no static lookup tables)
 * - Configurable size, wave parameters, and distortion patterns
 * - Modern C++ with RAII and smart pointers
 * - Trivial save/load (just 20 bytes per instance)
 * - Multiple visual effect patterns
 */
class ModernIonBlastClass
{
public:
	// ===== ENUMS =====

	/**
	 * Available distortion patterns for visual effects
	 */
	enum class DistortionPattern
	{
		Spiral,    ///< Original complex spiral coordinate pattern
		Box,       ///< Clean geometric grid distortion
		Radial,    ///< Pure outward displacement from center
		Wave       ///< Concentric ripple wave pattern
	};

	// ===== INSTANCE DATA =====

	CoordStruct coord;    ///< World position (12 bytes)
	int state;           ///< Animation frame 0-78 (4 bytes)
	int dword10;         ///< Flags/mode (4 bytes)
	TechnoClass* Invoker;
	HouseClass* Owner;
	WarheadTypeClass* Warhead;
	AnimTypeClass* Beam;
	AnimTypeClass* Blast;
	bool Rocking;
	int Damage;

	// Total: 20 bytes (save-friendly)

	// ===== CONSTRUCTORS =====

	/**
	 * Default constructor - creates blast at origin
	 */
	ModernIonBlastClass();

	/**
	 * Position constructor
	 * @param position World coordinates for blast center
	 */
	explicit ModernIonBlastClass(const CoordStruct& position);

	/**
	 * Position + state constructor
	 * @param position World coordinates for blast center
	 * @param initialState Starting animation frame (0-78)
	 */
	ModernIonBlastClass(const CoordStruct& position, int initialState);

	/**
	 * Full constructor
	 * @param position World coordinates for blast center
	 * @param initialState Starting animation frame (0-78)
	 * @param flags Custom flags/mode settings
	 */
	ModernIonBlastClass(const CoordStruct& position, int initialState, int flags);

	// Rule of 5 compliance
	ModernIonBlastClass(const ModernIonBlastClass& other) = default;
	ModernIonBlastClass(ModernIonBlastClass&& other) noexcept = default;
	ModernIonBlastClass& operator=(const ModernIonBlastClass& other) = default;
	ModernIonBlastClass& operator=(ModernIonBlastClass&& other) noexcept = default;
	~ModernIonBlastClass() = default;

	// ===== INSTANCE METHODS =====

	/**
	 * Updates blast animation and applies physics effects to nearby units
	 */
	bool Blast_AI();

	/**
	 * Renders the blast distortion effect to screen
	 */
	void Draw();

	/**
	 * Saves instance data to stream
	 * @param stream Output stream
	 * @return true if successful
	 */
	bool Save(IStream* stream) const;

	/**
	 * Loads instance data from stream
	 * @param stream Input stream
	 * @return true if successful
	 */
	bool Load(IStream* stream);

	// ===== FACTORY METHODS =====

	/**
	 * Creates a new blast instance with smart pointer
	 * @param position World coordinates
	 * @return Unique pointer to new blast
	 */
	static std::unique_ptr<ModernIonBlastClass> Create(const CoordStruct& position);

	/**
	 * Creates a new blast instance with smart pointer
	 * @param position World coordinates
	 * @param initialState Starting animation frame
	 * @return Unique pointer to new blast
	 */
	static std::unique_ptr<ModernIonBlastClass> Create(const CoordStruct& position, int initialState);

	/**
	 * Creates a new blast instance with smart pointer
	 * @param position World coordinates
	 * @param initialState Starting animation frame
	 * @param flags Custom flags
	 * @return Unique pointer to new blast
	 */
	static std::unique_ptr<ModernIonBlastClass> Create(const CoordStruct& position, int initialState, int flags);

	// ===== CONTAINER MANAGEMENT =====

	/**
	 * Invalidator
	 */
	static void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

	/**
	 * Removes all active blasts
	 */
	static void ClearAllBlasts();

	/**
	 * Gets number of active blasts
	 * @return Number of active blasts
	 */
	static size_t GetActiveBlastCount();

	/**
	 * Updates all active blasts and removes expired ones
	 */
	static void Update_All();

	/**
	 * Draws all active blasts
	 */
	static void Draw_All();

	// ===== SAVE/LOAD SYSTEM =====

	/**
	 * Saves all active blasts to stream
	 * @param stream Output stream
	 * @return true if successful
	 */
	static bool SaveAll(IStream* stream);

	/**
	 * Loads all blasts from stream
	 * @param stream Input stream
	 * @return true if successful
	 */
	static bool LoadAll(IStream* stream);

	// ===== SIZE CONFIGURATION =====

	/**
	 * Sets rectangular blast area
	 * @param width Blast width in pixels
	 * @param height Blast height in pixels
	 */
	static void SetBlastSize(int width, int height);

	/**
	 * Sets circular blast area
	 * @param radius Blast radius in pixels
	 */
	static void SetBlastRadius(int radius);

	/**
	 * Sets original game size (512x256)
	 */
	static void SetOriginalSize();

	/**
	 * Gets current blast dimensions
	 * @param width Output width
	 * @param height Output height
	 */
	static void GetCurrentSize(int& width, int& height);

	// ===== WAVE PARAMETER CONFIGURATION =====

	/**
	 * Sets all wave parameters at once
	 * @param frequency Wave frequency (default: 0.11)
	 * @param amplitude Wave amplitude (default: 3.5)
	 * @param speed Wave propagation speed (default: 7.1125)
	 */
	static void SetWaveParameters(double frequency, double amplitude, double speed);

	/**
	 * Sets wave frequency only
	 * @param frequency Wave frequency (0.01 - 1.0)
	 */
	static void SetWaveFrequency(double frequency);

	/**
	 * Sets wave amplitude only
	 * @param amplitude Wave amplitude (0.1 - 10.0)
	 */
	static void SetWaveAmplitude(double amplitude);

	/**
	 * Sets wave speed only
	 * @param speed Wave propagation speed (0.1 - 20.0)
	 */
	static void SetWaveSpeed(double speed);

	/**
	 * Resets to original wave parameters
	 */
	static void SetOriginalWaveParameters();

	static void SetAffectParam();

	/**
	 * Gets current wave parameters
	 * @param frequency Output frequency
	 * @param amplitude Output amplitude
	 * @param speed Output speed
	 */
	static void GetWaveParameters(double& frequency, double& amplitude, double& speed);

	// ===== PATTERN CONFIGURATION =====

	/**
	 * Sets distortion pattern type
	 * @param pattern Pattern to use
	 */
	static void SetDistortionPattern(DistortionPattern pattern);

	/**
	 * Gets current distortion pattern
	 * @return Current pattern
	 */
	static DistortionPattern GetDistortionPattern();

	/**
	 * Gets human-readable pattern name
	 * @param pattern Pattern to get name for
	 * @return Pattern name string
	 */
	static const char* GetPatternName(DistortionPattern pattern);

	// ===== PRESET CONFIGURATIONS =====

	/**
	 * Sets exact original game behavior
	 */
	static void SetPresetOriginal();

	/**
	 * Sets performance-optimized settings
	 */
	static void SetPresetOptimized();

	/**
	 * Sets large, dramatic blast settings
	 */
	static void SetPresetMassive();

	/**
	 * Sets small, gentle wave settings
	 */
	static void SetPresetSubtle();

private:
	// ===== PRIVATE TYPES =====

	/**
	 * Configuration structure for blast behavior
	 */
	struct BlastConfig
	{
		// Size settings
		int Width;
		int Height;
		bool UseCircular;
		int Radius;

		// Wave parameters
		double WaveFrequency;
		double WaveAmplitude;
		double WaveSpeed;
		double WaveBase;
		double DistanceScale;
		double WavePhaseOffset;
		float MinDistortionThreshold;

		// Pattern settings
		DistortionPattern Pattern;

		BlastConfig();
	};

	// ===== STATIC DATA =====

	static BlastConfig Config;   ///< Global configuration
public:

	static HelperedVector<std::unique_ptr<ModernIonBlastClass>> ActiveBlasts;  ///< Active blast container

	void InitializeBlastEffects(WarheadTypeClass* pDamagingWarhead, WarheadTypeClass* pSourceWarhead);
	void InitializeOwnership(TechnoClass* pInvoker, HouseClass* pOwner);

private:
	// ===== PRIVATE METHODS =====

	// Core rendering
	void DrawRealTime();

	// Wave mathematics
	float CalculateWaveIntensity(float distance, int frameState) const;
	Point2D CalculateDistortionOffset(float intensity, float distance, float angle, int deltaX, int deltaY) const;

	// Pattern-specific calculations
	Point2D CalculateSpiralOffset(float intensity, float distance, float angle) const;
	Point2D CalculateBoxOffset(float intensity, int deltaX, int deltaY) const;
	Point2D CalculateRadialOffset(float intensity, float distance, float angle) const;
	Point2D CalculateWaveOffset(float intensity, float distance, float angle, int frameState) const;

	// Utility methods
	bool IsPixelInBlastArea(int deltaX, int deltaY, int frameState) const;

	// Physics effects (based on original system)
	void ApplyBlastEffects();
	void ApplyBlastDamage();
	void ApplyVisualEffectsToUnits(const Point2D& blastPixel);

	// Additional helper methods for unit effects
	void ProcessUnitInBlast(FootClass* unit, const Point2D& blastPixel, int maxEffectDistance);
	void ApplyIonSurfaceEffectsToUnit(FootClass* unit, int distance);
	void ApplyPhysicsEffectsToUnit(FootClass* unit, int distance);
	void ApplyRotationEffectsToUnit(FootClass* unit, const Vector3D<float>& direction, float sinFacing, float cosFacing, float distance);
	Point2D IndexToCoordinates(Point2D* result, int number);
};