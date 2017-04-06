#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
using glm::vec3;

#include "Entity.hpp"

#include "rae/core/Types.hpp"
#include "rae/entity/Table.hpp"

#include "rae/ui/Box.hpp"
#include "System.hpp"
#include "ObjectFactory.hpp"
#include "TransformSystem.hpp"

struct NVGcontext;

namespace rae
{

class Input;

class UISystem : public System
{
public:
	UISystem(ObjectFactory& objectFactory, TransformSystem& transformSystem);

	bool update(double time, double deltaTime, Array<Entity>& entities) override;
	void render(double time, double deltaTime, Array<Entity>& entities, NVGcontext* vg,
				int windowWidth, int windowHeight, float screenPixelRatio);

	// Currently input is in pixels
	void renderWindow(NVGcontext* vg, const String& title, float x, float y, float w, float h,
					  float cornerRadius);
	void renderButton(NVGcontext* vg, const String& text, float x, float y, float w, float h,
					  float cornerRadius);

	void createButton(vec3 position, vec3 extents);
	void addBox(Id id, Box&& box);
	const Box& getBox(Id id);

private:
	ObjectFactory& m_objectFactory;
	TransformSystem& m_transformSystem;

	Table<Box> m_boxes;
};

}
