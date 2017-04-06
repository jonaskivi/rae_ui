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
		//std::cout << "Creating id: " << id << " size: " << m_idMap2.size() << "\n";

		int index = (int)id;
		while ((int)m_idMap2.size() <= index)
		{
			m_idMap2.push_back(-1);
			//std::cout << "Created: " << m_idMap2.size() << "\n";
		}

		//std::cout << "Size after: " << m_idMap2.size() <<"\n";

		m_idMap2[index] = (int)m_items.size();

		//m_idMap.emplace(id, m_items.size());
		m_items.emplace_back(std::move(comp));
	}

	//JONDE void remove(Id id)

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

	Comp m_empty;
	Array<Comp> m_items;
	Array<int> m_idMap2;
	//Map<Id, size_t> m_idMap;
};

};
