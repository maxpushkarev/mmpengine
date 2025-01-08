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

	std::shared_ptr<Core::Mesh> Mesh::GetUnderlyingMesh()
	{
		return _impl;
	}

	const Mesh::IndexBufferInfo& Mesh::GetIndexBufferInfo() const
	{
		return _impl->GetIndexBufferInfo();
	}

	Core::GeometryPrototype::Topology Mesh::GetTopology() const
	{
		return _impl->GetTopology();
	}

	const std::vector<Core::GeometryPrototype::Subset>& Mesh::GetSubsets() const
	{
		return _impl->GetSubsets();
	}

	const Mesh::VertexBufferInfo& Mesh::GetVertexBufferInfo(Core::VertexBufferPrototype::Semantics semantics, std::size_t semanticIndex) const
	{
		return _impl->GetVertexBufferInfo(semantics, semanticIndex);
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

	Mesh::Renderer::Renderer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings, const std::shared_ptr<Core::Mesh>& mesh, const std::shared_ptr<Core::Node>& node)
		: Core::Mesh::Renderer(settings, mesh, node)
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::Mesh::Renderer>(settings, mesh, node);
#else
			throw Core::UnsupportedException("unable to create mesh renderer for DX12 backend");
#endif
		}
	}

	std::shared_ptr<Core::UniformBuffer<Mesh::Renderer::Data>> Mesh::Renderer::CreateUniformBuffer()
	{
		throw std::logic_error("impossible exception");
	}

	std::shared_ptr<Core::BaseTask> Mesh::Renderer::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	std::shared_ptr<Core::BaseEntity> Mesh::Renderer::GetUniformDataEntity() const
	{
		return _impl->GetUniformDataEntity();
	}

	std::shared_ptr<Core::ContextualTask<Mesh::Renderer::UpdateDataTaskContext>> Mesh::Renderer::CreateTaskToUpdateAndWriteUniformData()
	{
		return _impl->CreateTaskToUpdateAndWriteUniformData();
	}

}