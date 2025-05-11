#pragma once
#include <iterator>

#define trivial_iterator_cmp_e(iterator_type) friend inline bool operator== (const iterator_type& lhs, const iterator_type& rhs) { return lhs.ptr == rhs.ptr; }
#define trivial_iterator_cmpne(iterator_type) friend inline bool operator!= (const iterator_type& lhs, const iterator_type& rhs) { return !(lhs == rhs); }
#define trivial_iterator_cmp_l(iterator_type) friend inline bool operator<(const iterator_type& lhs, const iterator_type& rhs) { return lhs.ptr < rhs.ptr; }
#define trivial_iterator_cmp_m(iterator_type) friend inline bool operator>(const iterator_type& lhs, const iterator_type& rhs) { return rhs < lhs; }
#define trivial_iterator_cmp_le(iterator_type) friend inline bool operator<=(const iterator_type& lhs, const iterator_type& rhs) { return !(rhs < lhs); }
#define trivial_iterator_cmp_me(iterator_type) friend inline bool operator>=(const iterator_type& lhs, const iterator_type& rhs) { return !(lhs < rhs); }

#define trivial_iterator_cmp_direct(iterator_type) \
trivial_iterator_cmp_e(iterator_type) \
trivial_iterator_cmpne(iterator_type) \

#define trivial_iterator_cmp_all(iterator_type) \
trivial_iterator_cmp_direct(iterator_type) \
trivial_iterator_cmp_l(iterator_type) \
trivial_iterator_cmp_m(iterator_type) \
trivial_iterator_cmp_le(iterator_type) \
trivial_iterator_cmp_me(iterator_type) \

template <typename T>
class trivial_random_access_iterator {
public:
	using iterator_category = std::random_access_iterator_tag;
	using difference_type   = std::ptrdiff_t;
	using value_type        = T;
	using pointer           = value_type*;
	using reference         = value_type&;
private:
	pointer ptr;

public:
	inline trivial_random_access_iterator(pointer ptr) : ptr(ptr) {}

	inline reference operator*() const { return *ptr; }
	inline pointer operator->() { return ptr; } 

	inline trivial_random_access_iterator& operator++() { ++ptr; return *this; }  
	inline trivial_random_access_iterator operator++(int) { trivial_random_access_iterator tmp = *this; ++(*this); return tmp; }
	inline trivial_random_access_iterator& operator--() { --ptr; return *this; }
	inline trivial_random_access_iterator operator--(int) { trivial_random_access_iterator tmp = *this; --(*this); return tmp; }
	inline trivial_random_access_iterator& operator+=(difference_type n) { ptr += n; return *this; }
	inline trivial_random_access_iterator& operator-=(difference_type n) { ptr -= n; return *this; }

	trivial_iterator_cmp_all(trivial_random_access_iterator)

	friend inline trivial_random_access_iterator operator+(const trivial_random_access_iterator& it, difference_type n) {
		trivial_random_access_iterator temp = it;
		temp += n;
		return temp;
	}
	friend inline trivial_random_access_iterator operator+(difference_type n, const trivial_random_access_iterator& it) { return it + n; }
	friend inline trivial_random_access_iterator operator-(const trivial_random_access_iterator& it, difference_type n) {
		trivial_random_access_iterator temp = it;
		temp -= n;
		return temp;
	}
	friend inline difference_type operator-(const trivial_random_access_iterator& lhs, const trivial_random_access_iterator& rhs) { return lhs.ptr - rhs.ptr; }
};

template <typename valueT, typename proxyT, typename sizeT = size_t>
class trivial_proxy_random_access_iterator {
public:
	using iterator_category = std::random_access_iterator_tag;
	using difference_type   = std::ptrdiff_t;
	using value_type        = proxyT;
	using pointer           = valueT*;
	//using reference         = valueT&;
private:
	pointer ptr;
	const sizeT siz;

public:
	inline trivial_proxy_random_access_iterator(pointer ptr, sizeT size) : ptr(ptr), siz(size) {}

	inline proxyT operator*() const { return proxyT(ptr, siz); }

	inline trivial_proxy_random_access_iterator& operator++() { ptr += siz; return *this; }  
	inline trivial_proxy_random_access_iterator operator++(int) { trivial_proxy_random_access_iterator tmp = *this; ++(*this); return tmp; }
	inline trivial_proxy_random_access_iterator& operator--() { ptr -= siz; return *this; }
	inline trivial_proxy_random_access_iterator operator--(int) { trivial_proxy_random_access_iterator tmp = *this; --(*this); return tmp; }
	inline trivial_proxy_random_access_iterator& operator+=(difference_type n) { ptr += n * siz; return *this; }
	inline trivial_proxy_random_access_iterator& operator-=(difference_type n) { ptr -= n * siz; return *this; }

	trivial_iterator_cmp_all(trivial_proxy_random_access_iterator)

	friend inline trivial_proxy_random_access_iterator operator+(const trivial_proxy_random_access_iterator& it, difference_type n) {
		trivial_proxy_random_access_iterator temp = it;
		temp += n;
		return temp;
	}
	friend inline trivial_proxy_random_access_iterator operator+(difference_type n, const trivial_proxy_random_access_iterator& it) { return it + n; }
	friend inline trivial_proxy_random_access_iterator operator-(const trivial_proxy_random_access_iterator& it, difference_type n) {
		trivial_proxy_random_access_iterator temp = it;
		temp -= n;
		return temp;
	}
	friend inline difference_type operator-(const trivial_proxy_random_access_iterator& lhs, const trivial_proxy_random_access_iterator& rhs) { return (lhs.ptr - rhs.ptr) / siz; }
};

#define iterator_defs(ptr, size) \
using reverse_iterator = std::reverse_iterator<iterator>; \
inline iterator begin() { return iterator(ptr); } \
inline iterator end() { return iterator(ptr + size); } \
inline reverse_iterator rbegin() { return reverse_iterator(ptr); } \
inline reverse_iterator rend() { return reverse_iterator(ptr + size); } \

#define const_iterator_defs(ptr, size) \
using const_reverse_iterator = std::reverse_iterator<const_iterator>; \
inline const_iterator begin() const { return const_iterator(ptr); } \
inline const_iterator end() const { return const_iterator(ptr + size); } \
inline const_iterator cbegin() const { return const_iterator(ptr); } \
inline const_iterator cend() const { return const_iterator(ptr + size); } \
inline const_reverse_iterator rbegin() const { return const_reverse_iterator(ptr); } \
inline const_reverse_iterator rend() const { return const_reverse_iterator(ptr + size); } \
inline const_reverse_iterator crbegin() const { return const_reverse_iterator(ptr); } \
inline const_reverse_iterator crend() const { return const_reverse_iterator(ptr + size); } \

#define trivial_iterator_defs(T, ptr, size) \
using iterator = trivial_random_access_iterator<T>; \
using const_iterator = trivial_random_access_iterator<const T>; \
iterator_defs(ptr, size) \
const_iterator_defs(ptr, size) \

#define proxy_iterator_defs(ptr, proxy_size, size) \
using reverse_iterator = std::reverse_iterator<iterator>; \
inline iterator begin() { return iterator(ptr, proxy_size); } \
inline iterator end() { return iterator(ptr + (size * proxy_size), proxy_size); } \
inline reverse_iterator rbegin() { return reverse_iterator(ptr, proxy_size); } \
inline reverse_iterator rend() { return reverse_iterator(ptr + (size * proxy_size), proxy_size); } \

#define proxy_const_iterator_defs(ptr, proxy_size, size) \
using const_reverse_iterator = std::reverse_iterator<const_iterator>; \
inline const_iterator begin() const { return const_iterator(ptr, proxy_size); } \
inline const_iterator end() const { return const_iterator(ptr + (size * proxy_size), proxy_size); } \
inline const_iterator cbegin() const { return const_iterator(ptr, proxy_size); } \
inline const_iterator cend() const { return const_iterator(ptr + (size * proxy_size), proxy_size); } \
inline const_reverse_iterator rbegin() const { return const_reverse_iterator(ptr, proxy_size); } \
inline const_reverse_iterator rend() const { return const_reverse_iterator(ptr + (size * proxy_size), proxy_size); } \
inline const_reverse_iterator crbegin() const { return const_reverse_iterator(ptr, proxy_size); } \
inline const_reverse_iterator crend() const { return const_reverse_iterator(ptr + (size * proxy_size), proxy_size); } \

#define proxy_all_iterator_defs(ptr, proxy_size, size) \
proxy_iterator_defs(ptr, proxy_size, size) \
proxy_const_iterator_defs(ptr, proxy_size, size) \
