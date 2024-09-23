#pragma once

#include <Phobos.CRT.h>
#include "Savegame.h"
#include "Constructs.h"

#include <algorithm>
#include <memory>
#include <vector>

#include <ArrayClasses.h>
#include <CCINIClass.h>
#include "Swizzle.h"

template <typename T> class Enumerable
{
	typedef std::vector<std::unique_ptr<T>> container_t;

public:
	static container_t Array;

	static int FindIndex(const char* Title)
	{
		auto result = std::find_if(Array.begin(), Array.end(), [Title](std::unique_ptr<T>& Item)
			{
				return !_strcmpi(Item->Name, Title);
			});

		if (result == Array.end())
			return -1;

		return std::distance(Array.begin(), result);
	}

	static T* Find(const char* Title)
	{
		auto result = FindIndex(Title);
		return (result < 0) ? nullptr : Array[static_cast<size_t>(result)].get();
	}

	static T* FindOrAllocate(const char* Title)
	{
		if (T* find = Find(Title))
			return find;

		Array.push_back(std::make_unique<T>(Title));

		return Array.back().get();
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

		for (const auto& item : Array)
			item->LoadFromINI(pINI);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		Clear();

		size_t Count = 0;
		if (!Stm.Load(Count))
			return false;


		for (size_t i = 0; i < Count; ++i) {
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

		for (const auto& item : Array)
		{
			// write old pointer and name, then delegate
			Stm.Save(item.get());
			Stm.Save(item->Name);
			item->SaveToStream(Stm);
		}

		return true;
	}

	static const char* GetMainSection();

	Enumerable(const char* Title)
	{
		this->Name = Title;
	}

	void LoadFromINI(CCINIClass* pINI) { static_cast<T*>(this)->LoadFromINI(pINI); }//=0;

	void LoadFromStream(PhobosStreamReader& Stm) { static_cast<T*>(this)->LoadFromStream(Stm); }//=0;

	void SaveToStream(PhobosStreamWriter& Stm) { static_cast<T*>(this)->SaveToStream(Stm); } //=0;

	PhobosFixedString<32> Name;
};

template <typename T>
Enumerable<T>::container_t Enumerable<T>::Array;
