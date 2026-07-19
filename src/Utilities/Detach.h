#pragma once

#include <vector>

class AbstractClass;

// Typed pointer-invalidation registry.
// An extension (or a static aggregate for high-volume types) subscribes to deletions
// of a game type T by inheriting Detach::Listener<T> and overriding OnDetach.
// The game's Detach_This_From_All funnel (hook at 0x7258D0) dispatches the dying
// object to Registry<T>::Notify for its concrete type and every base of it.
namespace Detach
{
	template <typename T>
	class Listener;

	template <typename T>
	class Registry final
	{
	public:
		Registry() = delete;

		static void Add(Listener<T>* const pListener)
		{
			pListener->RegistryIndex = Listeners.size();
			Listeners.push_back(pListener);
		}

		static void Remove(Listener<T>* const pListener)
		{
			const size_t index = pListener->RegistryIndex;
			Listeners[index] = Listeners.back();
			Listeners[index]->RegistryIndex = index;
			Listeners.pop_back();
		}

		static void Notify(T* const pTarget, const bool removed)
		{
			// backwards: a listener may unregister itself from within OnDetach
			for (size_t i = Listeners.size(); i-- > 0;)
				Listeners[i]->OnDetach(pTarget, removed);
		}

	private:
		static inline std::vector<Listener<T>*> Listeners {};
	};

	template <typename T>
	class Listener
	{
	public:
		Listener()
		{
			Registry<T>::Add(this);
		}

		Listener(const Listener&) = delete;
		Listener& operator=(const Listener&) = delete;

		virtual ~Listener()
		{
			Registry<T>::Remove(this);
		}

		virtual void OnDetach(T* pTarget, bool removed) = 0;

	private:
		size_t RegistryIndex;

		friend class Registry<T>;
	};

	void NotifyAbstract(AbstractClass* pTarget, bool removed);
}
