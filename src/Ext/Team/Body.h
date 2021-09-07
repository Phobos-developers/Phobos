#pragma once
#include <TeamClass.h>

#include <Helpers/Enumerators.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Phobos.h>

class TeamExt
{
public:
	using base_type = TeamClass;

	class ExtData final : public Extension<TeamClass>
	{
	public:
		double CloseEnough;
		int Countdown_regroupAtLeader;

		ExtData(TeamClass* OwnerObject) : Extension<TeamClass>(OwnerObject),
			CloseEnough(-1),
			Countdown_regroupAtLeader(-1)
		{ }

		virtual ~ExtData() = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TeamExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

};
