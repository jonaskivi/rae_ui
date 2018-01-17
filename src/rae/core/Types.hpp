#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

using uint = unsigned int;

namespace rae
{

using Id = int;
const Id InvalidId = 0;

using Colour = glm::vec4;

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;

// A boolean type only to be used to counter problems with std::vector<bool>.
// So always use rae::Array<bool_t> instead.
struct bool_t
{
	bool_t(){}
	bool_t(bool value) :
		value(value)
	{
	}

	operator bool&() { return value; }
	operator bool() const { return value; }

	bool value = false;
};

template < class T, class Allocator = std::allocator<T> >
using Array = std::vector<T, Allocator>;
using String = std::string;

template<
	class Key,
	class T,
	class Hash = std::hash<Key>,
	class KeyEqual = std::equal_to<Key>,
	class Allocator = std::allocator< std::pair<const Key, T> > >
using Map = std::unordered_map<Key, T, Hash, KeyEqual, Allocator>;

template<
	class Key,
	class T,
	class Hash = std::hash<Key>,
	class KeyEqual = std::equal_to<Key>,
	class Allocator = std::allocator< std::pair<const Key, T> > >
bool check(const Map<Key, T, Hash, KeyEqual, Allocator>& map, Key key)
{
	return map.count(key) > 0;
}

} // end namespace rae
