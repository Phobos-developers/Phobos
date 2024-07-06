#pragma once
#include <VoxelAnimTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>
#include <Helpers/Macro.h>

#include <New/Type/LaserTrailTypeClass.h>

class VoxelAnimTypeExt
{
public:
	using base_type = VoxelAnimTypeClass;

	static constexpr DWORD Canary = 0xAAAEEEEE;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<VoxelAnimTypeClass>
	{
	public:

		ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
		Valueable<bool> ExplodeOnWater;
		Valueable<bool> Warhead_Detonate;
		Valueable<AnimTypeClass*> WakeAnim;
		NullableVector<AnimTypeClass*> SplashAnims;
		Valueable<bool> SplashAnims_PickRandom;

		ExtData(VoxelAnimTypeClass* OwnerObject) : Extension<VoxelAnimTypeClass>(OwnerObject)
			, LaserTrail_Types()
			, ExplodeOnWater { false }
			, Warhead_Detonate { false }
			, WakeAnim {}
			, SplashAnims {}
			, SplashAnims_PickRandom { false }
		{ }

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void Initialize() override;
		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<VoxelAnimTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
