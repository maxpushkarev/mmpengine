#include <Backend/Dx12/Buffer.hpp>
#include <cassert>
#include <Backend/Dx12/d3dx12.h>

namespace MMPEngine::Backend::Dx12
{
	CopyBufferTask::CopyBufferTask(const std::shared_ptr<CopyBufferTaskContext>& context) : TaskWithInternalContext<CopyBufferTaskContext>(context)
	{
		const auto srcBuffer = _internalTaskContext->src.lock();
		const auto dstBuffer = _internalTaskContext->dst.lock();

		assert(srcBuffer);
		assert(dstBuffer);

		_commandTask = std::make_shared<CommandTask>(context);
		_switchSrcStateTask = srcBuffer->CreateSwitchStateTask(D3D12_RESOURCE_STATE_COPY_SOURCE);
		_switchDstStateTask = dstBuffer->CreateSwitchStateTask(D3D12_RESOURCE_STATE_COPY_DEST);
	}

	void CopyBufferTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		stream->Schedule(_switchSrcStateTask);
		stream->Schedule(_switchDstStateTask);
		stream->Schedule(_commandTask);
	}

	void CopyBufferTask::Finalize(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Finalize(stream);
	}

	CopyBufferTask::CommandTask::CommandTask(const std::shared_ptr<CopyBufferTaskContext>& context) : TaskWithInternalContext<CopyBufferTaskContext>(context)
	{
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
			sc->cmdList->CopyBufferRegion(
				dstBuffer->GetNativeResource().Get(),
				_internalTaskContext->dstByteOffset,
				srcBuffer->GetNativeResource().Get(),
				_internalTaskContext->srcByteOffset,
				_internalTaskContext->byteLength);
		}
	}

	void CopyBufferTask::CommandTask::Finalize(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Finalize(stream);
	}

	UploadBuffer::UploadBuffer(const Settings& settings) : Core::BaseEntity(settings.name), Core::UploadBuffer(settings)
	{
	}

	void UploadBuffer::Write(const void* src, std::size_t byteLength, std::size_t byteOffset)
	{
		std::memcpy((static_cast<char*>(_mappedBufferPtr) + byteOffset), src, byteLength);
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

	void ReadBackBuffer::Read(void* dst, std::size_t byteLength, std::size_t byteOffset)
	{
		std::memcpy(dst, (static_cast<char*>(_mappedBufferPtr) + byteOffset), byteLength);
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

	Buffer::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& context) : TaskWithInternalContext(context)
	{
	}

	void Buffer::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto sc = _specificStreamContext.lock();
		const auto ac = _specificAppContext.lock();
		const auto entity = _internalTaskContext->entity.lock();

		if(sc && ac && entity)
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

			if(const auto mappedEntity = std::dynamic_pointer_cast<MappedBuffer>(entity))
			{
				mappedEntity->Map();
			}

			if(_internalTaskContext->heapType == D3D12_HEAP_TYPE_UPLOAD)
			{
				stream->Schedule(entity->CreateSwitchStateTask(D3D12_RESOURCE_STATE_GENERIC_READ));
			}

			if (_internalTaskContext->heapType == D3D12_HEAP_TYPE_READBACK)
			{
				stream->Schedule(entity->CreateSwitchStateTask(D3D12_RESOURCE_STATE_COPY_DEST));
			}
		}
	}

	void Buffer::InitTask::Finalize(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Finalize(stream);
	}


	ResidentBuffer::ResidentBuffer(const Settings& settings) : Core::BaseEntity(settings.name), Core::ResidentBuffer(settings)
	{
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


}