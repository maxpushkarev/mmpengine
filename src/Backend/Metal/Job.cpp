#include <cassert>
#include <Backend/Metal/Job.hpp>
#include <Backend/Metal/Buffer.hpp>

namespace MMPEngine::Backend::Metal
{
    BaseJob::BaseJob() = default;
    BaseJob::~BaseJob() = default;

    void BaseJob::PrepareMaterialParameters(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params)
    {
        const auto& paramsVec = params.GetAll();
        _bufferDataCollection.clear();
        for(const auto& entry : paramsVec)
        {
            if(std::holds_alternative<Core::BaseMaterial::Parameters::Buffer>(entry.settings))
            {
                const auto buffer = std::dynamic_pointer_cast<const Core::Buffer>(entry.entity);
             
                const auto castedMetalBuffer = std::dynamic_pointer_cast<const Buffer>(buffer->GetUnderlyingBuffer());
                
                if(const auto uaBuffer = std::dynamic_pointer_cast<const Core::BaseUnorderedAccessBuffer>(buffer))
                {
                    _bufferDataCollection.emplace_back(castedMetalBuffer, static_cast<NS::UInteger>(uaBuffer->GetUnorderedAccessSettings().stride));
                }
                else
                {
                    _bufferDataCollection.emplace_back(castedMetalBuffer, std::nullopt);
                }
                
                if (const auto castedCounteredUaBuffer = std::dynamic_pointer_cast<const Metal::CounteredUnorderedAccessBuffer>(buffer->GetUnderlyingBuffer()))
                {
                    const auto counterBuffer = castedCounteredUaBuffer->GetCounterBuffer();
                    _bufferDataCollection.emplace_back(counterBuffer, static_cast<NS::UInteger>(sizeof(Core::CounteredUnorderedAccessBuffer::CounterValueType)));
                }
            }
        }
    }
}
