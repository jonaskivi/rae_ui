#pragma once

#include "rae/core/Types.hpp"
#include <set>

namespace rae
{

// Deprecated. Use Parent and Children instead.
class Hierarchy
{
public:
	Hierarchy();

	Id parent() const { return m_parent; }
	void setParent(Id parent) { m_parent = parent; }

	void addChild(Id child) { m_children.insert(child); }
	const std::set<Id>& children() const { return m_children; }

protected:
	Id m_parent;
	std::set<Id> m_children;
};

}
