#pragma once

#include "Stream.h"

#include <memory>
#include <type_traits>

namespace Savegame
{
	template <typename T>
	concept ImplementsUpperCaseSaveLoad = requires (PhobosStreamWriter& stmWriter, PhobosStreamReader& stmReader, T& value, bool registerForChange)
	{
		value.Save(stmWriter);
		value.Load(stmReader, registerForChange);
	};

	template <typename T>
	concept ImplementsLowerCaseSaveLoad = requires (PhobosStreamWriter & stmWriter, PhobosStreamReader & stmReader, T& value, bool registerForChange)
	{
		value.save(stmWriter);
		value.load(stmReader, registerForChange);
	};

	template <typename T>
	concept ImplementsSaveLoad = ImplementsUpperCaseSaveLoad<T> || ImplementsLowerCaseSaveLoad<T>;

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
