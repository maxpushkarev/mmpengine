#include <cassert>
#include <Backend/Dx12/Mesh.hpp>
#include <Backend/Dx12/Buffer.hpp>

namespace MMPEngine::Backend::Dx12
{
	Mesh::Mesh(Core::GeometryPrototype&& proto) : Core::Mesh(std::move(proto)), _indexBufferView {}
	{
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

	const char* Mesh::GetSemanticsName(Core::VertexBufferPrototype::Semantics semantics)
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

	DXGI_FORMAT Mesh::GetVertexBufferFormat(Core::VertexBufferPrototype::Format format)
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

	std::shared_ptr<Core::BaseTask> Mesh::CreateInternalInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->mesh = std::dynamic_pointer_cast<Mesh>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	const std::vector<D3D12_VERTEX_BUFFER_VIEW>& Mesh::GetVertexBufferViews() const
	{
		return _vertexBufferViews;
	}

	const D3D12_INDEX_BUFFER_VIEW& Mesh::GetIndexBufferView() const
	{
		return _indexBufferView;
	}

	const std::vector<D3D12_INPUT_ELEMENT_DESC>& Mesh::GetVertexInputLayout() const
	{
		return _vertexInputLayout;
	}

	Mesh::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task(ctx)
	{
	}

	void Mesh::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto mesh = GetTaskContext()->mesh;
		assert(mesh);

		const auto& ibInfo = mesh->_indexBufferInfo;
		const auto ib = std::dynamic_pointer_cast<ResourceEntity>(ibInfo.ptr->GetUnderlyingBuffer());
		assert(ib);

		mesh->_indexBufferView.Format = (ibInfo.format == Core::IndexBufferPrototype::Format::Uint16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		mesh->_indexBufferView.SizeInBytes = static_cast<std::uint32_t>(ibInfo.stride * ibInfo.elementsCount);
		mesh->_indexBufferView.BufferLocation = ib->GetNativeGPUAddressWithRequiredOffset();

		mesh->_vertexBufferViews.clear();
		mesh->_vertexInputLayout.clear();

		mesh->_vertexInputLayout.reserve(mesh->_vertexBufferInfos.size());
		mesh->_vertexBufferViews.reserve(mesh->_vertexBufferInfos.size());

		std::uint32_t currentBufferSlotIndex = 0;

		for(const auto& vbInfos : mesh->_vertexBufferInfos)
		{
			const auto semantics = vbInfos.first;

			for(std::size_t semanticIndex = 0; semanticIndex < vbInfos.second.size(); ++semanticIndex)
			{
				const auto& vbInfo = vbInfos.second.at(semanticIndex);
				const auto vb = std::dynamic_pointer_cast<ResourceEntity>(vbInfo.ptr->GetUnderlyingBuffer());
				assert(vb);

				D3D12_VERTEX_BUFFER_VIEW view {};

				view.SizeInBytes = static_cast<std::uint32_t>(vbInfo.stride * vbInfo.elementsCount);
				view.StrideInBytes = static_cast<std::uint32_t>(vbInfo.stride);
				view.BufferLocation = vb->GetNativeGPUAddressWithRequiredOffset();
				
				mesh->_vertexBufferViews.push_back(view);
				mesh->_vertexInputLayout.push_back({ GetSemanticsName(semantics), static_cast<std::uint32_t>(semanticIndex), GetVertexBufferFormat(vbInfo.format), currentBufferSlotIndex, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

				++currentBufferSlotIndex;
			}
		}
	}


	Mesh::Renderer::Renderer(const Settings& settings, const std::shared_ptr<Core::Mesh>& mesh, const std::shared_ptr<Core::Node>& node) : Core::Mesh::Renderer(settings, mesh, node)
	{
	}

	std::shared_ptr<Core::UniformBuffer<Core::Mesh::Renderer::Data>> Mesh::Renderer::CreateUniformBuffer()
	{
		return std::make_shared<UniformBuffer<Core::Mesh::Renderer::Data>>();
	}

}
