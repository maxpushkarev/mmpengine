#pragma once
#include <Core/Mesh.hpp>

namespace MMPEngine::Frontend
{
	class Mesh final : public Core::Mesh
	{
	public:
		Mesh(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::GeometryPrototype&& proto);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<const Core::Mesh> GetUnderlyingMesh() const override;

		const std::map<Core::VertexBufferPrototype::Semantics, std::vector<VertexBufferInfo>>& GetAllVertexBufferInfos() const override;
		const VertexBufferInfo& GetVertexBufferInfo(Core::VertexBufferPrototype::Semantics semantics, std::size_t semanticIndex) const override;
		const IndexBufferInfo& GetIndexBufferInfo() const override;
		const std::vector<Core::GeometryPrototype::Subset>& GetSubsets() const override;
		Core::GeometryPrototype::Topology GetTopology() const override;

		class Renderer final : public Core::Mesh::Renderer
		{
		public:
			Renderer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings, const std::shared_ptr<Core::Mesh>& mesh, const std::shared_ptr<Core::Node>& node);
			std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
			std::shared_ptr<Core::BaseEntity> GetUniformDataEntity() const override;
			std::shared_ptr<Core::ContextualTask<UpdateDataTaskContext>> CreateTaskToUpdateAndWriteUniformData() override;
		protected:
			std::shared_ptr<Core::UniformBuffer<Data>> CreateUniformBuffer() override;
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
