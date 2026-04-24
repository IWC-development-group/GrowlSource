#pragma once
#include "application.h"
#include "session.h"

class UiLayer;
class Growl : public Application {
private:
	UiLayer* gui;
	Session session;

public:
	Growl();

	UiLayer* getUiLayer() { return gui; }
	void onUpdate() override;
};
