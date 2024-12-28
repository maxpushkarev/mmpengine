#pragma once
#include <Core/Entity.hpp>
#include <Core/Geometry.hpp>
#include <Core/Context.hpp>
#include <Core/Task.hpp>
#include <Core/Buffer.hpp>

namespace MMPEngine::Core
{
	class Mesh : public BaseEntity
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
		Mesh(std::string_view name, GeometryPrototype&& proto);
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
	};
}
