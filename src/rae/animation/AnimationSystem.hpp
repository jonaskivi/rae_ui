#pragma once

#include "rae/core/Types.hpp"
#include "rae/entity/Table.hpp"

#include "rae/core/ISystem.hpp"
#include <rae/animation/Animator.hpp>

namespace rae
{

using PositionAnimator = Animator<vec3>;
class Time;
class TransformSystem;

class AnimationSystem : public ISystem
{
public:
	AnimationSystem(const Time& time, TransformSystem& transformSystem);

	UpdateStatus update() override;

	const Table<PositionAnimator>& positionAnimators() const { return m_positionAnimators; }
	bool hasPositionAnimator(Id id) const;
	void addPositionAnimator(Id id, PositionAnimator&& anim);
	void setPositionAnimator(Id id, PositionAnimator&& anim);
	void setPositionAnimator(Id id, const PositionAnimator& anim);
	const PositionAnimator& getPositionAnimator(Id id) const;

private:

	const Time&			m_time;
	TransformSystem&	m_transformSystem;

	Table<PositionAnimator>	m_positionAnimators;
};

}
