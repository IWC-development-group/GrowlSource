#include "event_worker.h"
#include <print>

CommandWorker::CommandWorker()
	: worker(&CommandWorker::job, this), running(false), processing(false) {
}

CommandWorker::~CommandWorker() {
	if (running.load()) stop();
	worker.join();
}

std::unique_ptr<Command> CommandWorker::getQueueFront() {
	std::lock_guard<std::mutex> lock(mtx);
	if (commandQueue.empty()) return nullptr;
	std::unique_ptr<Command> cmd = std::move(commandQueue.front());
	commandQueue.pop();
	return cmd;
}

void CommandWorker::job() {
	{
		std::unique_lock<std::mutex> initLock(mtx);
		condVar.wait(initLock, [this]() { return running.load(); });
	}

	while (running.load(std::memory_order_acquire)) {
		std::unique_ptr<Command> cmd = getQueueFront();
		if (cmd != nullptr) { // !!!
			auto it = reactions.find(cmd->getType());
			if (it != reactions.end()) {
				processing = true;
				TaskStatus status = it->second(cmd.get());
				if (status == TaskStatus::ERROR) clearQueue();
				processing = false;
			}
		}
		
		std::unique_lock<std::mutex> lock(mtx);
		condVar.wait(lock, [this]() {
			return !commandQueue.empty();
		});
	}
}

void CommandWorker::clearQueue() {
	std::lock_guard<std::mutex> lock(mtx);
	while (!commandQueue.empty()) commandQueue.pop();
}

void CommandWorker::submit(std::unique_ptr<Command> command) {
	std::lock_guard<std::mutex> lock(mtx);
	commandQueue.push(std::move(command));
	condVar.notify_one();
}

void CommandWorker::run() {
	running.store(true);
	condVar.notify_one();
}

void CommandWorker::stop() {
	running.store(false);
}

bool CommandWorker::isBusy() const {
	std::lock_guard<std::mutex> lock(mtx);
	return !commandQueue.empty() || processing;
}

bool CommandWorker::isRunning() const {
	return running.load(std::memory_order_relaxed);
}