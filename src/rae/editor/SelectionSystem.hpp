#pragma once

#include "rae/core/ISystem.hpp"
#include "rae/entity/Table.hpp"

namespace rae
{

struct Selected
{
};

class TransformSystem;

class SelectionSystem : public ISystem
{
public:
	SelectionSystem(TransformSystem& transformSystem);

	void clearSelection();
	void setSelection(const Array<Id>& ids);

	void toggleSelected(Id id);
	void setSelected(Id id, bool selected);
	bool isSelected(Id id) const;

	Event<SelectionSystem&> onSelectionChanged;

	// The average position of all selected entities
	vec3 selectionPosition() const;

protected:
	TransformSystem& m_transformSystem;

	void clearSelectionInternal();

	Table<Selected> m_selected;
};

}