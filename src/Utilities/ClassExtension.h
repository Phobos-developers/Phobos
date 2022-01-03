#pragma once

#include <Phobos.CRT.h>
#include "Savegame.h"
#include "Constructs.h"

#include <algorithm>
#include <memory>
#include <vector>

#include <ArrayClasses.h>
#include <CCINIClass.h>

template <typename BaseType, typename ExtType/*, DWORD ExtCanary*/> class NOVTABLE ClassExtension : BaseType
{
	// template<> static inline const DWORD Extension<BaseType>::Canary = ExtCanary;

	class ExtData final : public Extension<BaseType>
	{
		// template<> static inline const DWORD Canary = ExtCanary;

	public:
		ExtData(BaseType* pOwnerObject) : Extension<BaseType>(pOwnerObject) { }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override
		{
			Extension<BaseType>::LoadFromStream(Stm);
			this->Serialize(Stm);
		}

		virtual void SaveToStream(PhobosStreamWriter& Stm) override
		{
			Extension<BaseType>::SaveToStream(Stm);
			this->Serialize(Stm);
		}
	};

	class ExtContainer final : public Container<ExtType>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	// TODO write the rest

};