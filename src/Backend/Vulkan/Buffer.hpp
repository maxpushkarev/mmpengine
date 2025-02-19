#pragma once
#include <Core/Buffer.hpp>
#include <Backend/Vulkan/Entity.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class Buffer : public ResourceEntity
	{
	protected:
		class InitTaskContext final : public Core::EntityTaskContext<Buffer>
		{
		public:
			std::size_t byteSize = 0;
		};

		class InitTask final : public Task<InitTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<InitTaskContext>& context);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	};

	class UploadBuffer final : public Core::UploadBuffer, public Buffer
	{
	public:
		UploadBuffer(const Settings& settings);
		std::shared_ptr<Core::ContextualTask<Core::UploadBuffer::WriteTaskContext>> CreateWriteTask(const void* src, std::size_t byteLength, std::size_t byteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	};

	class ReadBackBuffer final : public Core::ReadBackBuffer, public Buffer
	{
	public:
		ReadBackBuffer(const Settings& settings);
		std::shared_ptr<Core::ContextualTask<Core::ReadBackBuffer::ReadTaskContext>> CreateReadTask(void* dst, std::size_t byteLength, std::size_t byteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
	};

	class ResidentBuffer : public Core::ResidentBuffer, public Buffer
	{
	public:
		ResidentBuffer(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	};
}