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

		for(const auto& vbInfos : mesh->_vertexBufferInfos)
		{
			const auto semantics = vbInfos.first;

			for(const auto& vbInfo : vbInfos.second)
			{
				const auto vb = std::dynamic_pointer_cast<ResourceEntity>(vbInfo.ptr->GetUnderlyingBuffer());
				assert(vb);

				D3D12_VERTEX_BUFFER_VIEW view {};

				view.SizeInBytes = static_cast<std::uint32_t>(vbInfo.stride * vbInfo.elementsCount);
				view.StrideInBytes = static_cast<std::uint32_t>(vbInfo.stride);
				view.BufferLocation = vb->GetNativeGPUAddressWithRequiredOffset();
				
				mesh->_vertexBufferViews.push_back(view);
			}
		}
	}

}
