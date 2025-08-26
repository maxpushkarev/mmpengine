#pragma once
#include <cassert>
#include <Core/DrawCall.hpp>
#include <Backend/Metal/Mesh.hpp>
#include <Backend/Metal/Shader.hpp>
#include <Backend/Metal/Job.hpp>
#include <Backend/Metal/Camera.hpp>
#include <Backend/Metal/Task.hpp>
#include <Backend/Metal/Texture.hpp>
#include <Backend/Metal/Buffer.hpp>

namespace MMPEngine::Backend::Metal
{
    class Camera::DrawCallsJob final : public Core::Camera::DrawCallsJob, public Metal::Job<Core::MeshMaterial>
    {
    private:

        class IterationImpl
        {
        protected:
            IterationImpl(const std::shared_ptr<DrawCallsJob>& job, const Item& item);
            Item _item;
            std::shared_ptr<DrawCallsJob> _drawCallsJob;
        };


        class InternalTaskContext final : public Core::TaskContext, public std::enable_shared_from_this<InternalTaskContext>
        {
        public:
            std::shared_ptr<DrawCallsJob> job;
            std::vector<std::shared_ptr<IColorTargetTexture>> colorRenderTargets;
            std::shared_ptr<IDepthStencilTexture> depthStencil;
        };

        class InitInternalTask final : public Task<InternalTaskContext>
        {
        public:
            InitInternalTask(const std::shared_ptr<InternalTaskContext>& ctx);
        protected:
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
        };

        class BeginPass final : public Task<InternalTaskContext>
        {
        public:
            BeginPass(const std::shared_ptr<InternalTaskContext>& ctx);
        protected:
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
        };

        class EndPass final : public Task<InternalTaskContext>
        {
        public:
            EndPass(const std::shared_ptr<InternalTaskContext>& ctx);
        protected:
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
        };

        template<typename TCoreMaterial>
        class IterationJob final : public Iteration, public IterationImpl, public Metal::Job<TCoreMaterial>
        {
        private:
            class TaskContext final : public Core::TaskContext
            {
            public:
                TaskContext(const std::shared_ptr<IterationJob>& job);
                std::shared_ptr<IterationJob> job;
                std::shared_ptr<Core::Mesh::Renderer> meshRenderer;
                std::shared_ptr<const Metal::Mesh> mesh;
            };

            class InitTask final : public Task<TaskContext>
            {
            public:
                InitTask(const std::shared_ptr<TaskContext>& ctx);
            protected:
                void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
            };

            class ExecutionTask final : public Task<TaskContext>
            {
            public:
                ExecutionTask(const std::shared_ptr<TaskContext>& ctx);
            protected:
                void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
            };

        public:
            IterationJob(const std::shared_ptr<DrawCallsJob>& job, const Item& item);
            IterationJob(const IterationJob&) = delete;
            IterationJob(IterationJob&&) noexcept = delete;
            IterationJob& operator=(const IterationJob&) = delete;
            IterationJob& operator=(IterationJob&&) noexcept = delete;
            ~IterationJob() override;
            std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
            std::shared_ptr<Core::BaseTask> CreateExecutionTask() override;
        private:
            NS::SharedPtr<MTL::RenderPipelineState> _pipelineState;
            NS::SharedPtr<MTL::DepthStencilState> _depthStencilState;
        };


    public:
        DrawCallsJob(const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items);
        ~DrawCallsJob() override;
        DrawCallsJob(const DrawCallsJob&) = delete;
        DrawCallsJob(DrawCallsJob&&) noexcept = delete;
        DrawCallsJob& operator=(const DrawCallsJob&) = delete;
        DrawCallsJob& operator=(DrawCallsJob&&) noexcept = delete;
        std::vector<std::shared_ptr<Core::BaseTask>>& GetMemoryBarrierTasks(Core::PassControl<true, IterationImpl>);
    protected:
        std::shared_ptr<Iteration> BuildIteration(const Item& item) const override;
        std::shared_ptr<Core::BaseTask> CreateTaskForIterationsStart() override;
        std::shared_ptr<Core::BaseTask> CreateInitializationTaskInternal() override;
        std::shared_ptr<Core::BaseTask> CreateTaskForIterationsFinish() override;
    private:
        std::shared_ptr<Camera::DrawCallsJob::InternalTaskContext> BuildInternalContext();
        MTL::RenderCommandEncoder* _renderCommandEncoder = nullptr;
    };


    template<typename TCoreMaterial>
    Camera::DrawCallsJob::IterationJob<TCoreMaterial>::IterationJob(const std::shared_ptr<DrawCallsJob>& job, const Item& item)
        : IterationImpl(job, item)
    {
    }

    template <typename TCoreMaterial>
    Camera::DrawCallsJob::IterationJob<TCoreMaterial>::~IterationJob<TCoreMaterial>() = default;

    template<typename TCoreMaterial>
    std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::IterationJob<TCoreMaterial>::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<TaskContext>(std::dynamic_pointer_cast<IterationJob>(shared_from_this()));
        return std::make_shared<InitTask>(ctx);
    }

    template<typename TCoreMaterial>
    std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::IterationJob<TCoreMaterial>::CreateExecutionTask()
    {
        const auto ctx = std::make_shared<TaskContext>(std::dynamic_pointer_cast<IterationJob>(shared_from_this()));
        return std::make_shared<ExecutionTask>(ctx);
    }

    template<typename TCoreMaterial>
    Camera::DrawCallsJob::IterationJob<TCoreMaterial>::InitTask::InitTask(const std::shared_ptr<TaskContext>& ctx) : Task<typename Metal::Camera::DrawCallsJob::IterationJob<TCoreMaterial>::TaskContext>(ctx)
    {
    }

    template<typename TCoreMaterial>
    void Camera::DrawCallsJob::IterationJob<TCoreMaterial>::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task<typename Metal::Camera::DrawCallsJob::IterationJob<TCoreMaterial>::TaskContext>::Run(stream);

        const auto ctx = this->GetTaskContext();
        const auto iteration = ctx->job;
        const auto pc = Core::PassKey {iteration.get()};
        const auto material = std::dynamic_pointer_cast<TCoreMaterial>(iteration->_item.material);
        const auto& matSettings = material->GetSettings();
        
        iteration->PrepareMaterialParameters(this->_specificGlobalContext, iteration->_item.material->GetParameters());
        
        const auto getCmpFunc = [](Core::RenderingMaterial::Settings::Comparision comparision) -> auto
        {
            switch (comparision)
            {
            case Core::RenderingMaterial::Settings::Comparision::Always:
                return MTL::CompareFunctionAlways;
            case Core::RenderingMaterial::Settings::Comparision::Equal:
                return MTL::CompareFunctionEqual;
            case Core::RenderingMaterial::Settings::Comparision::Greater:
                return MTL::CompareFunctionGreater;
            case Core::RenderingMaterial::Settings::Comparision::GreaterEqual:
                return MTL::CompareFunctionGreaterEqual;
            case Core::RenderingMaterial::Settings::Comparision::Less:
                return MTL::CompareFunctionLess;
            case Core::RenderingMaterial::Settings::Comparision::LessEqual:
                return MTL::CompareFunctionLessEqual;
            case Core::RenderingMaterial::Settings::Comparision::Never:
                return MTL::CompareFunctionNever;
            case Core::RenderingMaterial::Settings::Comparision::NotEqual:
                return MTL::CompareFunctionNotEqual;
            default:
                return MTL::CompareFunctionAlways;
            }
        };
        
        const auto getStencilOpFunc = [](Core::RenderingMaterial::Settings::Stencil::Op func) -> auto
        {
            switch (func)
            {
            case Core::RenderingMaterial::Settings::Stencil::Op::Keep:
                return MTL::StencilOperationKeep;
            case Core::RenderingMaterial::Settings::Stencil::Op::DecrementAndSaturate:
                return MTL::StencilOperationDecrementClamp;
            case Core::RenderingMaterial::Settings::Stencil::Op::DecrementAndWrap:
                return MTL::StencilOperationDecrementWrap;
            case Core::RenderingMaterial::Settings::Stencil::Op::IncrementAndSaturate:
                return MTL::StencilOperationIncrementClamp;
            case Core::RenderingMaterial::Settings::Stencil::Op::IncrementAndWrap:
                return MTL::StencilOperationIncrementWrap;
            case Core::RenderingMaterial::Settings::Stencil::Op::Invert:
                return MTL::StencilOperationInvert;
            case Core::RenderingMaterial::Settings::Stencil::Op::Replace:
                return MTL::StencilOperationReplace;
            case Core::RenderingMaterial::Settings::Stencil::Op::Zero:
                return MTL::StencilOperationZero;
            default:
                return MTL::StencilOperationKeep;
            }
        };
        
        const auto getBlendOp = [](Core::RenderingMaterial::Settings::Blend::Op op) -> auto
        {
            switch (op)
            {
            case Core::RenderingMaterial::Settings::Blend::Op::Add:
                return MTL::BlendOperationAdd;
            case Core::RenderingMaterial::Settings::Blend::Op::Max:
                return MTL::BlendOperationMax;
            case Core::RenderingMaterial::Settings::Blend::Op::Min:
                return MTL::BlendOperationMin;
            case Core::RenderingMaterial::Settings::Blend::Op::RevSub:
                return MTL::BlendOperationReverseSubtract;
            case Core::RenderingMaterial::Settings::Blend::Op::Sub:
                return MTL::BlendOperationSubtract;
            default:
                return MTL::BlendOperationAdd;
            }
        };
        
        const auto getBlendFactor = [](Core::RenderingMaterial::Settings::Blend::Factor factor) -> auto
        {
            switch (factor)
            {
            case Core::RenderingMaterial::Settings::Blend::Factor::Zero:
                return MTL::BlendFactorZero;
            case Core::RenderingMaterial::Settings::Blend::Factor::DstAlpha:
                return MTL::BlendFactorDestinationAlpha;
            case Core::RenderingMaterial::Settings::Blend::Factor::DstColor:
                return MTL::BlendFactorDestinationColor;
            case Core::RenderingMaterial::Settings::Blend::Factor::One:
                return MTL::BlendFactorOne;
            case Core::RenderingMaterial::Settings::Blend::Factor::OneMinusSrcAlpha:
                return MTL::BlendFactorOneMinusSourceAlpha;
            case Core::RenderingMaterial::Settings::Blend::Factor::OneMinusDstAlpha:
                return MTL::BlendFactorOneMinusDestinationAlpha;
            case Core::RenderingMaterial::Settings::Blend::Factor::OneMinusDstColor:
                return MTL::BlendFactorOneMinusDestinationColor;
            case Core::RenderingMaterial::Settings::Blend::Factor::OneMinusSrcColor:
                return MTL::BlendFactorOneMinusSourceColor;
            case Core::RenderingMaterial::Settings::Blend::Factor::SrcAlpha:
                return MTL::BlendFactorSourceAlpha;
            case Core::RenderingMaterial::Settings::Blend::Factor::SrcColor:
                return MTL::BlendFactorSourceColor;
            default:
                return MTL::BlendFactorOne;
            }
        };
        

        if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
        {
            auto rtPipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
            
            rtPipelineDesc->setVertexFunction(std::dynamic_pointer_cast<Metal::LibShader> (material->GetVertexShader())->GetNativeFunction());
            
            rtPipelineDesc->setFragmentFunction(std::dynamic_pointer_cast<Metal::LibShader> (material->GetPixelShader())->GetNativeFunction());
            
            
            NS::UInteger colorAttachmentIndex = 0U;
            for (const auto& crt : iteration->_drawCallsJob->_camera->GetTarget().color)
            {
                const auto& blendSettings = matSettings.blend.targets.at(static_cast<std::size_t>(colorAttachmentIndex));
                const auto mtlColorTarget = std::dynamic_pointer_cast<IColorTargetTexture>(crt.tex->GetUnderlyingTexture());
                
                auto mtlColorAttachment = rtPipelineDesc->colorAttachments()->object(colorAttachmentIndex);
                
                mtlColorAttachment->setPixelFormat(mtlColorTarget->GetFormat());
               
                MTL::ColorWriteMask colorWriteMask = 0;
                
                if (static_cast<std::uint8_t>(blendSettings.colorMask) & static_cast<std::uint8_t>(Core::RenderingMaterial::Settings::Blend::ColorMask::Red))
                {
                    colorWriteMask |= MTL::ColorWriteMaskRed;
                }

                if (static_cast<std::uint8_t>(blendSettings.colorMask) & static_cast<std::uint8_t>(Core::RenderingMaterial::Settings::Blend::ColorMask::Green))
                {
                    colorWriteMask |= MTL::ColorWriteMaskGreen;
                }

                if (static_cast<std::uint8_t>(blendSettings.colorMask) & static_cast<std::uint8_t>(Core::RenderingMaterial::Settings::Blend::ColorMask::Blue))
                {
                    colorWriteMask |= MTL::ColorWriteMaskBlue;
                }

                if (static_cast<std::uint8_t>(blendSettings.colorMask) & static_cast<std::uint8_t>(Core::RenderingMaterial::Settings::Blend::ColorMask::Alpha))
                {
                    colorWriteMask |= MTL::ColorWriteMaskAlpha;
                }

                mtlColorAttachment->setWriteMask(colorWriteMask);
                
                mtlColorAttachment->setBlendingEnabled(blendSettings.op != Core::RenderingMaterial::Settings::Blend::Op::None);
                
                mtlColorAttachment->setRgbBlendOperation(getBlendOp(blendSettings.op));
                mtlColorAttachment->setSourceRGBBlendFactor(getBlendFactor(blendSettings.src));
                mtlColorAttachment->setDestinationRGBBlendFactor(getBlendFactor(blendSettings.dst));

                mtlColorAttachment->setAlphaBlendOperation(mtlColorAttachment->rgbBlendOperation());
                mtlColorAttachment->setSourceAlphaBlendFactor(mtlColorAttachment->sourceRGBBlendFactor());
                mtlColorAttachment->setDestinationAlphaBlendFactor(mtlColorAttachment->destinationRGBBlendFactor());
                
                rtPipelineDesc->setSampleCount(mtlColorTarget->GetSamplesCount());
                rtPipelineDesc->setRasterSampleCount(rtPipelineDesc->sampleCount());
                
                ++colorAttachmentIndex;
            }
            
            if(const auto ds = iteration->_drawCallsJob->_camera->GetTarget().depthStencil.tex)
            {
                const auto mtlDs = std::dynamic_pointer_cast<IDepthStencilTexture>(ds->GetUnderlyingTexture());
                
                rtPipelineDesc->setDepthAttachmentPixelFormat(mtlDs->GetFormat());
                
                if(ds->StencilIncluded())
                {
                    rtPipelineDesc->setStencilAttachmentPixelFormat(mtlDs->GetFormat());
                }
            }
        
            rtPipelineDesc->setAlphaToOneEnabled(false);
            rtPipelineDesc->setAlphaToCoverageEnabled(matSettings.alphaToCoverage == Core::RenderingMaterial::Settings::AlphaToCoverage::On);
            
            switch (const auto topology = ctx->mesh->GetTopology())
            {
            case Core::GeometryPrototype::Topology::Triangles:
                rtPipelineDesc->setInputPrimitiveTopology(MTL::PrimitiveTopologyClassTriangle);
                break;
            default:
                rtPipelineDesc->setInputPrimitiveTopology(MTL::PrimitiveTopologyClassUnspecified);
                break;
            }
            
            rtPipelineDesc->setShaderValidation(this->_specificGlobalContext->settings.isDebug ? MTL::ShaderValidationEnabled : MTL::ShaderValidationDisabled);
            rtPipelineDesc->setRasterizationEnabled(true);
            
            //TODO::
            /*rtPipelineDesc->setVertexDescriptor()
            
            NS::Error* err = nullptr;
            iteration->_pipelineState = NS::TransferPtr(this->_specificGlobalContext->device->GetNative()->newRenderPipelineState(rtPipelineDesc, &err));
            
            assert(err == nullptr);*/
            rtPipelineDesc->release();
            
            auto dsDesc = MTL::DepthStencilDescriptor::alloc()->init();
            auto frontStencilDesc = MTL::StencilDescriptor::alloc()->init();
            auto backStencilDesc = MTL::StencilDescriptor::alloc()->init();
            
            dsDesc->setDepthCompareFunction(MTL::CompareFunctionAlways);
            if(matSettings.depth.test == Core::RenderingMaterial::Settings::Depth::Test::On)
            {
                dsDesc->setDepthCompareFunction(getCmpFunc(matSettings.depth.comparision));
            }
            dsDesc->setDepthWriteEnabled(matSettings.depth.write == Core::RenderingMaterial::Settings::Depth::Write::On);
            
            if (matSettings.stencil.has_value())
            {
                const auto& stencil = matSettings.stencil.value();
                
                frontStencilDesc->setReadMask(0xff);
                frontStencilDesc->setWriteMask(0xff);
                frontStencilDesc->setDepthFailureOperation(getStencilOpFunc(stencil.front.depthFail));
                frontStencilDesc->setStencilCompareFunction(getCmpFunc(stencil.front.comparision));
                frontStencilDesc->setStencilFailureOperation(getStencilOpFunc(stencil.front.stencilFail));
                frontStencilDesc->setDepthStencilPassOperation(getStencilOpFunc(stencil.front.stencilPass));
                
                backStencilDesc->setReadMask(0xff);
                backStencilDesc->setWriteMask(0xff);
                backStencilDesc->setDepthFailureOperation(getStencilOpFunc(stencil.back.depthFail));
                backStencilDesc->setStencilCompareFunction(getCmpFunc(stencil.back.comparision));
                backStencilDesc->setStencilFailureOperation(getStencilOpFunc(stencil.back.stencilFail));
                backStencilDesc->setDepthStencilPassOperation(getStencilOpFunc(stencil.back.stencilPass));
                
                dsDesc->setFrontFaceStencil(frontStencilDesc);
                dsDesc->setBackFaceStencil(backStencilDesc);
            }
            
            iteration->_depthStencilState = NS::TransferPtr(this->_specificGlobalContext->device->GetNative()->newDepthStencilState(dsDesc));
            
            frontStencilDesc->release();
            backStencilDesc->release();
            dsDesc->release();
        }
    }

    template <typename TCoreMaterial>
    Camera::DrawCallsJob::IterationJob<TCoreMaterial>::ExecutionTask::ExecutionTask(const std::shared_ptr<TaskContext>& ctx) : Task<TaskContext>(ctx)
    {
    }

    template <typename TCoreMaterial>
    void Camera::DrawCallsJob::IterationJob<TCoreMaterial>::ExecutionTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task<TaskContext>::Run(stream);

        const auto tc = this->GetTaskContext();
        const auto iteration = tc->job;
        const auto material = std::dynamic_pointer_cast<TCoreMaterial>(iteration->_item.material);
        const auto& matSettings = material->GetSettings();
        
        MTL::RenderCommandEncoder* encoder = iteration->_drawCallsJob->_renderCommandEncoder;
        
        encoder->setTriangleFillMode(matSettings.fillMode == Core::RenderingMaterial::Settings::FillMode::Solid ? MTL::TriangleFillModeFill : MTL::TriangleFillModeLines);
        encoder->setCullMode(matSettings.cullMode == Core::RenderingMaterial::Settings::CullMode::Back
                             ? MTL::CullModeBack :
                             (matSettings.cullMode == Core::RenderingMaterial::Settings::CullMode::Front ? MTL::CullModeFront : MTL::CullModeNone));
        
        encoder->setDepthBias(0.0f, 0.0f, 0.0f);
        encoder->setDepthClipMode(MTL::DepthClipModeClip);
        
        //encoder->setRenderPipelineState(iteration->_pipelineState.get());
        encoder->setDepthStencilState(iteration->_depthStencilState.get());
        
        if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
        {

            const auto& subsets = tc->mesh->GetSubsets();

            for (const auto& ss : subsets)
            {
            
            }
        }
    }

    template<typename TCoreMaterial>
    inline Camera::DrawCallsJob::IterationJob<TCoreMaterial>::TaskContext::TaskContext(const std::shared_ptr<IterationJob>& job) : job(job)
    {
        if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
        {
            meshRenderer = std::dynamic_pointer_cast<Core::Mesh::Renderer>(job->_item.renderer);
            mesh = std::dynamic_pointer_cast<const Metal::Mesh>(meshRenderer->GetMesh()->GetUnderlyingMesh());
        }
    }
}

