#pragma once
#include <RadSiteClass.h>

#include <Helpers/Macro.h>
#include "../_Container.hpp"
#include "../../Utilities/TemplateDef.h"

#include "../WeaponType/Body.h"

class RadType;

class RadSiteExt {
public:
	using base_type = RadSiteClass;

	class ExtData final : public Extension<RadSiteClass> 
	{
	public:
		WeaponTypeClass* Weapon;
		RadType* Type;
		HouseClass* RadHouse;

		ExtData(RadSiteClass* OwnerObject) : Extension<RadSiteClass>(OwnerObject),
			RadHouse(nullptr)
		{ };

		virtual ~ExtData() { }

		virtual size_t Size() const { return sizeof(*this); };

		virtual void InvalidatePointer(void *ptr, bool bRemoved) { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
        
		virtual void Add(int amount);
		virtual void SetRadLevel(int amount);
		virtual double GetRadLevelAt(CellStruct const& cell);
        
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static DynamicVectorClass<RadSiteExt::ExtData*> Instances;

	static void CreateInstance(CellStruct location, int spread, int amount, WeaponTypeExt::ExtData *pWeaponExt, HouseClass* const pOwner);

	class ExtContainer final : public Container<RadSiteExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
