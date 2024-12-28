#pragma once
#include <Core/Mesh.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Mesh final : public Core::Mesh
	{
	public:
		Mesh(Core::GeometryPrototype&& proto);
	protected:
		std::shared_ptr<Core::BaseTask> CreateInternalInitializationTask() override;
		std::shared_ptr<Core::VertexBuffer> CreateVertexBuffer(const Core::VertexBufferPrototype* vbPrototype) override;
		std::shared_ptr<Core::IndexBuffer> CreateIndexBuffer(const Core::IndexBufferPrototype* ibPrototype) override;
	};
}