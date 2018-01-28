#include "rae/editor/SelectionSystem.hpp"

#include "rae/core/Log.hpp"
#include "rae/visual/TransformSystem.hpp"
#include "rae/entity/Table.hpp"

using namespace rae;

SelectionSystem::SelectionSystem(TransformSystem& transformSystem) :
	m_transformSystem(transformSystem)
{
}

void SelectionSystem::clearSelection()
{
	clearSelectionInternal();
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

	onSelectionChanged.emit(*this);
}

void SelectionSystem::toggleSelected(Id id)
{
	if (isSelected(id))
	{
		m_selected.remove(id);
		rae_log("Deselected id: ", id);
	}
	else
	{
		m_selected.assign(id, std::move(Selected()));
		rae_log("Selected id: ", id);
	}

	onSelectionChanged.emit(*this);
}

void SelectionSystem::setSelected(Id id, bool selected)
{
	if (selected)
	{
		m_selected.assign(id, std::move(Selected()));
		rae_log("Selected id: ", id);
	}
	else
	{
		m_selected.remove(id);
		rae_log("Deselected id: ", id);

	}

	onSelectionChanged.emit(*this);
}

bool SelectionSystem::isSelected(Id id) const
{
	return m_selected.check(id);
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