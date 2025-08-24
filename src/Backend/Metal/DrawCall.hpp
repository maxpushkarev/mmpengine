#pragma once
#include <cassert>
#include <Core/DrawCall.hpp>
#include <Backend/Metal/Mesh.hpp>
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
        MTL::RenderCommandEncoder* _currentRenderCommandEncoder = nullptr;
        std::unordered_map<std::uint64_t, MTL::RenderCommandEncoder*> _encodersMap;
        std::vector<std::uint64_t> _encodersReleaseIds;
    };


    template<typename TCoreMaterial>
    Camera::DrawCallsJob::IterationJob<TCoreMaterial>::IterationJob(const std::shared_ptr<DrawCallsJob>& job, const Item& item)
        : IterationImpl(job, item)
    {
    }

    template <typename TCoreMaterial>
    Camera::DrawCallsJob::IterationJob<TCoreMaterial>::~IterationJob<TCoreMaterial>()
    {

    }

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

        if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
        {
            const auto& ibInfo = ctx->mesh->GetIndexBufferInfo();
            const auto& allBufferInfos = ctx->mesh->GetAllVertexBufferInfos();

            for (const auto& s2vbs : allBufferInfos)
            {
                for (const auto& vbInfo : s2vbs.second)
                {
                    
                }
            }
        }

        iteration->PrepareMaterialParameters(this->_specificGlobalContext, iteration->_item.material->GetParameters());


        if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
        {
    
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

