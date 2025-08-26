#pragma once
#include <Core/Mesh.hpp>
#include <Backend/Metal/Task.hpp>
#include <Backend/Metal/Buffer.hpp>
#include <Core/Node.hpp>

namespace MMPEngine::Backend::Metal
{
    class Mesh final : public Core::Mesh
    {
    private:
        class InitTaskContext final : public Core::TaskContext
        {
        public:
            std::shared_ptr<Mesh> mesh;
        };

        class InitTask final : public Task<InitTaskContext>
        {
        public:
            InitTask(const std::shared_ptr<InitTaskContext>& ctx);
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
        };

    public:
        Mesh(Core::GeometryPrototype&& proto);

        std::shared_ptr<Metal::Buffer> GetIndexBuffer() const;
        const std::vector<std::shared_ptr<Metal::Buffer>>& GetVertexBuffers() const;

        class Renderer final : public Core::Mesh::Renderer
        {
        public:
            Renderer(const Settings& settings, const std::shared_ptr<Core::Mesh>& mesh, const std::shared_ptr<Core::Node>& node);
        protected:
            std::shared_ptr<Core::UniformBuffer<Data>> CreateUniformBuffer() override;
        };

        MTL::VertexDescriptor* GetNativeVertexDescriptor() const;
        MTL::IndexType GetNativeIndexType() const;
        
    protected:
        std::shared_ptr<Core::BaseTask> CreateInternalInitializationTask() override;
        std::shared_ptr<Core::VertexBuffer> CreateVertexBuffer(const Core::VertexBufferPrototype* vbPrototype) override;
        std::shared_ptr<Core::IndexBuffer> CreateIndexBuffer(const Core::IndexBufferPrototype* ibPrototype) override;
    private:

        static MTL::VertexFormat GetVertexFormat(Core::VertexBufferPrototype::Format format);
            
        std::shared_ptr<Metal::Buffer> _indexBuffer;
        std::vector<std::shared_ptr<Metal::Buffer>> _vertexBuffers;
        NS::SharedPtr<MTL::VertexDescriptor> _mtlVertexDescriptor;
    };
}

