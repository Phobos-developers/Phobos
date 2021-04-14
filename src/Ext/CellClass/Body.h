#pragma once
#include <CellClass.h>

#include <Helpers/Macro.h>
#include "../_Container.hpp"
#include "../../Utilities/TemplateDef.h"

class CellExt {
public:
	using base_type = CellClass;

	class ExtData final : public Extension<CellClass>
	{
	public:
		Valueable<int> NewPowerups;
		ExtData(CellClass* OwnerObject) : Extension<CellClass>(OwnerObject),
			NewPowerups(-1)
		{ };

		virtual ~ExtData() { }

		virtual size_t Size() const { return sizeof(*this); };

		virtual void InvalidatePointer(void *ptr, bool bRemoved) { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;


	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<CellExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};
