#include "session.h"
#include "application.h"
#include "string_converting.h"

#include <print>
#include <iostream>
#include <fstream>
#include <cstdio>

const char* connectionStatusString(ConnectionStatus status) {
	switch (status) {
	case ConnectionStatus::DISCONNECTED: return "DISCONNECTED";
	case ConnectionStatus::CONNECTING: return "CONNECTING";
	case ConnectionStatus::CONNECTED: return "CONNECTED";
	case ConnectionStatus::STREAMING: return "STREAMING";
	case ConnectionStatus::FAILED: return "FAILED";
	}
}

Session::Session()
	: currentIndex(0), shout(nullptr), connectionStatus(ConnectionStatus::DISCONNECTED) {
	shout = shout_new();
	if (!shout) {
		std::cerr << "Failed to create shout_t\n";
	}

	setup();
}

Session::~Session() {
	if (shout) {
		shout_close(shout);
		shout_free(shout);
	}
}

void Session::addTrack(const Track& track) {
	std::lock_guard<std::mutex> lock(worker.getMutex());
	tracks.push_back(track);
}

void Session::setup() {
	worker.onCommand<CMD_CONNECTION, ConnectionCommand>([this](ConnectionCommand* command) -> TaskStatus {
		if (!shout) {
			std::println(stderr, "Shout not initialized!");
			return TaskStatus::ERROR;
		}

		const Connection& data = command->getConnection();

		shout_set_host(shout, data.ip.c_str());
		shout_set_port(shout, data.port);
		shout_set_protocol(shout, SHOUT_PROTOCOL_HTTP);
		shout_set_format(shout, SHOUT_FORMAT_MP3);
		shout_set_mount(shout, data.mount.c_str());
		shout_set_user(shout, data.username.c_str());
		shout_set_password(shout, data.password.c_str());

		int ret = shout_open(shout);
		if (ret != SHOUTERR_SUCCESS) {
			std::println(stderr, "shout_open failed: {} (code: {})", shout_get_error(shout), ret);
			connectionStatus.store(ConnectionStatus::FAILED);
			shout_close(shout);
			return TaskStatus::ERROR;
		}

		connectionStatus.store(ConnectionStatus::CONNECTED);
		std::println("Connected to {}", data.toString());
		return TaskStatus::COMPLETED;
	});

	worker.onCommand<CMD_STREAM_TRACK, StreamTrackCommand>([this](StreamTrackCommand* command) -> TaskStatus {
		if (connectionStatus.load() != ConnectionStatus::CONNECTED) {
			std::println(stderr, "Not connected! Can't start streaming.");
			setPlayRequested(false);
			return TaskStatus::COMPLETED;
		}

		FILE* f = openTrack(command->getTrack());
		if (!f) {
			std::println(stderr, "Can't open track!");
			setPlayRequested(false);
			return TaskStatus::COMPLETED;
		}

		std::vector<unsigned char> buffer(4096);
		shout_t* shout = command->getShout();
		connectionStatus.store(ConnectionStatus::STREAMING);

		while (!feof(f) && Application::current()->isRunning()) {
			size_t bytesRead = fread(buffer.data(), 1, buffer.size(), f);
			if (bytesRead > 0) {
				if (shout_send(shout, buffer.data(), bytesRead) != SHOUTERR_SUCCESS) { // !!!
					std::println(stderr, "shout_send error: {}", shout_get_error(shout));
					break;
				}
			}
			shout_sync(shout);
		}

		fclose(f);
		setPlayRequested(false);
		connectionStatus.store(ConnectionStatus::CONNECTED);
		return TaskStatus::COMPLETED;
	});

	worker.onCommand<CMD_NEXT_TRACK, NextTrackCommand>([this](NextTrackCommand* command) -> TaskStatus {
		std::lock_guard<std::mutex> lock(worker.getMutex());
		
		//std::println("Track list size: {}", tracks.size());
		if (!tracks.empty()) {
			currentIndex.store((currentIndex.load() + 1) % tracks.size()); // !!!
		}

		return TaskStatus::COMPLETED;
	});

	worker.run();
}

void Session::connect(const Connection& connection) {
	if (connectionStatus == ConnectionStatus::CONNECTED ||
		connectionStatus == ConnectionStatus::CONNECTING) {
		std::println("Connection is already initiated!");
	}

	connectionStatus.store(ConnectionStatus::CONNECTING);
	worker.submit(std::make_unique<ConnectionCommand>(connection));
}

void Session::nextTrack() {
	worker.submit(std::make_unique<NextTrackCommand>(currentIndex.load()));
}

void Session::playCurrent() {
	Track& track = getCurrentTrack();
	if (track.path.empty()) return;

	worker.submit(std::make_unique<StreamTrackCommand>(shout, track));
}

void Session::editTrack(int trackIndex, const Track& track) {
	std::lock_guard<std::mutex> lock(worker.getMutex());
	tracks[trackIndex] = track;
}

bool Session::loadPlaylistFromFile(const std::string& rawPath) {
	std::string path = sconv::utf8ToCp1251(rawPath);
	std::ifstream fin(path);

	if (!fin) {
		std::println(stderr, "Can't open playlist!");
		return false;
	}

	std::lock_guard<std::mutex> lock(worker.getMutex());

	std::string trackPath;
	size_t i = 0, trackListSize = tracks.size();

	while (std::getline(fin, trackPath)) {
		if (trackPath.empty()) continue;
		if (i < trackListSize) tracks[i] = Track(trackPath);
		else tracks.emplace_back(trackPath);
		i++;
	}

	fin.close();
	return true;
}

void Session::savePlaylist(const std::string& rawPath) {
	std::string path = sconv::utf8ToCp1251(rawPath);
	std::ofstream fout(path);

	std::lock_guard<std::mutex> lock(worker.getMutex());

	for (const Track& track : tracks) {
		fout << track.path << '\n';
	}

	fout.close();
}

Track& Session::getCurrentTrack() {
    std::lock_guard<std::mutex> lock(worker.getMutex());
    
	if (tracks.empty()) {
        static Track empty;
        return empty;
    }

    return tracks[currentIndex.load()];
}

Track& Session::getTrack(int trackIndex) {
	std::lock_guard<std::mutex> lock(worker.getMutex());
	return tracks[trackIndex];
}

FILE* Session::openTrack(const Track& track) {
	std::string path = sconv::utf8ToCp1251(track.path);
	if (path.empty()) {
		std::println("Can't open track: Path is empty!");
		return nullptr;
	}

	FILE* f = nullptr;

	if (fopen_s(&f, path.c_str(), "rb") != 0 || !f) {
		std::println("Can't open track: {}", track.path);
		return nullptr;
	}

	return f;
}

void Session::eachTrack(const std::function<void(int, Track&)>& func) {
	std::lock_guard<std::mutex> lock(worker.getMutex());
	for (int i = 0; i < tracks.size(); i++) {
		func(i, tracks[i]);
	}
}

bool Session::isListEmpty() {
	std::lock_guard<std::mutex> lock(worker.getMutex());
	return tracks.empty();
}