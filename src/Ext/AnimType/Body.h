#pragma once

#include <AnimTypeClass.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class AnimTypeExt
{
public:
	using base_type = AnimTypeClass;

	struct SavedMotionData
	{
		bool FromDeathUnit{ false };
		bool SourceHasTurret{ false };
		short UnitDeathFacing{ -1 };
		DirStruct UnitDeathTurretFacing{ };
	};

	class ExtData final : public Extension<AnimTypeClass>
	{
	public:

		Valueable<UnitTypeClass*> CreateUnit;
		Valueable<int> CreateUnit_Facing;
		Valueable<bool> CreateUnit_UseDeathFacings;
		Valueable<CoordStruct> CreateUnit_Offset;
		Valueable<bool> CreateUnit_RemapAnim;
		Valueable<Mission> CreateUnit_Mission;
		Valueable<bool> CreateUnit_Force;

		// Not accessible directly
		SavedMotionData SavedData;

		ExtData(AnimTypeClass* OwnerObject) : Extension<AnimTypeClass>(OwnerObject)
			, CreateUnit(nullptr)
			, CreateUnit_Facing(-1)
			, CreateUnit_UseDeathFacings(false)
			, CreateUnit_Offset({ 0,0,0 })
			, CreateUnit_RemapAnim(false)
			, CreateUnit_Mission(Mission::Guard)
			, CreateUnit_Force(true)
			, SavedData()
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
};
