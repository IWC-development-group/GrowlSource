#pragma once
#include "application.h"
#include "session.h"
#include "event_manager.h"

class UiLayer;
class Growl : public Application {
private:
	UiLayer* gui;
	EventManager events;
	Session session;

public:
	Growl();

	EventManager& getEventManager() { return events; }
	UiLayer* getUiLayer() { return gui; }
	void onUpdate() override;
};
