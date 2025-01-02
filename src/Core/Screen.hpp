#pragma once
#include <Core/Texture.hpp>

namespace MMPEngine::Core
{
	class Screen : public IInitializationTaskSource, public std::enable_shared_from_this<Screen>
	{
	public:
		struct Settings final
		{
			std::uint32_t vSync = 1;
			std::uint32_t buffersCount = 2;
			bool gammaCorrection = true;
			std::optional<Vector4Float> clearColor = std::nullopt;
		};
		virtual std::shared_ptr<BaseTask> CreateTaskToUpdate() = 0;
		virtual std::shared_ptr<BaseTask> CreateTaskToSwapBuffer() = 0;
		virtual std::shared_ptr<ColorTargetTexture> GetBackBuffer() const = 0;
		const Settings& GetSettings() const;
	protected:
		Screen(const Settings& settings);
		Settings _settings;
		std::uint32_t _currentBackBufferIndex = 0;
	};
}