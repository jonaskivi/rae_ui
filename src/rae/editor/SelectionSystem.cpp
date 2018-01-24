#include "rae/editor/SelectionSystem.hpp"

#include "rae/core/Log.hpp"

using namespace rae;

void SelectionSystem::clearSelection()
{
	clearSelectionInternal();
	onSelectionChanged.emit();
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

	onSelectionChanged.emit();
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

	onSelectionChanged.emit();
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

	onSelectionChanged.emit();
}

bool SelectionSystem::isSelected(Id id) const
{
	return m_selected.check(id);
}
