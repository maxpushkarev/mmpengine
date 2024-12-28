#include <Backend/Dx12/Mesh.hpp>
#include <Backend/Dx12/Buffer.hpp>

namespace MMPEngine::Backend::Dx12
{
	Mesh::Mesh(Core::GeometryPrototype&& proto) : Core::Mesh(std::move(proto))
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
	}

}
