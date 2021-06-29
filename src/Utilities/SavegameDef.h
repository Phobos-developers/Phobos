#pragma once

// include this file whenever something is to be saved.

#include "Savegame.h"

#include <vector>
#include <map>
#include <bitset>
#include <memory>

#include <ArrayClasses.h>
#include <FileSystem.h>
#include <FileFormats/SHP.h>
#include <RulesClass.h>
#include <SidebarClass.h>

#include "Swizzle.h"
#include "Debug.h"

namespace Savegame
{
	namespace detail
	{
		struct Selector
		{
			template <typename T>
			static bool ReadFromStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange)
			{
				return read_from_stream(Stm, Value, RegisterForChange, 0, 0);
			}

			template <typename T>
			static bool WriteToStream(PhobosStreamWriter& Stm, const T& Value)
			{
				return write_to_stream(Stm, Value, 0, 0);
			}

		private:
			// support for upper-case Load and lowercase load member functions.
			// this is more complex than needed, but allows for more consistency
			// in function naming.
			struct Dummy
			{
				Dummy(int a) { };
			};

			template <typename T>
			static auto read_from_stream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange, int, int)
				-> decltype(Value.Load(Stm, RegisterForChange))
			{
				return Value.Load(Stm, RegisterForChange);
			}

			template <typename T>
			static auto read_from_stream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange, Dummy, int)
				-> decltype(Value.load(Stm, RegisterForChange))
			{
				return Value.load(Stm, RegisterForChange);
			}

			template <typename T>
			static bool read_from_stream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange, Dummy, Dummy)
			{
				PhobosStreamObject<T> item;
				return item.ReadFromStream(Stm, Value, RegisterForChange);
			}

			template <typename T>
			static auto write_to_stream(PhobosStreamWriter& Stm, const T& Value, int, int)
				-> decltype(Value.Save(Stm))
			{
				return Value.Save(Stm);
			}

			template <typename T>
			static auto write_to_stream(PhobosStreamWriter& Stm, const T& Value, Dummy, int)
				-> decltype(Value.save(Stm))
			{
				return Value.save(Stm);
			}

			template <typename T>
			static bool write_to_stream(PhobosStreamWriter& Stm, const T& Value, Dummy, Dummy)
			{
				PhobosStreamObject<T> item;
				return item.WriteToStream(Stm, Value);
			}
		};
	}

	template <typename T>
	bool ReadPhobosStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange)
	{
		return detail::Selector::ReadFromStream(Stm, Value, RegisterForChange);
	}

	template <typename T>
	bool WritePhobosStream(PhobosStreamWriter& Stm, const T& Value)
	{
		return detail::Selector::WriteToStream(Stm, Value);
	}

	template <typename T>
	T* RestoreObject(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		T* ptrOld = nullptr;
		if (!Stm.Load(ptrOld))
			return nullptr;

		if (ptrOld)
		{
			std::unique_ptr<T> ptrNew = ObjectFactory<T>()(Stm);

			if (Savegame::ReadPhobosStream(Stm, *ptrNew, RegisterForChange))
			{
				PhobosSwizzle::Instance.RegisterChange(ptrOld, ptrNew.get());
				return ptrNew.release();
			}
		}

		return nullptr;
	}

	template <typename T>
	bool PersistObject(PhobosStreamWriter& Stm, const T* pValue)
	{
		if (!Savegame::WritePhobosStream(Stm, pValue))
			return false;

		if (pValue)
			return Savegame::WritePhobosStream(Stm, *pValue);

		return true;
	}

	template <typename T>
	bool PhobosStreamObject<T>::ReadFromStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange) const
	{
		bool ret = Stm.Load(Value);

		if (RegisterForChange)
			Swizzle swizzle(Value);

		return ret;
	}

	template <typename T>
	bool PhobosStreamObject<T>::WriteToStream(PhobosStreamWriter& Stm, const T& Value) const
	{
		Stm.Save(Value);
		return true;
	}


	// specializations

	template <typename T>
	struct Savegame::PhobosStreamObject<VectorClass<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, VectorClass<T>& Value, bool RegisterForChange) const
		{
			Value.Clear();
			int Capacity = 0;

			if (!Stm.Load(Capacity))
				return false;

			Value.Reserve(Capacity);

			for (auto ix = 0; ix < Capacity; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value.Items[ix], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const VectorClass<T>& Value) const
		{
			Stm.Save(Value.Capacity);

			for (auto ix = 0; ix < Value.Capacity; ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value.Items[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<DynamicVectorClass<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, DynamicVectorClass<T>& Value, bool RegisterForChange) const
		{
			Value.Purge();
			int Capacity = 0;

			if (!Stm.Load(Capacity))
				return false;

			Value.Reserve(Capacity);

			if (!Stm.Load(Value.Count) || !Stm.Load(Value.CapacityIncrement))
				return false;

			for (auto ix = 0; ix < Value.Count; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value.Items[ix], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const DynamicVectorClass<T>& Value) const
		{
			Stm.Save(Value.Capacity);
			Stm.Save(Value.Count);
			Stm.Save(Value.CapacityIncrement);

			for (auto ix = 0; ix < Value.Count; ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value.Items[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<TypeList<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, TypeList<T>& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream<DynamicVectorClass<T>>(Stm, Value, RegisterForChange))
				return false;

			return Stm.Load(Value.unknown_18);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const TypeList<T>& Value) const
		{
			if (!Savegame::WritePhobosStream<DynamicVectorClass<T>>(Stm, Value))
				return false;

			Stm.Save(Value.unknown_18);
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<CounterClass>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, CounterClass& Value, bool RegisterForChange) const
		{
			if (!Savegame::ReadPhobosStream<VectorClass<int>>(Stm, Value, RegisterForChange))
				return false;

			return Stm.Load(Value.Total);
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const CounterClass& Value) const
		{
			if (!Savegame::WritePhobosStream<VectorClass<int>>(Stm, Value))
				return false;

			Stm.Save(Value.Total);
			return true;
		}
	};

	template <size_t Size>
	struct Savegame::PhobosStreamObject<std::bitset<Size>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::bitset<Size>& Value, bool RegisterForChange) const
		{
			unsigned char value = 0;
			for (auto i = 0u; i < Size; ++i)
			{
				auto pos = i % 8;

				if (pos == 0 && !Stm.Load(value))
					return false;

				Value.set(i, ((value >> pos) & 1) != 0);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::bitset<Size>& Value) const
		{
			unsigned char value = 0;
			for (auto i = 0u; i < Size; ++i)
			{
				auto pos = i % 8;

				if (Value[i])
					value |= 1 << pos;

				if (pos == 7 || i == Size - 1)
				{
					Stm.Save(value);
					value = 0;
				}
			}

			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<std::string>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::string& Value, bool RegisterForChange) const
		{
			size_t size = 0;

			if (Stm.Load(size))
			{
				std::vector<char> buffer(size);

				if (!size || Stm.Read(reinterpret_cast<byte*>(&buffer[0]), size))
				{
					Value.assign(buffer.begin(), buffer.end());
					return true;
				}
			}
			return false;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::string& Value) const
		{
			Stm.Save(Value.size());
			Stm.Write(reinterpret_cast<const byte*>(Value.c_str()), Value.size());

			return true;
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::unique_ptr<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::unique_ptr<T>& Value, bool RegisterForChange) const
		{
			Value.reset(RestoreObject<T>(Stm, RegisterForChange));
			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::unique_ptr<T>& Value) const
		{
			return PersistObject(Stm, Value.get());
		}
	};

	template <typename T>
	struct Savegame::PhobosStreamObject<std::vector<T>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::vector<T>& Value, bool RegisterForChange) const
		{
			Value.clear();

			size_t Capacity = 0;

			if (!Stm.Load(Capacity))
				return false;

			Value.reserve(Capacity);

			size_t Count = 0;

			if (!Stm.Load(Count))
				return false;

			Value.resize(Count);

			for (auto ix = 0u; ix < Count; ++ix)
			{
				if (!Savegame::ReadPhobosStream(Stm, Value[ix], RegisterForChange))
					return false;
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::vector<T>& Value) const
		{
			Stm.Save(Value.capacity());
			Stm.Save(Value.size());

			for (auto ix = 0u; ix < Value.size(); ++ix)
			{
				if (!Savegame::WritePhobosStream(Stm, Value[ix]))
					return false;
			}

			return true;
		}
	};

	template <typename TKey, typename TValue>
	struct Savegame::PhobosStreamObject<std::map<TKey, TValue>>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, std::map<TKey, TValue>& Value, bool RegisterForChange) const
		{
			Value.clear();

			size_t Count = 0;
			if (!Stm.Load(Count))
			{
				return false;
			}

			for (auto ix = 0u; ix < Count; ++ix)
			{
				std::pair<TKey, TValue> buffer;
				if (!Savegame::ReadPhobosStream(Stm, buffer, RegisterForChange))
				{
					return false;
				}
				Value.insert(buffer);
			}

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const std::map<TKey, TValue>& Value) const
		{
			Stm.Save(Value.size());

			for (const auto& item : Value)
			{
				if (!Savegame::WritePhobosStream(Stm, item))
				{
					return false;
				}
			}
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<SHPStruct*>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, SHPStruct*& Value, bool RegisterForChange) const
		{
			if (Value && !Value->IsReference())
				Debug::Log("Value contains SHP file data. Possible leak.\n");

			Value = nullptr;

			bool hasValue = true;
			if (Savegame::ReadPhobosStream(Stm, hasValue) && hasValue)
			{
				std::string name;
				if (Savegame::ReadPhobosStream(Stm, name))
				{
					if (auto pSHP = FileSystem::LoadSHPFile(name.c_str()))
					{
						Value = pSHP;
						return true;
					}
				}
			}

			return !hasValue;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, SHPStruct* const& Value) const
		{
			const char* filename = nullptr;
			if (Value)
			{
				if (auto pRef = Value->AsReference())
					filename = pRef->Filename;
				else
					Debug::Log("Cannot save SHPStruct, because it isn't a reference.\n");
			}

			if (Savegame::WritePhobosStream(Stm, filename != nullptr))
			{
				if (filename)
				{
					std::string file(filename);
					return Savegame::WritePhobosStream(Stm, file);
				}
			}

			return filename == nullptr;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<RocketStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, RocketStruct& Value, bool RegisterForChange) const
		{
			if (!Stm.Load(Value))
				return false;

			if (RegisterForChange)
				Swizzle swizzle(Value.Type);

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const RocketStruct& Value) const
		{
			Stm.Save(Value);
			return true;
		}
	};

	template <>
	struct Savegame::PhobosStreamObject<CameoDataStruct>
	{
		bool ReadFromStream(PhobosStreamReader& Stm, CameoDataStruct& Value, bool RegisterForChange) const
		{
			if (!Stm.Load(Value))
				return false;

			if (RegisterForChange)
				Swizzle swizzle(Value.CurrentFactory);

			return true;
		}

		bool WriteToStream(PhobosStreamWriter& Stm, const CameoDataStruct& Value) const
		{
			Stm.Save(Value);
			return true;
		}
	};
}
