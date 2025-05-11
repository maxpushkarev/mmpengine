#pragma once
#include <Core/Entity.hpp>

namespace MMPEngine::Core
{
	class Buffer : public BaseEntity
	{
	public:
		struct Settings final
		{
			std::size_t byteLength;
			std::string name = {};
		};
		virtual std::shared_ptr<BaseTask> CreateCopyToBufferTask(
			const std::shared_ptr<Buffer>& dst, 
			std::size_t byteLength, 
			std::size_t srcByteOffset, 
			std::size_t dstByteOffset
		) const = 0;
		std::shared_ptr<BaseTask> CopyToBuffer(const std::shared_ptr<Buffer>& dst) const;
		std::shared_ptr<Buffer> GetUnderlyingBuffer() const;
		virtual std::shared_ptr<Buffer> GetUnderlyingBuffer();
		const Settings& GetSettings() const;
	protected:
		const Settings& GetSettingsInternal();
		Buffer(const Settings& settings);
		Settings _settings;
	};

	class UploadBuffer : public Buffer
	{
	protected:
		UploadBuffer(const Settings& settings);
	public:
		class WriteTaskContext : public TaskContext
		{
		public:
			const void* src = nullptr;
			std::size_t byteLength = 0;
			std::size_t byteOffset = 0;
		};
		virtual std::shared_ptr<ContextualTask<WriteTaskContext>> CreateWriteTask(const void* src, std::size_t byteLength, std::size_t byteOffset = 0) = 0;
	};

	class ReadBackBuffer : public Buffer
	{
	protected:
		ReadBackBuffer(const Settings& settings);
	public:
		class ReadTaskContext : public TaskContext
		{
		public:
			void* dst = nullptr;
			std::size_t byteLength = 0;
			std::size_t byteOffset = 0;
		};
		virtual std::shared_ptr<ContextualTask<ReadTaskContext>> CreateReadTask(void* dst, std::size_t byteLength, std::size_t byteOffset = 0) = 0;
	};

	class ResidentBuffer : public Buffer
	{
	protected:
		ResidentBuffer(const Settings& settings);
	};

	class BaseUnorderedAccessBuffer : public Buffer
	{
	public:
		struct Settings final
		{
			std::size_t stride = sizeof(std::uint32_t);
			std::size_t elementsCount = 0;
			std::string name = {};
		};
		BaseUnorderedAccessBuffer(const Settings& settings);
		const Settings& GetUnorderedAccessSettings() const;
	protected:
		Settings _uaSettings;
	};

	class UnorderedAccessBuffer : public BaseUnorderedAccessBuffer
	{
	protected:
		UnorderedAccessBuffer(const Settings& settings);
	};

	class CounteredUnorderedAccessBuffer : public BaseUnorderedAccessBuffer
	{
	protected:
		CounteredUnorderedAccessBuffer(const Settings& settings);
	public:
		using CounterValueType = std::uint32_t;

		virtual std::shared_ptr<BaseTask> CreateResetCounterTask() = 0;
		virtual std::shared_ptr<BaseTask> CreateCopyCounterTask(const std::shared_ptr<Buffer>& dst,
			std::size_t dstByteOffset) = 0;

	};

	class InputAssemblerBuffer : public Buffer
	{
	public:
		struct IASettings final
		{
			const void* rawData;
		};
		struct Settings final
		{
			IASettings ia;
			Buffer::Settings base;
		};
	protected:
		InputAssemblerBuffer(const Settings& settings);
		IASettings _ia;
	};

	class VertexBuffer : public InputAssemblerBuffer
	{
	protected:
		VertexBuffer(const Settings& settings);
	};

	class IndexBuffer : public InputAssemblerBuffer
	{
	protected:
		IndexBuffer(const Settings& settings);
	};

	class BaseUniformBuffer : public Buffer
	{
	protected:
		BaseUniformBuffer(const Settings& settings);
	};

	template<class TUniformBufferData>
	class UniformBuffer: public BaseUniformBuffer
	{
	protected:
		using TData = std::decay_t<TUniformBufferData>;
		static_assert(std::is_final_v<TData>, "TData should be final");
		static_assert(std::is_pod_v<TData>, "TData should be POD");

		UniformBuffer(const Settings& settings);
	public:
		class WriteTaskContext : public TaskContext
		{
		public:
			TData data;
		};
	public:
		virtual std::shared_ptr<ContextualTask<WriteTaskContext>> CreateWriteAsyncTask(const TData& data) = 0;
	};

	template<class TUniformBufferData>
	inline UniformBuffer<TUniformBufferData>::UniformBuffer(const Settings& settings) : BaseUniformBuffer(settings)
	{
	}
}