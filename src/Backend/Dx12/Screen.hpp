#pragma once
#include <Core/Screen.hpp>
#include <Backend/Dx12/Entity.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Screen final : public Core::Screen
	{
	private:
		class BackBuffer final : public Core::TargetTexture, public ResourceEntity
		{
		public:
			BackBuffer(const std::string& name);
		};

		class ScreenTaskContext final : public Core::EntityTaskContext<Screen>
		{
		};

		class InitTask final : public Task<ScreenTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<ScreenTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class UpdateTask final : public Task<ScreenTaskContext>
		{
		public:
			UpdateTask(const std::shared_ptr<ScreenTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class PresentTask final : public Task<ScreenTaskContext>
		{
		public:
			PresentTask(const std::shared_ptr<ScreenTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

	public:
		Screen(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateTaskToUpdate() override;
		std::shared_ptr<Core::BaseTask> CreateTaskToSwapBuffer() override;
		std::shared_ptr<Core::TargetTexture> GetBackBuffer() const override;
	private:
		std::shared_ptr<BackBuffer> _backBuffer;
	};
}