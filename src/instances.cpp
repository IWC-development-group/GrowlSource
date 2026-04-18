#include "instances.h"
#include <format>

void Track::clear() {
	path.clear();
	name.clear();
	author.clear();
}

std::string Track::toString() const {
	//return "Track:\n\tPath: " + path + "\n\tName: " + name + "\n\tAuthor: " + author;
	return std::format("Track:\n\tPath: {}\n\tName: {}\n\tAuthor: {}",
		path, name, author);
}

Connection::Connection() : port(0) {}

std::string Connection::toString() const {
	return std::format("Connection:\n\tIP: {}\n\tPort: {}\n\tMount: {}\n\tUsername: {}\n\tPassword: {}",
		ip, port, mount, username, password);
}