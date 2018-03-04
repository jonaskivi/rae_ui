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

	bool isSelection() const;

	void clearSelection();
	void setSelection(const Array<Id>& ids);

	void toggleSelected(Id id);
	void setSelected(Id id, bool selected);
	bool isSelected(Id id) const;

	Event<SelectionSystem&> onSelectionChanged;

	// The average position of all selected entities
	vec3 selectionPosition() const;

	void clearPixelClicked() { m_pixelClickedId = InvalidId; }
	void setPixelClicked(Id id) { m_pixelClickedId = id; }

	// RAE_TODO actually hovered instead of clicked
	Id hovered() { return m_pixelClickedId; }

	void translateSelected(vec3 delta);

protected:
	TransformSystem& m_transformSystem;

	void clearSelectionInternal();

	Table<Selected> m_selected;
	Id m_pixelClickedId = InvalidId;
};

}
