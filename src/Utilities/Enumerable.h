#pragma once

#include "Savegame.h"
#include "Constructs.h"

#include <algorithm>
#include <vector>

#include <CCINIClass.h>
#include "Swizzle.h"

template <typename T> class Enumerable
{
public:
	inline static std::vector<T> Array;

	static int FindIndex(const char* Title)
	{
		auto result = std::find_if(Array.begin(), Array.end(), [Title](const T& Item)
			{
				return !_strcmpi(Item.Name, Title);
			});

		if (result == Array.end())
			return -1;

		return std::distance(Array.begin(), result);
	}

	static T* Find(const char* Title)
	{
		auto result = FindIndex(Title);
		return (result < 0) ? nullptr : &Array[static_cast<size_t>(result)];
	}

	static T* FindOrAllocate(const char* Title)
	{
		if (T* found = Find(Title))
			return found;
		static_assert(std::constructible_from<T, const char*>);
		Array.emplace_back(Title);

		return &Array.back();
	}

	static void Clear()
	{
		Array.clear();
	}

	static void LoadFromINIList(CCINIClass* pINI)
	{
		const char* section = GetMainSection();
		int len = pINI->GetKeyCount(section);

		for (int i = 0; i < len; ++i)
		{
			if (pINI->ReadString(section, pINI->GetKeyName(section, i), "", Phobos::readBuffer))
				FindOrAllocate(Phobos::readBuffer);
		}

		for (auto& item : Array)
			item.LoadFromINI(pINI);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		Clear();

		size_t Count = 0;
		if (!Stm.Load(Count))
			return false;


		for (size_t i = 0; i < Count; ++i)
		{
			void* oldPtr = nullptr;
			decltype(Name) name;

			if (!Stm.Load(oldPtr) || !Stm.Load(name))
				return false;

			auto newPtr = FindOrAllocate(name);
			PhobosSwizzle::RegisterChange(oldPtr, newPtr);

			newPtr->LoadFromStream(Stm);
		}

		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		Stm.Save(Array.size());

		for (auto& item : Array)
		{
			// write old pointer and name, then delegate
			Stm.Save(&item);
			Stm.Save(item.Name);
			item.SaveToStream(Stm);
		}

		return true;
	}

	static const char* GetMainSection();

	Enumerable(const char* Title)
	{
		this->Name = Title;
	}

	void LoadFromINI(CCINIClass* pINI) = delete;

	void LoadFromStream(PhobosStreamReader& Stm) = delete;

	void SaveToStream(PhobosStreamWriter& Stm) = delete;

	PhobosFixedString<32> Name;
};
