#include "AresInterface.h"
#include <atlbase.h>
#include <Helpers/ComPtr.h>
#include "..//..//Utilities/Debug.h"

bool AresInterface::isInit = false;

IUnknown* AresInterface::Instance;
YRComPtr<IAresBuildTime> AresInterface::BuildTimeInterface;

void AresInterface::Initialize() {
	if (isInit == false) {
		Instance = *reinterpret_cast<IUnknown **>(0xB4557Cu);
		if (Instance) {
			BuildTimeInterface = YRComPtr<IAresBuildTime>(Instance);
		}
		isInit = true;
	}
};

long AresInterface::GetBuildTime(unsigned int abs, int idxType, HouseClass const* pOwner) {
	if (BuildTimeInterface) {
		AresBuildTimeInfo info{ 1 };

		if (SUCCEEDED(BuildTimeInterface->GetBuildTime(0, 0, pOwner, &info)) && info.BuildTime) {
			return info.BuildTime;
		}
	}
	return 0;
}
