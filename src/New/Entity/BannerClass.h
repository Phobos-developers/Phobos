#pragma once

#include <GeneralStructures.h>
#include <PCX.h>

#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Helpers/Template.h>
#include <Utilities/Enum.h>

class BannerClass
{
public:
	BannerClass();
	BannerClass(int id, CoordStruct position, BannerType type, char source[32]);
	~BannerClass() = default;
	static DynamicVectorClass<BannerClass*> Instances;

	void LoadContent();
	int GetId() { return this->Id; }
	void Render();

	void InvalidatePointer(void* ptr) { };

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);

	/// Properties ///
	int Id;
	BannerType Type;
	CoordStruct Position;
	char Source[32];

};
