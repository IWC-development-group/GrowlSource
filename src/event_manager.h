#pragma once
#include <any>
#include <unordered_map>
#include "lsid/lsid.h"

class EventManager {
private:
	std::unordered_map<uint32_t, std::any> eventTable;

public:
	EventManager() {}

	template <lsid::StringLiteral Name, typename EventT>
	void add(const EventT& event) {
		uint32_t id = lsid::Ssid<Name>{}.value();
		eventTable[id] = event;
	}

	template <typename EventT, lsid::StringLiteral Name>
	EventT* get() {
		auto it = eventTable.find(lsid::Ssid<Name>{}.value());
		return std::any_cast<EventT>(&it->second);
	}

	template <typename EventT, lsid::StringLiteral Name, typename ...Args>
	void fire(Args&&... args) { get<EventT, Name>()->fire(std::forward<Args>(args)...); }
};