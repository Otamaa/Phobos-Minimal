#include "Body.h"

#include <FactoryClass.h>

static void CaptureAuthorizedNodes(HouseExtData* pExt, HouseClass* pThis)
{
	if (pExt->AuthorizedNodesCaptured)
		return;

	pExt->AuthorizedNodesCaptured = true;
	pExt->AuthorizedNodeKeys.clear();

	for (int i = 0; i < pThis->Base.BaseNodes.Count; ++i)
	{
		BaseNodeClass& node = pThis->Base.BaseNodes[i];
		if (node.BuildingTypeIndex < 0)
			continue;

		AuthorizedNodeKey key;
		key.BuildingTypeIndex = node.BuildingTypeIndex;
		key.X = node.MapCoords.X;
		key.Y = node.MapCoords.Y;
		pExt->AuthorizedNodeKeys.push_back(key);
	}

}

static bool IsNodeBuiltByAI(short nodeType, short nodeX, short nodeY, HouseClass* pAI)
{
	if (nodeType < 0)
		return false;

	for (BuildingClass* pBld : *BuildingClass::Array)
	{
		if (!pBld || !pBld->Type)
			continue;
		if (pBld->Type->ArrayIndex != nodeType)
			continue;
		if (pBld->GetMapCoords().X != nodeX
			|| pBld->GetMapCoords().Y != nodeY)
			continue;

		if (pBld->Owner == pAI)
			return true;
	}

	return false;
}

static bool IsNodeOccupiedByOther(short nodeType, short nodeX, short nodeY, HouseClass* pAI)
{
	if (nodeType < 0)
		return false;

	for (BuildingClass* pBld : *BuildingClass::Array)
	{
		if (!pBld || !pBld->Type)
			continue;
		if (pBld->Type->ArrayIndex != nodeType)
			continue;
		if (pBld->GetMapCoords().X != nodeX
			|| pBld->GetMapCoords().Y != nodeY)
			continue;

		if (pBld->Owner == pAI)
			return false;

		return true; 
	}

	return false;
}

void HouseExtData::AuthorizeBaseNode(HouseClass* pHouse, int buildingTypeIndex, short x, short y, bool insertAtFront)
{
	auto pExt = HouseExtContainer::Instance.Find(pHouse);
	// 仅当 BaseNodeCrossOwners 启用时，授权列表才生效
	if (!pExt || !pExt->BaseNodeCrossOwners)
		return;

	CaptureAuthorizedNodes(pExt, pHouse);

	for (AuthorizedNodeKey& key : pExt->AuthorizedNodeKeys)
	{
		if (key.X == x && key.Y == y && key.BuildingTypeIndex == buildingTypeIndex)
			return;
	}

	AuthorizedNodeKey key;
	key.BuildingTypeIndex = buildingTypeIndex;
	key.X = x;
	key.Y = y;

	if (insertAtFront)
		pExt->AuthorizedNodeKeys.insert(pExt->AuthorizedNodeKeys.begin(), key);
	else
		pExt->AuthorizedNodeKeys.push_back(key);

}

void HouseExtData::RemoveAuthorizedNodeByCoord(HouseClass* pHouse, short x, short y)
{
	auto pExt = HouseExtContainer::Instance.Find(pHouse);
	if (!pExt) return;

	auto it = pExt->AuthorizedNodeKeys.begin();
	while (it != pExt->AuthorizedNodeKeys.end())
	{
		if (it->X == x && it->Y == y)
			it = pExt->AuthorizedNodeKeys.erase(it);
		else
			++it;
	}

	if (pExt->LastFrameTargetX == x && pExt->LastFrameTargetY == y)
	{
		pExt->LastFrameTargetType = -1;
		pExt->LastFrameTargetX = -1;
		pExt->LastFrameTargetY = -1;
	}

}

void HouseExtData::RemoveAuthorizedNodeByType(HouseClass* pHouse, int buildingTypeIndex)
{
	auto pExt = HouseExtContainer::Instance.Find(pHouse);
	if (!pExt) return;

	auto it = pExt->AuthorizedNodeKeys.begin();
	while (it != pExt->AuthorizedNodeKeys.end())
	{
		if (it->BuildingTypeIndex == buildingTypeIndex)
			it = pExt->AuthorizedNodeKeys.erase(it);
		else
			++it;
	}

	if (pExt->LastFrameTargetType == buildingTypeIndex)
	{
		pExt->LastFrameTargetType = -1;
		pExt->LastFrameTargetX = -1;
		pExt->LastFrameTargetY = -1;
	}
}

// ============================================================
// 用于跨越 sub_506EF0 入口/出口保存/恢复建筑列表
// 不太懂逆向工程, 可能有更好的办法, 我这个比较繁琐
// ============================================================
static DWORD s_SavedBldgBase = 0;          // Hook A 保存的原 [house+6Ch] 建筑列表指针
static int s_SavedBldgCount = 0;           // Hook A 保存的原 [house+78h] 建筑数量
static HouseClass* s_SavedHouse = nullptr; // Hook A 保存的 HouseClass*（Hook B 用，因为出口处 EBP 已变）

static int s_SavedOriginalNodeCount = 0;   // Hook B 保存的原 BaseNodes.Count（全部完成时清零，下帧恢复）

// ============================================================
// Hook A.start: sub_506EF0 入口
// 将 AI 自己的建筑列表替换为全局建筑列表，
// 使该函数能识别跨所属方建筑
// ============================================================
ASMJIT_PATCH(0x506EF0, HouseClass_BaseNode_SatisfactionCheck_Entry, 0x6)
{
	GET(HouseClass*, pThis, ECX);

	auto pExt = HouseExtContainer::Instance.Find(pThis);
	if (!pExt || !pExt->BaseNodeCrossOwners)
		return 0;

	s_SavedHouse = pThis;                      		     // 保存 House 指针，供 Hook B 出口恢复用
	s_SavedBldgBase = *(DWORD*)((DWORD)pThis + 0x6C);    // 保存原 [house+6Ch]（AI 自有建筑列表）
	s_SavedBldgCount = *(int*)((DWORD)pThis + 0x78);     // 保存原 [house+78h]（AI 自有建筑数量）

	// 替换为全阵营建筑列表，让 sub_506EF0 能看到玩家的建筑
	*(DWORD*)((DWORD)pThis + 0x6C) = (DWORD)BuildingClass::Array->Items;   // [house+6Ch] = 全局建筑列表
	*(int*)((DWORD)pThis + 0x78) = BuildingClass::Array->Count;            // [house+78h] = 全局建筑数量

	return 0;
}

// ============================================================
// Hook A.end: sub_506EF0 出口
// 恢复被 Hook A 替换的建筑列表
// ============================================================
ASMJIT_PATCH(0x50766E, HouseClass_BaseNode_SatisfactionCheck_Exit, 0xD)
{
	// 注意: 到出口时 EBP 已被覆盖为 off_7E38D0，
	// 所以不能用 GET(..., EBP) 获取 HouseClass*
	// 改用 Hook A 保存的 s_SavedHouse
	if (!s_SavedHouse)
		return 0;

	// 检查 [house+6Ch] 是否仍是全局列表（防止中间被其他代码改过）
	if (*(DWORD*)((DWORD)s_SavedHouse + 0x6C) == (DWORD)BuildingClass::Array->Items)
	{
		// 恢复 AI 自己的建筑列表
		*(DWORD*)((DWORD)s_SavedHouse + 0x6C) = s_SavedBldgBase;   // [house+6Ch] = 原 AI 建筑列表
		*(int*)((DWORD)s_SavedHouse + 0x78) = s_SavedBldgCount;    // [house+78h] = 原 AI 建筑数量
	}

	return 0;
}

// ============================================================
// Hook B: AI_BaseConstructionUpdate (0x4FE3E0) 入口
//
// 原版每帧遍历 HouseClass::Base.BaseNodes，调用 sub_506EF0 评估各节点
// 根据基地节点指南, auto-gen (sub_42EB20) 会动态补充新节点（如围墙、防御等）
//
// 我们每帧只保留一个授权节点，其余清空，让 AI 按顺序逐个建造。
// ============================================================
ASMJIT_PATCH(0x4FE3E0, HouseClass_AI_BaseConstructionUpdate_Entry, 0x5)
{
	GET(HouseClass*, pThis, ECX);

	auto pExt = HouseExtContainer::Instance.Find(pThis);
	if (!pExt || !pExt->BaseNodeCrossOwners)
		return 0;

	// 恢复 Count（上一帧全部完成时被设成0了）
	if (pThis->Base.BaseNodes.Count == 0 && s_SavedOriginalNodeCount > 0)
	{
		pThis->Base.BaseNodes.Count = s_SavedOriginalNodeCount;
		s_SavedOriginalNodeCount = 0;
	}

	// 捕获, 仅首次调用时执行
	CaptureAuthorizedNodes(pExt, pThis);

	int totalNodes = pExt->AuthorizedNodeKeys.size();
	int builtCount = 0, occupiedCount = 0, emptyCount = 0;

	// 找到第一个需要建造的授权节点（按顺序）
	int targetType = -1;
	short targetX = -1;
	short targetY = -1;

	// ========= 1, 每帧检查授权节点状态: 已建/被占/空置 ========
	for (AuthorizedNodeKey& key : pExt->AuthorizedNodeKeys)
	{
		if (IsNodeBuiltByAI(key.BuildingTypeIndex, key.X, key.Y, pThis))
		{
			++builtCount;
			continue;
		}

		if (IsNodeOccupiedByOther(key.BuildingTypeIndex, key.X, key.Y, pThis))
		{
			++occupiedCount;
			continue;
		}

		// 这个需要建造, 到这里的节点都是未完成的了
		// 获取第一个需要建造的节点坐标和类型
		++emptyCount;
		if (targetType < 0)
		{
			targetType = key.BuildingTypeIndex;
			targetX = key.X;
			targetY = key.Y;
		}
		break;
	}


	// ========= 2, 根据状态设置 BaseNodes ========
	// 所有节点已完成（AI已建或玩家占领）→ Count=0 让 sub_506EF0 跳过处理
	if (emptyCount == 0 && builtCount + occupiedCount == totalNodes)
	{
		pExt->LastFrameTargetType = -1;
		pExt->LastFrameTargetX = -1;
		pExt->LastFrameTargetY = -1;
		s_SavedOriginalNodeCount = pThis->Base.BaseNodes.Count;
		pThis->Base.BaseNodes.Count = 0;
		return 0;
	}

	// 把所有节点设为 -1，一个不留
	// 确保单节点模式
	for (BaseNodeClass& node : pThis->Base.BaseNodes)
	{
		node.BuildingTypeIndex = -1;
	}

	// 设 BaseNodes[0] = 需要建造的目标
	if (targetType >= 0)
	{
		BaseNodeClass& node = pThis->Base.BaseNodes[0];
		node.BuildingTypeIndex = targetType;
		node.MapCoords.X = targetX;
		node.MapCoords.Y = targetY;

		pExt->LastFrameTargetType = targetType;
		pExt->LastFrameTargetX = targetX;
		pExt->LastFrameTargetY = targetY;

		// ===== 每帧检查工厂: 是否在生产正确的东西? =====
		for (FactoryClass* pFact : *FactoryClass::Array)
		{
			if (pFact->Owner != pThis)
				continue;

			// 获取工厂的生产对象, 不是建筑就继续遍历
			TechnoClass* pObj = pFact->Object;
			if (!pObj || pObj->What_Am_I() != AbstractType::Building)
				continue;

			// 如果工厂暂停, 暂时不检查
			if (pFact->IsSuspended)
				break;

			// Production TechnoType
			TechnoTypeClass* pProdType = pObj->GetTechnoType();
			if (!pProdType) break;

			int prodTypeIdx = static_cast<BuildingTypeClass*>(pProdType)->ArrayIndex;
			// 如果当前生成的类型和目标一样, 不处理, 让他继续生产
			if (prodTypeIdx == targetType)
				break;

			pFact->AbandonProduction();
			BuildingTypeClass* pTargetType = BuildingTypeClass::Array->Items[targetType];
			if (pTargetType)
				pFact->DemandProduction(pTargetType, pThis, false);
			break;
		}
	}

	return 0;
}

// ============================================================
// Hook C: 拦截 FactoryClass::DemandProduction
// 阻止未授权的建筑类型进入工厂生产队列
// ============================================================
ASMJIT_PATCH(0x4C9C70, FactoryClass_DemandProduction_Intercept, 0x9)
{
	GET_STACK(TechnoTypeClass const*, pType, 0x4);
	GET_STACK(HouseClass*, pOwner, 0x8);

	if (!pOwner || !pType)
		return 0;

	auto pExt = HouseExtContainer::Instance.Find(pOwner);
	if (!pExt || !pExt->BaseNodeCrossOwners)
		return 0;

	// 只拦截建筑类型
	if (pType->WhatAmI() != AbstractType::BuildingType)
		return 0;

	int reqTypeIdx = static_cast<BuildingTypeClass const*>(pType)->ArrayIndex;
	const char* reqId = nullptr;
	BuildingTypeClass* pReqType = BuildingTypeClass::Array->Items[reqTypeIdx];
	if (pReqType)
		reqId = pReqType->get_ID();

	// 检查请求的类型是否在授权列表中, 在就不用管, 让它造
	for (AuthorizedNodeKey& key : pExt->AuthorizedNodeKeys)
	{
		if (key.BuildingTypeIndex == reqTypeIdx)
			return 0;
	}

	// 未授权 -> 寻找需要建造的空位，如果全部建成则重定向到第一个已建类型
	int fallbackType = -1;
	int nBuilt = 0, nOccupied = 0;
	for (AuthorizedNodeKey& key : pExt->AuthorizedNodeKeys)
	{
		if (IsNodeBuiltByAI((short)key.BuildingTypeIndex, key.X, key.Y, pOwner))
		{
			// 获取第一个已建的类型作为 fallback
			if (fallbackType < 0)
				fallbackType = key.BuildingTypeIndex;
			++nBuilt;
			continue;
		}
		if (IsNodeOccupiedByOther((short)key.BuildingTypeIndex, key.X, key.Y, pOwner))
		{
			++nOccupied;
			continue;
		}

		// 经过多次continue过滤, 找到空位 -> 正常重定向
		BuildingTypeClass* pTargetType = BuildingTypeClass::Array->Items[key.BuildingTypeIndex];
		if (pTargetType)
		{
			// 偷改参数, 让工厂去造AuthorizedNodeKeys里面的的东西
			R->Stack<TechnoTypeClass*>(0x4, pTargetType);
		}
		return 0;
	}

	// 经过过滤仍然没有空位了 -> 全部建成或被占领了
	// 全部建成/被占领 → 使用 fallback

	if (fallbackType >= 0)
	{
		BuildingTypeClass* pTargetType = BuildingTypeClass::Array->Items[fallbackType];
		if (pTargetType)
			R->Stack<TechnoTypeClass*>(0x4, pTargetType);
	}

	return 0;
}
