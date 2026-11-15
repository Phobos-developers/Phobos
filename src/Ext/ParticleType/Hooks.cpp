#include "Body.h"

DEFINE_HOOK(0x62BE30, ParticleClass_Gas_AI_DriftSpeed, 0x0)
{
	enum { ContinueAI = 0x62BE60 };

	GET(ParticleClass*, pParticle, EBP);

	const auto pExt = ParticleTypeExt::Fetch(pParticle->Type);
	const int maxDriftSpeed = pExt->Gas_MaxDriftSpeed;
	const int minDriftSpeed = -maxDriftSpeed;
	auto& gasDrift = pParticle->GasDrift;

	if (gasDrift.X > maxDriftSpeed)
		gasDrift.X = maxDriftSpeed;
	else if (gasDrift.X < minDriftSpeed)
		gasDrift.X = minDriftSpeed;

	if (gasDrift.Y > maxDriftSpeed)
		gasDrift.Y = maxDriftSpeed;
	else if (gasDrift.Y < minDriftSpeed)
		gasDrift.Y = minDriftSpeed;

	return ContinueAI;
}
