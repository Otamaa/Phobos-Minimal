#include"AttachmentLocomotionClass.h"

#include <CellSpread.h>
#include <ScenarioClass.h>

#include <ParticleSystemClass.h>
#include <ParticleSystemTypeClass.h>

#include <AnimClass.h>
#include <AircraftClass.h>
#include <AircraftTrackerClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <New/Entity/AttachmentClass.h>
#include <New/Type/AttachmentTypeClass.h>

#include <Ext/Techno/Body.h>

// TODO maybe some macros for repeated parent function calls?

bool AttachmentLocomotionClass::Is_Moving()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco && pParentLoco->Is_Moving();
}

Matrix3D* AttachmentLocomotionClass::Draw_Matrix(Matrix3D* pMatrix, VoxelIndexKey* key)
{
	if (auto const pParentFoot = flag_cast_to<FootClass*>(this->GetAttachmentParent()))
	{
		pParentFoot->Locomotor->Draw_Matrix(pMatrix, key);

		// adjust for the real facing which is the source of truth for hor. rotation
		double childRotation = this->LinkedTo->PrimaryFacing.Current().GetRadian<32>();
		double parentRotation = pParentFoot->PrimaryFacing.Current().GetRadian<32>();
		float adjustmentAngle = (float)(childRotation - parentRotation);

		pMatrix->RotateZ(adjustmentAngle);

		if (key && key->Is_Valid_Key())
			key->MainVoxel.FrameIndex = this->LinkedTo->PrimaryFacing.Current().GetFacing<32>();

		return pMatrix;
	}

	return LocomotionClass::Draw_Matrix(pMatrix ,key);
}

// Shadow drawing works acceptable as is. It draws separate units as normal.
// A possibly better solution would be to actually "merge" the shadows
// and draw them as a single one, but this needs calculating the extension
// of the parent slope plane to calculate the correct offset for Shadow_Point,
// complicated trigonometry that would be a waste of time at this point.

// If you want to work on this - Shadow_Matrix should be fine to copy from Draw_Matrix,
// (even the key shenanigans can be left the same), butShadow_Point would need to be
// calculated using height from the ramp extension plane - Kerbiter

Point2D AttachmentLocomotionClass::Draw_Point()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Draw_Point()
		: LocomotionClass::Draw_Point();
}

VisualType AttachmentLocomotionClass::Visual_Character(VARIANT_BOOL raw)
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Visual_Character(raw)
		: LocomotionClass::Visual_Character(raw);
}

int AttachmentLocomotionClass::Z_Adjust()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Z_Adjust()
		: LocomotionClass::Z_Adjust();
}

ZGradient AttachmentLocomotionClass::Z_Gradient()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Z_Gradient()
		: LocomotionClass::Z_Gradient();
}

bool AttachmentLocomotionClass::Process()
{
	if (this->LinkedTo->IsAlive)
	{
		Layer newLayer = this->In_Which_Layer();
		Layer oldLayer = this->PreviousLayer;

		bool changedAirborneStatus = false;

		if (oldLayer != newLayer)
		{
			DisplayClass::Instance->SubmitObject(this->LinkedTo);

			if (oldLayer < Layer::Air && Layer::Air <= newLayer)
			{
				AircraftTrackerClass::Instance->Add(this->LinkedTo);
				changedAirborneStatus = true;
			}
			else if (newLayer < Layer::Air && Layer::Air <= oldLayer)
			{
				AircraftTrackerClass::Instance->Remove(this->LinkedTo);
				changedAirborneStatus = true;
			}

			this->PreviousLayer = newLayer;
		}

		CellStruct oldPos = this->PreviousCell;
		CellStruct newPos = this->LinkedTo->GetMapCoords();

		if (oldPos != newPos)
		{
			if (Layer::Air <= newLayer && !changedAirborneStatus)
				AircraftTrackerClass::Instance->Update(this->LinkedTo, oldPos, newPos);

			if (this->LinkedTo->GetTechnoType()->SensorsSight)
			{
				this->LinkedTo->RemoveSensorsAt(oldPos);
				this->LinkedTo->AddSensorsAt(newPos);
			}
		}

		this->PreviousCell = newPos;
	}

	// sight is handled in FootClass::AI

	AttachmentClass* pAttachment = this->GetAttachment();
	if (pAttachment && pAttachment->GetType()->InheritHeightStatus)
	{
		this->LinkedTo->OnBridge = pAttachment->Parent->OnBridge;
	}
	else
	{
		this->LinkedTo->OnBridge = false;  // GetHeight returns different height depending on this
		this->LinkedTo->OnBridge = this->ShouldBeOnBridge();
	}

	return LocomotionClass::Process();
}

// I am not sure this does anything and could probably be removed
bool AttachmentLocomotionClass::Is_Powered()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		&& pParentLoco->Is_Powered();
}

bool AttachmentLocomotionClass::Is_Ion_Sensitive()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		&& pParentLoco->Is_Ion_Sensitive()
		|| LocomotionClass::Is_Ion_Sensitive();
}

Layer AttachmentLocomotionClass::In_Which_Layer()
{
	AttachmentClass* pAttachment = this->GetAttachment();
	if (!pAttachment || !pAttachment->GetType()->InheritHeightStatus)
		return this->CalculateLayer();

	auto const pParentAsFoot = flag_cast_to<FootClass*>(pAttachment->Parent);
	return pParentAsFoot && pParentAsFoot->Locomotor
		? pParentAsFoot->Locomotor->In_Which_Layer()
		: this->CalculateLayer();
}

bool AttachmentLocomotionClass::Is_Moving_Now()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco && pParentLoco->Is_Moving_Now();
}

int AttachmentLocomotionClass::Apparent_Speed()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Apparent_Speed()
		: LocomotionClass::Apparent_Speed();
}

FireError AttachmentLocomotionClass::Can_Fire()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Can_Fire()
		: LocomotionClass::Can_Fire();
}

int AttachmentLocomotionClass::Get_Status()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Get_Status()
		: LocomotionClass::Get_Status();
}

bool AttachmentLocomotionClass::Is_Surfacing()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		&& pParentLoco->Is_Surfacing()
		|| LocomotionClass::Is_Surfacing();
}

bool AttachmentLocomotionClass::Is_Really_Moving_Now()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco && pParentLoco->Is_Really_Moving_Now();
}

void AttachmentLocomotionClass::Clear_Coords()
{
	this->PreviousLayer = Layer::None;
	this->PreviousCell = CellStruct::Empty;
	// AircraftTracker is handled by FootClass::Limbo
}

HRESULT AttachmentLocomotionClass::Begin_Piggyback(ILocomotion* pointer)
{
	if (!pointer)
		return E_POINTER;

	if (this->Piggybacker)
		return E_FAIL;

	// since LinkedTo may've been managed by AircraftTracker before we need to remove the AircraftTracker entry
	if (this->LinkedTo && this->LinkedTo->GetLastFlightMapCoords() != CellStruct::Empty)
		AircraftTrackerClass::Instance->Remove(this->LinkedTo);

	this->Piggybacker = pointer;

	return S_OK;
}

HRESULT AttachmentLocomotionClass::End_Piggyback(ILocomotion** pointer)
{
	if (!pointer)
		return E_POINTER;

	if (!this->Piggybacker)
		return S_FALSE;

	// since LinkedTo may no longer be considered airborne we need to remove the AircraftTracker entry
	if (this->LinkedTo && this->LinkedTo->GetLastFlightMapCoords() != CellStruct::Empty)
		AircraftTrackerClass::Instance->Remove(this->LinkedTo);

	// since pointer is a dumb pointer, we don't need to call Release,
	// hence we use Detach, otherwise the locomotor gets trashed
	*pointer = this->Piggybacker.Detach();

	// in order to play nice with IsLocomotor warheads probably also should
	// handle IsAttackedByLocomotor etc. warheads here, but none of the vanilla
	// warheads do this (except JumpjetLocomotionClass::End_Piggyback)

	return S_OK;
}

bool AttachmentLocomotionClass::Is_Ok_To_End()
{
	// Actually a confusing name, should return true only if the piggybacking should be ended.
	return this->Piggybacker
		&& !this->GetAttachmentParent();
}

HRESULT AttachmentLocomotionClass::Piggyback_CLSID(GUID* classid)
{
	HRESULT hr;

	if (classid == nullptr)
		return E_POINTER;

	if (this->Piggybacker)
	{
		IPersistStreamPtr piggyAsPersist(this->Piggybacker);

		hr = piggyAsPersist->GetClassID(classid);
	}
	else
	{
		if (reinterpret_cast<IPiggyback*>(this) == nullptr)
			return E_FAIL;

		IPersistStreamPtr thisAsPersist(this);

		if (thisAsPersist == nullptr)
			return E_FAIL;

		hr = thisAsPersist->GetClassID(classid);
	}

	return hr;
}

bool AttachmentLocomotionClass::Is_Piggybacking()
{
	return this->Piggybacker != nullptr;
}

// non-virtuals

AttachmentClass* AttachmentLocomotionClass::GetAttachment()
{
	AttachmentClass* result = nullptr;

	if (this->LinkedTo)
	{
		if (auto const pExt = TechnoExtContainer::Instance.Find(this->LinkedTo))
			result = pExt->ParentAttachment;
	}

	return result;
}

TechnoClass* AttachmentLocomotionClass::GetAttachmentParent()
{
	TechnoClass* result = nullptr;

	if (auto const pAttachment = this->GetAttachment())
		result = pAttachment->Parent;

	return result;
}

ILocomotionPtr AttachmentLocomotionClass::GetAttachmentParentLoco()
{
	ILocomotionPtr result { };

	if (auto const pTechno = this->GetAttachmentParent())
	{
		if (auto const pFoot = flag_cast_to<FootClass*>(pTechno))
			result = pFoot->Locomotor;
	}

	return result;
}

Layer AttachmentLocomotionClass::CalculateLayer()
{
	auto const pExt = TechnoTypeExtContainer::Instance.Find(this->LinkedTo->GetTechnoType());
	int height = this->LinkedTo->GetHeight();

	if (this->LinkedTo->IsInAir())
	{
		if (!this->LinkedTo->OnBridge && this->ShouldBeOnBridge())
			height -= CellClass::BridgeHeight;

		return height >= pExt->AttachmentTopLayerMinHeight
			? Layer::Top : Layer::Air;
	}
	else if (this->LinkedTo->IsOnFloor())
	{
		return height <= pExt->AttachmentUndergroundLayerMaxHeight
			? Layer::Underground : Layer::Ground;
	}

	return Layer::None;
}

bool AttachmentLocomotionClass::ShouldBeOnBridge()
{
	return MapClass::Instance->GetCellAt(this->LinkedTo->Location)->ContainsBridge()
		&& this->LinkedTo->GetHeight() >= CellClass::BridgeHeight && !this->LinkedTo->IsFallingDown;
}