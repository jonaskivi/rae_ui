#pragma once

#include "rae/core/Types.hpp"

#include <iostream>
#include <functional>

namespace rae
{

const int InvalidIndex = -1;

class ITable
{
public:
	virtual void removeEntities(const Array<Id>& entities) = 0;
	virtual void defragment() = 0;
	virtual void onFrameEnd() = 0;
};

template <typename Comp>
class Table;

template <typename Comp>
void query(Table<Comp>& table, std::function<void(Id, Comp&)> process);

template <typename Comp>
void query(Table<Comp>& table, std::function<void(Id)> process);

template <typename Comp>
void query(const Table<Comp>& table, std::function<void(Id)> process);

template <typename Comp>
class Table : public ITable
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
			m_updated[m_idMap[id]] = true;

			//std::cout << "Table: Entity already exists, replacing: " << id << "\n";
			return;
		}

		// Reserve enough space in idMap to hold the id.
		//std::cout << "Creating id: " << id << " size: " << m_idMap.size() << "\n";
		int index = (int)id;
		while ((int)m_idMap.size() <= index)
		{
			m_idMap.emplace_back(InvalidIndex);
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
			m_updated[freeIndex] = true;

			//std::cout << "Table: Re-used existing entity: id: " << id << " at freeindex: " << freeIndex << "\n";
			return;
		}

		// else we need to create a new one.
		m_idMap[index] = (int)m_items.size();

		m_items.emplace_back(std::move(comp));
		m_updated.emplace_back(true);

		//std::cout << "Table: Created a completely new object: " << id << " idMap.size: " << m_idMap.size() << "\n";
	}

	void clear()
	{
		m_items.clear();
		m_idMap.clear();
		m_freeItems.clear();
		m_updated.clear(); // It is a bit wrong to clear the updated here, but we can't do anything else either.
	}

	void remove(Id id)
	{
		if (check(id))
		{
			m_freeItems.emplace_back(m_idMap[id]);
			m_idMap[id] = InvalidIndex;
		}
	}

	void removeEntities(const Array<Id>& entities) override
	{
		for (auto&& id : entities)
		{
			remove(id);
		}
	}

	void defragment() override
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

	int count() const { return m_items.size(); }

	// Check for existance of the component for the given Id
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

	const Comp& getF(Id id) const
	{
		return m_items[m_idMap[id]];
	}

	Comp& getF(Id id)
	{
		return m_items[m_idMap[id]];
	}

	// To be called on every frame
	void onFrameEnd() override
	{
		clearUpdated();
	}

	// The updated flags should be cleared at the end of each frame
	void clearUpdated()
	{
		for (auto&& value : m_updated)
		{
			value = false;
		}
	}

	bool isUpdated(Id id) const
	{
		if (check(id))
			return m_updated[m_idMap[id]];
		return false;
	}

	bool isUpdatedF(Id id) const
	{	
		return m_updated[m_idMap[id]];
	}

	friend void query<Comp>(Table<Comp>& table, std::function<void(Id, Comp&)> process);
	friend void query<Comp>(Table<Comp>& table, std::function<void(Id)> process);
	friend void query<Comp>(const Table<Comp>& table, std::function<void(Id)> process);

protected:

	Comp m_empty;
	Array<Comp> m_items; // Size is only the size of required number of components
	Array<int> m_idMap; // Size is the required size of Ids, so it will contain all the Ids in the World.
	Array<int> m_freeItems;

	Array<bool_t> m_updated; // Size is the same as m_items, so only required number of components.
};

template <typename Comp>
void query(Table<Comp>& table, std::function<void(Id, Comp&)> process)
{
	for (int i = 0; i < (int)table.m_idMap.size(); ++i)
	{
		if (table.m_idMap[i] != InvalidIndex)
		{
			process((Id)i, table.getF((Id)i));
		}
	}
}

template <typename Comp>
void query(Table<Comp>& table, std::function<void(Id)> process)
{
	for (int i = 0; i < (int)table.m_idMap.size(); ++i)
	{
		if (table.m_idMap[i] != InvalidIndex)
		{
			process((Id)i);
		}
	}
}

template <typename Comp>
void query(const Table<Comp>& table, std::function<void(Id)> process)
{
	for (int i = 0; i < (int)table.m_idMap.size(); ++i)
	{
		if (table.m_idMap[i] != InvalidIndex)
		{
			process((Id)i);
		}
	}
}

};
