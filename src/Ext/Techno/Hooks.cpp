#include "Body.h"

#include "../TechnoType/Body.h"

DEFINE_HOOK(6F9E50, TechnoClass_AI, 5)
{
	GET(TechnoClass*, pThis, ECX);

	// MindControlRangeLimit
	TechnoExt::ApplyMindControlRangeLimit(pThis);
	// Interceptor
	TechnoExt::ApplyInterceptor(pThis);
	// Powered.KillSpawns
	TechnoExt::ApplyPowered_KillSpawns(pThis);
	// Spawner.LimitRange & Spawner.ExtraLimitRange
	TechnoExt::ApplySpawn_LimitRange(pThis);

	return 0;
}