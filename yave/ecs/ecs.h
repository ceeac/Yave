/*******************************
Copyright (c) 2016-2020 Grégoire Angerand

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
#ifndef YAVE_ECS_ECS_H
#define YAVE_ECS_ECS_H

#include <y/utils/hash.h>
#include <y/utils/traits.h>

#include <typeindex>

#include "EntityId.h"

namespace yave {
namespace ecs {

class EntityWorld;

/*struct ComponentType {
	const usize index;
	bool operator==(const ComponentTypeIndex& other) const { return index == other.index; }
	bool operator!=(const ComponentTypeIndex& other) const { return index != other.index; }
};*/

struct ComponentTypeIndex {
	u64 type_hash = 0;

	bool operator==(const ComponentTypeIndex& other) const { return type_hash == other.type_hash; }
	bool operator!=(const ComponentTypeIndex& other) const { return type_hash != other.type_hash; }
};


template<typename T>
ComponentTypeIndex index_for_type() {
	static_assert(!std::is_reference_v<T>);
	using naked = remove_cvref_t<T>;
	return ComponentTypeIndex{type_hash_2<naked>()};
}

template<typename... Args>
struct EntityArchetype final {

	static constexpr usize component_count = sizeof...(Args);

	template<typename T>
	using with = EntityArchetype<T, Args...>;

	static inline auto indexes() {
		return std::array{index_for_type<Args>()...};
	}
};

template<typename... Args>
struct RequiredComponents {
	static inline constexpr auto required_components_archetype() {
		static_assert(std::is_default_constructible_v<std::tuple<Args...>>);
		return EntityArchetype<Args...>{};
	}

	// EntityWorld.h
	static inline void add_required_components(EntityWorld& world, EntityId id);

};


}
}

#endif // YAVE_ECS_ECS_H
