#pragma once
#include <Core/Texture.hpp>

namespace MMPEngine::Core
{
	class Screen : public IInitializationTaskSource, public std::enable_shared_from_this<Screen>
	{
	private:

		class TaskContext final : public Core::TaskContext
		{
		public:
			std::shared_ptr<Screen> screen;
		};

		class StreamValidationTask : public Core::ContextualTask<TaskContext>
		{
		public:
			StreamValidationTask(const std::shared_ptr<TaskContext>& ctx);
		protected:
			void OnScheduled(const std::shared_ptr<BaseStream>& stream) override;
		};

	public:
		struct Settings final
		{
			std::uint32_t vSync = 1;
			std::uint32_t buffersCount = 2;
			bool gammaCorrection = true;
			std::optional<Vector4Float> clearColor = std::nullopt;
		};

		std::shared_ptr<BaseTask> CreateInitializationTask() override;
		virtual std::shared_ptr<BaseTask> CreatePresentationTask();
		virtual std::shared_ptr<BaseTask> CreateStartFrameTask();


		virtual std::shared_ptr<ColorTargetTexture> GetBackBuffer() const = 0;
		const Settings& GetSettings() const;
	protected:

		virtual std::shared_ptr<BaseTask> CreateInitializationTaskInternal() = 0;
		virtual std::shared_ptr<BaseTask> CreatePresentationTaskInternal() = 0;
		virtual std::shared_ptr<BaseTask> CreateStartFrameTaskInternal() = 0;

		Screen(const Settings& settings);
		Settings _settings;
	private:
		Core::BaseStream* _streamPtr;
		bool _readyForPresentation = false;
	};
}