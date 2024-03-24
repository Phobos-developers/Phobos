#include "Body.h"

#include <ParticleClass.h>

DEFINE_HOOK(0x62BE30, ParticleClass_Gas_AI_DriftSpeed, 0x5)
{
	enum { ContinueAI = 0x62BE60 };

	GET(ParticleClass*, pParticle, EBP);

	auto pExt = ParticleTypeExt::ExtMap.Find(pParticle->Type);
	int maxDriftSpeed = pExt->Gas_MaxDriftSpeed;
	int minDriftSpeed = -maxDriftSpeed;

	if (pParticle->Velocity.X > maxDriftSpeed)
		pParticle->Velocity.X = maxDriftSpeed;
	else if (pParticle->Velocity.X < minDriftSpeed)
		pParticle->Velocity.X = minDriftSpeed;

	if (pParticle->Velocity.Y > maxDriftSpeed)
		pParticle->Velocity.Y = maxDriftSpeed;
	else if (pParticle->Velocity.Y < minDriftSpeed)
		pParticle->Velocity.Y = minDriftSpeed;

	return ContinueAI;
}
