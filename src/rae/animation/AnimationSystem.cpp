#include "rae/animation/AnimationSystem.hpp"

#include "rae/core/Time.hpp"
#include "rae/scene/TransformSystem.hpp"

using namespace rae;

//RAE_TODO Move to own file:--------------------------------------------

void AnimationTimeline::update()
{
	for (auto&& anim : m_vec3Animations)
	{
		anim.update(playhead);
	}

	playhead++;

	if (isLooping)
	{
		if (playhead > end)
		{
			playhead = start;
		}
	}
}

PropertyAnimation<vec3>& AnimationTimeline::createVec3Animation(
	Id id,
	std::function<const vec3&(Id)> getFunction,
	std::function<void(Id, const vec3&)> setFunction)
{
	m_vec3Animations.emplace_back(PropertyAnimation<vec3>(
		id, getFunction, setFunction));
	return m_vec3Animations.back();
}

//--------------------------------------------

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
		anim.update(static_cast<float>(m_time.time()));
		m_transformSystem.setLocalPosition(id, anim.value());

		//LOG_F(INFO, "anim: %i pos: %s", id, Utils::toString(anim.value()).c_str());
	});

	for (auto&& timeline : m_animationTimelines)
	{
		// RAE_TODO time as a parameter, and convert frames into time.
		timeline->update();
	}

	return UpdateStatus::NotChanged;
}

AnimationTimeline& AnimationSystem::createAnimationTimeline(int start, int end)
{
	m_animationTimelines.emplace_back(std::make_unique<AnimationTimeline>(start, end));
	return *m_animationTimelines.back();
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
