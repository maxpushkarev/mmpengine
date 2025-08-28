#include <cassert>
#include <Backend/Dx12/Mesh.hpp>
#include <Backend/Dx12/Buffer.hpp>

namespace MMPEngine::Backend::Dx12
{
	Mesh::Mesh(Core::GeometryPrototype&& proto) : Core::Mesh(std::move(proto))
	{
	}

	std::shared_ptr<Core::BaseTask> Mesh::CreateInternalInitializationTask()
	{
		return Core::BaseTask::kEmpty;
	}

	std::shared_ptr<Core::VertexBuffer> Mesh::CreateVertexBuffer(const Core::VertexBufferPrototype* vbPrototype)
	{
		return std::make_shared<VertexBuffer>(Core::InputAssemblerBuffer::Settings {
		{vbPrototype->GetDataPtr()}, {vbPrototype->GetByteLength()}
		});
	}

	std::shared_ptr<Core::IndexBuffer> Mesh::CreateIndexBuffer(const Core::IndexBufferPrototype* ibPrototype)
	{
		return std::make_shared<IndexBuffer>(Core::InputAssemblerBuffer::Settings {
			{ibPrototype->GetDataPtr()}, { ibPrototype->GetByteLength() }
		});
	}

	const char* Mesh::Renderer::GetSemanticsName(Core::VertexBufferPrototype::Semantics semantics)
	{
		switch (semantics)
		{
			case Core::VertexBufferPrototype::Semantics::Color:
				return "COLOR";
			case Core::VertexBufferPrototype::Semantics::Normal:
				return "NORMAL";
			case Core::VertexBufferPrototype::Semantics::Position:
				return "POSITION";
			case Core::VertexBufferPrototype::Semantics::UV:
				return "TEXCOORD";
			case Core::VertexBufferPrototype::Semantics::Tangent:
				return "TANGENT";
			case Core::VertexBufferPrototype::Semantics::BiNormal:
				return "BINORMAL";
			case Core::VertexBufferPrototype::Semantics::BlendIndices:
				return "BLENDINDICES";
			case Core::VertexBufferPrototype::Semantics::BlendWeight:
				return "BLENDWEIGHT";
			default:
				throw Core::UnsupportedException("unsupported dx12 semantics name");
		}
	}

	DXGI_FORMAT Mesh::Renderer::GetVertexBufferFormat(Core::VertexBufferPrototype::Format format)
	{
		switch (format)
		{
		case Core::VertexBufferPrototype::Format::Float1:
			return DXGI_FORMAT_R32_FLOAT;
		case Core::VertexBufferPrototype::Format::Float2:
			return DXGI_FORMAT_R32G32_FLOAT;
		case Core::VertexBufferPrototype::Format::Float3:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case Core::VertexBufferPrototype::Format::Float4:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case Core::VertexBufferPrototype::Format::Uint4:
			return DXGI_FORMAT_R32G32B32A32_UINT;
		default:
			throw Core::UnsupportedException("unsupported dx12 vertex buffer format");
		}
	}

	std::shared_ptr<Core::BaseTask> Mesh::Renderer::CreateInternalInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->renderer = std::dynamic_pointer_cast<Renderer>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	const std::vector<D3D12_VERTEX_BUFFER_VIEW>& Mesh::Renderer::GetVertexBufferViews() const
	{
		return _vertexBufferViews;
	}

	const D3D12_INDEX_BUFFER_VIEW& Mesh::Renderer::GetIndexBufferView() const
	{
		return _indexBufferView;
	}

	const std::vector<std::shared_ptr<Dx12::BaseEntity>>& Mesh::Renderer::GetVertexBufferPointers() const
	{
		return _vertexBuffers;
	}

	std::shared_ptr<Dx12::BaseEntity> Mesh::Renderer::GetIndexBufferPointer() const
	{
		return _indexBuffer;
	}

	const std::vector<D3D12_INPUT_ELEMENT_DESC>& Mesh::Renderer::GetVertexInputLayout() const
	{
		return _vertexInputLayout;
	}

	Mesh::Renderer::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task(ctx)
	{
	}

	void Mesh::Renderer::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto renderer = GetTaskContext()->renderer;
		assert(renderer);

		const auto& ibInfo = renderer->GetMesh()->GetIndexBufferInfo();
		const auto ib = std::dynamic_pointer_cast<ResourceEntity>(ibInfo.ptr->GetUnderlyingBuffer());
		assert(ib);

		renderer->_indexBuffer = ib;
		renderer->_indexBufferView.Format = (ibInfo.format == Core::IndexBufferPrototype::Format::Uint16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		renderer->_indexBufferView.SizeInBytes = static_cast<std::uint32_t>(ibInfo.stride * ibInfo.elementsCount);
		renderer->_indexBufferView.BufferLocation = ib->GetNativeGPUAddressWithRequiredOffset();

		renderer->_vertexBufferViews.clear();
		renderer->_vertexInputLayout.clear();
		renderer->_vertexBuffers.clear();

		std::uint32_t currentBufferSlotIndex = 0;

		renderer->ForEachAvailableVertexAttributes([&currentBufferSlotIndex, &renderer](const auto& vbInfo, const auto& attr)
		{
				const auto vb = std::dynamic_pointer_cast<ResourceEntity>(vbInfo.ptr->GetUnderlyingBuffer());
				assert(vb);

				D3D12_VERTEX_BUFFER_VIEW view{};

				view.SizeInBytes = static_cast<std::uint32_t>(vbInfo.stride * vbInfo.elementsCount);
				view.StrideInBytes = static_cast<std::uint32_t>(vbInfo.stride);
				view.BufferLocation = vb->GetNativeGPUAddressWithRequiredOffset();

				renderer->_vertexBufferViews.push_back(view);
				renderer->_vertexInputLayout.push_back({ GetSemanticsName(attr.type), static_cast<std::uint32_t>(attr.index), GetVertexBufferFormat(vbInfo.format), currentBufferSlotIndex, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
				renderer->_vertexBuffers.push_back(vb);

				++currentBufferSlotIndex;
		});
	}


	Mesh::Renderer::Renderer(const Settings& settings, const std::shared_ptr<Core::Mesh>& mesh, const std::shared_ptr<Core::Node>& node) : Core::Mesh::Renderer(settings, mesh, node)
	{
	}

	std::shared_ptr<Core::UniformBuffer<Core::Mesh::Renderer::Data>> Mesh::Renderer::CreateUniformBuffer()
	{
		return std::make_shared<UniformBuffer<Core::Mesh::Renderer::Data>>();
	}
}
