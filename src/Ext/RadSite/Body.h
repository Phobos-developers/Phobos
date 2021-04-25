#pragma once

#include <RadSiteClass.h>

#include <Helpers/Macro.h>
#include <Ext/_Container.hpp>
#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>

class RadTypeClass;

class RadSiteExt
{
public:
    using base_type = RadSiteClass;

    class ExtData final : public Extension<RadSiteClass>
    {
    public:
        Valueable<WeaponTypeClass*> Weapon;
        Valueable<RadTypeClass*> Type;
        Valueable<HouseClass*> RadHouse;

        ExtData(RadSiteClass* OwnerObject) : Extension<RadSiteClass>(OwnerObject),
            RadHouse(nullptr),
            Type(),
            Weapon(nullptr)
        {
        }

        virtual ~ExtData() = default;

        virtual size_t Size() const
        {
            return sizeof(*this);
        };

        virtual void InvalidatePointer(void* ptr, bool bRemoved) { }

        virtual void LoadFromStream(PhobosStreamReader& Stm) override;
        virtual void SaveToStream(PhobosStreamWriter& Stm) override;

        virtual void Add(int amount);
        virtual void SetRadLevel(int amount);
        virtual double GetRadLevelAt(CellStruct const& cell);

    private:
        template <typename T>
        void Serialize(T& Stm);
    };

    static DynamicVectorClass<RadSiteExt::ExtData*> Array;

    static void CreateInstance(CellStruct location, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt, HouseClass* const pOwner);
    static void CreateInstance(CellStruct location, int spread, int amount, RadTypeClass* pType, HouseClass* const pOwner);
    
    class ExtContainer final : public Container<RadSiteExt>
    {
    public:
        ExtContainer();
        ~ExtContainer();
    };

    static ExtContainer ExtMap;
};
