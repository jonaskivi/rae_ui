#pragma once

#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "rae/core/Types.hpp"
#include "rae/core/ISystem.hpp"
#include "rae/core/ScreenSystem.hpp"
#include "rae/visual/TransformSystem.hpp"
#include "rae_ray/RayTracer.hpp"
#include "rae/ui/UISystem.hpp"

#include "rae/entity/Table.hpp"
#include "rae/visual/Mesh.hpp"
#include "rae/visual/Material.hpp"

#include "rae/image/ImageBuffer.hpp"

struct NVGcontext;

namespace rae
{

class Time;
class EntitySystem;
class ScreenSystem;
class TransformSystem;
class CameraSystem;
class AssetSystem;
class SelectionSystem;
struct Transform;
class Material;
class Mesh;
struct Entity;
class Input;

class RenderSystem : public ISystem
{
public:
	RenderSystem(
		const Time& time,
		EntitySystem& entitySystem,
		GLFWwindow* setWindow,
		Input& input,
		ScreenSystem& screenSystem,
		TransformSystem& transformSystem,
		CameraSystem& cameraSystem,
		AssetSystem& assetSystem,
		SelectionSystem& selectionSystem,
		UISystem& uiSystem,
		RayTracer& rayTracer);
	~RenderSystem();

	String name() override { return "RenderSystem"; }

	void initNanoVG();
	void init();

	void checkErrors(const char *file, int line);

	Id createBox();
	Id createSphere();

	UpdateStatus update() override;
	void render();
	void renderPicking();
	void render2dBackground();
	void render2d();

	// RAE_TODO TEMP:
	void renderImageBuffer(NVGcontext* vg, ImageBuffer& readBuffer,
		float x, float y, float w, float h);
	ImageBuffer& getBackgroundImage() { return m_backgroundImage; }

	void renderMesh(const Transform& transform, const Material& material, const Mesh& mesh, bool isSelected);
	void renderMeshPicking(const Transform& transform, const Mesh& mesh, Id id);

	// Temp before we get keyboard Input class
	void clearImageRenderer();
	void toggleGlRenderer()
	{
		m_glRendererOn = !m_glRendererOn;
	}

	void addMeshLink(Id id, Id linkId);
	void addMaterialLink(Id id, Id linkId);

	String fpsString() { return m_fpsString; }

	void osEventResizeWindow(int width, int height);
	void osEventResizeWindowPixels(int width, int height);

protected:

	// basic shader

	GLuint shaderID;

	GLuint modelViewMatrixUni;
	GLuint viewMatrixUni;
	GLuint modelMatrixUni;
	GLuint lightPositionUni;
	GLuint tempBlendColorUni;
	GLuint textureUni;

	// picking shader

	GLuint pickingShaderID;
	GLuint pickingModelViewMatrixUni;
	GLuint entityUni;

protected:
	GLFWwindow* m_window;

	NVGcontext* m_nanoVG = nullptr;
	
	// dependencies
	const Time&			m_time;
	EntitySystem&		m_entitySystem;
	Input&				m_input;
	UISystem&			m_uiSystem;
	ScreenSystem&		m_screenSystem;
	TransformSystem&	m_transformSystem;
	CameraSystem&		m_cameraSystem;
	AssetSystem&		m_assetSystem;
	SelectionSystem&	m_selectionSystem;
	RayTracer&			m_rayTracer;

	Transform*		debugTransform = nullptr;
	Transform*		debugTransform2 = nullptr;

	int		m_nroFrames = 0;
	double	m_fpsTimer = 0.0;
	String	m_fpsString = "fps:";

	bool	m_glRendererOn = false;

	ImageBuffer			m_backgroundImage;

	Table<Id>			m_meshLinks;
	Table<Id>			m_materialLinks;
};

}
