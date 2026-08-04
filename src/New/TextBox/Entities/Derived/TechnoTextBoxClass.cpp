#include "TechnoTextBoxClass.h"
#include <New/TextBox/Types/TextBoxTypeClass.h>

#include <StringTable.h>
#include <TacticalClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <TeamClass.h>
#include <TagClass.h>
#include <CellClass.h>
#include <SessionClass.h>
#include <Unsorted.h>
#include <TeamTypeClass.h>

#include <Utilities/Debug.h>
#include <Utilities/Stream.h>

#include <Ext/Rules/Body.h>

#include <algorithm>

static TechnoClass* ResolveTargetByUID(DWORD uid);

// DIFF: no static array definition - MapTextBoxClass::Array is the sole owner.

// ========== 构造 ==========

TechnoTextBoxClass::TechnoTextBoxClass(
	TechnoClass* pTarget,
	const char* csfLabel,
	const char* typeName)
	: Target(pTarget)
{
	const TextBoxTypeClass* pType = TextBoxTypeClass::Find(typeName);

	if (!pType)
	{
		Debug::Log("[TechnoTextBoxClass] Warning: type \"%s\" not found!\n", typeName);
		this->CurrentLabel = csfLabel ? csfLabel : "";
		return;
	}

	// 从类型复制样式参数
	this->CurrentLabel = csfLabel ? csfLabel : "";
	this->MaxLineWidth = pType->MaxWidth;
	this->BackgroundOpacity = pType->BackgroundOpacity;
	this->ColorR = pType->ColorR;
	this->ColorG = pType->ColorG;
	this->ColorB = pType->ColorB;
	this->RemainingFrames = pType->Duration;

	// ADD: record the type index so RemoveByType can match on identity.
	for (size_t i = 0; i < TextBoxTypeClass::Array.size(); ++i)
	{
		if (TextBoxTypeClass::Array[i].get() == pType)
		{
			this->TypeIndex = static_cast<int>(i);
			break;
		}
	}
}

// ============================================================================
// 生存判定 - GAMESTATE ONLY
//
// Drives removal, so every machine must reach the same answer. No shroud, no
// TacticalClass, no CurrentPlayer. See MapTextBoxClass.h.
//
// DIFF: the ResolveTargetByUID retry that used to live at the top of CanDraw()
// - behind a const_cast, running an O(TechnoClass::Array) scan per stale label
// per frame - is gone. UID resolution is load-time repair and now happens once,
// in Load(). With PointerGotInvalid wired up, Target is nulled at the moment
// the engine frees the object, so there is nothing left to retry.
// ============================================================================

bool TechnoTextBoxClass::IsAlive() const
{
	if (!this->Target)
		return false;

	// Health <= 0 is gamestate and deterministic.
	if (this->Target->Health <= 0)
		return false;

	return true;
}

// ============================================================================
// 绘制判定 - may read local state freely
// ============================================================================

bool TechnoTextBoxClass::CanDraw() const
{
	if (!this->Target)
		return false;

	// InLimbo is gamestate, but it means "temporarily not on the map" (loaded
	// into a transport, etc.), not "destroyed" - so it suppresses drawing
	// rather than removing the label. Kept on this side deliberately.
	if (this->Target->InLimbo)
		return false;

	// 黑幕遮挡检测 - LOCAL, per-player. Must never influence removal.
	if (FakeRulesClass::Instance->ShowTextBoxInShroud_Techno)
	{
		CellStruct cell = CellClass::Coord2Cell(this->Target->GetCoords());

		// BUGFIX: was `char isShrouded` compared against -2, which relies on
		// char being signed. Correct under MSVC's default but silently wrong
		// under /J. int is unambiguous.
		const int occlusion = static_cast<int>(
			TacticalClass::Instance->GetOcclusion(cell, false));

		if (occlusion == -2) // -2 表示完全在黑幕中
			return false;
	}

	return true;
}

bool TechnoTextBoxClass::GetDrawPosition(Point2D& outPos) const
{
	if (!TacticalClass::Instance() || !this->Target)
		return false;

	CoordStruct coords = this->Target->GetCoords();
	const auto& _result = TacticalClass::Instance->GetCoordsToClientSituation(coords);
	outPos = _result.first;
	return _result.second;
}

// ============================================================================
// 指针失效通知
//
// Dispatched automatically by PhobosTypeRegistry::InvalidatePointer, which is
// driven by the AnnounceInvalidPointer hook. Runs before the object is freed,
// so nulling Target here closes the use-after-free window entirely.
//
// The label is NOT removed here: removal happens on the next DrawAll() pass via
// IsAlive(). Mutating the array from inside an engine callback risks re-entering
// it while a caller is mid-iteration.
// ============================================================================

void TechnoTextBoxClass::PointerGotInvalid(void* ptr, bool removed)
{
	MapTextBoxClass::Array.ForEachOf<TechnoTextBoxClass>(
		[ptr](TechnoTextBoxClass& label)
		{
			if (label.Target == ptr)
			{
				label.Target = nullptr;

				// The UID is kept deliberately - it stays meaningful for a
				// save written after this point, and IsAlive() will drop the
				// label on the next frame regardless.
			}
		});
}

// ========== 查找/创建 ==========

TechnoTextBoxClass* TechnoTextBoxClass::Find(TechnoClass* pTarget)
{
	if (!pTarget)
		return nullptr;

	return MapTextBoxClass::Array.FindOf<TechnoTextBoxClass>(
		[pTarget](TechnoTextBoxClass& label) { return label.Target == pTarget; });
}

TechnoTextBoxClass* TechnoTextBoxClass::FindOrCreate(TechnoClass* pTarget,
	const char* csfLabel, const char* typeName)
{
	if (!pTarget)
		return nullptr;

	const TextBoxTypeClass* pType = TextBoxTypeClass::Find(typeName);

	if (!pType)
		return nullptr;

	if (auto* pLabel = Find(pTarget))
	{
		// 更新已有实例的样式和内容
		pLabel->CurrentLabel = csfLabel ? csfLabel : "";
		pLabel->MaxLineWidth = pType->MaxWidth;
		pLabel->BackgroundOpacity = pType->BackgroundOpacity;
		pLabel->ColorR = pType->ColorR;
		pLabel->ColorG = pType->ColorG;
		pLabel->ColorB = pType->ColorB;
		pLabel->RemainingFrames = pType->Duration;

		for (size_t i = 0; i < TextBoxTypeClass::Array.size(); ++i)
		{
			if (TextBoxTypeClass::Array[i].get() == pType)
			{
				pLabel->TypeIndex = static_cast<int>(i);
				break;
			}
		}

		pLabel->UpdateLayout();
		return pLabel;
	}

	// DIFF: was make_shared + push_back into both arrays + the fragile
	// `return MapTextBoxClass::Array.back().get()` after a move.
	auto* pLabel = MapTextBoxClass::Array.Create<TechnoTextBoxClass>(
		pTarget, csfLabel, typeName);

	pLabel->UpdateLayout();

	return pLabel;
}

// ============================================================================
// 移除
//
// DIFF: each of these was ~25 lines - a manual while-loop over the derived
// array with an erase, then a nested find_if into the base array keyed on the
// raw pointer. Five near-identical copies. Forgetting either half leaked or
// dangled; this is where ClearAll()'s bug came from.
// ============================================================================

void TechnoTextBoxClass::Remove(TechnoClass* pTarget)
{
	if (!pTarget)
		return;

	MapTextBoxClass::Array.RemoveIfOf<TechnoTextBoxClass>(
		[pTarget](TechnoTextBoxClass& label) { return label.Target == pTarget; });
}

void TechnoTextBoxClass::RemoveByType(int typeIndex)
{
	if (typeIndex < 0 || static_cast<size_t>(typeIndex) >= TextBoxTypeClass::Array.size())
		return;

	// BUGFIX: the original compared MaxLineWidth, BackgroundOpacity and all
	// three colour components against the type's values, so any two
	// TextBoxTypes that happened to look identical were indistinguishable and
	// removing one removed the other's labels. It also called
	// TextBoxTypeClass::Find(typeName) once per iteration for a loop-invariant
	// result. Matching on the recorded index is exact and free.
	MapTextBoxClass::Array.RemoveIfOf<TechnoTextBoxClass>(
		[typeIndex](TechnoTextBoxClass& label) { return label.TypeIndex == typeIndex; });
}

void TechnoTextBoxClass::RemoveByTrigger(TriggerClass* pTrigger)
{
	if (!pTrigger)
		return;

	MapTextBoxClass::Array.RemoveIfOf<TechnoTextBoxClass>(
		[pTrigger](TechnoTextBoxClass& label)
		{
			// BUGFIX: the original dereferenced Target here without the
			// Health <= 0 guard used elsewhere, so a label whose unit had just
			// been freed read dead memory. With PointerGotInvalid wired up
			// Target is null by then, and this null check is now sufficient.
			if (!label.Target || !label.Target->AttachedTag)
				return false;

			return label.Target->AttachedTag->ContainsTrigger(pTrigger);
		});
}

void TechnoTextBoxClass::RemoveByTeam(int teamIndex)
{
	// SUSPECT: preserved verbatim. The team ID is built as "0" + index, which
	// breaks for teamIndex >= 10 and assumes a specific ID format. VERIFY
	// against how TeamTypeClass IDs are actually generated before relying on
	// this for anything but single-digit team indices.
	const std::string teamID = "0" + std::to_string(teamIndex);

	MapTextBoxClass::Array.RemoveIfOf<TechnoTextBoxClass>(
		[&teamID](TechnoTextBoxClass& label)
		{
			if (!label.Target)
				return false;

			FootClass* pFoot = flag_cast_to<FootClass*>(label.Target);

			return pFoot && pFoot->Team && pFoot->Team->Type
				&& pFoot->Team->Type->get_ID() == teamID;
		});
}

// ========== 全局清理 ==========

void TechnoTextBoxClass::ClearAll()
{
	// DIFF: was a manual loop over the base array doing an inner find_if into
	// the derived array for every element, then clearing the derived array.
	MapTextBoxClass::Array.RemoveAllOf<TechnoTextBoxClass>();
}

void TechnoTextBoxClass::Clear()
{
	ClearAll();
}

// ========== UID 解析 ==========

static TechnoClass* ResolveTargetByUID(DWORD uid)
{
	if (uid == 0)
		return nullptr;

	for (auto pTechno : *TechnoClass::Array) {
		if (pTechno && pTechno->UniqueID == uid)
			return pTechno;
	}

	return nullptr;
}

// ============================================================================
// 序列化
//
// DIFF: derived fields only; the base writes the shared ones. The original
// duplicated every base field here and never chained to the base at all.
// ============================================================================

template <typename T>
bool TechnoTextBoxClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->SavedTargetUID)
		.Success();
}

bool TechnoTextBoxClass::Save(PhobosStreamWriter& Stm) const
{
	if (!this->MapTextBoxClass::Save(Stm))
		return false;

	// Refresh the UID from the live pointer before writing.
	auto* pMutable = const_cast<TechnoTextBoxClass*>(this);
	pMutable->SavedTargetUID = this->Target ? this->Target->UniqueID : 0;

	return pMutable->Serialize(Stm);
}

bool TechnoTextBoxClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	if (!this->MapTextBoxClass::Load(Stm, RegisterForChange))
		return false;

	if (!this->Serialize(Stm))
		return false;

	// DIFF: UID resolution happens here, once, instead of lazily inside
	// CanDraw() behind a const_cast on every frame.
	//
	// VERIFY: this assumes TechnoClass::Array is already populated when global
	// Phobos data is loaded. If it is not, Target stays null and IsAlive()
	// drops the label on the first frame - which is a visible behaviour change
	// from the old lazy retry. Confirm the load ordering before shipping.
	this->Target = ResolveTargetByUID(this->SavedTargetUID);

	return true;
}