#pragma once
#include <Utilities/TemplateDef.h>
#include <TechnoClass.h>
#include <memory>
#include <vector>


class SquadManager
{
public:
	static std::vector<std::unique_ptr<SquadManager>> Array;

	ValueableVector<TechnoClass*> Squad_Members;
	bool isSelected;

	SquadManager();

	~SquadManager();
	

	void AddTechno(TechnoClass* const pTechno);
	void RemoveTechno(TechnoClass* const pTechno);
	void Remove(TechnoClass* const pTechno) noexcept;

	static void RemoveGlobals(SquadManager* const pSquadManager) noexcept;
	static void PointerGotInvalid(void* ptr, bool removed);

//=====================Save/Load========================
	bool Load(PhobosStreamReader& stm);
	bool Save(PhobosStreamWriter& stm);

	static SquadManager* Allocate()
	{
		Array.emplace_back(std::make_unique<SquadManager>());
		return Array.back().get();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		Array.clear();

		size_t Count = 0;
		if (!Stm.Load(Count))
			return false;

		Array.reserve(Count);
		for (size_t i = 0; i < Count; ++i)
		{
			void* oldPtr = nullptr;

			if (!Stm.Load(oldPtr))
				return false;

			auto newPtr = Allocate();
			PhobosSwizzle::RegisterChange(oldPtr, newPtr);

			newPtr->Load(Stm);
		}

		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		Stm.Save(Array.size());

		for (auto const& item : Array)
		{
			// write old pointer and name, then delegate
			Stm.Save(item.get());
			item->Save(Stm);
		}

		return true;
	}

private:
	template <typename T>
	bool Serialize(T& stm);
};
