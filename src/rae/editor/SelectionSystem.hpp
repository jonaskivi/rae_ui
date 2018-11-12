#pragma once

#include "rae/core/ISystem.hpp"
#include "rae/entity/Table.hpp"

namespace rae
{

class Box;

struct Selected
{
};

struct Hover
{
};

class TransformSystem;

class SelectionSystem : public ISystem
{
public:
	SelectionSystem(TransformSystem& transformSystem);

	bool isSelection() const;
	Array<Id> selectedIds() const;

	void clearSelection();
	void setSelection(const Array<Id>& ids);

	void toggleSelected(Id id);
	void setSelected(Id id, bool selected);
	bool isSelected(Id id) const; // Raw selected.
	bool isPartOfSelection(Id id) const; // Also including selectedByParent.

	void setHovered(Id id, bool hovered);
	bool isHovered(Id id) const;
	void clearHovers();

	Event<SelectionSystem&> onSelectionChanged;

	// The average position of all selected entities
	vec3 selectionPosition() const;
	Box selectionAABB() const;

	void clearPixelClicked() { m_pixelClickedId = InvalidId; }
	void setPixelClicked(Id id) { m_pixelClickedId = id; }

	// RAE_TODO actually hovered instead of clicked
	Id pixelHovered() { return m_pixelClickedId; }

	Id hovered() { return m_hoveredId; }

	void translateSelected(vec3 delta);

protected:
	TransformSystem& m_transformSystem;

	void clearSelectionInternal();

	Table<Selected>		m_selected;
	Table<Selected>		m_selectedByParent; // Also contains m_selected.

	// Hovers are a mess currently. Trying to decide if we need multiple hovers or only one.
	// So currently we have m_hoveredId which is the main hover, and then m_hovers is the hoveredByParent.
	Table<Hover>		m_hovers;

	Id m_hoveredId = InvalidId;
	Id m_pixelClickedId = InvalidId;
};

}
