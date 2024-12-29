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

	std::uint32_t Screen::GetCurrentBackBufferIndex() const
	{
		return _currentBackBufferIndex;
	}

	std::shared_ptr<TargetTexture> Screen::GetBackBuffer() const
	{
		return GetBackBuffer(GetCurrentBackBufferIndex());
	}

}