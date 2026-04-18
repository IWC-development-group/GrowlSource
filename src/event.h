#pragma once
#include <vector>
#include <functional>

template <typename ...Args>
class Event {
	using EventCallback = std::function<void(Args...)>;
private:
	std::vector<EventCallback> callbacks;

public:
	Event() {}

	void onEvent(const EventCallback& callback);
	void fire(Args... args);
};

template <typename ...Args>
void Event<Args...>::onEvent(const EventCallback& callback) {
	callbacks.push_back(callback);
}

template <typename ...Args>
void Event<Args...>::fire(Args... args) {
	for (const EventCallback& callback : callbacks) callback(args...);
}