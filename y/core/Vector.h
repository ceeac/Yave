/*******************************
Copyright (c) 2016-2017 Grégoire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/
#ifndef Y_CORE_VECTOR
#define Y_CORE_VECTOR

#include <y/utils.h>

namespace y {
namespace core {

struct DefaultVectorResizePolicy {
	static constexpr usize threshold = 4096;
	static constexpr usize minimum = 8;
	static constexpr usize step = 2048;

	usize ideal_capacity(usize size) const {
		if(!size) {
			return 0;
		}
		if(size < minimum) {
			return minimum;
		}
		if(size < threshold) {
			return 1 << (log2ui(size) + 1);
		}
		usize steps = (size - threshold) / step;
		return threshold + (steps + 1) * step;
	}

	bool shrink(usize size, usize capacity) const {
		return !size || (capacity - size) > 2 * step;
	}
};


template<typename Elem, typename ResizePolicy = DefaultVectorResizePolicy>
class Vector : ResizePolicy {

	using Data = typename std::remove_const<Elem>::type;

	public:
		using value_type = Elem;

		using iterator = Elem*;
		using const_iterator = Elem const*;

		Vector() = default;

		Vector(const Vector& other) : Vector() {
			set_min_capacity(other.size());
			for(const auto& e : other) {
				push_back(e);
			}
		}

		Vector(Vector&& other) : Vector() {
			swap(other);
		}

		Vector(std::initializer_list<Elem> l) : Vector() {
			set_min_capacity(l.size());
			for(const auto& e : l) {
				push_back(e);
			}
		}

		Vector(usize size, const Elem& elem) : Vector() {
			set_min_capacity(size);
			for(usize i = 0; i != size; i++) {
				push_back(elem);
			}
		}

		template<typename It>
		Vector(const It& beg_it, const It& end_it) : Vector() {
			push_back(beg_it, end_it);
		}

		Vector& operator=(Vector&& other) {
			swap(other);
			return *this;
		}

		Vector& operator=(const Vector& other) {
			make_empty();
			set_min_capacity(other.size());
			push_back(other.begin(), other.end());
			return *this;
		}

		void swap(Vector& v) {
			std::swap(_data, v._data);
			std::swap(_data_end, v._data_end);
			std::swap(_alloc_end, v._alloc_end);
		}

		~Vector() {
			clear();
		}

		void push_back(const value_type& elem) {
			if(_data_end == _alloc_end) {
				expend();
			}
			new(_data_end++) Data(elem);
		}

		void push_back(value_type&& elem) {
			if(_data_end == _alloc_end) {
				expend();
			}
			new(_data_end++) Data(std::move(elem));
		}

		template<typename It>
		void push_back(const It& beg_it, const It& end_it) {
			std::copy(beg_it, end_it, std::back_inserter(*this));
		}

		value_type pop() {
			--_data_end;
			Data r = std::move(*_data_end);
			_data_end->~Data();
			shrink();
			return r;
		}

		usize size() const {
			return _data_end - _data;
		}

		bool is_empty() const {
			return _data == _data_end;
		}

		usize capacity() const {
			return _alloc_end - _data;
		}

		const_iterator begin() const {
			return _data;
		}

		const_iterator end() const {
			return _data_end;
		}

		const_iterator cbegin() const {
			return _data;
		}

		const_iterator cend() const {
			return _data_end;
		}

		iterator begin() {
			return _data;
		}

		iterator end() {
			return _data_end;
		}

		const value_type& operator[](usize i) const {
			return _data[i];
		}

		value_type& operator[](usize i) {
			return _data[i];
		}

		const value_type& first() const {
			return *_data;
		}

		value_type& first() {
			return *_data;
		}

		const value_type& last() const {
			return *(_data_end - 1);
		}

		value_type& last() {
			return *(_data_end - 1);
		}

		void set_capacity(usize cap) {
			unsafe_set_capacity(size(), cap);
		}

		void set_min_capacity(usize min_cap) {
			unsafe_set_capacity(size(), this->ideal_capacity(min_cap));
		}

		void clear() {
			unsafe_set_capacity(0, 0);
		}

		void squeeze() {
			set_capacity(size());
		}

		void make_empty() {
			clear(_data, _data_end);
			_data_end = _data;
		}

		bool operator==(const Vector& v) const {
			return size() == v.size() ? std::equal(begin(), end(), v.begin(), v.end()) : false;
		}

		bool operator!=(const Vector& v) const {
			return !operator==(v);
		}


	private:
		void move_range(Data* dst, Data* src, usize n) {
			if(std::is_pod<Data>::value) {
				memmove(dst, src, sizeof(Data) * n);
			} else {
				for(; n; n--) {
					new(dst++) Data(std::move(*(src++)));
				}
			}
		}

		void clear(Data* beg, Data* en) {
			if(!std::is_pod<Data>::value) {
				while(en != beg) {
					(--en)->~Data();
				}
			}
		}

		void expend() {
			unsafe_set_capacity(size(), this->ideal_capacity(size() + 1));
		}

		void shrink() {
			usize current = size();
			usize cap = capacity();
			if(current < cap && ResizePolicy::shrink(current, cap)) {
				unsafe_set_capacity(current, ResizePolicy::ideal_capacity(current));
			}
		}

		// uses data_end !!
		void unsafe_set_capacity(usize num_to_move, usize new_cap) {
			num_to_move = num_to_move < new_cap ? num_to_move : new_cap;

			Data* new_data = new_cap ? reinterpret_cast<Data*>(new u8[new_cap * sizeof(Data)]) : nullptr;

			move_range(new_data, _data, num_to_move);
			clear(_data, _data_end);

			delete[] reinterpret_cast<u8*>(_data);
			_data = new_data;
			_data_end = _data + num_to_move;
			_alloc_end = _data + new_cap;
		}

		Owner<Data*> _data = nullptr;
		Data* _data_end = nullptr;
		Data* _alloc_end = nullptr;
};

template<typename T>
inline auto vector_with_capacity(usize cap) {
	auto vec = Vector<T>();
	vec.set_min_capacity(cap);
	return vec;
}

template<typename T>
inline auto vector(std::initializer_list<T> lst) {
	return Vector<T>(lst);
}




template<typename U, typename T>
inline Vector<U>& operator<<(Vector<U>& vec, T&& t) {
	vec.push_back(std::forward<T>(t));
	return vec;
}

template<typename U, typename T>
inline Vector<U>& operator+=(Vector<U>& vec, T&& t) {
	vec.push_back(std::forward<T>(t));
	return vec;
}

template<typename U, typename T>
inline Vector<U> operator+(Vector<U> vec, T&& t) {
	vec.push_back(std::forward<T>(t));
	return vec;
}

}
}



#endif
