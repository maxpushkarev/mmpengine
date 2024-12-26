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
			void Run(const std::shared_ptr<BaseStream>& stream) override;
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
	};
}
