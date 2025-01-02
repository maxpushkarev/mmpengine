#pragma once
#include <Core/Screen.hpp>
#include <Backend/Dx12/Entity.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Screen final : public Core::Screen
	{
	private:

		class Buffer final : public Core::BaseEntity, public ResourceEntity
		{
		public:
			Buffer();
			void SetUp(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource, BaseDescriptorPool::Handle&& rtvHandle);
		private:
			BaseDescriptorPool::Handle _rtvHandle;
		};

		class BackBuffer final : public Core::ColorTargetTexture, public ResourceEntity
		{
		public:
			BackBuffer(const Settings& settings, std::vector<std::shared_ptr<Buffer>>&& buffers);
			void Swap();
			std::shared_ptr<ResourceEntity> GetCurrentBackBuffer() const;
		private:
			std::size_t _currentBackBufferIndex = 0;
			std::vector<std::shared_ptr<Buffer>> _buffers;
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
		std::shared_ptr<Core::ColorTargetTexture> GetBackBuffer() const override;
	private:
		std::shared_ptr<BackBuffer> _backBuffer;
		Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;
	};
}