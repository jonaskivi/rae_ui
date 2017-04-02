#pragma once

#include <vector>
#include "Hitable.hpp"

namespace rae
{

class Ray;
class HitRecord;

class HitableList : public Hitable
{
public:
	HitableList(){}
	HitableList(int size)
	{
		m_list.reserve(size);
	}
	~HitableList()
	{
		clear();
	}

	void clear()
	{
		for(size_t i = 0; i < m_list.size(); ++i)
		{
			delete m_list[i];
		}
		m_list.clear();
	}

	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const;
	virtual Aabb getAabb(float t0, float t1) const;

	void add(Hitable* hitable)
	{
		m_list.push_back(hitable);
	}

	std::vector<Hitable*>& list() { return m_list; }

protected:
	std::vector<Hitable*> m_list;
};

}
