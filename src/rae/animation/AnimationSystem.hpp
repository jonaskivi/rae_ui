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

template <typename T>
struct KeyFrame
{
	KeyFrame() {}
	KeyFrame(int frame, T value) :
		frame(frame),
		value(value)
	{
	}

	int frame = 0;
	T value;
};

template <typename T>
class PropertyAnimation
{
public:
	PropertyAnimation(
		Id id,
		std::function<const T&(Id)> getFunction,
		std::function<void(Id, const T&)> setFunction) :
			m_id(id),
			m_getFunction(getFunction),
			m_setFunction(setFunction)
	{
	}

	void addKeyFrame(int frame, T value)
	{
		// Should sort these by frame.
		m_keyFrames.emplace_back(KeyFrame<T>(frame, value));
	}

	void update(float playheadFrames)
	{
		// Make this more efficient, so we don't need to go through all of them.
		for (int i = 0; i < (int)m_keyFrames.size()-1; ++i)
		{
			auto& keyFrame = m_keyFrames[i];
			auto& nextKeyFrame = m_keyFrames[i+1];

			if (playheadFrames >= float(keyFrame.frame) && playheadFrames < float(nextKeyFrame.frame))
			{
				m_setFunction(m_id,
					Animator<T>::cubicEaseInOut(
						playheadFrames - float(keyFrame.frame),
						keyFrame.value,
						nextKeyFrame.value - keyFrame.value,
						float(nextKeyFrame.frame - keyFrame.frame + 1))
				);
			}
		}
	}

protected:

	Id m_id;
	std::function<const T&(Id)>			m_getFunction;
	std::function<void(Id, const T&)>	m_setFunction;

	Array<KeyFrame<T>> m_keyFrames;
};

enum class TimelineState
{
	Pause,
	Play
};

class AnimationTimeline
{
public:
	AnimationTimeline() {}
	AnimationTimeline(int start, int end) :
		start(start),
		end(end)
	{
		rewind();
	}

	AnimationTimeline(int duration) :
		start(0),
		end(duration-1)
	{
		rewind();
	}

	void update(Time time);

	void rewind() { playheadSeconds = framesToSeconds((float)start); }

	float secondsToFrames(float seconds) { return seconds * framesPerSecond; }
	float framesToSeconds(float frames) { return frames / framesPerSecond; }

	PropertyAnimation<float>& createFloatAnimation(
		Id id,
		std::function<float(Id)> getFunction,
		std::function<void(Id, float)> setFunction);

	PropertyAnimation<vec3>& createVec3Animation(
		Id id,
		std::function<const vec3&(Id)> getFunction,
		std::function<void(Id, const vec3&)> setFunction);

	PropertyAnimation<vec4>& createVec4Animation(
		Id id,
		std::function<const vec4&(Id)> getFunction,
		std::function<void(Id, const vec4&)> setFunction);

	Array<PropertyAnimation<float>> m_floatAnimations;
	Array<PropertyAnimation<vec3>> m_vec3Animations;
	Array<PropertyAnimation<vec4>> m_vec4Animations;

	TimelineState m_timelineState = TimelineState::Play;
	// Current playhead position on the timeline. We keep it as float, as we want to animate smoothly,
	// and not according to the framesPerSecond. FPS only defines the "grid" for the keyframes.
	float playheadSeconds = 0.0f;

	// Start and end in frames. Start is usually 0, but could be moved.
	// End is included in the animation, so it is the last frame to be processed.
	// Default duration is then actually 1 frame.
	int start = 0;
	int end = 0;
	// +1 to make end inclusive.
	int duration() const { return end - start + 1; }
	bool isLooping = true;
	float framesPerSecond = 30.0f;
};

class AnimationSystem : public ISystem
{
public:
	AnimationSystem(const Time& time, TransformSystem& transformSystem);

	UpdateStatus update() override;

	AnimationTimeline& createAnimationTimeline(int start, int end);

	const Table<PositionAnimator>& positionAnimators() const { return m_positionAnimators; }
	bool hasPositionAnimator(Id id) const;
	void addPositionAnimator(Id id, const PositionAnimator& anim);
	void setPositionAnimator(Id id, const PositionAnimator& anim);
	const PositionAnimator& getPositionAnimator(Id id) const;

private:
	const Time&			m_time;
	TransformSystem&	m_transformSystem;

	// This could be a Table, but this component never shares Ids with UISystem ids.
	// So we'd probably need to make m_entitySystem into AnimationSystem, but
	// still AnimationSystem would operate also on UIIds in positionAnimators.
	// So need a way to separate clearly between different Id types for different
	// entitySystems.
	Array<UniquePtr<AnimationTimeline>> m_animationTimelines;

	Table<PositionAnimator> m_positionAnimators;
};

}
