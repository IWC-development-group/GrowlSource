#pragma once
#include <shout/shout.h>
#include <vector>
#include <string>

#include "instances.h"
#include "event_worker.h"

enum CommandType : uint32_t {
	CMD_CONNECTION,
	CMD_STREAM_TRACK,
	CMD_NEXT_TRACK,
	CMD_TEST
};

class ConnectionCommand : public Command {
private:
	Connection connection;

public:
	explicit ConnectionCommand(const Connection& _connection)
		: Command(CMD_CONNECTION), connection(_connection) {}

	const Connection& getConnection() const { return connection; }
};

class StreamTrackCommand : public Command {
private:
	shout_t* shout;
	Track track;

public:
	StreamTrackCommand(shout_t* _shout, const Track& _track)
		: Command(CMD_STREAM_TRACK), shout(_shout), track(_track) {}

	shout_t* getShout() const { return shout; }
	Track& getTrack() { return track; }
};

class NextTrackCommand : public Command {
private:
	int trackIndex;

public:
	NextTrackCommand(int _trackIndex) : Command(CMD_NEXT_TRACK), trackIndex(_trackIndex) {}

	int getTrackIndex() const { return trackIndex; }
};

enum class ConnectionStatus : uint8_t {
	DISCONNECTED,
	CONNECTING,
	CONNECTED,
	STREAMING,
	FAILED
};

const char* connectionStatusString(ConnectionStatus status);

class Session {
private:
	CommandWorker worker;
	std::vector<Track> tracks; // !!!
	shout_t* shout; // !!!
	std::atomic<int> currentIndex;
	std::atomic<ConnectionStatus> connectionStatus;

public:
	Session();
	~Session();

	FILE* openTrack(const Track& track);
	void connect(const Connection& connection);
	void addTrack(const Track& track);
	void nextTrack();
	void playCurrent();
	void editTrack(int trackIndex, const Track& track);
	void setup();
	void eachTrack(const std::function<void(int, Track&)>& func);

	int getCurrentTrackIndex() const { return currentIndex.load(); };
	Track& getCurrentTrack();
	Track& getTrack(int trackIndex);

	bool isListEmpty();
	bool isOnFirstTrack() const { return currentIndex == 0; }
	ConnectionStatus getConnectionStatus() const { return connectionStatus.load(); }
};