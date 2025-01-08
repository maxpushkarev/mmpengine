#pragma once
#include <optional>
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
		virtual std::shared_ptr<const Mesh> GetUnderlyingMesh() const;
		virtual const VertexBufferInfo& GetVertexBufferInfo(VertexBufferPrototype::Semantics semantics, std::size_t semanticIndex) const;
		virtual const IndexBufferInfo& GetIndexBufferInfo() const;
		virtual const std::vector<GeometryPrototype::Subset>& GetSubsets() const;
		virtual GeometryPrototype::Topology GetTopology() const;

		class Renderer : public IInitializationTaskSource, public IGeometryRenderer, public std::enable_shared_from_this<Renderer>
		{
		public:
			struct Data final
			{
				Matrix4x4 localToWorldMatrix;
				Matrix4x4 localToWorldMatrixIT;
			};
			struct Settings final
			{
				struct Static final
				{
					bool manageUniformData = true;
				};
				struct Dynamic final
				{
					std::size_t instancesCount = 1;
				};

				Static staticData {};
				Dynamic dynamicData {};
			};
			class UpdateDataTaskContext : public TaskContext
			{
			public:
				std::optional<Data> precomputed = std::nullopt;
			};
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

			class InternalUpdateDataTaskContext final : public UpdateDataTaskContext
			{
			public:
				std::shared_ptr<Renderer> renderer;
			};

			class InternalUpdateDataTask final : public ContextualTask<UpdateDataTaskContext>
			{
			public:
				InternalUpdateDataTask(const std::shared_ptr<InternalUpdateDataTaskContext>& ctx);
				void OnScheduled(const std::shared_ptr<BaseStream>& stream) override;
			private:
				std::shared_ptr<InternalUpdateDataTaskContext> _internalContext;
			};

			void FillData(const std::shared_ptr<GlobalContext>& globalContext, Data& data) const;
		public:
			Renderer(const Settings& settings, const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Node>& node);
			std::shared_ptr<BaseTask> CreateInitializationTask() override;
			std::shared_ptr<const Mesh> GetMesh() const;
			std::shared_ptr<const Node> GetNode() const;
			virtual std::shared_ptr<BaseEntity> GetUniformDataEntity() const;
			virtual std::shared_ptr<ContextualTask<UpdateDataTaskContext>> CreateTaskToUpdateAndWriteUniformData();
		protected:
			virtual std::shared_ptr<UniformBuffer<Data>> CreateUniformBuffer() = 0;
			Settings _settings;
		private:
			std::shared_ptr<Mesh> _mesh;
			std::shared_ptr<Node> _node;
			std::shared_ptr<UniformBuffer<Data>> _uniformBuffer;
			std::shared_ptr<ContextualTask<UniformBuffer<Data>::WriteTaskContext>> _uniformBufferWriteTask;
		};
	};
}
