#include <cassert>
#include <Backend/Metal/Mesh.hpp>
#include <Backend/Metal/Buffer.hpp>

namespace MMPEngine::Backend::Metal
{
    Mesh::Mesh(Core::GeometryPrototype&& proto) : Core::Mesh(std::move(proto))
    {
    }

    std::shared_ptr<Core::BaseTask> Mesh::CreateInternalInitializationTask()
    {
        return Core::BaseTask::kEmpty;
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

    std::shared_ptr<Core::BaseTask> Mesh::Renderer::CreateInternalInitializationTask()
    {
        const auto tc = std::make_shared<InitTaskContext>();
        tc->entity = std::dynamic_pointer_cast<Renderer>(shared_from_this());
        return std::make_shared<InitTask>(tc);
    }

    std::shared_ptr<Metal::Buffer> Mesh::Renderer::GetIndexBuffer() const
    {
        return _indexBuffer;
    }

    const std::vector<std::shared_ptr<Metal::Buffer>>& Mesh::Renderer::GetVertexBuffers() const
    {
        return _vertexBuffers;
    }

    MTL::IndexType Mesh::Renderer::GetNativeIndexType() const
    {
        switch (GetMesh()->GetIndexBufferInfo().format) {
            case Core::IndexBufferPrototype::Format::Uint16:
                return MTL::IndexTypeUInt16;
            case Core::IndexBufferPrototype::Format::Uint32:
                return MTL::IndexTypeUInt32;
            default:
                throw Core::UnsupportedException("unsupported Metal index buffer format");
        }
        
    }
    
    MTL::PrimitiveType Mesh::Renderer::GetNativePrimitiveType() const
    {
        switch (const auto topology = GetMesh()->GetTopology())
        {
        case Core::GeometryPrototype::Topology::Triangles:
            return MTL::PrimitiveTypeTriangle;
        default:
            throw Core::UnsupportedException("unsupported Metal primitive type");;
        }
    }

    MTL::PrimitiveTopologyClass Mesh::Renderer::GetNativePrimitiveTopologyClass() const
    {
        switch (const auto topology = GetMesh()->GetTopology())
        {
        case Core::GeometryPrototype::Topology::Triangles:
            return MTL::PrimitiveTopologyClassTriangle;
        default:
            return MTL::PrimitiveTopologyClassUnspecified;
        }
    }

    MTL::VertexDescriptor* Mesh::Renderer::GetNativeVertexDescriptor() const
    {
        return _mtlVertexDescriptor.get();
    }

    std::shared_ptr<Metal::Buffer> Mesh::Renderer::GetNativeIndexBuffer() const
    {
        return _indexBuffer;
    }

    const std::vector<std::shared_ptr<Metal::Buffer>>& Mesh::Renderer::GetNativeVertexBuffers() const
    {
        return _vertexBuffers;
    }

    MTL::VertexFormat Mesh::Renderer::GetVertexFormat(Core::VertexBufferPrototype::Format format)
    {
        switch (format)
        {
        case Core::VertexBufferPrototype::Format::Float1:
            return MTL::VertexFormatFloat;
        case Core::VertexBufferPrototype::Format::Float2:
            return MTL::VertexFormatFloat2;
        case Core::VertexBufferPrototype::Format::Float3:
            return MTL::VertexFormatFloat3;
        case Core::VertexBufferPrototype::Format::Float4:
            return MTL::VertexFormatFloat4;
        case Core::VertexBufferPrototype::Format::Uint4:
            return MTL::VertexFormatUInt4;
        default:
            throw Core::UnsupportedException("unsupported Metal vertex buffer format");
        }
    }

    Mesh::Renderer::Renderer(const Settings& settings, const std::shared_ptr<Core::Mesh>& mesh, const std::shared_ptr<Core::Node>& node) : Core::Mesh::Renderer(settings, mesh, node)
    {
    }

    std::shared_ptr<Core::UniformBuffer<Core::Mesh::Renderer::Data>> Mesh::Renderer::CreateUniformBuffer()
    {
        return std::make_shared<UniformBuffer<Core::Mesh::Renderer::Data>>();
    }



    Mesh::Renderer::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task(ctx)
    {
    }

    void Mesh::Renderer::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);

        const auto renderer = GetTaskContext()->entity;
        assert(renderer);
        
        renderer->_mtlVertexDescriptor = NS::TransferPtr(MTL::VertexDescriptor::alloc()->init());
        
        renderer->ForEachAvailableVertexAttributes([&renderer](const auto& vbInfo, const auto&){
            const auto vb = std::dynamic_pointer_cast<Buffer>(vbInfo.ptr->GetUnderlyingBuffer());
            
            const auto bufferIndex = static_cast<NS::UInteger>(renderer->_vertexBuffers.size());
            
            auto attribute = renderer->_mtlVertexDescriptor ->attributes()->object(bufferIndex);
            auto layout = renderer->_mtlVertexDescriptor->layouts()->object(bufferIndex);
            
            attribute->setFormat(GetVertexFormat(vbInfo.format));
            attribute->setOffset(0U);
            attribute->setBufferIndex(bufferIndex);
            
            layout->setStepRate(1);
            layout->setStepFunction(MTL::VertexStepFunctionPerVertex);
            layout->setStride(static_cast<NS::UInteger>(vbInfo.stride));
            
            assert(vb);
            renderer->_vertexBuffers.push_back(vb);
        });

        const auto& ibInfo = renderer->GetMesh()->GetIndexBufferInfo();
        const auto ib = std::dynamic_pointer_cast<Buffer>(ibInfo.ptr->GetUnderlyingBuffer());
        assert(ib);

        renderer->_indexBuffer = ib;
    }
}
