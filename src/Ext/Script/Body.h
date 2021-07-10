#pragma once

#include <ScriptClass.h>
#include <ScriptTypeClass.h>
#include <TeamClass.h>
#include <HouseClass.h>
#include <AircraftClass.h>
#include <MapClass.h>
#include <BulletClass.h>
#include <Helpers/Enumerators.h>
#include <WarheadTypeClass.h>
#include <SpawnManagerClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"

class ScriptExt
{
public:
	using base_type = ScriptClass;

	class ExtData final : public Extension<ScriptClass>
	{
	public:
		// Nothing yet

		ExtData(ScriptClass* OwnerObject) : Extension<ScriptClass>(OwnerObject)
			// Nothing yet
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(PhobosStreamReader& Stm);
		virtual void SaveToStream(PhobosStreamWriter& Stm);

	};

	class ExtContainer final : public Container<ScriptExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void ProcessAction(TeamClass * pTeam);
	static void ExecuteTimedAreaGuardAction(TeamClass * pTeam);
	static void LoadIntoTransports(TeamClass * pTeam);
	static void WaitUntillFullAmmoAction(TeamClass * pTeam);
    static void Mission_Attack(TeamClass *pTeam, bool repeatAction, int calcThreatMode);
    static TechnoClass* GreatestThreat(TechnoClass *pTechno, int method, int calcThreatMode, HouseClass* onlyTargetThisHouseEnemy);
    static bool EvaluateObjectWithMask(TechnoClass *pTechno, int mask);
    
	static ExtContainer ExtMap;
};
