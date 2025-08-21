#include <cassert>
#include <Backend/Metal/Mesh.hpp>
#include <Backend/Metal/Buffer.hpp>

namespace MMPEngine::Backend::Metal
{
    Mesh::Mesh(Core::GeometryPrototype&& proto) : Core::Mesh(std::move(proto))
    {
    }

    std::shared_ptr<Core::VertexBuffer> Mesh::CreateVertexBuffer(const Core::VertexBufferPrototype* vbPrototype)
    {
        return std::make_shared<VertexBuffer>(Core::InputAssemblerBuffer::Settings{
        {vbPrototype->GetDataPtr()}, {vbPrototype->GetByteLength()}
            });
    }

    std::shared_ptr<Core::IndexBuffer> Mesh::CreateIndexBuffer(const Core::IndexBufferPrototype* ibPrototype)
    {
        return std::make_shared<IndexBuffer>(Core::InputAssemblerBuffer::Settings{
            {ibPrototype->GetDataPtr()}, { ibPrototype->GetByteLength() }
            });
    }

    std::shared_ptr<Core::BaseTask> Mesh::CreateInternalInitializationTask()
    {
        const auto tc = std::make_shared<InitTaskContext>();
        tc->mesh = std::dynamic_pointer_cast<Mesh>(shared_from_this());
        return std::make_shared<InitTask>(tc);
    }

    std::shared_ptr<Metal::Buffer> Mesh::GetIndexBuffer() const
    {
        return _indexBuffer;
    }

    const std::vector<std::shared_ptr<Metal::Buffer>>& Mesh::GetVertexBuffers() const
    {
        return _vertexBuffers;
    }

    Mesh::Renderer::Renderer(const Settings& settings, const std::shared_ptr<Core::Mesh>& mesh, const std::shared_ptr<Core::Node>& node) : Core::Mesh::Renderer(settings, mesh, node)
    {
    }

    std::shared_ptr<Core::UniformBuffer<Core::Mesh::Renderer::Data>> Mesh::Renderer::CreateUniformBuffer()
    {
        return std::make_shared<UniformBuffer<Core::Mesh::Renderer::Data>>();
    }



    Mesh::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task(ctx)
    {
    }

    void Mesh::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);

        const auto mesh = GetTaskContext()->mesh;
        assert(mesh);

        for (const auto& vbInfos : mesh->_vertexBufferInfos)
        {
            for (std::size_t semanticIndex = 0; semanticIndex < vbInfos.second.size(); ++semanticIndex)
            {
                const auto& vbInfo = vbInfos.second.at(semanticIndex);
                const auto vb = std::dynamic_pointer_cast<Buffer>(vbInfo.ptr->GetUnderlyingBuffer());
                assert(vb);
                mesh->_vertexBuffers.push_back(vb);
            }
        }

        const auto& ibInfo = mesh->_indexBufferInfo;
        const auto ib = std::dynamic_pointer_cast<Buffer>(ibInfo.ptr->GetUnderlyingBuffer());
        assert(ib);

        mesh->_indexBuffer = ib;
    }
}
