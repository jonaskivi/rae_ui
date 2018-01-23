#pragma once

#include "rae/core/ISystem.hpp"
#include "rae/entity/Table.hpp"

namespace rae
{

struct Selected
{
};

class SelectionSystem : public ISystem
{
public:
	void clearSelection();
	void setSelection(const Array<Id>& ids);

	void toggleSelected(Id id);
	void setSelected(Id id, bool selected);
	bool isSelected(Id id) const;

	Event<int> onSelectionChanged;

protected:
	void clearSelectionInternal();

	Table<Selected> m_selected;
};

}
