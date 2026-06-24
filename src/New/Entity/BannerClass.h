#pragma once

#include <Utilities/Savegame.h>
#include <Utilities/VectorHelper.h>
#include <Utilities/ClassInterfaces.h>

#include <Surface.h>

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

	static COMPILETIMEEVAL FORCEDINLINE void Clamp(Point2D& point, int W, int H) {
		point.X = std::clamp(point.X, 0, std::max(0, DSurface::ViewBounds->Width - W));
		point.Y = std::clamp(point.Y, 0, std::max(0, DSurface::ViewBounds->Height - H));
	}

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

	virtual bool SaveGlobal(PhobosStreamWriter& root) {
		return root.Process(Array);
	}

	virtual bool LoadGlobal(PhobosStreamReader& root) {
		return root.Process(Array);
	}	
	virtual void Clear();

public:

	static BannerManagerClass Instance;
};

