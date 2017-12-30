#pragma once

#include <functional>

#include "rae/core/Types.hpp"

namespace rae
{

/// A command holds a function that can be executed or undone.
class Command
{
public:
	Command() {}
	Command(std::function<void()> handler);

	void execute() const;
	void connectExecuteEventHandler(std::function<void()> handler);

protected:
	void emitExecuteEvent() const { for (auto&& event : m_executeEvent) event(); }
	Array<std::function<void()>> m_executeEvent;
};

class Button
{
public:
	Button()
	: m_text("")
	{
	}

	Button(String text)
	: m_text(text)
	{
	}

	const String& text() const { return m_text; }
	void setText(const String& value) { m_text = value; }

	void execute() const;
	void connectExecuteEventHandler(std::function<void(const Button&)> handler);

protected:
	String m_text;

	void emitExecuteEvent() const { for (auto&& event : m_executeEvent) event(*this); }
	Array<std::function<void(const Button&)>> m_executeEvent;
};

}
