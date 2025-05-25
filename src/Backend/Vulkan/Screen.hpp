#pragma once
#include <Core/Screen.hpp>
#include <Backend/Vulkan/Entity.hpp>
#include <Backend/Vulkan/Texture.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class Screen final : public Core::Screen
	{
	private:
		
		class BackBuffer final : public Core::ColorTargetTexture, public BaseEntity, public IColorTargetTexture
		{
		private:
			class BackBufferContext : public Core::EntityTaskContext<BackBuffer>
			{
			};

			/*class SwitchStateTaskContext final : public BackBufferContext
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
			};*/

		public:
			BackBuffer(const Settings& settings, VkSwapchainKHR swapChain, const std::shared_ptr<Wrapper::Device>& device);
			void Swap();
			//std::shared_ptr<Buffer> GetCurrentImage() const;

		private:
			std::size_t _currentImageIndex = 0;
			std::vector<VkImage> _images;
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
		std::shared_ptr<Core::BaseTask> CreateTaskToSwapBufferInternal() override;
	private:
		std::shared_ptr<BackBuffer> _backBuffer;
		VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
		VkSurfaceKHR _surface = VK_NULL_HANDLE;
		std::shared_ptr<Wrapper::Device> _device;
		std::shared_ptr<Wrapper::Instance> _instance;
	};
}