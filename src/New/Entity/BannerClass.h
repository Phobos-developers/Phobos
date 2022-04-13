#pragma once

#include <GeneralStructures.h>
#include <PCX.h>
#include <TacticalClass.h>

#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Helpers/Template.h>
#include <Utilities/Enum.h>

#include <New/Type/BannerTypeClass.h>

class BannerClass
{
public:
	static DynamicVectorClass<BannerClass*> Array;

	BannerTypeClass* Type;
	int Id;
	CoordStruct Position;
	int Variables[4];
	bool IsGlobalVariable;

	BannerClass(BannerTypeClass* pBannerType, int id, CoordStruct position, int variable[4], bool isGlobalVariable) :
		Type(pBannerType),
		Id(id),
		Position(position),
		Variables(),
		IsGlobalVariable(isGlobalVariable)
	{
		BannerClass::Array.AddItem(this);
		this->Type->LoadImage();
		for (int i = 0; i < 4; i++)
			this->Variables[i] = variable[i];
	}

	BannerClass() :
		Type(),
		Id(),
		Position(),
		Variables(),
		IsGlobalVariable()
	{
		BannerClass::Array.AddItem(this);
	}

	void Render();

	void InvalidatePointer(void* ptr) { };

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);
	void RenderPCX(int x, int y);
	void RenderSHP(int x, int y);
	void RenderCSF(int x, int y);
};
