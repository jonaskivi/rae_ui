#pragma once

#include <array>

#include "rae/core/Types.hpp"
#include "rae/visual/Axis.hpp"
#include "rae/scene/Transform.hpp"

namespace rae
{

// Action Center in Modo.
enum class GizmoPivot
{
	Auto,
	Center,
	First,
	Last,
	Workplane,
	Count
};
String gizmoPivotToString(GizmoPivot value);

enum class GizmoAxis
{
	World,
	Local,
	Workplane,
	Screen,
	Count
};
String gizmoAxisToString(GizmoAxis value);

class IGizmo
{
public:
	const vec3& position() const { return m_position; }
	void setPosition(const vec3& position)
	{
		if (!isActive())
		{
			m_originalPosition = position;
		}
		m_position = position;
	}
	void setRotation(const qua& rotation)
	{
		if (!isActive())
		{
			m_originalRotation = rotation;
		}
		m_rotation = rotation;
	}
	void addToRotation(const qua& deltaRotation) { m_rotation = m_rotation * deltaRotation; }

	bool isVisible() const { return m_visible; }
	void show() { m_visible = true; }
	void hide() { m_visible = false; }

	bool isHovered() const
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			if (m_axisHovers[i])
				return true;
		}
		return false;
	}

	void setHovered(Axis axis) { m_axisHovers[int(axis)] = true; }

	void clearHovers()
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			m_axisHovers[i] = false;
		}
	}

	bool isActive() const
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			if (m_axisActives[i])
				return true;
		}
		return false;
	}

	bool isActiveAxis(Axis axis) const { return m_axisActives[int(axis)]; }

	Axis getActiveAxis() const
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			if (m_axisActives[i])
				return (Axis)i;
		}
		assert(0);
		return Axis::X;
	}

	void activateHovered()
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			m_axisActives[i] = m_axisHovers[i];
		}
	}

	void deactivate()
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			m_axisActives[i] = false;
		}
	}

protected:
	bool m_visible = false;

	// Current position, even when moving. Usually visualized.
	vec3 m_position;
	// Original position before action started.
	vec3 m_originalPosition;
	// Current rotation, even when rotating. Usually visualized.
	qua m_rotation;
	// Original rotation before action started.
	qua m_originalRotation;

	std::array<Transform,	(int)Axis::Count>	m_axisTransforms;
	std::array<bool,		(int)Axis::Count>	m_axisHovers = { false, false, false };
	std::array<bool,		(int)Axis::Count>	m_axisActives = { false, false, false };
};

struct AxisHover
{
	AxisHover() {}
	AxisHover(Axis axis, float distance) :
		axis(axis),
		distance(distance)
	{
	}

	Axis axis = Axis::X;
	float distance = 0.0f;
};

}
