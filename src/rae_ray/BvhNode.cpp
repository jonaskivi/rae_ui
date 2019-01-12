#include "rae_ray/BvhNode.hpp"
#include "rae_ray/HitRecord.hpp"

#include "loguru/loguru.hpp"

#include "rae/core/Random.hpp"

using namespace rae;

int g_deep = 0;

BvhNode::BvhNode(Array<Hitable*>& hitables, float time0, float time1)
{
	init(hitables, time0, time1);
}

void BvhNode::init(Array<Hitable*>& hitables, float time0, float time1)
{
	g_deep++;

	int axis = int(3 * drand48());

	// A much cleaner comparison function than in Shirley's book.
	// We use a lambda which captures the axis.
	// We can compare all three axes with just one lambda.
	auto aabbCompare = [axis](Hitable* a, Hitable* b) -> bool
	{
		Box left = a->getAabb(0,0);
		Box right = b->getAabb(0,0);
		if (left.valid() == false || right.valid() == false)
		{
			LOG_F(ERROR, "No aabb in BvhNode constructor.");
		}

		if (left.min()[axis] - right.min()[axis] < 0.0f)
			return true;
		return false;
	};

	if (hitables.size() > 1)
		std::sort(hitables.begin(), hitables.end(), aabbCompare);

	if (hitables.size() == 1)
	{
		m_left = hitables[0];
		m_right = hitables[0];
	}
	else if (hitables.size() == 2)
	{
		m_left = hitables[0];
		m_right = hitables[1];
	}
	else
	{
		const std::size_t halfSize = hitables.size() / 2;
		Array<Hitable*> splitLow(hitables.begin(), hitables.begin() + halfSize);
		Array<Hitable*> splitHigh(hitables.begin() + halfSize, hitables.end());

		m_left = new BvhNode(splitLow, time0, time1);
		m_right = new BvhNode(splitHigh, time0, time1);
	}

	Box left = m_left->getAabb(time0, time1);
	Box right = m_right->getAabb(time0, time1);
	if (left.valid() == false || right.valid() == false)
	{
		LOG_F(ERROR, "No aabb in BvhNode constructor.");
	}

	m_aabb.init(left, right);
}

bool BvhNode::hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const
{
	if (m_aabb.hit(ray, t_min, t_max))
	{
		HitRecord leftRecord, rightRecord;

		bool hitLeft = m_left->hit(ray, t_min, t_max, leftRecord);
		bool hitRight = m_right->hit(ray, t_min, t_max, rightRecord);
		if (hitLeft && hitRight)
		{
			if (leftRecord.t < rightRecord.t)
				record = leftRecord;
			else record = rightRecord;
			return true;
		}
		else if (hitLeft)
		{
			record = leftRecord;
			return true;
		}
		else if (hitRight)
		{
			record = rightRecord;
			return true;
		}
		return false;
	}
	return false;
}

Box BvhNode::getAabb(float t0, float t1) const
{
	return m_aabb;
}

void BvhNode::iterate(std::function<void(const Box&)> process) const
{
	process(m_aabb);

	if (m_left)
	{
		m_left->iterate(process);
	}

	if (m_right)
	{
		m_right->iterate(process);
	}
}
