#pragma once

#include "loguru/loguru.hpp"

#include "rae/core/Property.hpp"
#include "rae/core/Types.hpp"
#include "rae/entity/Table.hpp"

struct NVGcontext;

namespace rae
{

enum class UpdateStatus
{
	NotChanged,
	Changed,
	Disabled,
};

class Camera;
class Scene;
class UIScene;
class Window;
class RenderSystem;

class ISystem
{
public:
	ISystem()
	{
	}

	ISystem(const String& name) :
		m_name(name)
	{
	}

	virtual String name() const { return m_name; }

	virtual UpdateStatus update() { return UpdateStatus::NotChanged; }
	// Sometimes you need to do cleanup every frame, even when the system is disabled.
	virtual void updateWhenDisabled() {}
	virtual void prepareRender3D(Scene& scene) {}
	virtual void render3D(const Scene& scene, const Window& window, RenderSystem& renderSystem) {}
	virtual void render2D(UIScene& uiScene, NVGcontext* nanoVG) {} // RAE_TODO Should be const
	virtual void onFrameEnd()
	{
		for (auto&& table : m_tables)
		{
			table->onFrameEnd();
		}

		for (auto&& system : m_systems)
		{
			system->onFrameEnd();
		}
	}

	virtual void destroyEntities(const Array<Id>& entities)
	{
		for (auto&& table : m_tables)
		{
			table->removeEntities(entities);
		}
	}

	virtual void defragmentTables()
	{
		for (auto&& table : m_tables)
		{
			table->defragment();
		}
	}

	virtual void addTable(ITable& table)
	{
		m_tables.push_back(&table);
	}

	virtual void addSystem(ISystem& system)
	{
		m_systems.push_back(&system);
	}

	virtual bool toggleIsEnabled() { m_isEnabled = !m_isEnabled; return m_isEnabled; }
	virtual const Bool& isEnabled() const { return m_isEnabled; }
	virtual void setIsEnabled(bool set) { m_isEnabled = set; }

protected:

	String m_name = "System name not set";

	Array<ITable*> m_tables;
	// A system can have child systems. Like UIScene has TransformSystem etc. All systems need to be registered,
	// and all systems need to register all their tables, so that the onFrameEnd clears their updated tags.
	Array<ISystem*> m_systems;

	Bool m_isEnabled = true;
};

}
