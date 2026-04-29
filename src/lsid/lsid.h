#pragma once
#include <stdint.h>
#include <type_traits>
#include <unordered_map>

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

	template <typename T>
	inline T constSequential(const char* input) {
		static std::unordered_map<std::string_view, T> table;
		static T id = 0;

		auto it = table.find(input);
		if (it != table.end()) return it->second;
		return (table[input] = id++);
	}

	namespace literals {
		consteval uint32_t operator"" _sid(const char* input, size_t size) {
			return lsid::constHashDjb2(input);
		}

		consteval uint32_t operator"" _sid32(const char* input, size_t size) {
			return lsid::constHashDjb2(input);
		}

		consteval uint64_t operator""_sid64(const char* input, size_t size) {
			return lsid::constHashFnv(input);
		}

		inline uint32_t operator""_seq(const char* input, size_t size) {
			return lsid::constSequential<uint32_t>(input);
		}

		inline uint32_t operator""_seq32(const char* input, size_t size) {
			return lsid::constSequential<uint32_t>(input);
		}

		inline uint64_t operator""_seq64(const char* input, size_t size) {
			return lsid::constSequential<uint64_t>(input);
		}
	}

	template <size_t Size>
	struct StringLiteral {
		char data[Size];
		constexpr StringLiteral(const char(&_data)[Size]) { std::copy_n(_data, Size, data); }
	};

	template <typename HashT, StringLiteral Name, auto HashFunc>
	class BaseSsid {
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
		constexpr BaseSsid()
			: hash(HashFunc(Name.data)) {
		}

		constexpr HashT value() const { return hash; }
		explicit operator HashT() const { return hash; }

		bool operator==(const BaseSsid& other) const { return hash == other.hash; }
		bool operator!=(const BaseSsid& other) const { return hash != other.hash; }
		bool operator<(const BaseSsid& other) const { return hash < other.hash; }
		bool operator>(const BaseSsid& other) const { return hash > other.hash; }
		bool operator<=(const BaseSsid& other) const { return hash <= other.hash; }
		bool operator>=(const BaseSsid& other) const { return hash >= other.hash; }
	};

	template <StringLiteral Name>
	class Ssid32 : public BaseSsid<uint32_t, Name, constHashDjb2> {
	public:
	};

	template <StringLiteral Name>
	class Ssid64 : public BaseSsid<uint64_t, Name, constHashFnv> {
	public:
	};

	template <StringLiteral Name>
	using Ssid = Ssid32<Name>;

	template <typename HashT, auto HashFunc>
	class BaseSid {
	private:
		HashT hash;

	public:
		consteval BaseSid(const char* input) {
			static_assert(
				std::is_same_v<
					decltype(HashFunc(input)),
					HashT
				>,
				"Contained hash type must be the same as hash function return type!"
			);

			hash = HashFunc(input);
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

	class Sid32 : public lsid::BaseSid<uint32_t, lsid::constHashDjb2> {
	public:
	};

	class Sid64 : public lsid::BaseSid<uint64_t, lsid::constHashFnv> {
	public:
	};

	using Sid = Sid32;

	template <typename HashT = uint32_t>
	class Sequential {
	private:
		HashT hash;

	public:
		Sequential(const char* input) : hash(constSequential<HashT>(input)) {}
	};
}

namespace std {
	template <typename HashT, auto HashFunc>
	struct hash<lsid::BaseSid<HashT, HashFunc>> {
		size_t operator()(const lsid::BaseSid<HashT, HashFunc>& sid) const {
			return sid.value();
		}
	};
}