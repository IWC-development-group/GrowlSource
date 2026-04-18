#pragma once
#include <thread>
#include <chrono>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include <stdint.h>

#define NULL_COMMAND	UINT32_MAX

class Command {
private:
	uint32_t type;

public:
	Command(uint32_t _type) : type(_type) {}
	uint32_t getType() const { return type; }
};

enum class TaskStatus : uint8_t {
	COMPLETED,
	ERROR
};

class CommandWorker {
	using CommandReaction = std::function<TaskStatus(Command*)>;
private:
	std::condition_variable condVar;
	std::atomic_bool running;
	bool processing;
	mutable std::mutex mtx;
	std::thread worker;
	std::queue<std::unique_ptr<Command>> commandQueue;
	std::unordered_map<uint32_t, CommandReaction> reactions;

	void job();
	void clearQueue();
	std::unique_ptr<Command> getQueueFront();

public:
	CommandWorker();
	~CommandWorker();

	template <uint32_t Type, typename CommandType>
	void onCommand(std::function<TaskStatus(CommandType*)> callback);
	void submit(std::unique_ptr<Command> command);
	void run();
	void stop();

	bool isBusy() const;
	bool isRunning() const;

	std::mutex& getMutex() { return mtx; }
};

template <uint32_t Type, typename CommandType>
void CommandWorker::onCommand(std::function<TaskStatus(CommandType*)> callback) {
	std::lock_guard<std::mutex> lock(mtx);
	reactions[Type] = [func = std::move(callback)](Command* command) -> TaskStatus {
		return func((CommandType*)command);
	};
}