#include <Backend/Dx12/Buffer.hpp>
#include <cassert>
#include <memory>
#include <Backend/Dx12/d3dx12.h>

namespace MMPEngine::Backend::Dx12
{
	CopyBufferTask::CopyBufferTask(const std::shared_ptr<CopyBufferTaskContext>& context) : Task<CopyBufferTaskContext>(context)
	{
		const auto tc = GetTaskContext();
		const auto srcBuffer = tc->src;
		const auto dstBuffer = tc->dst;

		assert(srcBuffer);
		assert(dstBuffer);

		_commandTask = std::make_shared<Impl>(context);
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

	CopyBufferTask::Impl::Impl(const std::shared_ptr<CopyBufferTaskContext>& context) : Task<CopyBufferTaskContext>(context)
	{
	}

	void CopyBufferTask::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto tc = GetTaskContext();
		const auto srcBuffer = tc->src;
		const auto dstBuffer = tc->dst;

		assert(srcBuffer);
		assert(dstBuffer);

		const auto srcNative = srcBuffer->GetNativeResource().Get();
		const auto dstNative = dstBuffer->GetNativeResource().Get();

		const auto srcNativeAddress = srcBuffer->GetNativeGPUAddress();
		const auto dstNativeAddress = dstBuffer->GetNativeGPUAddress();

		assert(srcNativeAddress >= srcNative->GetGPUVirtualAddress());
		assert(dstNativeAddress >= dstNative->GetGPUVirtualAddress());

		_specificStreamContext->PopulateCommandsInList()->CopyBufferRegion(
			dstBuffer->GetNativeResource().Get(),
			static_cast<std::uint64_t>(tc->dstByteOffset) + static_cast<std::uint64_t>(dstNativeAddress - dstNative->GetGPUVirtualAddress()),
			srcBuffer->GetNativeResource().Get(),
			static_cast<std::uint64_t>(tc->srcByteOffset) + static_cast<std::uint64_t>(srcNativeAddress - srcNative->GetGPUVirtualAddress()),
			tc->byteLength);
		
	}

	UploadBuffer::UploadBuffer(const Settings& settings) : Core::UploadBuffer(settings)
	{
	}

	std::shared_ptr<Core::ContextualTask<Core::UploadBuffer::WriteTaskContext>> UploadBuffer::CreateWriteTask(const void* src, std::size_t byteLength, std::size_t byteOffset)
	{
		const auto ctx = std::make_shared<WriteTaskContext>();
		ctx->uploadBuffer = std::dynamic_pointer_cast<UploadBuffer>(shared_from_this());
		ctx->byteOffset = byteOffset;
		ctx->byteLength = byteLength;
		ctx->src = src;

		return std::make_shared<WriteTask>(ctx);
	}

	std::shared_ptr<Core::BaseTask> UploadBuffer::CreateCopyToBufferTask(
		const std::shared_ptr<Core::Buffer>& dst, 
		std::size_t byteLength, 
		std::size_t srcByteOffset,
		std::size_t dstByteOffset) const
	{
		const auto context = std::make_shared<CopyBufferTaskContext>();
		context->src = std::dynamic_pointer_cast<Dx12::BaseEntity>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
		context->dst = std::dynamic_pointer_cast<Dx12::BaseEntity>(dst->GetUnderlyingBuffer());
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


	ReadBackBuffer::ReadBackBuffer(const Settings& settings) : Core::ReadBackBuffer(settings)
	{
	}

	std::shared_ptr<Core::ContextualTask<Core::ReadBackBuffer::ReadTaskContext>> ReadBackBuffer::CreateReadTask(void* dst, std::size_t byteLength, std::size_t byteOffset)
	{
		const auto ctx = std::make_shared<ReadTaskContext>();
		ctx->readBackBuffer = std::dynamic_pointer_cast<ReadBackBuffer>(shared_from_this());
		ctx->byteOffset = byteOffset;
		ctx->byteLength = byteLength;
		ctx->dst = dst;

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
		context->src = std::dynamic_pointer_cast<Dx12::BaseEntity>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
		context->dst = std::dynamic_pointer_cast<Dx12::BaseEntity>(dst->GetUnderlyingBuffer());
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

	Buffer::CreateBufferTask::CreateBufferTask(const std::shared_ptr<InitTaskContext>& context) : Task(context)
	{
	}

	void Buffer::CreateBufferTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto tc = GetTaskContext();
		const auto sc = _specificStreamContext;
		const auto ac = _specificAppContext;
		const auto entity = tc->entity;

		if (sc && ac && entity)
		{
			const auto heapProperties = CD3DX12_HEAP_PROPERTIES(tc->heapType);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(tc->byteSize);
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			if(tc->unorderedAccess)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
			
			Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource = nullptr;

			//TODO: batch constant buffers
			ac->device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				ResourceEntity::_defaultState,
				nullptr,
				IID_PPV_ARGS(&bufferResource));

			assert(bufferResource);
			entity->SetNativeResource(bufferResource, 0);

			if (const auto mappedEntity = std::dynamic_pointer_cast<MappedBuffer>(entity))
			{
				mappedEntity->Map();
			}
		}
	}

	Buffer::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& context) : Task(context)
	{
	}

	void Buffer::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		if(const auto tc = GetTaskContext() ; const auto entity = tc->entity)
		{
			stream->Schedule(std::make_shared<CreateBufferTask>(tc));

			if (tc->heapType == D3D12_HEAP_TYPE_UPLOAD)
			{
				stream->Schedule(entity->CreateSwitchStateTask(D3D12_RESOURCE_STATE_GENERIC_READ));
			}

			if (tc->heapType == D3D12_HEAP_TYPE_READBACK)
			{
				stream->Schedule(entity->CreateSwitchStateTask(D3D12_RESOURCE_STATE_COPY_DEST));
			}
		}
	}

	const BaseDescriptorHeap::Handle* UaBuffer::GetShaderInVisibleDescriptorHandle() const
	{
		return &_shaderInVisibleHandle;
	}

	const BaseDescriptorHeap::Handle* UaBuffer::GetShaderVisibleDescriptorHandle() const
	{
		return &_shaderVisibleHandle;
	}

	UaBuffer::InitUaTask::InitUaTask(const std::shared_ptr<InitUaTaskContext>& context) : Task(context)
	{
	}

	void UaBuffer::InitUaTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		const auto initContext = std::make_shared<InitTaskContext>();
		const auto uaSettings = GetTaskContext()->settings;
		initContext->entity = GetTaskContext()->entity;
		initContext->byteSize = uaSettings.elementsCount * uaSettings.stride;
		initContext->heapType = D3D12_HEAP_TYPE_DEFAULT;
		initContext->unorderedAccess = true;

		stream->Schedule(std::make_shared<InitTask>(initContext));
		stream->Schedule(std::make_shared<CreateUaDescriptors>(GetTaskContext()));
	}


	UnorderedAccessBuffer::CreateUaDescriptors::CreateUaDescriptors(const std::shared_ptr<InitUaTaskContext>& context) : Task(context)
	{
	}

	void UnorderedAccessBuffer::CreateUaDescriptors::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		const auto tc = GetTaskContext();
		const auto sc = _specificStreamContext;
		const auto ac = _specificAppContext;
		const auto entity = tc->entity;

		if (tc && sc && ac && entity)
		{
			entity->_shaderVisibleHandle = ac->cbvSrvUavShaderVisibleHeap->Allocate();
			entity->_shaderInVisibleHandle = ac->cbvSrvUavShaderInVisibleHeap->Allocate();

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;

			uavDesc.Format = (tc->isCounter ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_UNKNOWN);
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			uavDesc.Buffer.CounterOffsetInBytes = 0;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = static_cast<std::uint32_t>(tc->settings.elementsCount);
			uavDesc.Buffer.StructureByteStride = tc->isCounter ? 0 : static_cast<std::uint32_t>(tc->settings.stride);

			const auto bufferResource = entity->GetNativeResource().Get();

			ID3D12Resource* counterResource = nullptr;
			if (tc->counterAttachment)
			{
				counterResource = tc->counterAttachment->GetNativeResource().Get();
			}

			ac->device->CreateUnorderedAccessView(
				bufferResource,
				counterResource,
				&uavDesc,
				entity->_shaderVisibleHandle.GetCPUDescriptorHandle()
			);


			ac->device->CreateUnorderedAccessView(
				bufferResource,
				counterResource,
				&uavDesc,
				entity->_shaderInVisibleHandle.GetCPUDescriptorHandle()
			);
		}
	}

	UnorderedAccessBuffer::UnorderedAccessBuffer(const Settings& settings) : Core::UnorderedAccessBuffer(settings)
	{
	}

	std::shared_ptr<Core::BaseTask> UnorderedAccessBuffer::CreateInitializationTask()
	{
		const auto tc = std::make_shared<InitUaTaskContext>();
		tc->entity = std::dynamic_pointer_cast<Dx12::UaBuffer>(shared_from_this());
		tc->settings = GetUnorderedAccessSettings();
		tc->isCounter = false;
		tc->counterAttachment = nullptr;

		return std::make_shared<InitUaTask>(tc);
	}

	std::shared_ptr<Core::BaseTask> UnorderedAccessBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		const auto context = std::make_shared<CopyBufferTaskContext>();
		context->src = std::dynamic_pointer_cast<Dx12::BaseEntity>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
		context->dst = std::dynamic_pointer_cast<Dx12::BaseEntity>(dst->GetUnderlyingBuffer());
		context->srcByteOffset = srcByteOffset;
		context->dstByteOffset = dstByteOffset;
		context->byteLength = byteLength;

		return std::make_shared<CopyBufferTask>(context);
	}

	CounteredUnorderedAccessBuffer::CounteredUnorderedAccessBuffer(const Settings& settings) : Core::CounteredUnorderedAccessBuffer(settings),
		_counter(std::make_shared<UnorderedAccessBuffer>(Core::BaseUnorderedAccessBuffer::Settings {sizeof(Core::CounteredUnorderedAccessBuffer::CounterValueType), 1, ""}))
	{
	}

	std::shared_ptr<Core::BaseTask> CounteredUnorderedAccessBuffer::CreateInitializationTask()
	{
		const auto counterInitContext = std::make_shared<InitUaTaskContext>();
		counterInitContext->entity = _counter;
		counterInitContext->settings = _counter->GetUnorderedAccessSettings();
		counterInitContext->isCounter = true;
		counterInitContext->counterAttachment = nullptr;

		const auto dataBufferInitContext = std::make_shared<InitUaTaskContext>();
		dataBufferInitContext->entity = std::dynamic_pointer_cast<Dx12::UaBuffer>(shared_from_this());
		dataBufferInitContext->settings = GetUnorderedAccessSettings();
		dataBufferInitContext->isCounter = false;
		dataBufferInitContext->counterAttachment = _counter;

		return std::make_shared<Core::BatchTask>(std::initializer_list<std::shared_ptr<Core::BaseTask>>{
			std::make_shared<InitUaTask>(counterInitContext),
			std::make_shared<InitUaTask>(dataBufferInitContext),
			CreateResetCounterTask()
		});
	}

	std::shared_ptr<Core::BaseTask> CounteredUnorderedAccessBuffer::CreateSwitchStateTask(D3D12_RESOURCE_STATES nextStateMask)
	{
		return std::make_shared<Core::BatchTask>(
			std::initializer_list<std::shared_ptr<Core::BaseTask>> {
				this->_counter->CreateSwitchStateTask(nextStateMask),
				ResourceEntity::CreateSwitchStateTask(nextStateMask)
			}
		);
	}

	CounteredUnorderedAccessBuffer::ResetCounter::ResetCounter(const std::shared_ptr<Core::EntityTaskContext<UaBuffer>>& ctx) : Task<MMPEngine::Core::EntityTaskContext<MMPEngine::Backend::Dx12::UaBuffer>>(ctx)
	{
	}

	void CounteredUnorderedAccessBuffer::ResetCounter::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		const auto bindDescHeapsCtx = std::make_shared<BindDescriptorHeapsTaskContext>();
		bindDescHeapsCtx->FillDescriptors(_specificAppContext);

		stream->Schedule(GetTaskContext()->entity->CreateSwitchStateTask(D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		stream->Schedule(std::make_shared<BindDescriptorHeapsTask>(bindDescHeapsCtx));
		stream->Schedule(std::make_shared<Impl>(GetTaskContext()));
	}


	CounteredUnorderedAccessBuffer::ResetCounter::Impl::Impl(const std::shared_ptr<Core::EntityTaskContext<UaBuffer>>& ctx) : Task<MMPEngine::Core::EntityTaskContext<MMPEngine::Backend::Dx12::UaBuffer>>(ctx)
	{
	}

	void CounteredUnorderedAccessBuffer::ResetCounter::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto entity = GetTaskContext()->entity;
		constexpr std::uint32_t clear[]{ 0,0,0,0 };

		_specificStreamContext->PopulateCommandsInList()->ClearUnorderedAccessViewUint(
			entity->GetShaderVisibleDescriptorHandle()->GetGPUDescriptorHandle(),
			entity->GetShaderInVisibleDescriptorHandle()->GetCPUDescriptorHandle(),
			entity->GetNativeResource().Get(),
			clear,
			0, nullptr
		);
	}

	std::shared_ptr<Core::BaseTask> CounteredUnorderedAccessBuffer::CreateResetCounterTask()
	{
		const auto resetCtx = std::make_shared<Core::EntityTaskContext<UaBuffer>>();
		resetCtx->entity = _counter;
		return std::make_shared<ResetCounter>(resetCtx);
	}

	std::shared_ptr<Core::BaseTask> CounteredUnorderedAccessBuffer::CreateCopyCounterTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t dstByteOffset)
	{
		return _counter->CreateCopyToBufferTask(dst, _counter->GetSettings().byteLength, 0, dstByteOffset);
	}

	std::shared_ptr<Core::BaseTask> CounteredUnorderedAccessBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		const auto context = std::make_shared<CopyBufferTaskContext>();
		context->src = std::dynamic_pointer_cast<Dx12::BaseEntity>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
		context->dst = std::dynamic_pointer_cast<Dx12::BaseEntity>(dst->GetUnderlyingBuffer());
		context->srcByteOffset = srcByteOffset;
		context->dstByteOffset = dstByteOffset;
		context->byteLength = byteLength;

		return std::make_shared<CopyBufferTask>(context);
	}


	ResidentBuffer::ResidentBuffer(const Settings& settings) : Core::ResidentBuffer(settings)
	{
	}

	std::shared_ptr<Core::BaseTask> ResidentBuffer::CreateInitializationTask()
	{
		const auto tc = std::make_shared<InitTaskContext>();
		tc->entity = std::dynamic_pointer_cast<Dx12::Buffer>(shared_from_this());
		tc->byteSize = _settings.byteLength;
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
		context->src = std::dynamic_pointer_cast<Dx12::BaseEntity>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
		context->dst = std::dynamic_pointer_cast<Dx12::BaseEntity>(dst->GetUnderlyingBuffer());
		context->srcByteOffset = srcByteOffset;
		context->dstByteOffset = dstByteOffset;
		context->byteLength = byteLength;

		return std::make_shared<CopyBufferTask>(context);
	}


	InputAssemblerBuffer::InputAssemblerBuffer(const Core::InputAssemblerBuffer::Settings& settings) : _ia(settings.ia)
	{
		_upload = std::make_shared<UploadBuffer>(settings.base);
		_resident = std::make_shared<ResidentBuffer>(settings.base);
	}

	InputAssemblerBuffer::~InputAssemblerBuffer() = default;

	VertexBuffer::VertexBuffer(const Core::InputAssemblerBuffer::Settings& settings) : Core::VertexBuffer(settings), Dx12::InputAssemblerBuffer(settings)
	{
	}

	IndexBuffer::IndexBuffer(const Core::InputAssemblerBuffer::Settings& settings) : Core::IndexBuffer(settings), Dx12::InputAssemblerBuffer(settings)
	{
	}

	std::shared_ptr<Core::BaseTask> IndexBuffer::CreateInitializationTask()
	{
		const auto context = std::make_shared<TaskContext>();
		context->entity = std::dynamic_pointer_cast<Dx12::InputAssemblerBuffer>(shared_from_this());
		return std::make_shared<InitTask>(context);
	}

	std::shared_ptr<Core::Buffer> IndexBuffer::GetUnderlyingBuffer()
	{
		return _resident;
	}

	std::shared_ptr<Core::BaseTask> IndexBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		return _resident->CreateCopyToBufferTask(dst, byteLength, srcByteOffset, dstByteOffset);
	}

	std::shared_ptr<Core::BaseTask> VertexBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		return _resident->CreateCopyToBufferTask(dst, byteLength, srcByteOffset, dstByteOffset);
	}

	std::shared_ptr<Core::BaseTask> VertexBuffer::CreateInitializationTask()
	{
		const auto context = std::make_shared<TaskContext>();
		context->entity = std::dynamic_pointer_cast<Dx12::InputAssemblerBuffer>(shared_from_this());
		return std::make_shared<InitTask>(context);
	}

	std::shared_ptr<Core::Buffer> VertexBuffer::GetUnderlyingBuffer()
	{
		return _resident;
	}

	InputAssemblerBuffer::InitTask::InitTask(const std::shared_ptr<TaskContext>& context) : Task(context)
	{
	}

	void InputAssemblerBuffer::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		if (const auto tc = GetTaskContext() ; const auto entity = tc->entity)
		{
			stream->Schedule(entity->_upload->CreateInitializationTask());
			stream->Schedule(entity->_resident->CreateInitializationTask());
			stream->Schedule(Core::StreamBarrierTask::kInstance);
			stream->Schedule(entity->_upload->CreateWriteTask(entity->_ia.rawData, entity->_upload->GetSettings().byteLength, 0));
			stream->Schedule(entity->_upload->CopyToBuffer(entity->_resident));
			stream->Schedule(entity->_resident->CreateSwitchStateTask(D3D12_RESOURCE_STATE_GENERIC_READ));
		}
	}

	void InputAssemblerBuffer::InitTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);

		if(const auto tc = GetTaskContext() ; const auto entity = tc->entity)
		{
			entity->_upload.reset();
		}
	}

	void* MappedBuffer::MappedBufferTask::GetMappedPtr(const std::shared_ptr<MappedBuffer>& mappedBuffer)
	{
		return mappedBuffer->_mappedBufferPtr;
	}


	UploadBuffer::WriteTask::WriteTask(const std::shared_ptr<WriteTaskContext>& context) : Task(context)
	{
		const auto buffer= context->uploadBuffer;
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

	UploadBuffer::WriteTask::Impl::Impl(const std::shared_ptr<WriteTaskContext>& context) : Task(context)
	{
	}

	void UploadBuffer::WriteTask::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		if (const auto tc = GetTaskContext() ; const auto entity = tc->uploadBuffer)
		{
			std::memcpy((static_cast<char*>(GetMappedPtr(entity)) + tc->byteOffset), tc->src, tc->byteLength);
		}
	}

	ReadBackBuffer::ReadTask::ReadTask(const std::shared_ptr<ReadTaskContext>& context) : Task(context)
	{
		const auto buffer = context->readBackBuffer;
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

	ReadBackBuffer::ReadTask::Impl::Impl(const std::shared_ptr<ReadTaskContext>& context) : Task(context)
	{
	}

	void ReadBackBuffer::ReadTask::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		if (const auto tc = GetTaskContext() ; const auto entity = tc->readBackBuffer)
		{
			std::memcpy(tc->dst, (static_cast<char*>(GetMappedPtr(entity)) + tc->byteOffset), tc->byteLength);
		}
	}
}