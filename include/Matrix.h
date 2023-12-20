#ifndef TDMA_MATRIX_H
#define TDMA_MATRIX_H

#include <stddef.h>
#include <array>
#include <valarray>

template <typename T>
struct matrix_t {

private:
	size_t _N, _M;
	T *_data = nullptr;

public:

	using type = T;
	using pointer = T*;
	using refrence = T&;

	matrix_t() : _N(0), _M(0), _data(nullptr) {};
	matrix_t(const matrix_t& other) : _N(other._N), _M(other._M), _data(nullptr) {
		init(other.N(), other.M());
		std::memcpy(_data, other._data, _N * _M * sizeof(T));
	}
	~matrix_t() {clear();}
	matrix_t& operator= (const matrix_t& other) {
		if (this == &other) return *this;
		if (other.N() * other.M() != N() * M()) {
			clear();
			init(other.N(), other.M());
		}
		std::memcpy(_data, other._data, _N * _M * sizeof(T));
		return *this;
	}

	void swap(matrix_t& other) {
		size_t t = _N;
		_N = other._N;
		other._N = t;

		t = _M;
		_M = other._M;
		other._M = t;

		T *p = _data;
		_data = other._data;
		other._data = p;
	}
	void init(size_t N, size_t M) {
		if (N == this->N() && M == this->M() && _data) return;
		this->clear();
		_N = N + 2; _M = M + 2;
		_data = new T[_N * _M];
	}
	pointer data() const {return _data;}
	void clear() {
		if (_data == nullptr) return;
		delete[] _data;
		_N = _M = 0;
		_data = nullptr;
	}

	pointer operator[] (size_t i) const {return _data + (i * _M);}

	size_t N() const {return _N - 2;}
	size_t M() const {return _M - 2;}
};

#endif //TDMA_MATRIX_H
