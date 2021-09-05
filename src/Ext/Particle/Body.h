#pragma once
#include <ParticleClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Entity/LaserTrailClass.h>

class ParticleExt
{
public:
	using base_type = ParticleClass;

	class ExtData final : public Extension<ParticleClass>
	{
	public:
		ValueableVector<std::unique_ptr<LaserTrailClass>> LaserTrails;

		ExtData(ParticleClass* OwnerObject) : Extension<ParticleClass>(OwnerObject),
			LaserTrails()
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ParticleExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void InitializeLaserTrails(ParticleClass* pThis);

	static ExtContainer ExtMap;
};
