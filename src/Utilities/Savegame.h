#pragma once

#include "Stream.h"

#include <memory>
#include <type_traits>

namespace Savegame {
	template <typename T>
	bool ReadPhobosStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange = true);

	template <typename T>
	bool WritePhobosStream(PhobosStreamWriter& Stm, const T& Value);

	template <typename T>
	T* RestoreObject(PhobosStreamReader& Stm, bool RegisterForChange = true);

	template <typename T>
	bool PersistObject(PhobosStreamWriter& Stm, const T* pValue);

	template <typename T>
	struct PhobosStreamObject
	{
		bool ReadFromStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange) const;
		bool WriteToStream(PhobosStreamWriter& Stm, const T& Value) const;
	};

	template <typename T>
	struct ObjectFactory
	{
		std::unique_ptr<T> operator() (PhobosStreamReader& Stm) const
		{
			return std::make_unique<T>();
		}
	};
}
