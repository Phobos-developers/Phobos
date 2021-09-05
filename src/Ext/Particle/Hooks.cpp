#include "Body.h"

// Init in constructor because yes - Morton
DEFINE_HOOK(0x62BB13, ParticleClass_CTOR_SetLaserTrail, 0x5)
{
	GET(ParticleClass*, pThis, ESI);
	ParticleExt::InitializeLaserTrails(pThis);
	return 0;
}

DEFINE_HOOK(0x62CE86, ParticleClass_AI, 0x7) //this is the end, here's the beginning: 0x62CE49 0x6
{
	GET(ParticleClass*, pThis, ESI);
	auto pParticleExt = ParticleExt::ExtMap.Find(pThis);

	if (!pParticleExt)
		return 0;

	// TODO: Check this - Morton
	// LaserTrails update routine is in BulletClass::AI hook because BulletClass::Draw
	// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
	if (pParticleExt->LaserTrails.size())
	{
		CoordStruct location = pThis->GetCoords();
		//Vector3D<float> velocity = pThis->unknown_vector3d_10C * pThis->Velocity; // I think that's how 3d velocity is stored here?
		CoordStruct drawnCoords = location;

		// TODO: Think about this part, because particle behaviours controls movement differently
		// Ergo, we can't universally "forsee" where this is going - Morton
		/*
		// We adjust LaserTrails to account for vanilla bug of drawing stuff one frame ahead.
		// Pretty meh solution but works until we fix the bug - Kerbiter
		CoordStruct drawnCoords
		{
			(int)(location.X + velocity.X),
			(int)(location.Y + velocity.Y),
			(int)(location.Z + velocity.Z)
		};
		*/

		for (auto const& trail : pParticleExt->LaserTrails)
		{
			// Left this here for now - Morton
			// We insert initial position so the first frame of trail doesn't get skipped - Kerbiter
			// TODO move hack to BulletClass creation
			if (!trail->LastLocation.isset())
				trail->LastLocation = location;

			trail->Update(drawnCoords);
		}

	}

	return 0;
}
