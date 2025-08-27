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

	GeometryPrototype::Topology Mesh::GetTopology() const
	{
		return _topology;
	}

	std::shared_ptr<const Mesh> Mesh::GetUnderlyingMesh() const
	{
		return shared_from_this();
	}

	const Mesh::VertexBufferInfo& Mesh::GetVertexBufferInfo(VertexBufferPrototype::Semantics semantics, std::size_t semanticIndex) const
	{
		return _vertexBufferInfos.at(semantics).at(semanticIndex);
	}

	const std::map<VertexBufferPrototype::Semantics, std::vector<Mesh::VertexBufferInfo>>& Mesh::GetAllVertexBufferInfos() const
	{
		return _vertexBufferInfos;
	}


	const Mesh::IndexBufferInfo& Mesh::GetIndexBufferInfo() const
	{
		return _indexBufferInfo;
	}

	Mesh::Renderer::Renderer(const Settings& settings, const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Node>& node) : _settings(settings), _mesh(mesh), _node(node)
	{
	}

	std::shared_ptr<BaseTask> Mesh::Renderer::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<RendererTaskContext>();
		ctx->renderer = shared_from_this();
		return std::make_shared<Core::BatchTask>(std::initializer_list<std::shared_ptr<Core::BaseTask>>{
			std::make_shared<InitTask>(ctx),
			std::make_shared<FunctionalTask>(
				FunctionalTask::Handler {},
				[ctx](const auto& stream)
				{
					if(ctx->renderer->_settings.staticData.manageUniformData)
					{
						Data data{};
						ctx->renderer->FillData(stream->GetGlobalContext(), data);
						ctx->renderer->_uniformBufferWriteTask = ctx->renderer->_uniformBuffer->CreateWriteAsyncTask(data);
					}
				},
				FunctionalTask::Handler {}
			)
		});
	}

	Mesh::Renderer::InitTask::InitTask(const std::shared_ptr<RendererTaskContext>& ctx) : ContextualTask<MMPEngine::Core::Mesh::Renderer::RendererTaskContext>(ctx)
	{
	}

	void Mesh::Renderer::InitTask::OnScheduled(const std::shared_ptr<BaseStream>& stream)
	{
		ContextualTask::OnScheduled(stream);

		const auto renderer = GetTaskContext()->renderer;

		if(renderer->_settings.staticData.manageUniformData)
		{
			renderer->_uniformBuffer = renderer->CreateUniformBuffer();
			stream->Schedule(renderer->_uniformBuffer->CreateInitializationTask());
		}
	}

	std::shared_ptr<const Mesh> Mesh::Renderer::GetMesh() const
	{
		return _mesh;
	}

	std::shared_ptr<const Node> Mesh::Renderer::GetNode() const
	{
		return _node;
	}

	const Mesh::Renderer::Settings& Mesh::Renderer::GetSettings() const
	{
		return _settings;
	}

    Mesh::Renderer::Settings::Dynamic& Mesh::Renderer::GetDynamicSettings()
    {
        return _settings.dynamicData;
    }

	std::shared_ptr<BaseEntity> Mesh::Renderer::GetUniformDataEntity() const
	{
		return _uniformBuffer;
	}

    bool Mesh::Renderer::IsActive() const
    {
        return _settings.dynamicData.instancesCount > 0;
    }

	void Mesh::Renderer::FillData(const std::shared_ptr<GlobalContext>& globalContext, Data& data) const
	{
		globalContext->math->CalculateLocalToWorldSpaceMatrix(data.localToWorldMatrix, GetNode());
		globalContext->math->InverseTranspose(data.localToWorldMatrixIT, data.localToWorldMatrix);
	}

	std::shared_ptr<ContextualTask<Mesh::Renderer::UpdateDataTaskContext>> Mesh::Renderer::CreateTaskToUpdateAndWriteUniformData()
	{
		const auto ctx = std::make_shared<InternalUpdateDataTaskContext>();
		ctx->renderer = shared_from_this();
		return std::make_shared<InternalUpdateDataTask>(ctx);
	}


	Mesh::Renderer::InternalUpdateDataTask::InternalUpdateDataTask(const std::shared_ptr<InternalUpdateDataTaskContext>& ctx) : ContextualTask(ctx), _internalContext(ctx)
	{
	}

	void Mesh::Renderer::InternalUpdateDataTask::OnScheduled(const std::shared_ptr<BaseStream>& stream)
	{
		ContextualTask::OnScheduled(stream);
		const auto ctx = _internalContext;

		if(ctx->renderer->_settings.staticData.manageUniformData)
		{
			if (ctx->precomputed.has_value())
			{
				ctx->renderer->_uniformBufferWriteTask->GetTaskContext()->data = ctx->precomputed.value();
			}
			else
			{
				ctx->renderer->FillData(stream->GetGlobalContext(), ctx->renderer->_uniformBufferWriteTask->GetTaskContext()->data);
			}

			stream->Schedule(ctx->renderer->_uniformBufferWriteTask);
		}
	}

}
