#pragma once

#ifdef _WIN32
#include <ciso646> // and or not keywords
#endif

#include <unordered_map>
#include <vector>
#include <string>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using uint = unsigned int;

namespace rae
{

//namespace scene
//{
	using Id = int;
	const Id InvalidId = 0;
//}

namespace asset
{
	using Id = int;
	const Id InvalidId = 0;
}

namespace ui
{
	using Id = int;
	const Id InvalidId = 0;
}

using Color = glm::vec4;
using Color3 = glm::vec3;

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;

using qua = glm::quat;
using mat4 = glm::mat4;
using mat3 = glm::mat3;

// A custom pivot can be defined, and it will affect how we interpret the origin of the current entity.
using Pivot = glm::vec3;

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
template<class T> using UniquePtr = std::unique_ptr<T>;

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

struct Colors
{
	static Color red;
	static Color green;
	static Color blue;
	static Color yellow;
	static Color magenta;
	static Color cyan;
	static Color orange;
	static Color black;
	static Color white;
	static Color gray;
	static Color lightGray;
	static Color darkGray;
};

struct Pivots
{
	static Pivot Center;
	static Pivot TopLeft2D;
};

struct Line
{
	Array<vec3> points;
	Color color;
};

} // end namespace rae
