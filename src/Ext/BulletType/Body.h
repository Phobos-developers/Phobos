	#pragma once
#include <BulletTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class BulletTypeExt
{
public:
	using base_type = BulletTypeClass;

	class ExtData final : public Extension<BulletTypeClass>
	{
	public:
		Valueable<bool> Interceptable;
		Nullable<Leptons> BallisticScatter_Min;
		Nullable<Leptons> BallisticScatter_Max;

		ExtData(BulletTypeClass* OwnerObject) : Extension<BulletTypeClass>(OwnerObject),
			Interceptable(false)
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass * pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(PhobosStreamReader & Stm) override;

		virtual void SaveToStream(PhobosStreamWriter & Stm) override;
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BulletTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
