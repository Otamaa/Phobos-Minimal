#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>

#include <Misc/Kratos/Common/EventSystems/EventSystem.h>

class PaintballState;

class PaintballSyncManager
{
public:
	/**
	 * @brief 注册Paintball到同步源
	 * @param sourceId 同步源ID
	 * @param paintball 要注册的Paintball
	 */
	static void Register(const std::string& sourceId, PaintballState* paintball);

	/**
	 * @brief 解除Paintball的注册
	 * @param paintball 要解除的Paintball
	 */
	static void Unregister(PaintballState* paintball);

	/**
	 * @brief 从源同步数据给所有注册的Paintball
	 * @param sourceId 同步源ID
	 * @param paintball 要同步的数据
	 */
	static void Sync(const std::string& sourceId, const PaintballState* paintball);

	/**
	 * @brief 获取Paintball注册的源ID
	 * @param paintball Paintball
	 * @return 源ID，如果未注册返回空字符串
	 */
	static std::string GetSource(PaintballState* paintball);

	/**
	 * @brief 检查Paintball是否已注册
	 * @param paintball Paintball
	 * @return 是否已注册
	 */
	static bool IsRegistered(PaintballState* paintball);

	/**
	 * @brief 清空所有注册
	 */
	static void ClearAll();

	static void Clear(EventSystem* sender, Event e, void* args);
private:

	// 源ID -> 注册的Paintball集合
	static std::unordered_map<std::string, std::unordered_set<PaintballState*>> _sourceToPaintball;

	// Paintball -> 源ID（用于快速查找）
	static std::unordered_map<PaintballState*, std::string> _paintballToSource;
};
