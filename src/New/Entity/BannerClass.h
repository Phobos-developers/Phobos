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
	int Variable;
	bool IsGlobalVariable;

	BannerClass(BannerTypeClass* pBannerType, int id, CoordStruct position, int variable, bool isGlobalVariable) :
		Type(pBannerType),
		Id(id),
		Position(position),
		Variable(variable),
		IsGlobalVariable(isGlobalVariable)
	{
		BannerClass::Array.AddItem(this);
		PCX::Instance->LoadFile(this->Type->Content_PCX.data());
	}

	BannerClass() :
		Type(),
		Id(),
		Position(),
		Variable(),
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
