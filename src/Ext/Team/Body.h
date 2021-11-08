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
		int WaitNoTargetAttempts;
		double NextSuccessWeightAward;
		int IdxSelectedObjectFromAIList;
		double CloseEnough;
		int Countdown_RegroupAtLeader;
		int AngerNodeModifier;
		bool OnlyTargetHouseEnemy;
		int OnlyTargetHouseEnemyMode;

		ExtData(TeamClass* OwnerObject) : Extension<TeamClass>(OwnerObject)
			, WaitNoTargetAttempts(0)
			, NextSuccessWeightAward(0)
			, IdxSelectedObjectFromAIList(-1)
			, CloseEnough(-1)
			, Countdown_RegroupAtLeader(-1)
			, AngerNodeModifier(5000)
			, OnlyTargetHouseEnemy(false)
			, OnlyTargetHouseEnemyMode(-1)
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
