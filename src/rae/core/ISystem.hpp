#pragma once

#include "rae/core/Property.hpp"
#include "rae/core/Types.hpp"
#include "rae/entity/Table.hpp"

struct NVGcontext;

namespace rae
{

enum class UpdateStatus
{
	NotChanged,
	Changed,
	Disabled,
};

class ISystem
{
public:
	ISystem()
	{
	}

	virtual String name() { return "System name"; }

	virtual UpdateStatus update() { return UpdateStatus::NotChanged; }
	virtual void render3D() {};
	virtual void render2D(NVGcontext* nanoVG) {};
	virtual void onFrameEnd()
	{
		for (auto&& table : m_tables)
		{
			table->onFrameEnd();
		}
	}

	virtual void destroyEntities(const Array<Id>& entities)
	{
		for (auto&& table : m_tables)
		{
			table->removeEntities(entities);
		}
	}

	virtual void defragmentTables()
	{
		for (auto&& table : m_tables)
		{
			table->defragment();
		}
	}

	virtual void addTable(ITable& table)
	{
		m_tables.push_back(&table);
	}

	virtual bool toggleIsEnabled() { m_isEnabled = !m_isEnabled; return m_isEnabled; }
	virtual Bool& isEnabled() { return m_isEnabled; }
	virtual void setIsEnabled(bool set) { m_isEnabled = set; }

protected:

	Array<ITable*> m_tables;

	Bool m_isEnabled = true;
};

}
