#include <Backend/Dx12/Buffer.hpp>
#include <cassert>
#include <Backend/Dx12/d3dx12.h>

namespace MMPEngine::Backend::Dx12
{
	CopyBufferTask::CopyBufferTask(const std::shared_ptr<CopyBufferTaskContext>& context) : TaskWithContext<CopyBufferTaskContext>(context)
	{
		const auto srcBuffer = _internalTaskContext->src.lock();
		const auto dstBuffer = _internalTaskContext->dst.lock();

		assert(srcBuffer);
		assert(dstBuffer);

		_commandTask = std::make_shared<CommandTask>(context);
		_switchSrcStateTask = srcBuffer->CreateSwitchStateTask(D3D12_RESOURCE_STATE_COPY_SOURCE);
		_switchDstStateTask = dstBuffer->CreateSwitchStateTask(D3D12_RESOURCE_STATE_COPY_DEST);
	}

	void CopyBufferTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		stream->Schedule(_switchSrcStateTask);
		stream->Schedule(_switchDstStateTask);
		stream->Schedule(_commandTask);
	}


	void CopyBufferTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

	void CopyBufferTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	CopyBufferTask::CommandTask::CommandTask(const std::shared_ptr<CopyBufferTaskContext>& context) : TaskWithContext<CopyBufferTaskContext>(context)
	{
	}

	void CopyBufferTask::CommandTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}


	void CopyBufferTask::CommandTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto srcBuffer = _internalTaskContext->src.lock();
		const auto dstBuffer = _internalTaskContext->dst.lock();

		assert(srcBuffer);
		assert(dstBuffer);

		if (const auto sc = _specificStreamContext.lock())
		{
			sc->PopulateCommandsInList()->CopyBufferRegion(
				dstBuffer->GetNativeResource().Get(),
				_internalTaskContext->dstByteOffset,
				srcBuffer->GetNativeResource().Get(),
				_internalTaskContext->srcByteOffset,
				_internalTaskContext->byteLength);
		}
	}

	void CopyBufferTask::CommandTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	UploadBuffer::UploadBuffer(const Settings& settings) : Core::BaseEntity(settings.name), Core::UploadBuffer(settings)
	{
	}

	std::shared_ptr<Core::TaskWithContext<Core::UploadBuffer::WriteTaskContext>> UploadBuffer::CreateWriteTask(const void* src, std::size_t byteLength, std::size_t byteOffset)
	{
		const auto ctx = std::make_shared<WriteTaskContext>();
		const auto props = std::make_shared<WriteTaskProps>();
		props->uploadBuffer = std::dynamic_pointer_cast<UploadBuffer>(shared_from_this());

		ctx->byteOffset = byteOffset;
		ctx->byteLength = byteLength;
		ctx->src = src;
		ctx->properties = props;

		return std::make_shared<WriteTask>(ctx);
	}

	std::shared_ptr<Core::BaseTask> UploadBuffer::CreateCopyToBufferTask(
		const std::shared_ptr<Core::Buffer>& dst, 
		std::size_t byteLength, 
		std::size_t srcByteOffset,
		std::size_t dstByteOffset) const
	{
		const auto context = std::make_shared<CopyBufferTaskContext>();
		context->src = std::dynamic_pointer_cast<BaseEntity>(std::const_pointer_cast<Core::BaseEntity>(shared_from_this()));
		context->dst = std::dynamic_pointer_cast<BaseEntity>(dst);
		context->srcByteOffset = srcByteOffset;
		context->dstByteOffset = dstByteOffset;
		context->byteLength = byteLength;

		return std::make_shared<CopyBufferTask>(context);
	}

	std::shared_ptr<Core::BaseTask> UploadBuffer::CreateInitializationTask()
	{
		const auto tc = std::make_shared<InitTaskContext>();
		tc->entity = std::dynamic_pointer_cast<MappedBuffer>(shared_from_this());
		tc->byteSize = _settings.byteLength;
		tc->unorderedAccess = false;
		tc->heapType = D3D12_HEAP_TYPE_UPLOAD;

		return std::make_shared<InitTask>(tc);
	}


	ReadBackBuffer::ReadBackBuffer(const Settings& settings) : Core::BaseEntity(settings.name), Core::ReadBackBuffer(settings)
	{
	}

	std::shared_ptr<Core::TaskWithContext<Core::ReadBackBuffer::ReadTaskContext>> ReadBackBuffer::CreateReadTask(void* dst, std::size_t byteLength, std::size_t byteOffset)
	{
		const auto ctx = std::make_shared<ReadTaskContext>();
		const auto props = std::make_shared<ReadTaskProps>();
		props->readBackBuffer = std::dynamic_pointer_cast<ReadBackBuffer>(shared_from_this());

		ctx->byteOffset = byteOffset;
		ctx->byteLength = byteLength;
		ctx->dst = dst;
		ctx->properties = props;

		return std::make_shared<ReadTask>(ctx);
	}

	std::shared_ptr<Core::BaseTask> ReadBackBuffer::CreateInitializationTask()
	{
		const auto tc = std::make_shared<InitTaskContext>();
		tc->entity = std::dynamic_pointer_cast<MappedBuffer>(shared_from_this());
		tc->byteSize = _settings.byteLength;
		tc->unorderedAccess = false;
		tc->heapType = D3D12_HEAP_TYPE_READBACK;

		return std::make_shared<InitTask>(tc);
	}

	std::shared_ptr<Core::BaseTask> ReadBackBuffer::CreateCopyToBufferTask(
		const std::shared_ptr<Core::Buffer>& dst,
		std::size_t byteLength,
		std::size_t srcByteOffset,
		std::size_t dstByteOffset) const
	{
		const auto context = std::make_shared<CopyBufferTaskContext>();
		context->src = std::dynamic_pointer_cast<BaseEntity>(std::const_pointer_cast<Core::BaseEntity>(shared_from_this()));
		context->dst = std::dynamic_pointer_cast<BaseEntity>(dst);
		context->srcByteOffset = srcByteOffset;
		context->dstByteOffset = dstByteOffset;
		context->byteLength = byteLength;

		return std::make_shared<CopyBufferTask>(context);
	}

	Buffer::Buffer() = default;
	Buffer::Buffer(std::string_view name) : ResourceEntity(name)
	{
	}


	MappedBuffer::MappedBuffer(std::string_view name) : Buffer(name)
	{
	}

	MappedBuffer::MappedBuffer() = default;

	MappedBuffer::~MappedBuffer()
	{
		Unmap();
	}

	void MappedBuffer::Map()
	{
		assert(_nativeResource);
		_nativeResource->Map(0, nullptr, &_mappedBufferPtr);
	}

	void MappedBuffer::Unmap()
	{
		if (_nativeResource)
		{
			_nativeResource->Unmap(0, nullptr);
			_mappedBufferPtr = nullptr;
		}
	}

	Buffer::CreateBufferTask::CreateBufferTask(const std::shared_ptr<InitTaskContext>& context) : TaskWithContext(context)
	{
	}

	void Buffer::CreateBufferTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}

	void Buffer::CreateBufferTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto sc = _specificStreamContext.lock();
		const auto ac = _specificAppContext.lock();
		const auto entity = _internalTaskContext->entity.lock();

		if (sc && ac && entity)
		{
			const auto heapProperties = CD3DX12_HEAP_PROPERTIES(_internalTaskContext->heapType);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(_internalTaskContext->byteSize);
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			if (_internalTaskContext->unorderedAccess)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}

			Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource = nullptr;

			ac->device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				ResourceEntity::_defaultState,
				nullptr,
				IID_PPV_ARGS(&bufferResource));

			assert(bufferResource);

			entity->SetNativeResource(bufferResource);

			if (const auto mappedEntity = std::dynamic_pointer_cast<MappedBuffer>(entity))
			{
				mappedEntity->Map();
			}
		}
	}

	void Buffer::CreateBufferTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	Buffer::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& context) : TaskWithContext(context)
	{
	}

	void Buffer::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		if(const auto entity = _internalTaskContext->entity.lock())
		{
			stream->Schedule(std::make_shared<CreateBufferTask>(_internalTaskContext));

			if (_internalTaskContext->heapType == D3D12_HEAP_TYPE_UPLOAD)
			{
				stream->Schedule(entity->CreateSwitchStateTask(D3D12_RESOURCE_STATE_GENERIC_READ));
			}

			if (_internalTaskContext->heapType == D3D12_HEAP_TYPE_READBACK)
			{
				stream->Schedule(entity->CreateSwitchStateTask(D3D12_RESOURCE_STATE_COPY_DEST));
			}
		}
	}


	void Buffer::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

	void Buffer::InitTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}


	ResidentBuffer::ResidentBuffer(const Settings& settings) : Core::BaseEntity(settings.name), Core::ResidentBuffer(settings)
	{
	}

	InputAssemblerBuffer::InputAssemblerBuffer(const Core::InputAssemblerBuffer::Settings& settings) : Core::InputAssemblerBuffer(settings)
	{
		_upload = std::make_shared<UploadBuffer>(settings.base);
		_resident = std::make_shared<ResidentBuffer>(settings.base);
	}

	std::shared_ptr<Core::BaseTask> InputAssemblerBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		return _resident->CreateCopyToBufferTask(dst, byteLength, srcByteOffset, dstByteOffset);
	}

	std::shared_ptr<Core::Buffer> InputAssemblerBuffer::GetUnderlyingBuffer()
	{
		return _resident;
	}

	std::shared_ptr<Core::BaseTask> InputAssemblerBuffer::CreateInitializationTask()
	{
		const auto context = std::make_shared<TaskContext>();
		context->entity = std::dynamic_pointer_cast<Dx12::InputAssemblerBuffer>(shared_from_this());
		return std::make_shared<InitTask>(context);
	}

	VertexBuffer::VertexBuffer(const Core::InputAssemblerBuffer::Settings& settings) : Core::InputAssemblerBuffer(settings), Core::VertexBuffer(settings), InputAssemblerBuffer(settings)
	{
	}

	IndexBuffer::IndexBuffer(const Core::InputAssemblerBuffer::Settings& settings) : Core::InputAssemblerBuffer(settings), Core::IndexBuffer(settings), InputAssemblerBuffer(settings)
	{
	}

	std::shared_ptr<Core::BaseTask> IndexBuffer::CreateInitializationTask()
	{
		return InputAssemblerBuffer::CreateInitializationTask();
	}

	std::shared_ptr<Core::Buffer> IndexBuffer::GetUnderlyingBuffer()
	{
		return InputAssemblerBuffer::GetUnderlyingBuffer();
	}

	std::shared_ptr<Core::BaseTask> IndexBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		return InputAssemblerBuffer::CreateCopyToBufferTask(dst, byteLength, srcByteOffset, dstByteOffset);
	}

	std::shared_ptr<Core::BaseTask> VertexBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		return InputAssemblerBuffer::CreateCopyToBufferTask(dst, byteLength, srcByteOffset, dstByteOffset);
	}

	std::shared_ptr<Core::BaseTask> VertexBuffer::CreateInitializationTask()
	{
		return InputAssemblerBuffer::CreateInitializationTask();
	}

	std::shared_ptr<Core::Buffer> VertexBuffer::GetUnderlyingBuffer()
	{
		return InputAssemblerBuffer::GetUnderlyingBuffer();		
	}

	InputAssemblerBuffer::InitTask::InitTask(const std::shared_ptr<TaskContext>& context) : TaskWithContext(context)
	{
	}

	void InputAssemblerBuffer::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		if (const auto entity = _internalTaskContext->entity.lock())
		{
			stream->Schedule(entity->_upload->CreateInitializationTask());
			stream->Schedule(entity->_resident->CreateInitializationTask());
			stream->Schedule(Core::StreamBarrierTask::kInstance);
			stream->Schedule(entity->_upload->CreateWriteTask(entity->_ia.rawData, entity->GetSettings().byteLength, 0));
			stream->Schedule(entity->_upload->CopyToBuffer(entity->_resident));
			stream->Schedule(entity->_resident->CreateSwitchStateTask(D3D12_RESOURCE_STATE_GENERIC_READ));
		}
	}

	void InputAssemblerBuffer::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}


	void InputAssemblerBuffer::InitTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);

		if(const auto entity = _internalTaskContext->entity.lock())
		{
			entity->_upload.reset();
		}
	}


	std::shared_ptr<Core::BaseTask> ResidentBuffer::CreateInitializationTask()
	{
		const auto tc = std::make_shared<InitTaskContext>();
		tc->entity = std::dynamic_pointer_cast<Dx12::Buffer>(shared_from_this());
		tc->byteSize = _settings.byteLength;
		tc->unorderedAccess = false;
		tc->heapType = D3D12_HEAP_TYPE_DEFAULT;

		return std::make_shared<InitTask>(tc);
	}

	std::shared_ptr<Core::BaseTask> ResidentBuffer::CreateCopyToBufferTask(
		const std::shared_ptr<Core::Buffer>& dst,
		std::size_t byteLength,
		std::size_t srcByteOffset,
		std::size_t dstByteOffset) const
	{
		const auto context = std::make_shared<CopyBufferTaskContext>();
		context->src = std::dynamic_pointer_cast<BaseEntity>(std::const_pointer_cast<Core::BaseEntity>(shared_from_this()));
		context->dst = std::dynamic_pointer_cast<BaseEntity>(dst);
		context->srcByteOffset = srcByteOffset;
		context->dstByteOffset = dstByteOffset;
		context->byteLength = byteLength;

		return std::make_shared<CopyBufferTask>(context);
	}

	void* MappedBuffer::MappedBufferTask::GetMappedPtr(const std::shared_ptr<MappedBuffer>& mappedBuffer)
	{
		return mappedBuffer->_mappedBufferPtr;
	}


	UploadBuffer::WriteTask::WriteTask(const std::shared_ptr<WriteTaskContext>& context) : TaskWithContext(context)
	{
		const auto buffer = std::static_pointer_cast<WriteTaskProps>(_internalTaskContext->properties)->uploadBuffer.lock();
		assert(buffer);

		_implTask = std::make_shared<Impl>(context);
		_prepareStateTask = buffer->CreateSwitchStateTask(D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	void UploadBuffer::WriteTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		stream->Schedule(_prepareStateTask);
		stream->Schedule(Core::StreamBarrierTask::kInstance);
		stream->Schedule(_implTask);
	}

	void UploadBuffer::WriteTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

	void UploadBuffer::WriteTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	UploadBuffer::WriteTask::Impl::Impl(const std::shared_ptr<WriteTaskContext>& context) : TaskWithContext(context)
	{
	}

	void UploadBuffer::WriteTask::Impl::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}

	void UploadBuffer::WriteTask::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		if (const auto entity = std::static_pointer_cast<WriteTaskProps>(_internalTaskContext->properties)->uploadBuffer.lock())
		{
			std::memcpy((static_cast<char*>(GetMappedPtr(entity)) + _internalTaskContext->byteOffset), _internalTaskContext->src, _internalTaskContext->byteLength);
		}
	}

	void UploadBuffer::WriteTask::Impl::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}


	ReadBackBuffer::ReadTask::ReadTask(const std::shared_ptr<ReadTaskContext>& context) : TaskWithContext(context)
	{
		const auto buffer = std::static_pointer_cast<ReadTaskProps>(_internalTaskContext->properties)->readBackBuffer.lock();
		assert(buffer);

		_implTask = std::make_shared<Impl>(context);
		_prepareStateTask = buffer->CreateSwitchStateTask(D3D12_RESOURCE_STATE_COPY_DEST);
	}

	void ReadBackBuffer::ReadTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		stream->Schedule(_prepareStateTask);
		stream->Schedule(Core::StreamBarrierTask::kInstance);
		stream->Schedule(_implTask);
	}

	void ReadBackBuffer::ReadTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

	void ReadBackBuffer::ReadTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	ReadBackBuffer::ReadTask::Impl::Impl(const std::shared_ptr<ReadTaskContext>& context) : TaskWithContext(context)
	{
	}

	void ReadBackBuffer::ReadTask::Impl::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}

	void ReadBackBuffer::ReadTask::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		if (const auto entity = std::static_pointer_cast<ReadTaskProps>(_internalTaskContext->properties)->readBackBuffer.lock())
		{
			std::memcpy(_internalTaskContext->dst, (static_cast<char*>(GetMappedPtr(entity)) + _internalTaskContext->byteOffset), _internalTaskContext->byteLength);
		}
	}

	void ReadBackBuffer::ReadTask::Impl::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

}