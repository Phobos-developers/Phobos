#pragma once
#include <AnimClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class AnimExt
{
public:
    using base_type = AnimClass;

    class ExtData final : public Extension<AnimClass>
    {
    public:

		short DeathUnitFacing;
		DirStruct DeathUnitTurretFacing;
		bool Fromdeathunit;
		bool DeathUnitHasTurrent;

        ExtData(AnimClass* OwnerObject) : Extension<AnimClass>(OwnerObject)
			, DeathUnitFacing(0)
			, DeathUnitTurretFacing(0)
			, Fromdeathunit(false)
			, DeathUnitHasTurrent(false)
        { }

        virtual ~ExtData() = default;
        virtual void InvalidatePointer(void *ptr, bool bRemoved) override {}
        virtual void LoadFromStream(PhobosStreamReader& Stm)override;
        virtual void SaveToStream(PhobosStreamWriter& Stm)override;

    private:
        template <typename T>
        void Serialize(T& Stm);
    };

    class ExtContainer final : public Container<AnimExt> 
    {
    public:
        ExtContainer();
        ~ExtContainer();
    };

    static ExtContainer ExtMap;
	static const OwnerHouseKind SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker , HouseClass* pVictim, HouseClass* pKiller , bool defaultToVictimOwner = true);

};
