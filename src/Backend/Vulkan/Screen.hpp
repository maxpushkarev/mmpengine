#pragma once
#include <Core/Screen.hpp>
#include <Backend/Vulkan/Entity.hpp>
#include <Backend/Vulkan/Texture.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class Screen final : public Core::Screen
	{
	private:
		/*class Buffer final : public Core::BaseEntity, public ResourceEntity, public IColorTargetTexture
		{
		public:
			Buffer();
		protected:
			std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
		};

		class BackBuffer final : public Core::ColorTargetTexture, public BaseEntity, public IColorTargetTexture
		{
		private:
			class BackBufferContext : public Core::EntityTaskContext<BackBuffer>
			{
			};

			class SwitchStateTaskContext final : public BackBufferContext
			{
			};

			class SwitchStateTask final : public Task<SwitchStateTaskContext>
			{
			public:
				SwitchStateTask(const std::shared_ptr<SwitchStateTaskContext>& ctx);
			protected:
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			private:
				//std::vector<std::shared_ptr<ResourceEntity::SwitchStateTask>> _internalTasks;
			};

		public:
			BackBuffer(const Settings& settings, std::vector<std::shared_ptr<Buffer>>&& buffers);
			void Swap();
			std::shared_ptr<Buffer> GetCurrentBackBuffer() const;
			//std::shared_ptr<Core::BaseTask> CreateSwitchStateTask() override;
			//D3D12_GPU_VIRTUAL_ADDRESS GetNativeGPUAddressWithRequiredOffset() const override;
			//Microsoft::WRL::ComPtr<ID3D12Resource> GetNativeResource() const override;
			//const BaseDescriptorPool::Handle* GetRTVDescriptorHandle() const override;
			//DXGI_SAMPLE_DESC GetSampleDesc() const override;
			//DXGI_FORMAT GetRTVFormat() const override;
		private:
			std::size_t _currentBackBufferIndex = 0;
			std::vector<std::shared_ptr<Buffer>> _buffers;
		};
		*/
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

		class PresentTask final : public Task<ScreenTaskContext>
		{
		public:
			PresentTask(const std::shared_ptr<ScreenTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
		
	public:
		Screen(const Settings& settings);
		std::shared_ptr<Core::ColorTargetTexture> GetBackBuffer() const override;
	protected:
		std::shared_ptr<Core::BaseTask> CreateInitializationTaskInternal() override;
		std::shared_ptr<Core::BaseTask> CreateTaskToSwapBufferInternal() override;
	private:
		//std::shared_ptr<BackBuffer> _backBuffer;
		//Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;
	};
}