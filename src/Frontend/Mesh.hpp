#pragma once
#include <Core/Mesh.hpp>

namespace MMPEngine::Frontend
{
	class Mesh final : public Core::Mesh
	{
	public:
		Mesh(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::GeometryPrototype&& proto);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	protected:
		std::shared_ptr<Core::BaseTask> CreateInternalInitializationTask() override;
		std::shared_ptr<Core::VertexBuffer> CreateVertexBuffer(const Core::VertexBufferPrototype* vbPrototype) override;
		std::shared_ptr<Core::IndexBuffer> CreateIndexBuffer(const Core::IndexBufferPrototype* ibPrototype) override;
	private:
		std::shared_ptr<Core::Mesh> _impl;
	};
}