#pragma once
#include <string>

namespace sconv {
	std::string cp1251ToUTF8(const std::string& str);
	std::string utf8ToCp1251(const std::string& str);
	void setProperEncoding();
}