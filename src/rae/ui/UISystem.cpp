#include "rae/ui/UISystem.hpp"
#include <iostream>

#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "rae/core/Utils.hpp"

using namespace rae;

static const int ReserveBoxes = 1000;

UISystem::UISystem(ObjectFactory& objectFactory, TransformSystem& transformSystem)
: m_objectFactory(objectFactory),
m_transformSystem(transformSystem),
m_boxes(ReserveBoxes)
{

}

bool UISystem::update(double time, double deltaTime, Array<Entity>& entities)
{
	/* JONDE REMOVE
	std::cout << "Printing out UISystem:\n";
	int i = 0;
	for (auto&& box : m_boxes.items())
	{
		std::cout << "i: " << i << " box: " << box.toString() << "\n";
		++i;
	}
	*/
	return true;
}

void UISystem::render(double time, double deltaTime, Array<Entity>& entities, NVGcontext* vg,
	int windowWidth, int windowHeight, float screenPixelRatio)
{
	//nanovg

	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	nvgBeginFrame(vg, windowWidth, windowHeight, screenPixelRatio);

		int i = 0;
		for (auto& entity : entities)
		{
			Id id = entity.id();
			if (m_transformSystem.hasTransform(id) && m_boxes.check(id))
			{
				i++;
				const Transform& transform = m_transformSystem.getTransform(id);
				const Box& box = m_boxes.get(id);

				/*
				renderWindow(vg, "Pihlaja HDR Flow " + transform.toString() + " " + std::to_string(i),
					transform.position.x,
					transform.position.y,
					box.dimensions().x,
					box.dimensions().y,
					0.2f // cornerRadius
					);
				*/
				renderButton(vg, "Export",
					transform.position.x,
					transform.position.y,
					box.dimensions().x,
					box.dimensions().y,
					0.0f // cornerRadius
					);
			}
		}


		nvgFontFace(vg, "sans");

		float vertPos = 10.0f;

		nvgFontSize(vg, 18.0f);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgFillColor(vg, nvgRGBA(128, 128, 128, 192));
		nvgText(vg, 10.0f, vertPos, "fps_here", nullptr); vertPos += 20.0f;

		nvgText(vg, 10.0f, vertPos, "HELLO WORLD. Esc to quit, R reset, F autofocus, H visualize focus, VB focus distance,"
			" NM aperture, KL bounces, G debug view, T text, U fastmode", nullptr); vertPos += 20.0f;
		nvgText(vg, 10.0f, vertPos, "Movement: Second mouse button, WASDQE, Arrows", nullptr); vertPos += 20.0f;
		nvgText(vg, 10.0f, vertPos, "Y toggle resolution", nullptr); vertPos += 20.0f;

		std::string entity_count_str = "Entities: " + std::to_string(m_objectFactory.entityCount());
		nvgText(vg, 10.0f, vertPos, entity_count_str.c_str(), nullptr); vertPos += 20.0f;

		std::string transform_count_str = "Transforms: " + std::to_string(m_transformSystem.transformCount());
		nvgText(vg, 10.0f, vertPos, transform_count_str.c_str(), nullptr); vertPos += 20.0f;

		std::string mesh_count_str = "Meshes: " + std::to_string(m_objectFactory.meshCount());
		nvgText(vg, 10.0f, vertPos, mesh_count_str.c_str(), nullptr); vertPos += 20.0f;

		std::string material_count_str = "Materials: " + std::to_string(m_objectFactory.materialCount());
		nvgText(vg, 10.0f, vertPos, material_count_str.c_str(), nullptr); vertPos += 20.0f;

		//nvgText(vg, 10.0f, vertPos, m_pickedString.c_str(), nullptr);

	nvgEndFrame(vg);
}

void UISystem::renderWindow(NVGcontext* vg, const String& title, float x, float y, float w, float h,
							float cornerRadius)
{
	//float cornerRadius = 30.0f;
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// no negative windows please:
	if(w < 30.0f) w = 30.0f;
	if(h < 30.0f) h = 30.0f;

	nvgSave(vg);
//	nvgClearState(vg);

	// Window

	headerPaint = nvgLinearGradient(vg, x,y,x,y+15, nvgRGBA(255,255,255,135), nvgRGBA(0,0,0,135));
	//JONDE headerPaint = nvgLinearGradient(vg, x,y,x,y+15, nvgRGBAf(1.0f,1.0f,1.0f, a()), nvgRGBAf(r(),g(),b(),a()));

	nvgBeginPath(vg);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	//nvgFillColor(vg, nvgRGBA(28,30,34,192));
	//	nvgFillColor(vg, nvgRGBA(0,0,0,128));
	//nvgFill(vg);

	//nvgFillColor(vg, nvgRGBA(155,155,155,255));
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);
	//JONDE nvgStrokeColor(vg, nvgRGBAf(0.2f,0.2f,0.2f,0.75f * a()));
	nvgStrokeColor(vg, nvgRGBAf(0.2f,0.2f,0.2f,0.75f));
	nvgStrokeWidth(vg, 1.0f);
	nvgStroke(vg);

	// Drop shadow
	//JONDE shadowPaint = nvgBoxGradient(vg, x,y+5, w,h, cornerRadius, 20, nvgRGBAf(0.0f,0.0f,0.0f,0.5f*a()), nvgRGBAf(0.0f,0.0f,0.0f,0.0f));
	shadowPaint = nvgBoxGradient(vg, x,y+5, w,h, cornerRadius, 20, nvgRGBAf(0.0f,0.0f,0.0f,0.5f), nvgRGBAf(0.0f,0.0f,0.0f,0.0f));
	nvgBeginPath(vg);
	nvgRect(vg, x-60,y-60, w+120,h+120);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	nvgPathWinding(vg, NVG_HOLE);
	nvgFillPaint(vg, shadowPaint);
	nvgFill(vg);
/*
	// Header
	headerPaint = nvgLinearGradient(vg, x,y,x,y+15, nvgRGBA(255,255,255,8), nvgRGBA(0,0,0,16));
	nvgBeginPath(vg);
	nvgRoundedRect(vg, x+1,y+1, w-2,30, cornerRadius-1);
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);
	nvgBeginPath(vg);
	nvgMoveTo(vg, x+0.5f, y+0.5f+30);
	nvgLineTo(vg, x+0.5f+w-1, y+0.5f+30);
	nvgStrokeColor(vg, nvgRGBA(0,0,0,32));
	nvgStroke(vg);
*/
	nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

	// Shadow
	nvgFontBlur(vg,2);
	//JONDE nvgFillColor(vg, nvgRGBAf(0.0f,0.0f,0.0f,0.5f*a()));
	nvgFillColor(vg, nvgRGBAf(0.0f,0.0f,0.0f,0.5f));
	nvgText(vg, x+w/2,y+16+1, title.c_str(), nullptr);

	// Actual title text
	nvgFontBlur(vg,0);
	//textcolor:
	//nvgFillColor(vg, nvgRGBA(220,220,220,160));
	//JONDE nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,a()));
	nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,1.0f));
	nvgText(vg, x+w/2,y+16, title.c_str(), nullptr);

	nvgRestore(vg);
}

void UISystem::renderButton(NVGcontext* vg, const String& text, float x, float y, float w, float h,
							float cornerRadius)
{
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// no negative buttons please:
	if(w < 5.0f) w = 5.0f;
	if(h < 5.0f) h = 5.0f;

	nvgSave(vg);

	headerPaint = nvgLinearGradient(vg, x,y,x,y+15, nvgRGBAf(0.1f, 0.1f, 0.1f, 1.0f), nvgRGBAf(0.1f, 0.1f, 0.1f, 1.0f));

	nvgBeginPath(vg);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);

	nvgStrokeColor(vg, nvgRGBAf(0.2f,0.2f,0.2f,0.75f));
	nvgStrokeWidth(vg, 1.0f);
	nvgStroke(vg);

	// Drop shadow
	shadowPaint = nvgBoxGradient(vg, x,y+5, w,h, cornerRadius, 20, nvgRGBAf(0.0f,0.0f,0.0f,0.5f), nvgRGBAf(0.0f,0.0f,0.0f,0.0f));
	nvgBeginPath(vg);
	nvgRect(vg, x-60,y-60, w+120,h+120);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	nvgPathWinding(vg, NVG_HOLE);
	nvgFillPaint(vg, shadowPaint);
	nvgFill(vg);

	nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

	// Text shadow
	nvgFontBlur(vg,2);
	nvgFillColor(vg, nvgRGBAf(0.0f,0.0f,0.0f,0.5f));
	nvgText(vg, x+w/2,y+16+1, text.c_str(), nullptr);

	// Actual text
	nvgFontBlur(vg,0);
	nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,1.0f));
	nvgText(vg, x+w/2,y+16, text.c_str(), nullptr);

	nvgRestore(vg);
}

void UISystem::createButton(vec3 position, vec3 extents)
{
	Entity& entity = m_objectFactory.createEmptyEntity();
	//m_transformSystem.addTransform(entity.id(), Transform(vec3(0.0f, 0.0f, 0.0f)));
	m_transformSystem.addTransform(entity.id(), Transform(position));
	//m_transformSystem.setPosition(entity.id(), position); //JONDE TEMP animation

	vec3 halfExtents = extents / 2.0f;
	addBox(entity.id(), Box(-(halfExtents), halfExtents));
}

void UISystem::addBox(Id id, Box&& box)
{
	m_boxes.create(id, std::move(box));
}

const Box& UISystem::getBox(Id id)
{
	return m_boxes.get(id);
}
