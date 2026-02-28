#include "PaintballSyncManager.h"
#include <Misc/Kratos/Ext/StateType/State/PaintballState.h>

/**
 * @brief 注册Paintball到同步源
 * @param sourceId 同步源ID
 * @param paintball 要注册的Paintball
 */
void PaintballSyncManager::Register(const std::string& sourceId, PaintballState* paintball)
{
	if (sourceId.empty() || !paintball) return;

	// 如果已经注册到其他源，先移除
	auto it = _paintballToSource.find(paintball);
	if (it != _paintballToSource.end())
	{
		if (it->second == sourceId) return; // 已经在同一个源

		// 从旧源中移除
		_sourceToPaintball[it->second].erase(paintball);
	}

	// 注册到新源
	_sourceToPaintball[sourceId].insert(paintball);
	_paintballToSource[paintball] = sourceId;
}

/**
 * @brief 解除Paintball的注册
 * @param paintball 要解除的Paintball
 */
void PaintballSyncManager::Unregister(PaintballState* paintball)
{
	if (!paintball) return;

	auto it = _paintballToSource.find(paintball);
	if (it == _paintballToSource.end()) return;

	// 从源中移除
	_sourceToPaintball[it->second].erase(paintball);

	// 从反向映射中移除
	_paintballToSource.erase(it);
}

/**
 * @brief 从源同步数据给所有注册的Paintball
 * @param sourceId 同步源ID
 * @param paintball 要同步的数据
 */
void PaintballSyncManager::Sync(const std::string& sourceId, const PaintballState* paintball)
{
	auto it = _sourceToPaintball.find(sourceId);
	if (it == _sourceToPaintball.end()) return;

	for (auto* state : it->second)
	{
		if (state)
		{
			state->Data = paintball->Data; // 颜色数据
			// 激活状态
			if (paintball->IsActive())
			{
				state->Activate();
			}
			else
			{
				state->Deactivate();
			}
		}
	}
}

/**
 * @brief 获取Paintball注册的源ID
 * @param paintball Paintball
 * @return 源ID，如果未注册返回空字符串
 */
std::string PaintballSyncManager::GetSource(PaintballState* paintball)
{
	auto it = _paintballToSource.find(paintball);
	if (it != _paintballToSource.end())
	{
		return it->second;
	}
	return "";
}

/**
 * @brief 检查Paintball是否已注册
 * @param paintball Paintball
 * @return 是否已注册
 */
bool PaintballSyncManager::IsRegistered(PaintballState* paintball)
{
	return _paintballToSource.find(paintball) != _paintballToSource.end();
}

/**
 * @brief 清空所有注册
 */
void PaintballSyncManager::ClearAll()
{
	_sourceToPaintball.clear();
	_paintballToSource.clear();
}

void PaintballSyncManager::Clear(EventSystem* sender, Event e, void* args)
{
	ClearAll();
}

// 源ID -> 注册的Paintball集合
std::unordered_map<std::string, std::unordered_set<PaintballState*>> PaintballSyncManager::_sourceToPaintball{};

// Paintball -> 源ID（用于快速查找）
std::unordered_map<PaintballState*, std::string> PaintballSyncManager::_paintballToSource{};
