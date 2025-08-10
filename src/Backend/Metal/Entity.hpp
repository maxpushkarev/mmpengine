#pragma once
#include <Core/Entity.hpp>

namespace MMPEngine::Backend::Metal
{
    class BaseEntity : public virtual std::enable_shared_from_this<Core::BaseEntity>
    {
    public:
        BaseEntity();
        BaseEntity(const BaseEntity&) = delete;
        BaseEntity(BaseEntity&&) noexcept = delete;
        BaseEntity& operator=(const BaseEntity&) = delete;
        BaseEntity& operator=(BaseEntity&&) noexcept = delete;
        virtual ~BaseEntity();
    };

    class ResourceEntity : public BaseEntity
    {
    };
}
