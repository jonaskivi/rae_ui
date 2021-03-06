#pragma once

#include "rae/core/Types.hpp"
#include "loguru/loguru.hpp"

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
void query(const Table<Comp>& table, std::function<void(Id, const Comp&)> process);

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

	Table(Table&&) = default;

	void reserve(int reserveSize)
	{
		m_idMap.reserve(reserveSize);
		m_items.reserve(reserveSize);
	}

	// Hmm, get rid of one of these...
	int size() const { return m_count; }
	int count() const { return m_count; }

	bool empty() const { return m_count <= 0; }

	void assign(Id id)
	{
		assign(id, Comp());
	}

	void assign(Id id, Comp&& comp)
	{
		if (check(id))
		{
			m_items[m_idMap[id]] = std::move(comp);
			setUpdatedF(id);

			//LOG_F(INFO, "Table: Entity already exists, replacing: %i", id);
			return;
		}

		m_count++;

		// Reserve enough space in idMap to hold the id.
		//LOG_F(INFO, "Creating id: %i size: %i", id, (int)m_idMap.size());
		int index = (int)id;
		while ((int)m_idMap.size() <= index)
		{
			m_idMap.emplace_back(InvalidIndex);
			//LOG_F(INFO, "Created: %i", (int)m_idMap.size());
		}
		//LOG_F(INFO, "Size after: %i", (int)m_idMap.size());

		// Find next free place
		if (m_freeItems.size() > 0)
		{
			int freeIndex = m_freeItems.back();
			m_freeItems.pop_back();
			m_idMap[id] = freeIndex;
			m_items[freeIndex] = std::move(comp);
			// setUpdated free index:
			m_updated[freeIndex] = true;
			m_anyUpdated = true;

			//LOG_F(INFO, "Table: Re-used existing entity: id: %i at freeindex: %i", id, freeIndex);
			return;
		}

		// else we need to create a new one.
		m_idMap[index] = (int)m_items.size();

		m_items.emplace_back(std::move(comp));
		// setUpdated new entity
		m_updated.emplace_back(true);
		m_anyUpdated = true;

		//LOG_F(INFO, "Table: Created a completely new object: %i idMap.size: %i", id, (int)m_idMap.size());
	}

	void assign(Id id, const Comp& comp)
	{
		if (check(id))
		{
			m_items[m_idMap[id]] = std::move(comp);
			setUpdatedF(id);

			//LOG_F(INFO, "Table: Entity already exists, replacing: %i", id);
			return;
		}

		m_count++;

		// Reserve enough space in idMap to hold the id.
		//LOG_F(INFO, "Creating id: %i size: %i", id, (int)m_idMap.size());
		int index = (int)id;
		while ((int)m_idMap.size() <= index)
		{
			m_idMap.emplace_back(InvalidIndex);
			//LOG_F(INFO, "Created: %i", (int)m_idMap.size());
		}
		//LOG_F(INFO, "Size after: %i", (int)m_idMap.size());

		// Find next free place
		if (m_freeItems.size() > 0)
		{
			int freeIndex = m_freeItems.back();
			m_freeItems.pop_back();
			m_idMap[id] = freeIndex;
			m_items[freeIndex] = std::move(comp);
			// setUpdated free index:
			m_updated[freeIndex] = true;
			m_anyUpdated = true;

			//LOG_F(INFO, "Table: Re-used existing entity: id: %i at freeindex: %i", id, freeIndex);
			return;
		}

		// else we need to create a new one.
		m_idMap[index] = (int)m_items.size();

		m_items.emplace_back(std::move(comp));
		// setUpdated new entity
		m_updated.emplace_back(true);
		m_anyUpdated = true;

		//LOG_F(INFO, "Table: Created a completely new object: %i idMap.size: %i", id, (int)m_idMap.size());
	}

	void clear()
	{
		m_count = 0;
		m_items.clear();
		m_idMap.clear();
		m_freeItems.clear();
		m_updated.clear(); // It is a bit wrong to clear the updated here, but we can't do anything else either.
		m_anyUpdated = true; // And this might be unexpected or not.
	}

	// This remove is currently quite "lazy", so it doesn't actually remove the item, but just
	// marks it as free. This is also currently the reason why we need to count the items separately
	// (and also because we are using array idMap instead of a proper unordered_map which would tell
	// the count easily).
	void remove(Id id)
	{
		if (check(id))
		{
			m_freeItems.emplace_back(m_idMap[id]);
			m_idMap[id] = InvalidIndex;
			m_count--;
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

	void printInfo()
	{
		LOG_F(INFO, "m_idMap size: %i", (int)m_idMap.size());
		for (auto&& index : m_idMap)
		{
			if (index != InvalidIndex)
				LOG_F(INFO, "idMap OK: %i", (int)index);
			else LOG_F(INFO, "idMap INVALID: %i", (int)index);
		}

		LOG_F(INFO, "m_freeItems size: %i", (int)m_freeItems.size());
		for (auto&& index : m_freeItems)
		{
			if (index != InvalidIndex)
				LOG_F(INFO, "freeItem OK: %i", (int)index);
			else LOG_F(INFO, "freeItem INVALID: %i", (int)index);
		}

		LOG_F(INFO, "m_items size: %i", (int)m_items.size());
		int i = 0;
		for (auto&& item : m_items)
		{
			LOG_F(INFO, "item: %i", i);
			++i;
		}
	}

	const Array<Comp>& items() const { return m_items; }
	Array<Comp>& items() { return m_items; }

	Array<Id> ids() const
	{
		Array<Id> result;
		query<Comp>(*this, [&](Id id)
		{
			result.emplace_back(id);
		});
		return result;
	}

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
		//LOG_F(ERROR, "Table: invalid get: %i", id);
		//assert(false);
		return m_empty;
	}

	Comp& modify(Id id)
	{
		if (check(id))
			return m_items[m_idMap[id]];
		//LOG_F(ERROR, "Table: invalid get: %i", id);
		//assert(false);
		return m_empty;
	}

	const Comp& getF(Id id) const
	{
		return m_items[m_idMap[id]];
	}

	Comp& modifyF(Id id)
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
		m_anyUpdated = false;
	}

	bool isAnyUpdated() const
	{
		return m_anyUpdated;
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

	void setUpdated(Id id)
	{
		if (check(id))
		{
			m_updated[m_idMap[id]] = true;
			m_anyUpdated = true;
		}
	}

	void setUpdatedF(Id id)
	{
		m_updated[m_idMap[id]] = true;
		m_anyUpdated = true;
	}

	friend void query<Comp>(Table<Comp>& table, std::function<void(Id, Comp&)> process);
	friend void query<Comp>(Table<Comp>& table, std::function<void(Id)> process);
	friend void query<Comp>(const Table<Comp>& table, std::function<void(Id, const Comp&)> process);
	friend void query<Comp>(const Table<Comp>& table, std::function<void(Id)> process);

protected:

	Comp m_empty;
	// Because we don't properly remove items when remove is called, but we mark them as free, so we need
	// a way to count the items. Another option would be again to use std::unordered_map instead of current idMap.
	int m_count = 0;
	Array<Comp> m_items; // Size is only the size of required number of components
	// A map from an Id (which is the index [id]) to a value, which is the index in the m_items.
	// So to get an item with known Id, we do: m_items[m_idMap[id]].
	// Size is the required size of Ids, so it will contain all the Ids in the World.
	// This could be replaced with an std::unordered_map, and then we should measure if it is fast enough.
	// At least this array map takes a huge amount of wasted space on each table for all the world Ids.
	Array<int> m_idMap;
	Array<int> m_freeItems;

	bool m_anyUpdated = false;
	Array<bool_t> m_updated; // Size is the same as m_items, so only required number of components.
};

template <typename Comp>
void query(Table<Comp>& table, std::function<void(Id, Comp&)> process)
{
	for (int i = 0; i < (int)table.m_idMap.size(); ++i)
	{
		if (table.m_idMap[i] != InvalidIndex)
		{
			process((Id)i, table.modifyF((Id)i));
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
void query(const Table<Comp>& table, std::function<void(Id, const Comp&)> process)
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
