#pragma once
#include <Core/Buffer.hpp>
#include <Core/Context.hpp>

namespace MMPEngine::Frontend
{
	class UploadBuffer final : public Core::UploadBuffer
	{
	public:
		UploadBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		void Write(const void* src, std::size_t byteLength, std::size_t byteOffset = 0) override;
		std::shared_ptr<Buffer> GetUnderlyingBuffer() override;
	private:
		std::shared_ptr<Core::UploadBuffer> _impl;
	};


	class ReadBackBuffer final : public Core::ReadBackBuffer
	{
	public:
		ReadBackBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		void Read(void* dst, std::size_t byteLength, std::size_t byteOffset) override;
		std::shared_ptr<Buffer> GetUnderlyingBuffer() override;
	private:
		std::shared_ptr<Core::ReadBackBuffer> _impl;
	};
}
