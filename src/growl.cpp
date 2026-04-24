#include <print>

#include "growl.h"
#include "custom_events.h"
#include "ui_layer.h"

Growl::Growl() : Application("Growl", 854, 480) {
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

	gui = layers().attach<UiLayer>(session, trackAdded, clientConnects);
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