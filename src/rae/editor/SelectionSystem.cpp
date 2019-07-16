#include "rae/editor/SelectionSystem.hpp"

#include "loguru/loguru.hpp"
#include "rae/scene/TransformSystem.hpp"
#include "rae/entity/Table.hpp"
#include "rae/visual/Box.hpp"

using namespace rae;

SelectionSystem::SelectionSystem(TransformSystem& transformSystem) :
	m_transformSystem(transformSystem)
{
	addTable(m_selected);
	addTable(m_hovers);
}

bool SelectionSystem::isSelection() const
{
	return m_selected.count() > 0;
}

Array<Id> SelectionSystem::selectedIds() const
{
	return m_selected.ids();
}

void SelectionSystem::clearSelection()
{
	clearSelectionInternal();
	LOG_F(INFO, "Clear selection. Selection table: %i", m_selected.count());
	onSelectionChanged.emit(*this);
}

void SelectionSystem::clearSelectionInternal()
{
	m_selected.clear();
	m_selectedByParent.clear();
}

void SelectionSystem::setSelection(const Array<Id>& ids)
{
	clearSelectionInternal();

	for (auto&& id : ids)
	{
		m_selected.assign(id, std::move(Selected()));
		m_transformSystem.processHierarchy(id, [this](Id id)
		{
			m_selectedByParent.assign(id, std::move(Selected()));
		});
	}

	if (ids.size() > 1)
	{
		LOG_F(INFO, "Selected multiple ids: %i and %i others.", ids[0], (int)ids.size());
	}
	else if (ids.size() == 1)
	{
		LOG_F(INFO, "Selected id: %i", ids[0]);
	}

	onSelectionChanged.emit(*this);
}

void SelectionSystem::toggleSelected(Id id)
{
	if (isSelected(id))
	{
		m_selected.remove(id);
		m_transformSystem.processHierarchy(id, [this](Id id)
		{
			m_selectedByParent.remove(id);
		});
		LOG_F(INFO, "Deselected id: %i selection table: %i", id, m_selected.count());
	}
	else
	{
		m_selected.assign(id, std::move(Selected()));
		m_transformSystem.processHierarchy(id, [this](Id id)
		{
			m_selectedByParent.assign(id, std::move(Selected()));
		});
		LOG_F(INFO, "Selected id: %i selection table: %i", id, m_selected.count());
	}

	onSelectionChanged.emit(*this);
}

void SelectionSystem::setSelected(Id id, bool selected)
{
	if (selected)
	{
		m_selected.assign(id, std::move(Selected()));
		m_transformSystem.processHierarchy(id, [this](Id id)
		{
			m_selectedByParent.assign(id, std::move(Selected()));
		});
		LOG_F(INFO, "Selected id: %i selection table: %i", id, m_selected.count());
	}
	else
	{
		m_selected.remove(id);
		m_transformSystem.processHierarchy(id, [this](Id id)
		{
			m_selectedByParent.remove(id);
		});
		LOG_F(INFO, "Deselected id: %i selection table: %i", id, m_selected.count());
	}

	onSelectionChanged.emit(*this);
}

bool SelectionSystem::isSelected(Id id) const
{
	return m_selected.check(id);
}

bool SelectionSystem::isPartOfSelection(Id id) const
{
	return m_selectedByParent.check(id);
}

void SelectionSystem::setHovered(Id id, bool hovered)
{
	if (hovered)
	{
		m_hoveredId = id;
		m_hovers.assign(id, std::move(Hover()));
	}
	else
	{
		if (id == m_hoveredId)
		{
			m_hoveredId = InvalidId;
		}

		m_hovers.remove(id);
	}
}

void SelectionSystem::setHoveredHierarchy(Id id, bool hovered)
{
	if (hovered)
	{
		m_hoveredId = id;

		m_transformSystem.processHierarchy(id, [this](Id id)
		{
			m_hovers.assign(id, std::move(Hover()));
		});
	}
	else
	{
		if (id == m_hoveredId)
		{
			m_hoveredId = InvalidId;
		}

		m_transformSystem.processHierarchy(id, [this](Id id)
		{
			m_hovers.remove(id);
		});
	}
}

bool SelectionSystem::isHovered(Id id) const
{
	if (m_hoveredId == id)
		return true;
	return m_hovers.check(id);
}

bool SelectionSystem::isAnyHovered() const
{
	return m_hovers.count() > 0;
}

void SelectionSystem::clearHovers()
{
	m_hoveredId = InvalidId;
	m_hovers.clear();
}

bool SelectionSystem::hasSelectionChanged() const
{
	return m_selected.isAnyUpdated();
}

vec3 SelectionSystem::selectionWorldPosition() const
{
	vec3 pos;
	query<Selected>(m_selected, [&](Id id)
	{
		pos += m_transformSystem.getWorldPosition(id);
	});

	return vec3(pos.x / m_selected.count(), pos.y / m_selected.count(), pos.z / m_selected.count());
}

Box SelectionSystem::selectionAABB() const
{
	Box aabb;

	query<Selected>(m_selectedByParent, [&](Id id)
	{
		if (m_transformSystem.hasBox(id))
		{
			auto box = m_transformSystem.getAABBWorldSpace(id);
			aabb.grow(box);
		}
		else if (m_transformSystem.hasWorldTransform(id))
		{
			aabb.grow(m_transformSystem.getWorldPosition(id));
		}
	});

	return aabb;
}

Box SelectionSystem::hoveredAABB() const
{
	Box aabb;

	query<Hover>(m_hovers, [&](Id id)
	{
		if (m_transformSystem.hasBox(id))
		{
			auto box = m_transformSystem.getAABBWorldSpace(id);
			aabb.grow(box);
		}
		else if (m_transformSystem.hasWorldTransform(id))
		{
			aabb.grow(m_transformSystem.getWorldPosition(id));
		}
	});

	return aabb;
}

void SelectionSystem::translateSelected(vec3 delta)
{
	Array<Id> selected;
	query<Selected>(m_selected, [&](Id id)
	{
		selected.emplace_back(id);
	});

	m_transformSystem.translate(selected, delta);
}
