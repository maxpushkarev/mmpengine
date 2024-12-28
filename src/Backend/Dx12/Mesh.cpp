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
		return Core::BaseTask::kEmpty;
	}

}
