#pragma once
#include <string>

struct Track {
	std::string path;
	std::string name;
	std::string author;

	Track() = default;
	Track(const std::string& _path) : path(_path) {}

	void clear();
	std::string toString() const;
};

struct Connection {
	std::string ip;
	int port;
	std::string mount;
	std::string username;
	std::string password;

	Connection();
	std::string toString() const;
};