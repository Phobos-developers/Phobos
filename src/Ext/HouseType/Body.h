#pragma once
#include <HouseTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class HouseTypeExt
{
public:
	using base_type = HouseTypeClass;
	static constexpr DWORD Canary = 0x11112222;

	class ExtData final : public Extension<HouseTypeClass>
	{
	public:
		Nullable<int> NewTeamsSelector_MergeUnclassifiedCategoryWith;
		Nullable<double> NewTeamsSelector_UnclassifiedCategoryPercentage;
		Nullable<double> NewTeamsSelector_GroundCategoryPercentage;
		Nullable<double> NewTeamsSelector_NavalCategoryPercentage;
		Nullable<double> NewTeamsSelector_AirCategoryPercentage;

		ExtData(HouseTypeClass* OwnerObject) : Extension<HouseTypeClass>(OwnerObject)
			, NewTeamsSelector_MergeUnclassifiedCategoryWith { }
			, NewTeamsSelector_UnclassifiedCategoryPercentage { }
			, NewTeamsSelector_GroundCategoryPercentage { }
			, NewTeamsSelector_NavalCategoryPercentage { }
			, NewTeamsSelector_AirCategoryPercentage { }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void CompleteInitialization();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool Load(HouseTypeClass* pThis, IStream* pStm) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
