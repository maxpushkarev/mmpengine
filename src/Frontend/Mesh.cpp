#include <Frontend/Mesh.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Mesh.hpp>
#endif


namespace MMPEngine::Frontend
{
	Mesh::Mesh(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::GeometryPrototype&& proto) : Core::Mesh(Core::GeometryPrototype {})
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::Mesh>(std::move(proto));
#else
			throw Core::UnsupportedException("unable to create mesh for DX12 backend");
#endif
		}
	}

	std::shared_ptr<Core::BaseTask> Mesh::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	std::shared_ptr<Core::IndexBuffer> Mesh::CreateIndexBuffer(const Core::IndexBufferPrototype* ibPrototype)
	{
		throw std::logic_error("impossible exception");
	}

	std::shared_ptr<Core::VertexBuffer> Mesh::CreateVertexBuffer(const Core::VertexBufferPrototype* vbPrototype)
	{
		throw std::logic_error("impossible exception");
	}

	std::shared_ptr<Core::BaseTask> Mesh::CreateInternalInitializationTask()
	{
		throw std::logic_error("impossible exception");
	}

}