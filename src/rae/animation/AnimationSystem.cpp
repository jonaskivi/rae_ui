#include "rae/animation/AnimationSystem.hpp"

#include "rae/core/Time.hpp"
#include "rae/scene/TransformSystem.hpp"

using namespace rae;

AnimationSystem::AnimationSystem(
	const Time& time,
	TransformSystem& transformSystem) :
		m_time(time),
		m_transformSystem(transformSystem)
{
}

UpdateStatus AnimationSystem::update()
{
	query<PositionAnimator>(m_positionAnimators, [&](Id id, PositionAnimator& anim)
	{
		anim.update(m_time.time());
		m_transformSystem.setLocalPosition(id, anim.value());

		//LOG_F(INFO, "anim: %i pos: %s", id, Utils::toString(anim.value()).c_str());
	});

	return UpdateStatus::NotChanged;
}

bool AnimationSystem::hasPositionAnimator(Id id) const
{
	return m_positionAnimators.check(id);
}

void AnimationSystem::addPositionAnimator(Id id, PositionAnimator&& anim)
{
	m_positionAnimators.assign(id, std::move(anim));
}

void AnimationSystem::setPositionAnimator(Id id, PositionAnimator&& anim)
{
	m_positionAnimators.assign(id, std::move(anim));
}

void AnimationSystem::setPositionAnimator(Id id, const PositionAnimator& anim)
{
	m_positionAnimators.assign(id, anim);
}

const PositionAnimator& AnimationSystem::getPositionAnimator(Id id) const
{
	return m_positionAnimators.get(id);
}
