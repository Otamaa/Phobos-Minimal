#pragma once

#include <Utilities/Savegame.h>
#include <Utilities/VectorHelper.h>

#include <Point2D.h>

#include <Utilities/Interfaces.h>

class BannerTypeClass;

class BannerClass
{
public:

	BannerTypeClass* Type {};
	int ID {};
	Point2D Position {};
	int Variable {};
	int ShapeFrameIndex {};
	bool IsGlobalVariable {};
	int Duration { -1 };
	int Delay { -1 };


	BannerClass() = default;

	BannerClass
	(
		BannerTypeClass* pBannerType,
		int id,
		Point2D position,
		int variable,
		bool isGlobalVariable
	);

	void Render();

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);

	void RenderPCX(Point2D position);
	void RenderSHP(Point2D position);
	void RenderCSF(Point2D position);
};

struct BannerManagerClass : public GlobalSaveable
{
	HelperedVector<BannerClass> Array;

public:
	BannerManagerClass() = default;
	virtual ~BannerManagerClass() = default;

	virtual bool SaveGlobal(json& root);
	virtual bool LoadGlobal(const json& root);
	virtual void Clear();

public:

	static BannerManagerClass Instance;
};

