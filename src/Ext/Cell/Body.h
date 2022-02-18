#pragma once
#include <CellClass.h>

#include <Helpers/Macro.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Entity/FoggedObject.h>

class FoggedObject;

class CellExt
{
public:
	using base_type = CellClass;

	class ExtData final : public Extension<CellClass>
	{
	public:
		DynamicVectorClass<FoggedObject*> FoggedObjects;

		ExtData(CellClass* OwnerObject) : Extension<CellClass>(OwnerObject),
			FoggedObjects {}
		{ }

		virtual ~ExtData() = default;

		// virtual void LoadFromINIFile(CCINIClass * pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

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
};