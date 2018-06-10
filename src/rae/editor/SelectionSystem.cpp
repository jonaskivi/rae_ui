#include "rae/editor/SelectionSystem.hpp"

#include "loguru/loguru.hpp"
#include "rae/scene/TransformSystem.hpp"
#include "rae/entity/Table.hpp"

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

void SelectionSystem::clearSelection()
{
	clearSelectionInternal();
	LOG_F(INFO, "Clear selection. Selection table: %i", m_selected.count());
	onSelectionChanged.emit(*this);
}

void SelectionSystem::clearSelectionInternal()
{
	m_selected.clear();
}

void SelectionSystem::setSelection(const Array<Id>& ids)
{
	clearSelectionInternal();

	for (auto&& id : ids)
	{
		m_selected.assign(id, std::move(Selected()));
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
		LOG_F(INFO, "Deselected id: %i selection table: %i", id, m_selected.count());
	}
	else
	{
		m_selected.assign(id, std::move(Selected()));
		LOG_F(INFO, "Selected id: %i selection table: %i", id, m_selected.count());
	}

	onSelectionChanged.emit(*this);
}

void SelectionSystem::setSelected(Id id, bool selected)
{
	if (selected)
	{
		m_selected.assign(id, std::move(Selected()));
		LOG_F(INFO, "Selected id: %i selection table: %i", id, m_selected.count());
	}
	else
	{
		m_selected.remove(id);
		LOG_F(INFO, "Deselected id: %i selection table: %i", id, m_selected.count());
	}

	onSelectionChanged.emit(*this);
}

bool SelectionSystem::isSelected(Id id) const
{
	return m_selected.check(id);
}

void SelectionSystem::setHovered(Id id, bool hovered)
{
	if (hovered)
		m_hovers.assign(id, std::move(Hover()));
	else m_hovers.remove(id);
}

bool SelectionSystem::isHovered(Id id)
{
	return m_hovers.check(id);
}

void SelectionSystem::clearHovers()
{
	m_hovers.clear();
}

vec3 SelectionSystem::selectionPosition() const
{
	vec3 pos;
	query<Selected>(m_selected, [&](Id id)
	{
		pos += m_transformSystem.getPosition(id);
	});

	return vec3(pos.x / m_selected.count(), pos.y / m_selected.count(), pos.z / m_selected.count());
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
