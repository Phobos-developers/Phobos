#pragma once
#include <GeneralStructures.h>
#include <TechnoClass.h>

#include <Utilities/TemplateDef.h>
#include <Utilities/AresFunctions.h>

class RadarJammerClass
{
public:
	int LastScan;							//!< Frame number when the last scan was performed.
	TechnoClass* Techno;			//!< Pointer to game object this jammer is on
	bool Registered;

	void Update();												//!< Updates this Jammer's status on all eligible structures.

	void Jam(BuildingClass* pBuilding);				//!< Attempts to jam the given building. (Actually just registers the Jammer with it, the jamming happens in a hook.)
	void Unjam(BuildingClass* pBuilding);				//!< Attempts to unjam the given building. (Actually just unregisters the Jammer with it, the unjamming happens in a hook.)
};
