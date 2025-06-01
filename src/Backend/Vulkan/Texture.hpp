#pragma once
#include <Core/Texture.hpp>
#include <Backend/Vulkan/Entity.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class ITargetTexture
	{
	public:
		ITargetTexture();
		ITargetTexture(const ITargetTexture&) = delete;
		ITargetTexture(ITargetTexture&&) noexcept = delete;
		ITargetTexture& operator=(const ITargetTexture&) = delete;
		ITargetTexture& operator=(ITargetTexture&&) noexcept = delete;
		virtual ~ITargetTexture();
	};

	class IDepthStencilTexture : public ITargetTexture
	{
	};

	class IColorTargetTexture : public ITargetTexture
	{
	};

	class BaseTexture : public ResourceEntity
	{
	public:
		virtual std::shared_ptr<Core::BaseTask> CreateMemoryBarrierTask(
			VkAccessFlags srcAccess, 
			VkAccessFlags dstAccess,
			VkImageLayout newLayout,
			const VkImageSubresourceRange& subResourceRange,
			VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		);

		class MemoryBarrierContext final : public Core::EntityTaskContext<BaseTexture>
		{
		public:
			struct Data final
			{
				VkAccessFlags srcAccess = VK_ACCESS_MEMORY_READ_BIT;
				VkAccessFlags dstAccess = VK_ACCESS_MEMORY_WRITE_BIT;
				VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				VkImageSubresourceRange subresourceRange = {};
			};
			Data data;
		};

		class MemoryBarrierTask final : public Task<MemoryBarrierContext>
		{
		public:
			MemoryBarrierTask(const std::shared_ptr<MemoryBarrierContext>& ctx);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	protected:

		static VkSampleCountFlagBits GetSampleCount(Core::TargetTexture::Settings::Antialiasing aa);

		VkImage _nativeImage = VK_NULL_HANDLE;
		VkImageView _view = VK_NULL_HANDLE;
		VkImageLayout _layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	class ResourceTexture : public BaseTexture
	{
	public:
		ResourceTexture();
		~ResourceTexture() override;
		ResourceTexture(const ResourceTexture&) = delete;
		ResourceTexture(ResourceTexture&&) noexcept = delete;
		ResourceTexture& operator=(const ResourceTexture&) = delete;
		ResourceTexture& operator=(ResourceTexture&&) noexcept = delete;
	protected:

		class TaskContext final : public Core::EntityTaskContext<ResourceTexture>
		{
		};

		class BindTask final : public Task<TaskContext>
		{
		public:
			BindTask(const std::shared_ptr<TaskContext>& context);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;

		std::shared_ptr<Wrapper::Device> _device;
	};

	class DepthStencilTargetTexture final : public Core::DepthStencilTargetTexture, public ResourceTexture, public IDepthStencilTexture
	{
	private:
		class InitTaskContext final : public Core::EntityTaskContext<DepthStencilTargetTexture>
		{
		};
		class InitTask final : public Task<InitTaskContext>
		{
		private:
			class Create final : public Task<InitTaskContext>
			{
			public:
				Create(const std::shared_ptr<InitTaskContext>& context);
				void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			};
		public:
			InitTask(const std::shared_ptr<InitTaskContext>& ctx);
		protected:
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	public:
		DepthStencilTargetTexture(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	};
}
