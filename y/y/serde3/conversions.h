/*******************************
Copyright (c) 2016-2020 Gr�goire Angerand

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
#ifndef Y_SERDE3_CONVERSIONS_H
#define Y_SERDE3_CONVERSIONS_H

#include "headers.h"
#include "result.h"

Y_TODO(Remove this (is this a GCC bug?))
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#define y_serde3_try_convert(prim)														\
	do {																				\
		if constexpr(std::is_convertible_v<prim, T>) {									\
			static constexpr auto type_hash = detail::header_type_hash<prim>();			\
			if(type_hash == type.type_hash) {											\
				t = static_cast<T>(*static_cast<const prim*>(data));					\
				return core::Ok(Success::Full);											\
			}																			\
		}																				\
	} while(false)

namespace y {
namespace serde3 {

template<typename T>
Result try_convert(T& t, detail::TypeHeader type, const void* data) {
	unused(t, type, data);

	y_serde3_try_convert(u8);
	y_serde3_try_convert(u16);
	y_serde3_try_convert(u32);
	y_serde3_try_convert(u64);

	y_serde3_try_convert(i8);
	y_serde3_try_convert(i16);
	y_serde3_try_convert(i32);
	y_serde3_try_convert(i64);

	y_serde3_try_convert(float);
	y_serde3_try_convert(double);

	return core::Ok(Success::Partial);
}

}
}

#undef y_serde3_try_convert

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif // Y_SERDE3_CONVERSIONS_H
