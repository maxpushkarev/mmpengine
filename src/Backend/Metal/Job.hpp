#pragma once
#include <Core/Material.hpp>
#include <Backend/Metal/Context.hpp>
#include <Backend/Metal/Task.hpp>

namespace MMPEngine::Backend::Metal
{
    class BaseJob
    {
    public:
        BaseJob(const BaseJob&) = delete;
        BaseJob(BaseJob&&) noexcept = delete;
        BaseJob& operator=(const BaseJob&) = delete;
        BaseJob& operator=(BaseJob&&) noexcept = delete;

    protected:
        BaseJob();
        virtual ~BaseJob();

        void PrepareMaterialParameters(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params);


        class TaskContext final : public Core::TaskContext
        {
        public:
            std::shared_ptr<BaseJob> job;
        };
    };

    template<typename TCoreMaterial>
    class Job : public BaseJob
    {
        static_assert(std::is_base_of_v<Core::BaseMaterial, TCoreMaterial>, "TCoreMaterial must be derived from Core::BaseMaterial");
    };
}
