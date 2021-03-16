#pragma once

#include <functional>
#include <set>

template<typename T>
struct funcless
{
	using function_type = std::function<void(T*)>;
	bool operator()(const function_type& a, const function_type& b) const
	{
		return a.target<void(T*)>() < b.target<void(T*)>();
	}
};

template<typename T>
class EventQueue {
public:
    using function_type = std::function<void(T*)>;
	using container_type = std::set<function_type, funcless<T>>;
    
	EventQueue(function_type item) { insert(item); }

	EventQueue(void(*item)(T*)) 
	{
		function_type func = item;
		insert(func); 
	}

	template<typename TFunc>
	EventQueue(std::initializer_list<TFunc> items)
	{
		for (auto item : items)
		{
			function_type func = item;
			insert(func);
		}
	}

	bool operator() (function_type item) {
		insert(item);
		return true;
	}

	void insert(function_type value) {
		_events.insert(value);
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
