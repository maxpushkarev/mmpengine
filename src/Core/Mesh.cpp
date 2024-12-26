#include <Core/Mesh.hpp>
#include <Core/Task.hpp>

namespace MMPEngine::Core
{
	Mesh::Mesh(GeometryPrototype&& proto) : _proto(std::move(proto))
	{
	}

	Mesh::CreateBuffers::CreateBuffers(const std::shared_ptr<InitTaskContext>& ctx) : ContextualTask(ctx)
	{
	}

	void Mesh::CreateBuffers::Run(const std::shared_ptr<BaseStream>& stream)
	{
		ContextualTask::Run(stream);
		const auto mesh = GetTaskContext()->mesh;

		for(const auto& vbProto : mesh->_proto.vertexBuffers)
		{
			const auto vertexBuffer = mesh->CreateVertexBuffer(vbProto.get());

			auto& semanticBuffers = mesh->_vertexBufferInfos[vbProto->GetVBSettings().semantics];
			semanticBuffers.push_back(VertexBufferInfo{
				vertexBuffer,
	{vbProto->GetVBSettings(), vbProto->GetSettings()},
				vbProto->GetStride(),
				vbProto->GetElementsCount()
			});
		}

		const auto indexBuffer = mesh->CreateIndexBuffer(mesh->_proto.indexBuffer.get());
		mesh->_indexBufferInfo = IndexBufferInfo {
			indexBuffer,
			mesh->_proto.indexBuffer->GetSettings(),
			mesh->_proto.indexBuffer->GetStride(),
			mesh->_proto.indexBuffer->GetElementsCount()
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
						for(const auto& vbInfos : ctx->mesh->_vertexBufferInfos)
						{
							for(const auto& vbInfo : vbInfos.second)
							{
								stream->Schedule(vbInfo.ptr->CreateInitializationTask());
							}
						}

						stream->Schedule(ctx->mesh->_indexBufferInfo.ptr->CreateInitializationTask());
					},
					[ctx](const auto&)
					{
						GeometryPrototype empty{};
						std::swap(ctx->mesh->_proto, empty);
					},
					FunctionalTask::Handler {}
			),
			CreateInternalInitializationTask()
		});
	}
}
