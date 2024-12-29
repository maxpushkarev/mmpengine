#pragma once
#include <Core/Base.hpp>
#include <Core/Geometry.hpp>
#include <Core/Context.hpp>
#include <Core/Task.hpp>
#include <Core/Buffer.hpp>

namespace MMPEngine::Core
{
	class Mesh : public IInitializationTaskSource, public std::enable_shared_from_this<Mesh>
	{
	private:
		class InitTaskContext final : public TaskContext
		{
		public:
			std::shared_ptr<Mesh> mesh;
		};

		class CreateBuffers final : public ContextualTask<InitTaskContext>
		{
		public:
			CreateBuffers(const std::shared_ptr<InitTaskContext>& ctx);
			void OnScheduled(const std::shared_ptr<BaseStream>& stream) override;
		};
	public:
		Mesh(GeometryPrototype&& proto);
		std::shared_ptr<BaseTask> CreateInitializationTask() override;
		const std::vector<GeometryPrototype::Subset>& GetSubsets() const;
	protected:
		virtual std::shared_ptr<BaseTask> CreateInternalInitializationTask() = 0;
		virtual std::shared_ptr<VertexBuffer> CreateVertexBuffer(const VertexBufferPrototype* vbPrototype) = 0;
		virtual std::shared_ptr<IndexBuffer> CreateIndexBuffer(const IndexBufferPrototype* ibPrototype) = 0;
	protected:

		struct VertexBufferInfo final
		{
			std::shared_ptr<VertexBuffer> ptr;
			VertexBufferPrototype::Settings settings;
			VertexBufferPrototype::Format format;
			std::size_t stride;
			std::size_t elementsCount;
		};

		struct IndexBufferInfo final
		{
			std::shared_ptr<IndexBuffer> ptr;
			IndexBufferPrototype::Settings settings;
			IndexBufferPrototype::Format format;
			std::size_t stride;
			std::size_t elementsCount;
		};

		GeometryPrototype _proto;
		std::unordered_map<VertexBufferPrototype::Semantics, std::vector<VertexBufferInfo>> _vertexBufferInfos;
		IndexBufferInfo _indexBufferInfo;
		GeometryPrototype::Topology _topology;
		std::vector<GeometryPrototype::Subset> _subsets;

	public:
		const VertexBufferInfo& GetVertexBufferInfo(VertexBufferPrototype::Semantics semantics, std::size_t semanticIndex) const;
		const IndexBufferInfo& GetIndexBufferInfo() const;

		struct RendererData final
		{
			Matrix4x4 localToWorldMatrix;
			Matrix4x4 localToWorldMatrixIT;
		};

		class Renderer : public IInitializationTaskSource, public std::enable_shared_from_this<Renderer>
		{
		private:
			class InitTaskContext final : public TaskContext
			{
			public:
				std::shared_ptr<Renderer> renderer;
			};
			class InitTask final : public ContextualTask<InitTaskContext>
			{
			public:
				InitTask(const std::shared_ptr<InitTaskContext>& ctx);
				void OnScheduled(const std::shared_ptr<BaseStream>& stream) override;
			};
			void FillData(const std::shared_ptr<GlobalContext>& globalContext, RendererData& data) const;
		public:
			Renderer(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Node>& node);
			std::shared_ptr<BaseTask> CreateInitializationTask() override;
			virtual std::shared_ptr<Mesh> GetMesh() const;
			virtual std::shared_ptr<Node> GetNode() const;
			virtual std::shared_ptr<BaseEntity> GetUniformDataEntity() const;
		protected:
			virtual std::shared_ptr<UniformBuffer<RendererData>> CreateUniformBuffer() = 0;
		private:
			std::shared_ptr<Mesh> _mesh;
			std::shared_ptr<Node> _node;
			std::shared_ptr<UniformBuffer<RendererData>> _uniformBuffer;
			std::shared_ptr<ContextualTask<UniformBuffer<RendererData>::WriteTaskContext>> _uniformBufferWriteTask;
		};
	};
}
