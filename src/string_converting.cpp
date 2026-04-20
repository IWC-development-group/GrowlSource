#include <windows.h>
#include <vector>

#include "string_converting.h"

namespace sconv {

#ifdef _WIN32

	std::string cp1251ToUTF8(const std::string& str) {
		int len = MultiByteToWideChar(1251, 0, str.c_str(), -1, NULL, 0);
		std::vector<wchar_t> wstr(len);
		MultiByteToWideChar(1251, 0, str.c_str(), -1, &wstr[0], len);

		len = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, NULL, 0, 0, 0);

		std::string utf8; utf8.resize(len - 1);
		WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, &utf8[0], len, NULL, NULL);

		return utf8;
	}

	std::string utf8ToCp1251(const std::string& str) {
		int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		std::vector<wchar_t> wstr(len);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);

		len = WideCharToMultiByte(1251, 0, &wstr[0], -1, NULL, 0, NULL, NULL);

		std::string cp1251; cp1251.resize(len - 1);
		WideCharToMultiByte(1251, 0, wstr.data(), -1, &cp1251[0], len, NULL, NULL);

		return cp1251;
	}
	
	void setProperEncoding() {
		SetConsoleCP(1251);
		SetConsoleOutputCP(1251);
	}

#else
	std::string cp1251ToUTF8(const std::string& str) { return str; }
	std::string utf8ToCp1251(const std::string& str) { return str; }
	void setProperEncoding() { setlocale(LC_ALL, "ru_RU.UTF-8"); }
#endif

}