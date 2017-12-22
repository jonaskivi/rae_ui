#include "rae/ui/Button.hpp"

#include <iostream>

using namespace rae;

Command::Command(std::function<void()> handler)
{
	connectExecuteEventHandler(handler);
}

void Command::execute() const
{
	std::cout << "Going to execute command... " << std::endl;
	emitExecuteEvent();
}

void Command::connectExecuteEventHandler(std::function<void()> handler)
{
	m_executeEvent.push_back(handler);
}

void Button::execute() const
{
	std::cout << "Going to execute button: " << m_text << std::endl;
	emitExecuteEvent();
}

void Button::connectExecuteEventHandler(std::function<void(const Button&)> handler)
{
	m_executeEvent.push_back(handler);
}