#pragma once
#include <Core/Texture.hpp>

namespace MMPEngine::Core
{
	class Screen : public IInitializationTaskSource
	{
	public:
		struct Settings final
		{
			std::uint32_t vSync = 1;
			std::uint32_t buffersCount = 2;
			bool gammaCorrection = true;
		};
		virtual std::shared_ptr<BaseTask> CreateTaskToUpdate() = 0;
		virtual std::shared_ptr<BaseTask> CreateTaskToSwapBuffer() = 0;
		const Settings& GetSettings() const;
	protected:
		Screen(const Settings& settings);
		Settings _settings;
	};
}