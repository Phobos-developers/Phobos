#pragma once

#include <vector>

template<typename T>
class EventQueue {
public:
    using function_type = void(*)(T*);
	using container_type = std::vector<function_type>;

	template<typename TFunc>
	EventQueue(std::initializer_list<TFunc> items)
	{
		for (auto item : items)
			insert(item);
	}

	bool operator() (function_type item) {
		insert(item);
		return true;
	}

	void insert(function_type value) {
		_events.push_back(value);
	}

	size_t size() const {
		return _events.size();
	}

	typename container_type::const_iterator begin() const {
		return _events.begin();
	}

	typename container_type::const_iterator end() const {
		return _events.end();
	}

	void run_each(T* pointer) const {
		for (auto event : _events)
			event(pointer);
	}

private:
	container_type _events;
};
