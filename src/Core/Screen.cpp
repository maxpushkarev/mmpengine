#include <Core/Screen.hpp>

namespace MMPEngine::Core
{
	Screen::Screen(const Settings& settings) : _settings(settings)
	{
	}

	const Screen::Settings& Screen::GetSettings() const
	{
		return _settings;
	}
}