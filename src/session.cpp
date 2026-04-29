#include "session.h"
#include "application.h"
#include "string_converting.h"

#include <print>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <numeric>
#include <random>

const char* connectionStatusString(ConnectionStatus status) {
	switch (status) {
	case ConnectionStatus::DISCONNECTED: return "DISCONNECTED";
	case ConnectionStatus::CONNECTING: return "CONNECTING";
	case ConnectionStatus::CONNECTED: return "CONNECTED";
	case ConnectionStatus::STREAMING: return "STREAMING";
	case ConnectionStatus::FAILED: return "FAILED";
	default: return "UNKNOWN";
	}
}

Session::Session()
	: currentIndex(0), currentTrackIndex(0), shout(nullptr),
	connectionStatus(ConnectionStatus::DISCONNECTED),
	trackListNew(true) {

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
	trackIndicies.push_back(tracks.size() - 1);
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
		if (tracks.empty()) return TaskStatus::COMPLETED;
		
		if (isReshufflingNeeded() &&
			worker.getLastCompleted() != CommandType::CMD_SHUFFLE) shuffleTracks(false);
		currentIndex.store((currentIndex.load() + 1) % tracks.size()); // !!!
		updateTrackIndex();

		return TaskStatus::COMPLETED;
	});

	worker.onCommand<CMD_SHUFFLE, ShuffleTracksCommand>([this](ShuffleTracksCommand* command) -> TaskStatus {
		std::lock_guard<std::mutex> lock(worker.getMutex());
		shuffleTracks(false);
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
	std::ifstream fin(rawPath);

	if (!fin) {
		std::println(stderr, "Can't open playlist!");
		return false;
	}

	std::lock_guard<std::mutex> lock(worker.getMutex());

	std::string trackPath;
	size_t i = 0;

	tracks.clear();
	while (std::getline(fin, trackPath)) {
		if (trackPath.empty()) continue;
		tracks.emplace_back(trackPath);
		i++;
	}

	if (tracks.size() <= currentIndex.load()) currentIndex.store(0);

	trackIndicies.resize(tracks.size());
	std::iota(trackIndicies.begin(), trackIndicies.end(), 0);

	trackListNew.store(false);
	fin.close();
	return true;
}

void Session::savePlaylist(const std::string& rawPath) {
	std::ofstream fout(rawPath);

	std::lock_guard<std::mutex> lock(worker.getMutex());

	for (const Track& track : tracks) {
		fout << track.path << '\n';
	}

	fout.close();
}

void Session::shuffleTracks(bool lock) {
	std::println("Track list needs to be shuffled");

	if (lock) worker.getMutex().lock();
	std::iota(trackIndicies.begin(), trackIndicies.end(), 0);
	std::minstd_rand random(std::random_device{}());
	std::shuffle(trackIndicies.begin(), trackIndicies.end(), random);
	updateTrackIndex();
	if (lock) worker.getMutex().unlock();
}

bool Session::isReshufflingNeeded() {
	return randomPlaybackEnabled.load() && currentIndex.load() >= tracks.size() - 1;
}

void Session::updateTrackIndex() {
	currentTrackIndex.store(trackIndicies[currentIndex.load()]);
}

Track& Session::getCurrentTrack() {
    std::lock_guard<std::mutex> lock(worker.getMutex());
    
	if (tracks.empty()) {
        static Track empty;
        return empty;
    }

	int index = trackIndicies[currentIndex.load()];
    return tracks[index];
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
	for (int i = 0; i < tracks.size(); i++) func(i, tracks[i]);
}

void Session::eachShuffledTrack(const std::function<void(int, Track&)>& func) {
	std::lock_guard<std::mutex> lock(worker.getMutex());
	for (int index : trackIndicies) func(index, tracks[index]);
}

bool Session::isListEmpty() {
	std::lock_guard<std::mutex> lock(worker.getMutex());
	return tracks.empty();
}