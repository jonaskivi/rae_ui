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
	virtual void render3D(const Scene& scene, const Window& window, RenderSystem& renderSystem) {};
	virtual void render2D(UIScene& uiScene, NVGcontext* nanoVG) {}; // RAE_TODO Should be const
	virtual void onFrameEnd()
	{
		for (auto&& table : m_tables)
		{
			table->onFrameEnd();
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

	virtual bool toggleIsEnabled() { m_isEnabled = !m_isEnabled; return m_isEnabled; }
	virtual Bool& isEnabled() { return m_isEnabled; }
	virtual void setIsEnabled(bool set) { m_isEnabled = set; }

protected:

	String m_name = "System name not set";

	Array<ITable*> m_tables;

	Bool m_isEnabled = true;
};

}
