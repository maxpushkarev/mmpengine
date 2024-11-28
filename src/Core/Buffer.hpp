#pragma once
#include <Core/Entity.hpp>

namespace MMPEngine::Core
{
	class Buffer : public virtual BaseEntity
	{
	public:
		struct Settings final
		{
			std::size_t byteSize;
			std::string name = {};
		};
	protected:
		Buffer(const Settings& settings);
		Settings _settings;
	};

	class UploadBuffer : public Buffer
	{
	protected:
		UploadBuffer(const Settings& settings);
	public:
		virtual void Write(const void* src, std::size_t byteLength, std::size_t byteOffset = 0) = 0;
	};

	class ReadBackBuffer : public Buffer
	{
	protected:
		ReadBackBuffer(const Settings& settings);
	public:
		virtual void Read(void* dst, std::size_t byteLength, std::size_t byteOffset = 0) = 0;
	};

	class ResidentBuffer : public Buffer
	{
	protected:
		ResidentBuffer(const Settings& settings);
	};

	template<class TConstantBufferData>
	class ConstantBuffer : public Buffer, public std::enable_shared_from_this<ConstantBuffer<TConstantBufferData>>
	{
	protected:
		using TData = std::decay_t<TConstantBufferData>;
		static_assert(std::is_pod_v<TData>, "TData should be POD type");
		static_assert(std::is_final_v<TData>, "TData should be final");

		class InitializationContext final : public TaskContext
		{
		public:
			std::shared_ptr<ConstantBuffer> constantBuffer;
		};

		class InitializationTask : public TaskWithInternalContext<InitializationContext>
		{
		protected:
			InitializationTask(const std::shared_ptr<InitializationContext> taskContext);
			void Run(const std::shared_ptr<BaseStream>& stream) final;
			virtual std::unique_ptr<UploadBuffer> CreateUploadBuffer(const std::shared_ptr<BaseStream>& stream) = 0;
		};

		ConstantBuffer(std::string_view name);
		ConstantBuffer();

		virtual std::shared_ptr<InitializationTask> CreateInitializationTaskInternal() = 0;

	public:
		void Write(const TData& data);
		std::shared_ptr<BaseTask> CreateInitializationTask() final;
	private:
		std::unique_ptr<UploadBuffer> _uploadBuffer;
	};

	class InputAssemblerBuffer : public Buffer
	{
	protected:
		struct IASettings final
		{
			const void* rawData;
		};
		struct Settings final
		{
			IASettings ia;
			Buffer::Settings base;
		};
		InputAssemblerBuffer(const Settings& settings);
		IASettings _ia;
	};

	template<class TConstantBufferData>
	inline ConstantBuffer<TConstantBufferData>::ConstantBuffer(std::string_view name) : Buffer({ sizeof(TData), std::string{name}})
	{
	}

	template<class TConstantBufferData>
	inline ConstantBuffer<TConstantBufferData>::ConstantBuffer() : Buffer({ sizeof(TData), std::string{}})
	{
	}

	template<class TConstantBufferData>
	inline void ConstantBuffer<TConstantBufferData>::Write(const TData& data)
	{
		_uploadBuffer->Write(std::addressof(data), sizeof(TData), 0);
	}

	template<class TConstantBufferData>
	inline std::shared_ptr<BaseTask> ConstantBuffer<TConstantBufferData>::CreateInitializationTask()
	{
		return CreateInitializationTaskInternal();
	}

	template<class TConstantBufferData>
	inline ConstantBuffer<TConstantBufferData>::InitializationTask::InitializationTask(const std::shared_ptr<InitializationContext> taskContext)
		: TaskWithInternalContext<ConstantBuffer<TConstantBufferData>::InitializationTask>(taskContext)
	{
	}

	template<class TConstantBufferData>
	inline void ConstantBuffer<TConstantBufferData>::InitializationTask::Run(const std::shared_ptr<BaseStream>& stream)
	{
		this->_internalTaskContext->constantBuffer->_uploadBuffer = CreateUploadBuffer(stream);
	}
}