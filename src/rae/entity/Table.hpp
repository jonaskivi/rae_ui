#pragma once

#include "rae/core/Types.hpp"

#include <iostream>

namespace rae
{

const int InvalidIndex = -1;

template <typename Comp>
class Table
{
public:
	Table(int reserveSize = 10)
	{
		m_idMap.reserve(reserveSize);
		m_items.reserve(reserveSize);
	}

	void reserve(int reserveSize)
	{
		m_idMap.reserve(reserveSize);
		m_items.reserve(reserveSize);
	}

	int size()
	{
		return (int)m_items.size();
	}

	void assign(Id id, Comp&& comp)
	{
		if (check(id))
		{
			m_items[m_idMap[id]] = std::move(comp);

			//std::cout << "Table: Entity already exists, replacing: " << id << "\n";
			return;
		}

		// Reserve enough space in idMap to hold the id.
		//std::cout << "Creating id: " << id << " size: " << m_idMap.size() << "\n";
		int index = (int)id;
		while ((int)m_idMap.size() <= index)
		{
			m_idMap.push_back(InvalidIndex);
			//std::cout << "Created: " << m_idMap.size() << "\n";
		}
		//std::cout << "Size after: " << m_idMap.size() <<"\n";

		// Find next free place
		if (m_freeItems.size() > 0)
		{
			int freeIndex = m_freeItems.back();
			m_freeItems.pop_back();
			m_idMap[id] = freeIndex;
			m_items[freeIndex] = std::move(comp);

			//std::cout << "Table: Re-used existing entity: id: " << id << " at freeindex: " << freeIndex << "\n";
			return;
		}

		// else we need to create a new one.
		m_idMap[index] = (int)m_items.size();

		m_items.emplace_back(std::move(comp));

		//std::cout << "Table: Created a completely new object: " << id << " idMap.size: " << m_idMap.size() << "\n";
	}

	void remove(Id id)
	{
		if (check(id))
		{
			m_freeItems.emplace_back(m_idMap[id]);
			m_idMap[id] = InvalidIndex;
		}
	}

	void removeEntities(const Array<Id>& entities)
	{
		for (auto&& id : entities)
		{
			remove(id);
		}
	}

	void defragment()
	{
		m_freeItems.clear();

		// Count valid entities
		int validEntities = 0;
		for (auto&& index : m_idMap)
		{
			if (index != InvalidIndex)
				validEntities++;
		}

		Array<Comp> newItems;
		newItems.reserve(validEntities);

		for (int i = 0; i < (int)m_idMap.size(); ++i)
		{
			int index = m_idMap[i];
			if (index != InvalidIndex)
			{
				newItems.emplace_back(std::move(m_items[index]));
				m_idMap[i] = (int)newItems.size()-1;
			}
		}

		m_items = std::move(newItems);
	}

	const Array<Comp>& items() const { return m_items; }
	Array<Comp>& items() { return m_items; }

	bool check(Id id) const
	{
		int index = (int)id;
		if (index < (int)m_idMap.size() && m_idMap[index] != InvalidIndex)
		{
			assert(m_idMap[index] < (int)m_items.size()); // "idMap index must be smaller than table items size."
			return true;
		}
		return false;
	}

	const Comp& get(Id id) const
	{
		if (check(id))
			return m_items[m_idMap[id]];
		//std::cout << "Table: invalid get: " << id << "\n";
		//assert(false);
		return m_empty;
	}

	Comp& get(Id id)
	{
		if (check(id))
			return m_items[m_idMap[id]];
		//std::cout << "Table: invalid get: " << id << "\n";
		//assert(false);
		return m_empty;
	}

protected:

	Comp m_empty;
	Array<Comp> m_items;
	Array<int> m_idMap;
	Array<int> m_freeItems;
	
};

};
