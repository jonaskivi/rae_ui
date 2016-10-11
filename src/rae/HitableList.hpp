#pragma once

#include <vector>
#include "Hitable.hpp"

namespace Rae
{

class Ray;
class HitRecord;

class HitableList : public Hitable
{
public:
	HitableList(){}
	HitableList(int size)
	{
		list.reserve(size);
	}
	~HitableList()
	{
		clear();
	}

	void clear()
	{
		for(size_t i = 0; i < list.size(); ++i)
		{
			delete list[i];
		}
		list.clear();
	}

	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const;

	void add(Hitable* hitable)
	{
		list.push_back(hitable);
	}

	std::vector<Hitable*> list;
};

}
