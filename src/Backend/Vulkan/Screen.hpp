#pragma once
#include <Core/Screen.hpp>
#include <Backend/Vulkan/Entity.hpp>
#include <Backend/Vulkan/Texture.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class Screen final : public Core::Screen
	{
	private:
		class Image final : public Core::BaseEntity, public BaseTexture, public IColorTargetTexture
		{
		public:
			Image(VkImage image);
			Image(const Image&) = delete;
			Image(Image&&) noexcept = delete;
			Image& operator=(const Image&) = delete;
			Image& operator=(Image&&) noexcept = delete;
			~Image() override;
			std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
		};

		class BackBuffer final : public Core::ColorTargetTexture, public Vulkan::BaseTexture, public IColorTargetTexture
		{
		private:
			class BackBufferContext : public Core::EntityTaskContext<BackBuffer>
			{
			};

			class MemoryBarrierContextInternal final : public BackBufferContext
			{
			public:
				MemoryBarrierContext::Data data;
			};

			class MemoryBarrierTaskInternal final : public Task<MemoryBarrierContextInternal>
			{
			public:
				MemoryBarrierTaskInternal(const std::shared_ptr<MemoryBarrierContextInternal>& ctx);
			protected:
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			private:
				std::vector<std::shared_ptr<MemoryBarrierTask>> _internalTasks;
			};

		public:
			BackBuffer(const Settings& settings, VkSwapchainKHR swapChain, const std::shared_ptr<Wrapper::Device>& device);
			void Swap();
			std::shared_ptr<Image> GetCurrentImage() const;
			std::size_t GetCurrentImageIndex() const;
			std::shared_ptr<Core::BaseTask> CreateMemoryBarrierTask(VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkImageLayout newLayout, const VkImageSubresourceRange& subResourceRange, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) override;
		protected:
			std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
		private:
			std::size_t _currentImageIndex = 0;
			std::vector<std::shared_ptr<Image>> _images;
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

		class StartFrameTask final : public Task<ScreenTaskContext>
		{
		public:
			StartFrameTask(const std::shared_ptr<ScreenTaskContext>& ctx);
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
		Screen(const Screen&) = delete;
		Screen(Screen&&) noexcept = delete;
		Screen& operator=(const Screen&) = delete;
		Screen& operator=(Screen&&) noexcept = delete;
		~Screen() override;
		std::shared_ptr<Core::ColorTargetTexture> GetBackBuffer() const override;
	protected:
		std::shared_ptr<Core::BaseTask> CreateInitializationTaskInternal() override;
		std::shared_ptr<Core::BaseTask> CreateStartFrameTaskInternal() override;
		std::shared_ptr<Core::BaseTask> CreatePresentationTaskInternal() override;
	private:
		std::size_t GetCurrentImageIndex() const;
		std::shared_ptr<BackBuffer> _backBuffer;
		VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
		VkSurfaceKHR _surface = VK_NULL_HANDLE;
		VkSemaphore _semaphore = VK_NULL_HANDLE;
		std::shared_ptr<Wrapper::Device> _device;
		std::shared_ptr<Wrapper::Instance> _instance;
		std::uint32_t _acquireNextImageIndex = 0;
	};
}