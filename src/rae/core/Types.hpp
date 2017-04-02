#pragma once

#include <unordered_map>
#include <vector>
#include <string>

namespace rae
{

using Id = int;
using CompId = int;

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

class Exist {}; // Temporary test component for the unordered_map version (which was slow).

}//end namespace rae
