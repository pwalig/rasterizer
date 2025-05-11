#pragma once
#include <algorithm>
#include "trivial_iterator.hpp"

template <typename T>
class sa_vector {
public:
	using value_type = T;
	using reference = value_type&;
	using pointer = value_type*;
	using const_reference = const T&;
	using const_pointer = const T*;
	using size_type = size_t;

private:
	pointer storage;
	size_type siz;
public:
	inline sa_vector() : storage(nullptr), siz(0) {}
	inline sa_vector(size_type Size) : storage(new T[Size]), siz(Size) {}
	inline ~sa_vector() { delete[] storage; }

	inline sa_vector(const sa_vector& other) : storage(new T[other.siz]), siz(other.siz) {
		std::copy(other.begin(), other.end(), begin());
	}
	inline sa_vector(sa_vector&& other) : storage(other.storage), siz(other.siz) {
		other.siz = 0;
		other.storage = nullptr;
	}

	inline sa_vector& operator= (const sa_vector& other) {
		if (this != &other) {
			if (storage) delete[] storage;
			storage = new T[other.siz];
			std::copy(other.begin(), other.end(), begin());
			siz = other.siz;
		}
		return *this;
	}
	inline sa_vector& operator= (sa_vector&& other) {
		if (this != &other) {
			if (storage) delete[] storage;
			storage = other.storage;
			siz = other.siz;

			other.storage = nullptr;
			other.siz = 0;
		}
		return *this;
	}

	trivial_iterator_defs(value_type, storage, siz)

	inline pointer data() { return storage; }
	inline const_pointer data() const { return storage; }

	inline size_type size() const { return siz; }

	inline reference operator[] (size_type i) { return storage[i]; }
	inline const_reference operator[] (size_type i) const { return storage[i]; }

	inline reference at(size_type i) { return storage[i]; }
	inline const_reference at(size_type i) const { return storage[i]; }

	inline reference front() { return storage[0]; }
	inline const_reference front() const { return storage[0]; }
	inline reference back() { return storage[siz-1]; }
	inline const_reference back() const { return storage[siz-1]; }

	inline bool empty() const { return siz > 0; }
	inline void reserve(size_type newCapacity) const {
		if (newCapacity > siz) throw std::bad_alloc("required capacity exeeds allocated size");
	}
};
