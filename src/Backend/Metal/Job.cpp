#include <cassert>
#include <Backend/Metal/Job.hpp>
#include <Backend/Metal/Buffer.hpp>

namespace MMPEngine::Backend::Metal
{
    BaseJob::BaseJob() = default;
    BaseJob::~BaseJob()
    {
        //TODO: destructor
    };

    void BaseJob::PrepareMaterialParameters(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params)
    {
        const auto& paramsVec = params.GetAll();
        //TODO: bake parameters
    }
}
