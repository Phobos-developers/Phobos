#pragma once

#include <vector>

#include <GeneralStructures.h>

#include <Utilities/Savegame.h>

class BannerTypeClass;

class BannerClass
{
public:
	static std::vector<std::unique_ptr<BannerClass>> Array;

	BannerTypeClass* Type;
	int ID;
	int PositionX;
	int PositionY;

	//I don't know what is this for
	int Variable;

	int ShapeFrameIndex;
	bool IsGlobalVariable;

	BannerClass() = default;
	BannerClass
	(
		BannerTypeClass* pBannerType,
		int id,
		int positionX,
		int positionY,
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

	void RenderPCX(int x, int y);
	void RenderSHP(int x, int y);
	void RenderCSF(int x, int y);
};
