#include <Frontend/Material.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Material.hpp>
#endif

#ifdef MMPENGINE_BACKEND_VULKAN
#include <Backend/Vulkan/Material.hpp>
#endif

#ifdef MMPENGINE_BACKEND_METAL
#include <Backend/Metal/Material.hpp>
#endif


namespace MMPEngine::Frontend
{
    MeshMaterial::MeshMaterial(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings, Parameters&& params, const std::shared_ptr<Core::Shader>& vs, const std::shared_ptr<Core::Shader>& ps)
        : Core::MeshMaterial(settings, std::move(params), vs, ps)
    {
        
        if (globalContext->settings.backend == Core::BackendType::Dx12)
        {
#ifdef MMPENGINE_BACKEND_DX12
            _impl = std::make_shared<Backend::Dx12::MeshMaterial>(camera, std::move(_items));
#else
            throw Core::UnsupportedException("unable to create MeshMaterial for DX12 backend");
#endif
        } else     if (globalContext->settings.backend == Core::BackendType::Vulkan)
        {
#ifdef MMPENGINE_BACKEND_VULKAN
            _impl = std::make_shared<Backend::Vulkan::Camera::DrawCallsJob>(camera, std::move(_items));
#else
            throw Core::UnsupportedException("unable to create MeshMaterial for Vulkan backend");
#endif
        } else if (globalContext->settings.backend == Core::BackendType::Metal)
        {
#ifdef MMPENGINE_BACKEND_METAL
            auto paramsCopy = GetParameters();
            _impl = std::make_shared<Backend::Metal::MeshMaterial>(settings, std::move(paramsCopy), vs, ps);
#else
            throw Core::UnsupportedException("unable to create MeshMaterial for Metal backend");
#endif
        }
    }

    std::shared_ptr<Core::BaseTask> MeshMaterial::CreateInitializationTask()
    {
        return _impl->CreateInitializationTask();
    }

    ComputeMaterial::ComputeMaterial(const std::shared_ptr<Core::GlobalContext>& globalContext, Parameters&& params, const std::shared_ptr<Core::Shader>& cs)
        : Core::ComputeMaterial(std::move(params), cs)
    {
        if (globalContext->settings.backend == Core::BackendType::Dx12)
        {
#ifdef MMPENGINE_BACKEND_DX12
            _impl = std::make_shared<Backend::Dx12::MeshMaterial>(camera, std::move(_items));
#else
            throw Core::UnsupportedException("unable to create ComputeMaterial for DX12 backend");
#endif
        } else     if (globalContext->settings.backend == Core::BackendType::Vulkan)
        {
#ifdef MMPENGINE_BACKEND_VULKAN
            _impl = std::make_shared<Backend::Vulkan::Camera::DrawCallsJob>(camera, std::move(_items));
#else
            throw Core::UnsupportedException("unable to create ComputeMaterial for Vulkan backend");
#endif
        } else if (globalContext->settings.backend == Core::BackendType::Metal)
        {
#ifdef MMPENGINE_BACKEND_METAL
            auto paramsCopy = GetParameters();
            _impl = std::make_shared<Backend::Metal::ComputeMaterial>(std::move(paramsCopy), cs);
#else
            throw Core::UnsupportedException("unable to create ComputeMaterial for Metal backend");
#endif
        }
    }

    std::shared_ptr<Core::BaseTask> ComputeMaterial::CreateInitializationTask()
    {
        return _impl->CreateInitializationTask();
    }
}

