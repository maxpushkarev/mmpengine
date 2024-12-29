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
		virtual std::shared_ptr<TargetTexture> GetBackBuffer(std::uint32_t index) const = 0;
		std::shared_ptr<TargetTexture> GetBackBuffer() const;
		const Settings& GetSettings() const;
		virtual std::uint32_t GetCurrentBackBufferIndex() const;
	protected:
		Screen(const Settings& settings);
		Settings _settings;
		std::uint32_t _currentBackBufferIndex = 0;
	};
}