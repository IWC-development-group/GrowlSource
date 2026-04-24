#pragma once
#include <stdint.h>
#include <type_traits>

namespace lsid {

	static constexpr uint32_t constHashDjb2(const char* input) {
		return *input ?
			static_cast<unsigned int>(*input) + 33 * constHashDjb2(input + 1) :
			5381;
	}

	static constexpr uint64_t constHashFnv(const char* input) {
		uint64_t hash = 0xcbf29ce484222325ULL;
		const uint64_t prime = 0x100000001b3ULL;

		while (*input) {
			hash ^= static_cast<uint64_t>(*input);
			hash *= prime;
			++input;
		}

		return hash;
	}

	template <size_t Size>
	struct StringLiteral {
		char data[Size];
		constexpr StringLiteral(const char(&_data)[Size]) { std::copy_n(_data, Size, data); }
	};

	template <typename HashT, StringLiteral Name, auto HashFunc>
	class BaseSid {
	private:
		static_assert(
			std::is_same_v<
				decltype(HashFunc(Name.data)),
				HashT
			>,
			"Contained hash type must be the same as hash function return type!"
		);

		HashT hash;

	public:
		constexpr BaseSid()
			: hash(HashFunc(Name.data)) {
		}

		constexpr HashT value() const { return hash; }
		explicit operator HashT() const { return hash; }

		bool operator==(const BaseSid& other) const { return hash == other.hash; }
		bool operator!=(const BaseSid& other) const { return hash != other.hash; }
		bool operator<(const BaseSid& other) const { return hash < other.hash; }
		bool operator>(const BaseSid& other) const { return hash > other.hash; }
		bool operator<=(const BaseSid& other) const { return hash <= other.hash; }
		bool operator>=(const BaseSid& other) const { return hash >= other.hash; }
	};

	template <StringLiteral Name>
	class Sid32 : public BaseSid<uint32_t, Name, constHashDjb2> {
	public:
	};

	template <StringLiteral Name>
	class Sid64 : public BaseSid<uint64_t, Name, constHashFnv> {
	public:
	};

	template <StringLiteral Name>
	using Sid = Sid32<Name>;

}