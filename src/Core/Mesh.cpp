#include <Core/Mesh.hpp>
#include <Core/Task.hpp>

namespace MMPEngine::Core
{
	Mesh::Mesh(GeometryPrototype&& proto) : _proto(std::move(proto)), _indexBufferInfo {}, _topology(GeometryPrototype::Topology::Triangles)
	{
	}

	Mesh::CreateBuffers::CreateBuffers(const std::shared_ptr<InitTaskContext>& ctx) : ContextualTask(ctx)
	{
	}

	void Mesh::CreateBuffers::OnScheduled(const std::shared_ptr<BaseStream>& stream)
	{
		ContextualTask::OnScheduled(stream);
		const auto mesh = GetTaskContext()->mesh;

		for(const auto& vbProto : mesh->_proto.vertexBuffers)
		{
			const auto vertexBuffer = mesh->CreateVertexBuffer(vbProto.get());

			auto& semanticBuffers = mesh->_vertexBufferInfos[vbProto->GetVBSettings().semantics];
			semanticBuffers.push_back(VertexBufferInfo{
				vertexBuffer,
	{vbProto->GetVBSettings(), vbProto->GetSettings()},
				vbProto->GetFormat(),
				vbProto->GetStride(),
				vbProto->GetElementsCount()
			});
		}

		const auto& ibProto = mesh->_proto.indexBuffer;
		const auto indexBuffer = mesh->CreateIndexBuffer(ibProto.get());
		mesh->_indexBufferInfo = IndexBufferInfo {
			indexBuffer,
			ibProto->GetSettings(),
			ibProto->GetFormat(),
			ibProto->GetStride(),
			ibProto->GetElementsCount()
		};
	}


	std::shared_ptr<BaseTask> Mesh::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->mesh = shared_from_this();

		return std::make_shared<BatchTask>(std::initializer_list<std::shared_ptr<BaseTask>>{
			std::make_shared<CreateBuffers>(ctx),
			std::make_shared<FunctionalTask>(
					[ctx](const auto& stream)
					{
						ctx->mesh->_topology = ctx->mesh->_proto.topology;
						ctx->mesh->_subsets = ctx->mesh->_proto.subsets;

						for(const auto& vbInfos : ctx->mesh->_vertexBufferInfos)
						{
							for(const auto& vbInfo : vbInfos.second)
							{
								stream->Schedule(vbInfo.ptr->CreateInitializationTask());
							}
						}

						stream->Schedule(ctx->mesh->_indexBufferInfo.ptr->CreateInitializationTask());
					},
					FunctionalTask::Handler {},
					FunctionalTask::Handler {}
			),
			CreateInternalInitializationTask(),
			std::make_shared<FunctionalTask>(
				FunctionalTask::Handler {},
				FunctionalTask::Handler {},
				[ctx](const auto&)
				{
					GeometryPrototype empty{};
					std::swap(ctx->mesh->_proto, empty);
				}
			)
		});
	}

	const std::vector<GeometryPrototype::Subset>& Mesh::GetSubsets() const
	{
		return _subsets;
	}

	const Mesh::VertexBufferInfo& Mesh::GetVertexBufferInfo(VertexBufferPrototype::Semantics semantics, std::size_t semanticIndex) const
	{
		return _vertexBufferInfos.at(semantics).at(semanticIndex);
	}

	const Mesh::IndexBufferInfo& Mesh::GetIndexBufferInfo() const
	{
		return _indexBufferInfo;
	}

}
