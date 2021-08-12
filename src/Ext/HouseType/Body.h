#pragma once

#include <CCINIClass.h>
#include <HouseTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SuperWeaponTypeClass;
class HouseTypeExt
{
public:
	using base_type = HouseTypeClass;

	class ExtData final : public Extension<HouseTypeClass>
	{
	public:
		struct ScoreSuperWeaponDataEntry
		{
			ValueableIdx<SuperWeaponTypeClass> IdxType{ };
			Valueable<int> Score{ 0 };
			Valueable<bool> OneTime{ false };
			Valueable<bool> InitialReady{ false };
			bool AlreadyGranted{ false };

			bool Load(PhobosStreamReader& stm, bool registerForChange);
			bool Save(PhobosStreamWriter& stm) const;

		private:
			template <typename T>
			bool Serialize(T& stm);
		};

		ValueableVector<ScoreSuperWeaponDataEntry> ScoreSuperWeaponData;

		ExtData(HouseTypeClass* OwnerObject) : Extension<HouseTypeClass>(OwnerObject),
			ScoreSuperWeaponData()
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {	}

		virtual void LoadFromStream(PhobosStreamReader &Stm) override;
		virtual void SaveToStream(PhobosStreamWriter &Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static ExtContainer ExtMap;
};