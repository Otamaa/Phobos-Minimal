#include "ModernIonBlastClass.h"

#include "ModernIonBlastClass.h"
#include <algorithm>
#include <cmath>
#include <cstring>

#include <Misc/DamageArea.h>
#include <Utilities/Swizzle.h>

#include <Surface.h>
#include <TacticalClass.h>
#include <Drawing.h>
#include <GameOptionsClass.h>
#include <UnitClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <AnimClass.h>

// ===== STATIC MEMBER DEFINITIONS =====
Vector3D<float> CalculateBlastVector(ModernIonBlastClass* pIon , FootClass* unit)
{
	return Vector3D<float>(
		pIon->coord.X - unit->Location.X,
		unit->Location.Y - pIon->coord.Y,
		pIon->coord.Z - unit->Location.Z
	);
}

Vector3D<float> NormalizeVector(const Vector3D<float>& vector)
{
	Vector3D normalized = { 0.0f, 0.0f, 0.0f };

	const float magnitude = std::sqrt(vector.X * vector.X + vector.Y * vector.Y + vector.Z * vector.Z);

	if (Math::abs(magnitude) > 0.00002) {
		const double horizontalMagnitude = std::sqrt(vector.Y * vector.Y + vector.X * vector.X);

		if (horizontalMagnitude != 0.0) {
			normalized.X = vector.X / horizontalMagnitude;
			normalized.Y = vector.Y / horizontalMagnitude;
			normalized.Z = 0.0f / horizontalMagnitude;
		} else {
			normalized.X = vector.X;
			normalized.Y = vector.Y;
			normalized.Z = 0.0f;
		}
	}

	return normalized;
}

ModernIonBlastClass::BlastConfig ModernIonBlastClass::Config;
HelperedVector<std::unique_ptr<ModernIonBlastClass>> ModernIonBlastClass::ActiveBlasts;

// ===== BLASTCONFIG CONSTRUCTOR =====

ModernIonBlastClass::BlastConfig::BlastConfig()
	: Width(512), Height(256), UseCircular(false), Radius(128)
	, WaveFrequency(0.11), WaveAmplitude(3.5), WaveSpeed(7.1125)
	, WaveBase(3.0), DistanceScale(51.0), WavePhaseOffset(38.0)
	, MinDistortionThreshold(0.1f), Pattern(DistortionPattern::Spiral)
{ }

// ===== CONSTRUCTORS =====

ModernIonBlastClass::ModernIonBlastClass()
	: coord { 0, 0, 0 }
	, state(0)
	, dword10(0)
	, Invoker(nullptr)
	, Owner(nullptr)
	, Warhead(nullptr)
	, Beam(nullptr)
	, Blast(nullptr)
	, Rocking(true)
	, Damage(0)
{ }

ModernIonBlastClass::ModernIonBlastClass(const CoordStruct& position)
	: coord(position)
	, state(0)
	, dword10(0)
	, Invoker(nullptr)
	, Owner(nullptr)
	, Warhead(nullptr)
	, Beam(nullptr)
	, Blast(nullptr)
	, Rocking(true)
	, Damage(0)
{ }

ModernIonBlastClass::ModernIonBlastClass(const CoordStruct& position, int initialState)
	: coord(position)
	, state(initialState)
	, dword10(0)
	, Invoker(nullptr)
	, Owner(nullptr)
	, Warhead(nullptr)
	, Beam(nullptr)
	, Blast(nullptr)
	, Rocking(true)
	, Damage(0)
{
	// Validate state parameter
	if (state < 0 || state >= 79)
	{
		state = 0;
	}
}

ModernIonBlastClass::ModernIonBlastClass(const CoordStruct& position, int initialState, int flags)
	: coord(position)
	, state(initialState)
	, dword10(flags)
	, Invoker(nullptr)
	, Owner(nullptr)
	, Warhead(nullptr)
	, Beam(nullptr)
	, Blast(nullptr)
	, Rocking(true)
	, Damage(0)
{
	// Validate parameters
	if (state < 0 || state >= 79)
	{
		state = 0;
	}
}

// ===== FACTORY METHODS =====

std::unique_ptr<ModernIonBlastClass> ModernIonBlastClass::Create(const CoordStruct& position)
{
	return std::make_unique<ModernIonBlastClass>(position);
}

std::unique_ptr<ModernIonBlastClass> ModernIonBlastClass::Create(const CoordStruct& position, int initialState)
{
	return std::make_unique<ModernIonBlastClass>(position, initialState);
}

std::unique_ptr<ModernIonBlastClass> ModernIonBlastClass::Create(const CoordStruct& position, int initialState, int flags)
{
	return std::make_unique<ModernIonBlastClass>(position, initialState, flags);
}

// ===== CONTAINER MANAGEMENT =====

void ModernIonBlastClass::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	if(bRemoved) {
		for (auto& blast : ActiveBlasts) {
			if (blast && blast->Invoker == ptr) {
				blast->Invoker = nullptr;
			}
		}
	}
}

void ModernIonBlastClass::ClearAllBlasts()
{
	ActiveBlasts.clear();
}

size_t ModernIonBlastClass::GetActiveBlastCount()
{
	return ActiveBlasts.size();
}

void ModernIonBlastClass::Update_All()
{
	// Update all blasts and remove expired ones
	ActiveBlasts.remove_all_if([](const std::unique_ptr<ModernIonBlastClass>& ptr) {
		return !ptr || ptr->Blast_AI();// Remove when expired
	});
}

void ModernIonBlastClass::Draw_All()
{
	// Draw all active blasts
	for (auto& blast : ActiveBlasts) {
		if(blast) {
			blast->Draw();
		}
	}
}

// ===== SAVE/LOAD SYSTEM =====

bool ModernIonBlastClass::SaveAll(IStream* stream)
{
	static const int CONTAINER_VERSION = 1;
	ULONG written;

	// Write version
	if (FAILED(stream->Write(&CONTAINER_VERSION, sizeof(CONTAINER_VERSION), &written)))
	{
		return false;
	}

	// Write count
	size_t count = ActiveBlasts.size();
	if (FAILED(stream->Write(&count, sizeof(count), &written)))
	{
		return false;
	}

	// Write each blast
	for (const auto& blast : ActiveBlasts)
	{
		if (!blast->Save(stream))
		{
			return false;
		}
	}

	return true;
}

bool ModernIonBlastClass::LoadAll(IStream* stream)
{
	int version;
	size_t count;
	ULONG read;

	// Read version
	if (FAILED(stream->Read(&version, sizeof(version), &read)) || version != 1)
	{
		return false;
	}

	// Clear existing blasts
	ClearAllBlasts();

	// Read count
	if (FAILED(stream->Read(&count, sizeof(count), &read)))
	{
		return false;
	}

	// Read each blast
	ActiveBlasts.reserve(count);
	for (size_t i = 0; i < count; ++i)
	{
		auto blast = std::make_unique<ModernIonBlastClass>();
		if (!blast->Load(stream))
		{
			return false;
		}
		ActiveBlasts.push_back(std::move(blast));
	}

	return true;
}

bool ModernIonBlastClass::Save(IStream* stream) const
{
	static const int SAVE_VERSION = 3; // Real-time version

	ULONG written;
	HRESULT hr;

	hr = stream->Write(&SAVE_VERSION, sizeof(SAVE_VERSION), &written);
	if (FAILED(hr)) return false;

	hr = stream->Write(&coord, sizeof(coord), &written);
	if (FAILED(hr)) return false;

	hr = stream->Write(&state, sizeof(state), &written);
	if (FAILED(hr)) return false;

	hr = stream->Write(&dword10, sizeof(dword10), &written);
	if (FAILED(hr)) return false;

	hr = stream->Write(&Invoker, sizeof(Invoker), &written);
	if (FAILED(hr)) return false;

	hr = stream->Write(&Owner, sizeof(Owner), &written);
	if (FAILED(hr)) return false;

	hr = stream->Write(&Warhead, sizeof(Warhead), &written);
	if (FAILED(hr)) return false;

	hr = stream->Write(&Beam, sizeof(Beam), &written);
	if (FAILED(hr)) return false;

	hr = stream->Write(&Blast, sizeof(Blast), &written);
	if (FAILED(hr)) return false;

	hr = stream->Write(&Rocking, sizeof(Rocking), &written);
	if (FAILED(hr)) return false;

	hr = stream->Write(&Damage, sizeof(Damage), &written);
	if (FAILED(hr)) return false;

	return true;
}

bool ModernIonBlastClass::Load(IStream* stream)
{
	int version;
	ULONG read;
	HRESULT hr;

	hr = stream->Read(&version, sizeof(version), &read);
	if (FAILED(hr) || version != 3) return false;

	hr = stream->Read(&coord, sizeof(coord), &read);
	if (FAILED(hr)) return false;

	hr = stream->Read(&state, sizeof(state), &read);
	if (FAILED(hr)) return false;

	hr = stream->Read(&dword10, sizeof(dword10), &read);
	if (FAILED(hr)) return false;

	hr = stream->Read(&Invoker, sizeof(Invoker), &read);
	if (FAILED(hr)) return false;

	PHOBOS_SWIZZLE_REQUEST_POINTER_REMAP(Invoker, "TechnoClass");

	hr = stream->Read(&Owner, sizeof(Owner), &read);
	if (FAILED(hr)) return false;

	PHOBOS_SWIZZLE_REQUEST_POINTER_REMAP(Invoker, "HouseClass");

	hr = stream->Read(&Warhead, sizeof(Warhead), &read);
	if (FAILED(hr)) return false;

	PHOBOS_SWIZZLE_REQUEST_POINTER_REMAP(Warhead, "WarheadTypeClass");

	hr = stream->Read(&Beam, sizeof(Beam), &read);
	if (FAILED(hr)) return false;

	PHOBOS_SWIZZLE_REQUEST_POINTER_REMAP(Beam, "AnimTypeClass");

	hr = stream->Read(&Blast, sizeof(Blast), &read);
	if (FAILED(hr)) return false;

	PHOBOS_SWIZZLE_REQUEST_POINTER_REMAP(Blast, "AnimTypeClass");

	hr = stream->Read(&Rocking, sizeof(Rocking), &read);
	if (FAILED(hr)) return false;

	hr = stream->Read(&Damage, sizeof(Damage), &read);
	if (FAILED(hr)) return false;

	// Validate loaded data
	if (state < 0 || state >= 79) {
		state = 0;
	}

	return true;
}

// ===== SIZE CONFIGURATION =====

void ModernIonBlastClass::SetBlastSize(int width, int height)
{
	Config.Width = std::max(32, std::min(1024, width));   // Reasonable bounds
	Config.Height = std::max(32, std::min(1024, height));
	Config.UseCircular = false;
}

void ModernIonBlastClass::SetBlastRadius(int radius)
{
	Config.Radius = std::max(16, std::min(512, radius));  // Reasonable bounds
	Config.UseCircular = true;
}

void ModernIonBlastClass::SetOriginalSize()
{
	// Exact original implementation size
	Config.Width = 512;
	Config.Height = 256;
	Config.UseCircular = false;
}

void ModernIonBlastClass::GetCurrentSize(int& width, int& height)
{
	if (Config.UseCircular)
	{
		width = height = Config.Radius * 2;
	}
	else
	{
		width = Config.Width;
		height = Config.Height;
	}
}

// ===== WAVE PARAMETER CONFIGURATION =====

void ModernIonBlastClass::SetWaveParameters(double frequency, double amplitude, double speed)
{
	Config.WaveFrequency = std::max(0.01, std::min(1.0, frequency));    // Reasonable bounds
	Config.WaveAmplitude = std::max(0.1, std::min(10.0, amplitude));
	Config.WaveSpeed = std::max(0.1, std::min(20.0, speed));
}

void ModernIonBlastClass::SetWaveFrequency(double frequency)
{
	Config.WaveFrequency = std::max(0.01, std::min(1.0, frequency));
}

void ModernIonBlastClass::SetWaveAmplitude(double amplitude)
{
	Config.WaveAmplitude = std::max(0.1, std::min(10.0, amplitude));
}

void ModernIonBlastClass::SetWaveSpeed(double speed)
{
	Config.WaveSpeed = std::max(0.1, std::min(20.0, speed));
}

void ModernIonBlastClass::SetOriginalWaveParameters()
{
	Config.WaveFrequency = 0.11;
	Config.WaveAmplitude = 3.5;
	Config.WaveSpeed = 7.1125;
}

void ModernIonBlastClass::GetWaveParameters(double& frequency, double& amplitude, double& speed)
{
	frequency = Config.WaveFrequency;
	amplitude = Config.WaveAmplitude;
	speed = Config.WaveSpeed;
}

// ===== PATTERN CONFIGURATION =====

void ModernIonBlastClass::SetDistortionPattern(DistortionPattern pattern)
{
	Config.Pattern = pattern;
}

ModernIonBlastClass::DistortionPattern ModernIonBlastClass::GetDistortionPattern()
{
	return Config.Pattern;
}

const char* ModernIonBlastClass::GetPatternName(DistortionPattern pattern)
{
	switch (pattern)
	{
	case DistortionPattern::Spiral: return "Spiral";
	case DistortionPattern::Box:    return "Box";
	case DistortionPattern::Radial: return "Radial";
	case DistortionPattern::Wave:   return "Wave";
	default: return "Unknown";
	}
}

// ===== PRESET CONFIGURATIONS =====

void ModernIonBlastClass::SetPresetOriginal()
{
	SetOriginalSize();           // 512x256
	SetOriginalWaveParameters(); // 0.11, 3.5, 7.1125
	SetDistortionPattern(DistortionPattern::Spiral);
}

void ModernIonBlastClass::SetPresetOptimized()
{
	SetBlastRadius(100);         // Smaller, faster
	SetWaveParameters(0.15, 2.0, 5.0);  // Faster, smaller waves
	SetDistortionPattern(DistortionPattern::Radial);
}

void ModernIonBlastClass::SetPresetMassive()
{
	SetBlastRadius(300);         // Large area
	SetWaveParameters(0.08, 5.0, 10.0); // Slow, large waves
	SetDistortionPattern(DistortionPattern::Spiral);
}

void ModernIonBlastClass::SetPresetSubtle()
{
	SetBlastRadius(80);          // Small area
	SetWaveParameters(0.20, 1.0, 3.0);  // Fast, gentle waves
	SetDistortionPattern(DistortionPattern::Wave);
}

// ===== CORE METHODS =====

bool ModernIonBlastClass::Blast_AI()
{
	const int MAX_BLAST_DURATION = 79;

	if (state < MAX_BLAST_DURATION)
	{
		if (dword10 == 1)
		{
			++state;
		}	else{
			Point2D blastPixel = TacticalClass::Instance->CoordsToClient(coord);

			if (state == 0)
			{
				ApplyBlastEffects();
				ApplyBlastDamage();
			}

			ApplyVisualEffectsToUnits(blastPixel);
			++state;
		}

	}

	return state >= MAX_BLAST_DURATION;
}

void ModernIonBlastClass::Draw()
{
	// Only draw on highest detail level
	if (GameOptionsClass::Instance->DetailLevel != 2) {
		return;
	}

	// Bounds check
	if (state < 0 || state >= 79)
	{
		return;
	}

	DrawRealTime();
}

// ===== RENDERING IMPLEMENTATION =====

void ModernIonBlastClass::DrawRealTime()
{
	auto [blastPixel, cond] = TacticalClass::Instance->GetCoordsToClientSituation(coord);

	if (!cond)
	{
		return;
	}

	DSurface* destSurface = DSurface::Temp();
	if (!destSurface) return;

	// Lock surface for direct pixel access
	int surfacePitch = destSurface->Get_Pitch();
	uint16_t* destPixels = (uint16_t*)destSurface->Lock(0, 0);
	if (!destPixels) return;

	// Get surface dimensions
	int surfaceWidth = destSurface->Get_Width();
	int surfaceHeight = destSurface->Get_Height();

	// ORIGINAL: Fixed 512x256 rectangle (matches original exactly)
	const int rectWidth = 512;
	const int rectHeight = 256;
	const int rectCenterX = 256;
	const int rectCenterY = 128;

	int rectLeft = blastPixel.X - rectCenterX;
	int rectTop = blastPixel.Y - rectCenterY;

	// Calculate Z-buffer base value (matches original)
	int16_t baseZValue = LOWORD(ZBuffer::Instance->MaxValue) - Game::AdjustHeight(coord.Z);
	int16_t currentZValue = baseZValue - rectTop - 3;

	// Clamp to screen bounds
	int startX = std::max(0, rectLeft);
	int endX = std::min(surfaceWidth, rectLeft + rectWidth);
	int startY = std::max(0, rectTop);
	int endY = std::min(surfaceHeight, rectTop + rectHeight);

	// Process each pixel in the rectangle
	for (int screenY = startY; screenY < endY; ++screenY)
	{
		uint16_t* rowPixels = &destPixels[(screenY * surfacePitch) / 2];
		int16_t zValue = currentZValue;

		for (int screenX = startX; screenX < endX; ++screenX)
		{
			// Position relative to rectangle (0-511, 0-255)
			int relX = screenX - rectLeft;
			int relY = screenY - rectTop;

			// Calculate position from center (matches original coordinate system)
			int deltaX = relX - rectCenterX;  // -256 to +255
			int deltaY = relY - rectCenterY;  // -128 to +127

			// CRITICAL: Use elliptical distance (4*y*y) to match original
			// This compensates for isometric projection
			float distanceSquared = (float)(deltaX * deltaX + 4 * deltaY * deltaY);
			float distance = std::sqrt(distanceSquared);

			// Calculate wave intensity using original formula
			float intensity = CalculateWaveIntensity(distance, state);

			// Skip if intensity too low (matches original > 0 check)
			if (intensity <= 0.0f)
				continue;

			// Z-buffer check (matches original exactly)
			int16_t* zBufferPixel = (int16_t*)ZBuffer::Instance->GetBuffer(screenX, screenY);
			if (!zBufferPixel || (uint16_t)*zBufferPixel <= zValue)
				continue;

			// Calculate distortion offset
			float angle = std::atan2((float)deltaY, (float)deltaX);
			Point2D offset = CalculateDistortionOffset(intensity, distance, angle, deltaX, deltaY);

			// Apply offset to get source pixel
			int sourceX = screenX + offset.X;
			int sourceY = screenY + offset.Y;

			// Bounds check and copy pixel
			if (sourceX >= 0 && sourceX < surfaceWidth &&
				sourceY >= 0 && sourceY < surfaceHeight)
			{
				uint16_t* sourcePixel = &destPixels[(sourceY * surfacePitch) / 2 + sourceX];
				rowPixels[screenX] = *sourcePixel;
			}
		}

		// Decrement Z-value per row (matches original --v9)
		--currentZValue;
	}

	destSurface->Unlock();
}

// ===== WAVE MATHEMATICS =====

float ModernIonBlastClass::CalculateWaveIntensity(float distance, int frameState) const
{
	// Use configurable wave parameters
	double wavePhase = distance - (double)frameState * Config.WaveSpeed + Config.WavePhaseOffset;
	double waveValue = std::sin(wavePhase * Config.WaveFrequency) * Config.WaveAmplitude + Config.WaveBase;
	double distanceFalloff = distance / Config.DistanceScale + 1.0;

	return (float)(waveValue / distanceFalloff + 0.5);
}

Point2D ModernIonBlastClass::CalculateDistortionOffset(float intensity, float distance, float angle, int deltaX, int deltaY) const
{
	if (intensity < Config.MinDistortionThreshold)
	{
		return { 0, 0 };
	}

	switch (Config.Pattern)
	{
	case DistortionPattern::Spiral:
		return CalculateSpiralOffset(intensity, distance, angle);

	case DistortionPattern::Box:
		return CalculateBoxOffset(intensity, deltaX, deltaY);

	case DistortionPattern::Radial:
		return CalculateRadialOffset(intensity, distance, angle);

	case DistortionPattern::Wave:
		return CalculateWaveOffset(intensity, distance, angle, state);

	default:
		return { 0, 0 };
	}
}

// ===== PATTERN-SPECIFIC CALCULATIONS =====

Point2D ModernIonBlastClass::CalculateSpiralOffset(float intensity, float distance, float angle) const
{
	// Original spiral pattern (complex but authentic)
	int indexValue = (int)(intensity * 10.0f);
	if (indexValue <= 0) return { 0, 0 };

	int ring = 1;
	int adjustedIndex = indexValue - 1;

	if (adjustedIndex >= 8)
	{
		for (int ringSize = 8; ringSize <= adjustedIndex && ring < 20; ringSize += 8)
		{
			adjustedIndex -= ringSize;
			++ring;
		}
	}

	int spiralX, spiralY;
	if (adjustedIndex >= 2 * ring + 1)
	{
		if (adjustedIndex >= 4 * ring + 1)
		{
			if (adjustedIndex >= 6 * ring + 1)
			{
				spiralX = adjustedIndex - 7 * ring;
				spiralY = -ring;
			}
			else
			{
				spiralX = -ring;
				spiralY = 5 * ring - adjustedIndex;
			}
		}
		else
		{
			spiralX = 3 * ring - adjustedIndex;
			spiralY = ring;
		}
	}
	else
	{
		spiralX = ring;
		spiralY = adjustedIndex - ring;
	}

	return { spiralX, spiralY };
}

Point2D ModernIonBlastClass::CalculateBoxOffset(float intensity, int deltaX, int deltaY) const
{
	// Simple box/grid pattern - clean geometric distortion
	float displacement = intensity * 2.0f;

	// Create grid-like distortion
	int gridX = (deltaX / 16) * (int)displacement;  // 16-pixel grid
	int gridY = (deltaY / 16) * (int)displacement;

	return { gridX, gridY };
}

Point2D ModernIonBlastClass::CalculateRadialOffset(float intensity, float distance, float angle) const
{
	// Pure radial displacement - pushes pixels outward from center
	float displacement = intensity * 3.0f;

	return {
		(int)(std::cos(angle) * displacement),
		(int)(std::sin(angle) * displacement)
	};
}

Point2D ModernIonBlastClass::CalculateWaveOffset(float intensity, float distance, float angle, int frameState) const
{
	// Ripple wave pattern - concentric circular waves
	float wavePhase = distance * 0.1f - frameState * 0.2f;
	float waveFactor = std::sin(wavePhase) * intensity * 2.0f;

	// Displacement perpendicular to radius (tangential)
	float perpAngle = angle + 1.5708f; // +90 degrees

	return {
		(int)(std::cos(perpAngle) * waveFactor),
		(int)(std::sin(perpAngle) * waveFactor)
	};
}

// ===== UTILITY METHODS =====

bool ModernIonBlastClass::IsPixelInBlastArea(int deltaX, int deltaY, int frameState) const
{
	if (Config.UseCircular)
	{
		// Circular area check
		float distanceSquared = (float)(deltaX * deltaX + deltaY * deltaY);
		float radiusSquared = (float)(Config.Radius * Config.Radius);
		return distanceSquared <= radiusSquared;
	}
	else
	{
		// Rectangular area check (matches original exactly)
		int halfWidth = Config.Width / 2;
		int halfHeight = Config.Height / 2;
		return (abs(deltaX) <= halfWidth && abs(deltaY) <= halfHeight);
	}
}

// ===== PHYSICS EFFECTS (Based on original system) =====
#include <Ext/WarheadType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Anim/Body.h>

void ModernIonBlastClass::InitializeOwnership(TechnoClass* pInvoker, HouseClass* pOwner)
{
	if (dword10 == 1)
		return;

	this->Invoker = pInvoker;

	if (!pOwner && pInvoker && pInvoker->Owner)
		this->Owner = pInvoker->Owner;
	else if (pOwner)
		this->Owner = pOwner;
	else
		this->Owner = HouseExtData::FindFirstCivilianHouse();
}

void ModernIonBlastClass::InitializeBlastEffects(WarheadTypeClass* pDamagingWarhead, WarheadTypeClass* pSourceWarhead)
{
	if (dword10 == 1)
		return;

	this->Warhead = pDamagingWarhead;

	if (!this->Warhead) {
		this->Warhead = RulesClass::Instance->IonCannonWarhead;
	}

	auto nLoc = this->coord;
	const auto pCell = MapClass::Instance->GetCellAt(nLoc);

	//all other properties were using Source Warhead
	//the Damaging Warhead only handle the damaging part
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pSourceWarhead);

	AnimTypeClass* pBlast = nullptr;
	if (pCell->LandType == LandType::Water)
		this->Blast = RulesClass::Instance->SplashList[RulesClass::Instance->SplashList.Count - 1];
	else
		this->Blast = pWHExt->Ion_Blast.Get(RulesClass::Instance->IonBlast);

	this->Beam = pWHExt->Ion_Beam.Get(RulesClass::Instance->IonBeam);
	this->Damage = pWHExt->Ion_Damage.Get(RulesClass::Instance->IonCannonDamage);
	this->Rocking = pWHExt->Ion_Rocking;
}

void ModernIonBlastClass::ApplyBlastEffects()
{
	CoordStruct animCoord = { coord.X, coord.Y, coord.Z + 5 };

	if (this->Blast) {
		// Create splash effect for water terrain
		AnimClass* splashAnim = new AnimClass(
			this->Blast,
			animCoord,
			0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0
		);

		AnimExtData::SetAnimOwnerHouseKind(splashAnim, this->Owner,
			nullptr, this->Invoker, false, false);
	}

	if(this->Beam){
		// Always create ion beam effect
		AnimClass* beamAnim = new AnimClass(
			this->Beam, animCoord,
			0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0
		);

		AnimExtData::SetAnimOwnerHouseKind(beamAnim, this->Owner,
			nullptr, this->Invoker, false, false);
	}
}

void ModernIonBlastClass::ApplyBlastDamage()
{
	CellClass* targetCell = MapClass::Instance->GetCellAt(coord);

	// Apply damage to bridges if present
	if (targetCell->ContainsBridge())
	{
		CoordStruct bridgeCoord = { coord.X, coord.Y, coord.Z + Unsorted::BridgeHeight };
		DamageArea::Apply(&bridgeCoord,
			this->Damage, this->Invoker,
			this->Warhead, 1, this->Owner);
	}

	// Apply main blast damage
	DamageArea::Apply(&coord,
		this->Damage, this->Invoker,
		this->Warhead, 1, this->Owner);

	// Create screen flash effect
	MapClass::FlashbangWarheadAt(
		this->Damage,
		this->Warhead
		, coord, 0, SpotlightFlags::None);
}

void ModernIonBlastClass::ApplyVisualEffectsToUnits(const Point2D& blastPixel)
{
	if (!this->Rocking)
		return;

	// Use configurable parameters instead of hardcoded values
	int gridRadius = 3; // Base grid radius
	int maxEffectDistance;

	// Scale effect distance based on current blast configuration
	if (Config.UseCircular)
	{
		maxEffectDistance = Config.Radius;
		gridRadius = std::max(3, Config.Radius / 64); // Scale grid with radius
	}
	else
	{
		maxEffectDistance = std::max(Config.Width, Config.Height) / 2;
		gridRadius = std::max(3, maxEffectDistance / 64);
	}

	// Convert blast coordinates to cell coordinates
	CellStruct blastCell = { coord.X / 256, coord.Y / 256 };

	// Check units in a grid around the blast center (size based on blast configuration)
	for (int deltaX = -gridRadius; deltaX <= gridRadius; ++deltaX)
	{
		for (int deltaY = -gridRadius; deltaY <= gridRadius; ++deltaY)
		{

			CellStruct checkCell = { blastCell.X + deltaX, blastCell.Y + deltaY };
			CellClass* cell = MapClass::Instance->GetCellAt(checkCell);

			FootClass* unit = flag_cast_to<FootClass*>(cell->FirstObject);
			while (unit != nullptr)
			{

				// Only affect infantry and vehicles
				AbstractType unitType = unit->WhatAmI();

				if (unitType == AbstractType::Infantry || unitType == AbstractType::Unit) {
					ProcessUnitInBlast(unit, blastPixel, maxEffectDistance);
				}

				unit = (FootClass*)unit->NextObject;
			}
		}
	}
}

void ModernIonBlastClass::ProcessUnitInBlast(FootClass* unit, const Point2D& blastPixel, int maxEffectDistance)
{
	// Convert unit coordinates to screen pixels
	Point2D unitPixel = TacticalClass::Instance->CoordsToClient(unit->Location);

	// Calculate distance from blast center
	int deltaX = unitPixel.X - blastPixel.X;
	int deltaY = unitPixel.Y - blastPixel.Y;
	int distance = (int)std::sqrt((double)(deltaX * deltaX + deltaY * deltaY)) + 8;

	// Only affect units within configurable blast radius
	if (distance >= maxEffectDistance)
	{
		return;
	}

	// Apply ion blast surface effects
	ApplyIonSurfaceEffectsToUnit(unit, distance);

	// Apply physics effects if unit has voxel model
	ApplyPhysicsEffectsToUnit(unit, distance);
}

void ModernIonBlastClass::ApplyIonSurfaceEffectsToUnit(FootClass* unit, int distance)
{
	// Use configurable wave parameters instead of hardcoded values
	float waveDistance = (float)distance;
	float intensity = CalculateWaveIntensity(waveDistance, state);

	// Use configurable threshold instead of hardcoded 0.5f
	if (intensity > Config.MinDistortionThreshold * 5.0f)
	{ // Scale threshold for unit effects
		// Stop unit movement
		unit->SetSpeedPercentage(0.0);

		// Apply visual distortion effect based on configurable wave intensity
		float scaledIntensity = intensity * (Config.WaveAmplitude / 3.5f); // Scale relative to original amplitude
		int distortionValue = (int)(scaledIntensity * 10.0f);
		Point2D coords;
		IndexToCoordinates(&coords, distortionValue);

		// Scale the height effect based on wave amplitude
		float heightScale = (Config.WaveAmplitude / 3.5f) * 2.0f;
		unit->height_subtract_6B4 = (int)(coords.Y * heightScale);
	}
}

void ModernIonBlastClass::ApplyRotationEffectsToUnit(FootClass* unit, const Vector3D<float>& direction,
											 float sinFacing, float cosFacing, float distance)
{
	// Use configurable wave parameters instead of hardcoded values
	double wavePhase = distance - (double)state * Config.WaveSpeed + Config.WavePhaseOffset;
	double waveAmplitude = std::sin(wavePhase * Config.WaveFrequency) * Config.WaveAmplitude + Config.WaveBase;
	double waveDerivative = std::cos(wavePhase * Config.WaveFrequency) * Config.WaveFrequency * Config.DistanceScale * Config.WaveAmplitude;

	// Calculate rotation vectors
	float rotationFactorX = direction.X * cosFacing + direction.Y * sinFacing;
	float rotationFactorY = direction.X * sinFacing - direction.Y * cosFacing;
	float rotationMagnitude = std::sqrt(rotationFactorY * rotationFactorY +
										   direction.Z * direction.Z);

	// Validate rotation direction (same validation logic but configurable thresholds)
	float validationThreshold = Config.MinDistortionThreshold * 0.02f; // Scale threshold
	if (Math::abs(cosFacing * rotationFactorX - sinFacing * rotationMagnitude - direction.X) > validationThreshold ||
		Math::abs(cosFacing * rotationMagnitude + sinFacing * rotationFactorX - direction.Y) > validationThreshold)
	{
		rotationMagnitude = -rotationMagnitude;
	}

	// Apply calculated rotations to unit with configurable scaling
	double totalDistance = distance + Config.DistanceScale;
	double rotationIntensity = (waveDerivative * totalDistance - waveAmplitude * Config.DistanceScale) /
		(totalDistance * totalDistance);

	// Scale rotation effects based on wave amplitude
	float rotationScale = (Config.WaveAmplitude / 3.5f) * Math::GAME_TWOPI; // Scale relative to original

	unit->AngleRotatedSideways = rotationMagnitude * rotationIntensity * rotationScale;
	unit->AngleRotatedForwards = -(rotationFactorX * rotationIntensity * rotationScale);
}

void ModernIonBlastClass::ApplyPhysicsEffectsToUnit(FootClass* unit, int distance)
{
	TechnoTypeClass* unitType = unit->GetTechnoType();

	// Check if unit has voxel model for physics simulation
	if (!unitType->MainVoxel.VXL || unitType->MainVoxel.VXL->LoadFailed)
	{
		return;
	}

	// Only apply physics if intensity is above configurable threshold
	float waveDistance = (float)distance;
	float intensity = CalculateWaveIntensity(waveDistance, state);

	if (intensity <= Config.MinDistortionThreshold)
	{
		return; // Not strong enough for physics effects
	}

	// Calculate physics vectors and rotations
	Vector3D<float> blastToUnit = CalculateBlastVector(this, unit);
	float magnitude = std::sqrt(blastToUnit.X * blastToUnit.X + blastToUnit.Y * blastToUnit.Y + blastToUnit.Z * blastToUnit.Z);
	Vector3D<float> normalizedDirection = NormalizeVector(blastToUnit);

	// Get unit facing direction
	const DirStruct unitFacing = unit->PrimaryFacing.Current();
	double facingAngle = (unitFacing.Raw - Math::BINARY_ANGLE_MASK) * Math::DIRECTION_FIXED_MAGIC;

	// Calculate rotation effects based on configurable blast wave
	float sinFacing = std::sin(facingAngle);
	float cosFacing = std::cos(facingAngle);

	ApplyRotationEffectsToUnit(unit, normalizedDirection, sinFacing, cosFacing, magnitude);
}

// Converts index back to 2D coordinates (from original IndexToCoordinates logic)
Point2D ModernIonBlastClass::IndexToCoordinates(Point2D* result, int number)
{
	// Bounds checking to prevent issues
	if (number <= 0)
	{
		result->X = 0;
		result->Y = 0;
		return *result;
	}

	int adjustedNumber = number - 1;
	int ring = 1;

	// Find which ring this index belongs to
	if (adjustedNumber >= 8)
	{
		for (int ringSize = 8; ringSize <= adjustedNumber && ring < 100; ringSize += 8)
		{
			adjustedNumber -= ringSize;
			++ring;
		}
	}

	// Determine position within the ring based on remaining index
	if (adjustedNumber >= 2 * ring + 1)
	{
		if (adjustedNumber >= 4 * ring + 1)
		{
			if (adjustedNumber >= 6 * ring + 1)
			{
				// Bottom edge (4th quadrant)
				result->X = adjustedNumber - 7 * ring;
				result->Y = -ring;
			}
			else
			{
				// Left edge (3rd quadrant)
				result->X = -ring;
				result->Y = 5 * ring - adjustedNumber;
			}
		}
		else
		{
			// Top edge (2nd quadrant)
			result->X = 3 * ring - adjustedNumber;
			result->Y = ring;
		}
	}
	else
	{
		// Right edge (1st quadrant)
		result->X = ring;
		result->Y = adjustedNumber - ring;
	}

	return *result;
}