#pragma once

#include "rae/core/Types.hpp"

//#include <iostream>

namespace rae
{

const int InvalidIndex = -1;

template <typename Comp>
class Table
{
public:
	Table(int reserveSize = 10)
	{
		m_idMap2.reserve(reserveSize);
		m_items.reserve(reserveSize);
	}

	void reserve(int reserveSize)
	{
		m_idMap2.reserve(reserveSize);
		m_items.reserve(reserveSize);
	}

	int size()
	{
		return m_items.size();
	}

	void create(Id id, Comp&& comp)
	{
		if (check(id))
		{
			m_items[m_idMap2[id]] = std::move(comp);
			return;
		}

		// Find next free place
		if (m_freeItems.size() > 0)
		{
			int freeIndex = m_freeItems.back();
			m_freeItems.pop_back();
			m_idMap2[id] = freeIndex;
			m_items[freeIndex] = std::move(comp);
			return;
		}

		// else we need to create a new one.
		//std::cout << "Creating id: " << id << " size: " << m_idMap2.size() << "\n";
		int index = (int)id;
		while ((int)m_idMap2.size() <= index)
		{
			m_idMap2.push_back(InvalidIndex);
			//std::cout << "Created: " << m_idMap2.size() << "\n";
		}

		//std::cout << "Size after: " << m_idMap2.size() <<"\n";

		m_idMap2[index] = (int)m_items.size();

		//m_idMap.emplace(id, m_items.size());
		m_items.emplace_back(std::move(comp));
	}

	void remove(Id id)
	{
		if (check(id))
		{
			m_freeItems.emplace_back(m_idMap2[id]);
			m_idMap2[id] = InvalidIndex;
		}
	}

	const Array<Comp>& items() const { return m_items; }
	Array<Comp>& items() { return m_items; }

	bool check(Id id) const
	{
		//if (m_idMap.find(id) != m_idMap.end())
		int index = (int)id;
		if (index < m_idMap2.size() && m_idMap2[index] != InvalidIndex)
			return true;
		return false;
	}

	const Comp& get(Id id) const
	{
		if (check(id))
			return m_items[m_idMap2[id]];
		return m_empty;
	}

	Comp& get(Id id)
	{
		if (check(id))
			return m_items[m_idMap2[id]];
		return m_empty;
	}

	// JONDE possibly temp:
	/*
	Comp* get(Id id)
	{
		if (check(id))
			return &m_items[m_idMap2[id]];
		return nullptr;
	}
	*/
protected:

	Comp m_empty;
	Array<Comp> m_items;
	Array<int> m_idMap2;
	Array<int> m_freeItems;
	
};

};
