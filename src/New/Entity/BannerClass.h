#pragma once

#include <vector>

#include <GeneralStructures.h>

#include <Utilities/Savegame.h>

class BannerTypeClass;

class BannerClass
{
public:
	static std::vector<std::unique_ptr<BannerClass>> Array;

	BannerTypeClass* Type {};
	int ID {};
	Point2D Position {};
	int Variable {};
	int ShapeFrameIndex {};
	bool IsGlobalVariable {};

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

	static void Clear();
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

private:
	template <typename T>
	bool Serialize(T& Stm);

	void RenderPCX(Point2D position);
	void RenderSHP(Point2D position);
	void RenderCSF(Point2D position);
};
