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

struct DisableHovering
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
	void setHoveredHierarchy(Id id, bool hovered);
	bool isHovered(Id id) const;
	bool isAnyHovered() const;
	void clearHovers();

	void addDisableHovering(Id id) { m_disableHoverings.assign(id); }
	bool isDisableHovering(Id id) { return m_disableHoverings.check(id); }

	Event<SelectionSystem&> onSelectionChanged;
	bool hasSelectionChanged() const;

	// The average position of all selected entities
	vec3 selectionWorldPosition() const;
	Box selectionAABB() const;
	Box hoveredAABB() const;

	void clearPixelClicked() { m_pixelClickedId = InvalidId; }
	void setPixelClicked(Id id) { m_pixelClickedId = id; }

	// RAE_TODO actually hovered instead of clicked
	Id pixelHovered() const { return m_pixelClickedId; }

	Id hovered() const { return m_hoveredId; }
	Id anySelected() const { return isSelection() ? selectedIds()[0] : InvalidId; }

	void translateSelected(vec3 delta);

	const Table<Selected>& selectedByParent() const { return m_selectedByParent; }
	const Table<Hover>& hovers() const { return m_hovers; }

protected:
	TransformSystem& m_transformSystem;

	void clearSelectionInternal();

	Table<Selected>		m_selected;
	Table<Selected>		m_selectedByParent; // Also contains m_selected.

	// Hovers are a mess currently. Trying to decide if we need multiple hovers or only one.
	// So currently we have m_hoveredId which is the main hover, and then m_hovers is the hoveredByParent.
	Table<Hover>		m_hovers;

	// An entity with DisableHovering can't be selected with hovering (but can be selected by some other way).
	// Basically hover bypass, so the thing under it will hover instead.
	Table<DisableHovering> m_disableHoverings;

	Id m_hoveredId = InvalidId;
	Id m_pixelClickedId = InvalidId;
};

}
