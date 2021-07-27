#pragma once

#include <AnimTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Enum.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
class AnimTypeExt
{
public:
	using base_type = AnimTypeClass;

	class ExtData final : public Extension<AnimTypeClass>
	{
	public:
		CustomPalette Palette;
		Valueable<UnitTypeClass*> CreateUnit;
		Valueable<int> CreateUnit_Facing;
		Valueable<bool> CreateUnit_UseDeathFacings;
		Valueable<bool> CreateUnit_useDeathTurrentFacings;
		Valueable<bool> CreateUnit_RemapAnim;
		Valueable<Mission> CreateUnit_Mission;
		Valueable<OwnerHouseKind> CreateUnit_Owner;

		ExtData(AnimTypeClass* OwnerObject) : Extension<AnimTypeClass>(OwnerObject)
			, Palette(CustomPalette::PaletteMode::Temperate)
			, CreateUnit_Facing(-1)
			, CreateUnit_UseDeathFacings(false)
			, CreateUnit_RemapAnim(false)
			, CreateUnit_Mission(Mission::Guard)
			, CreateUnit_Owner(OwnerHouseKind::Victim)
		{ }

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<AnimTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static const void ProcessDestroyAnims(UnitClass* pThis, TechnoClass* pKiller = nullptr, HouseClass* pHouseKiller = nullptr);
};