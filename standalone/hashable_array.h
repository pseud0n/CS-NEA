//07873 856679

#ifndef HASHABLE_ARRAY_H
#define HASHABLE_ARRAY_H

#include <memory>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/functional/hash.hpp>

using std::clog;

template <typename T>
class HashableArray {
private:
	static T* make_buffer(size_t count) {
		T *buffer = reinterpret_cast<T*>(::operator new(count * sizeof(T)));
		std::memset(buffer, 0, count * sizeof(T));
		// Zeroes out memory (BAD IDEA but works because a zeroed ExternalObject is Null)
		return buffer;
	}

	static void free_buffer(void* buffer) {
		::operator delete(buffer);
	}

	T *_buffer;
	size_t _size, _capacity;
	mutable bool _valid_hash = false;
	mutable size_t _cached_hash = 0;

	void reallocate(size_t new_capacity) {
		// golden ratio - 1.5 is close enough
		//clog << "Reallocating " << addition << "\n";
		//T *new_buffer = reinterpret_cast<T*>(::operator new(_capacity * sizeof(T)));

		T *new_buffer = make_buffer(new_capacity);
		// new and delete call constructurs and destructors respectively
		// increases size without calling unnecessary default-constructors
		if (_buffer) {
			for (size_t i = 0; i < _size; ++i)
				new_buffer[i] = std::move(_buffer[i]);	
			_clear();
			free_buffer(_buffer); // pairs with malloc
		}
		//::operator delete(reinterpret_cast<void*>(_buffer), _capacity * sizeof(T));
		_buffer = new_buffer;
		_capacity = new_capacity;
	}

	void maybe_reallocate() {
		if (_size >= _capacity) {
			_capacity = _buffer ? capacity_increaser() : capacity_init;
			reallocate(_capacity);
		}
	}

	size_t capacity_increaser() const {
		return _capacity + _capacity / 2;
	}

	void _clear() {
		for (size_t i = 0; i < _size; ++i)
			_buffer[i].~T(); // destructor for each element
	}
public:
	static inline size_t capacity_init = 4;

	HashableArray()
		: _buffer(nullptr),
		  _size(0),
		  _capacity(0),
		  _cached_hash(get_hash()) {
	}

	HashableArray(size_t size)
		: _buffer(make_buffer(size)),
		  _size(size),
		  _capacity(size) { }

	HashableArray(std::initializer_list<T> il)
		: _buffer(make_buffer(std::distance(il.begin(), il.end()))), _size(0), _capacity(std::distance(il.begin(), il.end())) {
		//clog << "HERE\n";
		for (auto& el: il) {
			_buffer[_size++] = std::move(el);
		}
	}

	template <typename U>
	HashableArray(std::initializer_list<U> il)
		: _buffer(make_buffer(std::distance(il.begin(), il.end()))),
		  _size(0),
		  _capacity(std::distance(il.begin(), il.end())) {
		//clog << "HERE\n";
		for (auto& el: il) {
			new (_buffer + _size++) T(el);
		}
	}

	HashableArray(const HashableArray<T>& array)
		: _buffer(make_buffer(array._size)),
		  _size(array._size),
		  _capacity(_size),
		  _valid_hash(array._valid_hash) {
		// Capacity should be same as size - only allocate what is necessary
		clog << "COPY CONSTRUCTION\n";
		if (_valid_hash)
			_cached_hash = array._cached_hash;
		for (size_t i = 0; i < _size; ++i) {
			_buffer[i] = array._buffer[i]; // copies
		}
	}

	HashableArray& operator =(const HashableArray<T>& array) {
		clog << "COPY ASSIGNMENT\n";
		~HashableArray<T>();
		_buffer = make_buffer(array._size);
		_size = array._size;
		_capacity = _size;
		_valid_hash = array._valid_hash;
		if (_valid_hash)
			_cached_hash = array._cached_hash;
		for (size_t i = 0; i < _size; ++i) {
			_buffer[i] = array._buffer[i]; // copies
		}
		return *this;
	}

	HashableArray(HashableArray<T>&& array) noexcept
		: _buffer(std::exchange(array._buffer, nullptr)),
		  _size(array._size), _capacity(_size),
		  _valid_hash(array._valid_hash) {
		if (_valid_hash)
			_cached_hash = array._cached_hash;
	}

	HashableArray& operator =(HashableArray<T>&& array) noexcept {
		~HashableArray<T>();
		_buffer = std::exchange(array._buffer, nullptr);
		_capacity = _size = array._size;
		_valid_hash = array._valid_hash;
		if (_valid_hash)
			_cached_hash = array._cached_hash;
		return *this;
	}

	~HashableArray() {
		if(_buffer) {
			clear(); // leaves capacity unchanged
			//::operator delete((void*)_buffer, _capacity * sizeof(T));
			free_buffer(_buffer);
		}
	}

	const T& operator [](size_t index) const {
		return _buffer[index];
		// cannot make modifications but used for access in a const function
	}

	T& operator [](size_t index) {
		invalidate_hash();
		// Non-const; modifications made (e.g. a[b] = c)
		return _buffer[index];
	}

	bool operator ==(const HashableArray<T>& other) const {
		if (_size != other._size)
			return false;
		for (size_t i = 0; i < _size; ++i) {
			if (!(_buffer[i] == other._buffer[i]))
				return false;
		}
		return true;
	}

	void operator +=(const HashableArray<T>& other) {
		size_t prev_size = _size;
		reallocate(_size + other.size()); // Move across _size elements
		_size += other.size(); // Now size is modified (avoids out-of-bounds)
		clog << "Size: " << _size << ", Capacity: " << _capacity << "\n";

		for (size_t i = 0; i < _size - prev_size; ++i) {
			clog << "i: " << i + prev_size << "\n";
			_buffer[i + prev_size] = other._buffer[i];
		}

		/*
		[1,2,3]
		[4,5,6,7]

		[1,2,3,_,_,_,_]
		[4,5,6,7]

		[1,2,3,4,5,6,7]
		[4,5,6,7]
		*/
	}

	size_t size() const {
		return _size;
	}

	size_t last() const {
		return _size - 1;
	}

	size_t capacity() const {
		return _capacity;
	}

	T& back() const {
		return _buffer[_size - 1];
	}

	[[nodiscard]] bool empty() const {
		return _size == 0;
	}

	bool valid_hash() const {
		return _valid_hash;
	}

	size_t get_hash() const {
		if (!_valid_hash) {
			clog << "Recalculating hash\n";
			_valid_hash = true;
			_cached_hash = 0;
			for (size_t i = 0; i < _size; ++i)
				_cached_hash ^= std::hash<T>()(_buffer[i]) + 0x9e3779b9 + (_cached_hash << 6) + (_cached_hash >> 2);
			// hash-combinining algorithm made by people much smarter than me
			// https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html
		}
		return _cached_hash;
	}

	void invalidate_hash() {
		_valid_hash = false;
	}

	void resize(size_t size) {
		if (size > _capacity)
			reallocate(size);
	}

	void push_back(const T& value) {
		invalidate_hash();
		//clog << "Pushing " << value << "; size = " << _size << "; capacity = " << _capacity << "\n";
		maybe_reallocate();
		_buffer[_size++] = value;
		// Works since counting starts from 0 so _buffer[_size - 1] is last
	}

	void push_back(T&& value) {
		invalidate_hash();
		maybe_reallocate();
		_buffer[_size++] = std::move(value);
		// Will copy if no move-ctor found
	}

	template <typename... Ts>
	void emplace_back(Ts&&... args) {
		invalidate_hash();
		maybe_reallocate();
		//_buffer[_size++] = T(std::forward<Ts>(args)...);
		new (_buffer + (_size++)) T(std::forward<Ts>(args)...);
	}

	template <typename... Ts>
	void emplace(size_t index, Ts&&... args) {
		invalidate_hash();
		maybe_reallocate(); // If there isn't enough space, reallocate
		++_size;
		/*
		[a,b,c,d,e,f,g]
		_size = 7
		index = 2
		Then we insert before index 2: between b,c
		[a,b,c,d,e,f,g,_]
		Allocate one extra space at the end
		[a,b,_,c,d,e,f,g]
		Starting from the end, move each element forward 1
		i = 7, decrement while i > 2 (i = 7 to 3)
		*/
		// Move each element forward one
		for (size_t i = _size - 1; i > index; --i) {
			_buffer[i] = std::move(_buffer[i - 1]);
		}
		new (_buffer + index) T(std::forward<Ts>(args)...);
		// _buffer[_size++] = T(std::forward<Ts>(args)...); requires move then delete
	}

	template <typename... Ts>
	void replace(size_t index, Ts&&... args) {
		invalidate_hash();
		_buffer[index].~T();
		new (_buffer + index) T(std::forward<Ts>(args)...);
	}

	void pop_back() {
		invalidate_hash();
		_buffer[_size - 1].~T();
		// Explicit destructor call but _buffer memory not deallocated
		--_size;
	}

	void erase(size_t index) {
		invalidate_hash();
		/*
		[a,b,c,d,e,f,g]
		_size = 7
		index = 2
		[a,b,_,d,e,f,g]
		Delete element at index
		Move each element after backwards 1
		[a,b,d,e,f,g,_]
		Destruct last object (though it should not own anything)
		Reduce size by 1
		[a,b,d,e,f,g]
		*/
		_buffer[index].~T(); // call destructor
		for (size_t i = index; i < _size; ++i)
			_buffer[i] = std::move(_buffer[i + 1]);
		_buffer[--_size].~T();
	}

	void clear() {
		invalidate_hash();
		_clear();
		_size = 0;
	}

	void reserve(size_t count) {
	//	if (count > _capacity) 
	}

	friend std::ostream& operator <<(std::ostream& stream, const HashableArray<T>& ha) {
		stream << "[";
		for (size_t i = 0; i < ha.size(); ++i) {
			if (i != 0) stream << ", ";
			stream << ha._buffer[i];
		}
		return stream << "]";
	}

	/*
	template<
		class Category,
		class T,
		class Distance = std::ptrdiff_t,
		class Pointer = T*,
		class Reference = T&
	> struct iterator;
	*/

	class Iterator {
	public:
		using Category = std::random_access_iterator_tag;
		//using Distance = std::ptrdiff_t;

		Iterator(T* pointer) : _pointer(pointer) {
		}

		T& operator *() {
			return *_pointer;
		}

		T* operator ->() {
			return _pointer;
		}

		Iterator operator ++(int) {
			// post-increment
			// returns result before increment
			Iterator temp_iterator = *this;
			++*this;
			return temp_iterator;
		}

		Iterator& operator++() {
			// pre-increment
			// increments and returns that
			++_pointer;
			return *this;
		} 

		bool operator ==(const Iterator& it) const {
			return _pointer == it._pointer;
		}

		bool operator !=(const Iterator& it) {
			return _pointer != it._pointer;
		}
	private:
		T *_pointer;
	};

	Iterator begin() const {
		return Iterator(_buffer);
	}

	Iterator end() const {
		return Iterator(_buffer + _size);
		// out of bounds
	}

	const Iterator cbegin() const {
		return Iterator(_buffer);
	}

	const Iterator cend() const {
		return Iterator(_buffer + _size);
		// out of bounds
	}

	//using iterator = Iterator;
	//using const_iterator = const Iterator;
	// Inconsistent convention for consistency with STL
};


namespace std {
  template <typename T>
  struct hash<HashableArray<T>> {
    std::size_t operator()(const HashableArray<T>& array) const {
      return array.get_hash();
    }
  };
}

#endif
