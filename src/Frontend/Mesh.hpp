#pragma once
#include <Core/Mesh.hpp>

namespace MMPEngine::Frontend
{
	class Mesh final : public Core::Mesh
	{
	public:
		Mesh(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::GeometryPrototype&& proto);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;

		class Renderer final : public Core::Mesh::Renderer
		{
		public:
			Renderer(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Core::Node>& node);
			std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
			std::shared_ptr<Core::Node> GetNode() const override;
			std::shared_ptr<Core::Mesh> GetMesh() const override;
			std::shared_ptr<Core::BaseEntity> GetUniformDataEntity() const override;
			std::shared_ptr<Core::ContextualTask<UpdateDataTaskContext>> CreateTaskToUpdateAndWriteUniformData() override;
		protected:
			std::shared_ptr<Core::UniformBuffer<RendererData>> CreateUniformBuffer() override;
		private:
			std::shared_ptr<Core::Mesh::Renderer> _impl;
		};

	protected:
		std::shared_ptr<Core::BaseTask> CreateInternalInitializationTask() override;
		std::shared_ptr<Core::VertexBuffer> CreateVertexBuffer(const Core::VertexBufferPrototype* vbPrototype) override;
		std::shared_ptr<Core::IndexBuffer> CreateIndexBuffer(const Core::IndexBufferPrototype* ibPrototype) override;
	private:
		std::shared_ptr<Core::Mesh> _impl;
	};
}
