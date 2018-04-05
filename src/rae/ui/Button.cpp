#include "rae/ui/Button.hpp"

//#include "loguru/loguru.hpp"

using namespace rae;

Command::Command(std::function<void()> handler)
{
	connectExecuteEventHandler(handler);
}

void Command::execute() const
{
	//LOG_F(INFO, "Going to execute command...\n");
	emitExecuteEvent();
}

void Command::connectExecuteEventHandler(std::function<void()> handler)
{
	m_executeEvent.push_back(handler);
}

void Button::execute() const
{
	//LOG_F(INFO, "Going to execute button: %s", m_text.c_str());
	emitExecuteEvent();
}

void Button::connectExecuteEventHandler(std::function<void(const Button&)> handler)
{
	m_executeEvent.push_back(handler);
}
