#include "instances.h"
#include "string_converting.h"

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <print>
#include <format>
#include <iostream>
#include <fstream>
#include <filesystem>

Track::Track(const std::string& _path) : path(_path) {
	std::string cp1251Path = sconv::utf8ToCp1251(path.c_str());

	TagLib::FileRef file(cp1251Path.c_str());
	if (file.isNull()) {
		std::println(stderr, "Can't open or detect format: {}", cp1251Path);
		return;
	}

	TagLib::Tag* tags = file.tag();
	if (!tags) {
		std::println(stderr, "File has no audio tags: {}", cp1251Path);
		return;
	}

	TagLib::String title = tags->title();
	TagLib::String artist = tags->artist();

	if (!title.isEmpty()) name = title.toCString(true);
	else name = std::filesystem::path(path).stem().string();

	if (!artist.isEmpty()) author = artist.toCString(true);
}

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