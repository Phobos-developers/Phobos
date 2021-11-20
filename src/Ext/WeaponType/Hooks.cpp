#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Scenario/Body.h>

DEFINE_HOOK(0x6FF660, TechnoClass_FireBullet, 0x6)
{
	GET(TechnoClass* const, pSource, ESI);
	GET_BASE(AbstractClass* const, pTarget, 0x8);
	//GET(WeaponTypeClass* const, pWeaponType, EBX);

	// Interceptor
	auto const pSourceTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());
	bool interceptor = pSourceTypeExt->Interceptor;

	if (pSourceTypeExt && interceptor)
	{
		bool interceptor_Veteran = pSourceTypeExt->Interceptor_Veteran.Get(true);
		bool interceptor_Elite = pSourceTypeExt->Interceptor_Elite.Get(true);

		if (pSource->Veterancy.IsVeteran() && !interceptor_Veteran)
			interceptor = false;

		if (pSource->Veterancy.IsElite() && !interceptor_Elite)
			interceptor = false;

		if (!interceptor)
			return 0;

		if (auto const pTargetObject = specific_cast<BulletClass* const>(pTarget))
		{
			if (auto const pSourceExt = TechnoExt::ExtMap.Find(pSource))
			{
				if (pSourceExt->InterceptedBullet && pSourceExt->InterceptedBullet->IsAlive)
				{
					if (auto const pBulletExt = BulletExt::ExtMap.Find(pSourceExt->InterceptedBullet))
					{
						int probability = ScenarioClass::Instance->Random.RandomRanged(1, 100);
						int successProbability = pSourceTypeExt->Interceptor_Success;

						if (!pSource->Veterancy.IsRookie())
						{
							if (pSource->Veterancy.IsVeteran())
							{
								if (pSourceTypeExt->Interceptor_VeteranSuccess >= 0)
								{
									successProbability = pSourceTypeExt->Interceptor_VeteranSuccess;
								}
							}
							else
							{
								if (pSource->Veterancy.IsElite())
								{
									if (pSourceTypeExt->Interceptor_EliteSuccess >= 0)
									{
										successProbability = pSourceTypeExt->Interceptor_EliteSuccess;
									}
									else
									{
										if (pSourceTypeExt->Interceptor_VeteranSuccess >= 0)
										{
											successProbability = pSourceTypeExt->Interceptor_VeteranSuccess;
										}
									}
								}
							}
						}
							
						//Debug::Log("DEBUG: Interceptor: %d\% <= %d\% ??? R:%d V:%d E:%d\n", probability, successProbability, pSource->Veterancy.IsRookie(), pSource->Veterancy.IsVeteran(), pSource->Veterancy.IsElite());
						if (probability <= successProbability)
						{
							//Debug::Log("DEBUG: Intercepted projectile!\n");
							pBulletExt->Intercepted = true;
							pSourceExt->InterceptedBullet = nullptr;
						}
					}
				}
			}
		}
	}

	return 0;
}
