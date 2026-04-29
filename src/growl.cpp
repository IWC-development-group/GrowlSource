#include <print>

#include "growl.h"
#include "custom_events.h"
#include "demo_layer.h"
#include "ui_layer.h"
#include "event_manager.h"

Growl::Growl() : Application("Growl", 854, 480) {
	EventManager eventManager;
	TrackAddedEvent trackAdded;

	trackAdded.onEvent([this](const Track& track) {
		std::println("Adding new track!");
		session.addTrack(track);
	});

	ConnectionEvent clientConnects;

	clientConnects.onEvent([this](const Connection& connection) {
		std::println("Connecting...");
		session.connect(connection);
	});

	ShufflingSelectionEvent shufflingChanged;
	
	shufflingChanged.onEvent([this](bool enabled) {
		session.setRandomPlaybackEnabled(enabled);
		if (enabled) session.shuffleTracks(true);
	});

	events.add<"EVT_TRACK_ADDED">(trackAdded);
	events.add<"EVT_CONNECTION">(clientConnects);
	events.add<"EVT_SHUFFLING_SELECTION">(shufflingChanged);

	gui = layers().attach<UiLayer>(session, trackAdded, clientConnects);
	//layers().attach<DemoLayer>();
}

void Growl::onUpdate() {
	if (session.isListEmpty()) return;

	ConnectionStatus status = session.getConnectionStatus();
	if (status == ConnectionStatus::CONNECTED && !session.isPlayRequested()) {
		std::println("Requesting next track!");
		session.playCurrent();
		session.nextTrack();
		session.setPlayRequested(true);
	}
}