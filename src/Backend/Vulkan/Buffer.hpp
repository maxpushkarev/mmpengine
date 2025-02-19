#pragma once
#include <Core/Buffer.hpp>
#include <Backend/Vulkan/Entity.hpp>
#include <Backend/Vulkan/Context.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class Buffer : public ResourceEntity
	{
	public:
		Buffer();
		~Buffer() override;
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) noexcept = delete;
		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&&) noexcept = delete;
	protected:
		class InitTaskContext final : public Core::EntityTaskContext<Buffer>
		{
		public:
			std::size_t byteSize = 0;
			VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
			
		};

		class InitTask final : public Task<InitTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<InitTaskContext>& context);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

	protected:
		VkBuffer _nativeBuffer = nullptr;
		std::shared_ptr<Wrapper::Device> _device;
	};

	class UploadBuffer final : public Core::UploadBuffer, public Buffer
	{
	public:
		UploadBuffer(const Settings& settings);
		std::shared_ptr<Core::ContextualTask<Core::UploadBuffer::WriteTaskContext>> CreateWriteTask(const void* src, std::size_t byteLength, std::size_t byteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	protected:
		std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
	};

	class ReadBackBuffer final : public Core::ReadBackBuffer, public Buffer
	{
	public:
		ReadBackBuffer(const Settings& settings);
		std::shared_ptr<Core::ContextualTask<Core::ReadBackBuffer::ReadTaskContext>> CreateReadTask(void* dst, std::size_t byteLength, std::size_t byteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
	protected:
		std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
	};

	class ResidentBuffer : public Core::ResidentBuffer, public Buffer
	{
	public:
		ResidentBuffer(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	protected:
		std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
	};
}