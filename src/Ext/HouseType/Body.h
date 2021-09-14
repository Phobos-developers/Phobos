#pragma once
#include <HouseTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class HouseTypeExt
{
public:
	using base_type = HouseTypeClass;
	class ExtData final : public Extension<HouseTypeClass>
	{
	public:
		Valueable<bool> CountryCrew;
		Nullable<InfantryTypeClass*> CountryCrew_Type;

		ExtData(HouseTypeClass* OwnerObject) : Extension<HouseTypeClass>(OwnerObject)
			, CountryCrew(false)
			, CountryCrew_Type()
		{}

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;

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
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};