#pragma once
#include "event.h"
#include "instances.h"

class TrackAddedEvent : public Event<const Track&> {
public:
	TrackAddedEvent() {}
};

class TrackEditedEvent : public Event<int, const Track&> {
public:
	TrackEditedEvent() {}
};

class ConnectionEvent : public Event<const Connection&> {
public:
	ConnectionEvent() {}
};

class PlaylistManageEvent : public Event<void*> {
public:
	PlaylistManageEvent() {}
};

class ShufflingSelectionEvent : public Event<bool> {
public:
	ShufflingSelectionEvent() {}
};