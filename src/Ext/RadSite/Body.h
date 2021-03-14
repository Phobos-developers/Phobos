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

		ExtData(RadSiteClass* OwnerObject) : Extension<RadSiteClass>(OwnerObject)
		{ };

		virtual ~ExtData() { }

		virtual size_t Size() const { return sizeof(*this); };

		virtual void InvalidatePointer(void *ptr, bool bRemoved) { }
		virtual void LoadFromStream(IStream* Stm);
		virtual void SaveToStream(IStream* Stm);

	};
	static DynamicVectorClass<RadSiteExt::ExtData*> RadSiteInstance;

	static void CreateInstance(CellStruct location, int spread, int amount, WeaponTypeExt::ExtData *pWeaponExt);
	static void RadSiteAdd(RadSiteClass* pRad, int lvmax, int amount);
	static void SetRadLevel(RadSiteClass* pRad, RadType* Type, int amount);
	static double GetRadLevelAt(RadSiteClass* pThis, CellStruct const& cell);

	class ExtContainer final : public Container<RadSiteExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};
	static ExtContainer ExtMap;
};
