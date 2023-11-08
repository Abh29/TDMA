#ifndef TDMA_MATRIX_H
#define TDMA_MATRIX_H

#include <stddef.h>
#include <array>
#include <valarray>

template <typename T>
struct matrix_t {

private:
	T *_data = nullptr;
	size_t _N, _M;

public:

	using type = T;
	using pointer = T*;
	using refrence = T&;

	matrix_t() : _N(0), _M(0), _data(nullptr) {};
	matrix_t(const matrix_t& other) : _N(other._N), _M(other._M) {
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

	void init(size_t N, size_t M) {
		if (N == this->N() && M == this->M()) return;
		this->clear();
		_N = N + 2; _M = M + 2;
		_data = new T[_N * _M];
	}

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
