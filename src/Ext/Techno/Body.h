#pragma once
#include <TechnoClass.h>

#include <Helpers/Macro.h>
#include "../_Container.hpp"
#include "../../Utilities/TemplateDef.h"

#include "../../Misc/Shield.h"

class BulletClass;

class TechnoExt
{
public:
	using base_type = TechnoClass;

	class ExtData final : public Extension<TechnoClass>
	{
	public:
		Valueable<BulletClass*> InterceptedBullet;
		std::unique_ptr<ShieldTechnoClass> ShieldData;

		ExtData(TechnoClass* OwnerObject) : Extension<TechnoClass>(OwnerObject),
			InterceptedBullet(nullptr),
			ShieldData()
		{ }

		virtual ~ExtData() = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TechnoExt> {
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static bool IsHarvesting(TechnoClass* pThis);

	static bool HasAvailableDock(TechnoClass* pThis);

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
