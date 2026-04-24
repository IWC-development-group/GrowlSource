#pragma once
#include <unordered_map>

template <size_t Size>
struct StringLiteral {
	char data[Size];
	constexpr StringLiteral(const char(&_data)[Size]) { std::copy_n(_data, Size, data); }
};

template <StringLiteral Name>
class Eid {
private:
	unsigned int hash;

	static constexpr unsigned int constHashDjb2(const char* input) {
		return *input ?
			static_cast<unsigned int>(*input) + 33 * constHash(input + 1) :
			5381;
	}

public:
	constexpr Eid()
		: hash(constHash(Name.data)) {
	}

	constexpr unsigned int value() const { return hash; }

	explicit operator unsigned int() const { return value; }

	bool operator==(const Eid& other) const { return hash == other.hash; }
	bool operator!=(const Eid& other) const { return hash != other.hash; }
	bool operator<(const Eid& other) const { return hash < other.hash; }
	bool operator>(const Eid& other) const { return hash > other.hash; }
	bool operator<=(const Eid& other) const { return hash <= other.hash; }
	bool operator>=(const Eid& other) const { return hash >= other.hash; }
};