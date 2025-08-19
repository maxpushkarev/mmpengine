#pragma once
#include <Core/Compute.hpp>
#include <Backend/Metal/Task.hpp>
#include <Backend/Metal/Job.hpp>

namespace MMPEngine::Backend::Metal
{
    class DirectComputeJob final : public Core::DirectComputeJob, public Metal::Job<Core::ComputeMaterial>
    {
    private:
        class InitContext final : public Core::TaskContext
        {
        public:
            std::shared_ptr<DirectComputeJob> job;
        };

        class ExecutionContext final : public Core::DirectComputeContext
        {
        public:
            std::shared_ptr<DirectComputeJob> job;
        };

        class InitTask final : public Task<InitContext>
        {
        public:
            InitTask(const std::shared_ptr<InitContext>& ctx);
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
        };

        class ExecutionTask final : public Task<Core::DirectComputeContext>
        {
        private:
            class Impl final : public Task<ExecutionContext>
            {
            public:
                Impl(const std::shared_ptr<ExecutionContext>& ctx);
                void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
            };
        public:
            ExecutionTask(const std::shared_ptr<ExecutionContext>& ctx);
            void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
        private:
            std::shared_ptr<Impl> _impl;
        };

    public:
        DirectComputeJob(const std::shared_ptr<Core::ComputeMaterial>& material);
        ~DirectComputeJob() override;
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
        std::shared_ptr<Core::ContextualTask<Core::DirectComputeContext>> CreateExecutionTask() override;
    private:
        MTL::ComputePipelineState* _computePipelineState = nullptr;
    };
}

