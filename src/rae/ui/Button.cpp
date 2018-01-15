#include "rae/ui/Button.hpp"

#include <iostream>

//#include "rae/core/Log.hpp"

using namespace rae;

Command::Command(std::function<void()> handler)
{
	connectExecuteEventHandler(handler);
}

void Command::execute() const
{
	//log("Going to execute command...\n");
	emitExecuteEvent();
}

void Command::connectExecuteEventHandler(std::function<void()> handler)
{
	m_executeEvent.push_back(handler);
}

void Button::execute() const
{
	//log("Going to execute button: ", m_text, "\n");
	emitExecuteEvent();
}

void Button::connectExecuteEventHandler(std::function<void(const Button&)> handler)
{
	m_executeEvent.push_back(handler);
}
