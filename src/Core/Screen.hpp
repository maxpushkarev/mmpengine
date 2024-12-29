#pragma once
#include <Core/Texture.hpp>

namespace MMPEngine::Core
{
	class Screen : public IInitializationTaskSource
	{
	public:
		struct Settings final
		{
			bool gammaCorrection = true;
		};
		virtual std::shared_ptr<BaseTask> CreateTaskToUpdate() = 0;
	protected:
		Screen(const Settings& settings);
		Settings _settings;
	};
}