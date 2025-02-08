#include <Feature/App.hpp>
#include <array>
#include <cassert>
#include <Backend/Shared/Math.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Stream.hpp>
#include <Backend/Dx12/Math.hpp>
#endif

#ifdef MMPENGINE_BACKEND_VULKAN
#include <Backend/Vulkan/Stream.hpp>
#endif


namespace MMPEngine::Feature
{
	App::App(const std::shared_ptr<BaseLogger>& logger) : _logger(logger)
	{
	}

	App::~App() = default;

	void App::Initialize()
	{
	}

	void App::OnNativeWindowUpdated()
	{
	}

	void App::OnPause()
	{
	}

	void App::OnResume()
	{
	}

	void App::OnUpdate(std::float_t dt)
	{
	}

	void App::OnRender()
	{
	}

	std::unique_ptr<BaseRootApp> App::BuildRootApp(
		const Core::GlobalContext::Settings& globalContextSettings,
		std::unique_ptr<UserApp>&& userApp,
		std::unique_ptr<Core::Math>&& math,
		const std::shared_ptr<BaseLogger>& logger
	)
	{
		if (globalContextSettings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			if (!math)
			{
				math = std::make_unique<Backend::Dx12::Math>();
			}
			auto rootApp = std::make_unique<Dx12::RootApp>(std::make_shared<Backend::Dx12::GlobalContext>(globalContextSettings, std::move(math)), logger);
			rootApp->Attach(std::move(userApp));
			return rootApp;
#else
			throw Core::UnsupportedException("unable to create root app for DX12 backend");
#endif
		}

		if (globalContextSettings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			if (!math)
			{
				math = std::make_unique<Backend::Shared::GLMMath>();
			}
			auto rootApp = std::make_unique<Vulkan::RootApp>(std::make_shared<Backend::Vulkan::GlobalContext>(globalContextSettings, std::move(math)), logger);
			rootApp->Attach(std::move(userApp));
			return rootApp;
#else
			throw Core::UnsupportedException("unable to create root app for Vulkan backend");
#endif
		}


		throw std::runtime_error("unable to create root app");
	}

	BaseRootApp::BaseRootApp(const std::shared_ptr<BaseLogger>& logger) : App(logger), _input(std::make_shared<Input>())
	{
	}

	void BaseRootApp::Initialize()
	{
		App::Initialize();
		if (_userApp)
		{
			_userApp->Initialize();
		}
	}

	void BaseRootApp::OnNativeWindowUpdated()
	{
		App::OnNativeWindowUpdated();
		if (_userApp)
		{
			_userApp->OnNativeWindowUpdated();
		}
	}

	BaseRootApp::~BaseRootApp()
	{
		if(_userApp)
		{
			_userApp->UnjoinFromRootApp();
		}
	}

	void BaseRootApp::OnPause()
	{
		App::OnPause();
		if (_userApp)
		{
			_userApp->OnPause();
		}
	}

	void BaseRootApp::OnResume()
	{
		App::OnResume();
		if (_userApp)
		{
			_userApp->OnResume();
		}
	}

	void BaseRootApp::OnUpdate(std::float_t dt)
	{
		App::OnUpdate(dt);
		if(_userApp)
		{
			_userApp->OnUpdate(dt);
		}
	}

	void BaseRootApp::OnRender()
	{
		App::OnRender();
		if (_userApp)
		{
			_userApp->OnRender();
		}
	}

	std::shared_ptr<Feature::Input> BaseRootApp::GetInput() const
	{
		return _input;
	}

	void BaseRootApp::Attach(std::unique_ptr<UserApp>&& userApp)
	{
		_userApp = std::move(userApp);
		_userApp->JoinToRootApp(this);
	}

	UserApp::UserApp(const std::shared_ptr<BaseLogger>& logger) : App(logger), _rootApp(nullptr)
	{
	}

	std::shared_ptr<Core::GlobalContext> UserApp::GetContext() const
	{
		if (const auto root = _rootApp)
		{
			return root->GetContext();
		}

		return nullptr;
	}

	std::shared_ptr<Core::BaseStream> UserApp::GetDefaultStream() const
	{
		if (const auto root = _rootApp)
		{
			return root->GetDefaultStream();
		}

		return nullptr;
	}

    std::shared_ptr<Feature::Input> UserApp::GetInput() const
    {
		if (const auto root = _rootApp)
		{
			return root->GetInput();
		}

		return nullptr;
    }


	void UserApp::JoinToRootApp(const BaseRootApp* root)
	{
		_rootApp = root;
	}

	void UserApp::UnjoinFromRootApp()
	{
		_rootApp = nullptr;
	}


#ifdef MMPENGINE_BACKEND_DX12
	namespace Dx12
	{
		RootApp::RootApp(const std::shared_ptr<Backend::Dx12::GlobalContext>& context, const std::shared_ptr<BaseLogger>& logger)
			: Feature::RootApp<Backend::Dx12::GlobalContext>(context, logger)
		{
		}

		void RootApp::Initialize()
		{
			if (_rootContext->settings.isDebug)
			{
				Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
				D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
				debugController->EnableDebugLayer();
			}

			CreateDXGIFactory(IID_PPV_ARGS(&_rootContext->factory));


			constexpr std::array<D3D_FEATURE_LEVEL, 3> featureLevels = { D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0 };

			for (const auto featureLevel : featureLevels)
			{
				std::uint32_t i = 0;
				IDXGIAdapter* adapter = nullptr;

				while (_rootContext->factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
				{
					DXGI_ADAPTER_DESC desc{};
					adapter->GetDesc(&desc);

					D3D12CreateDevice(adapter, featureLevel, IID_PPV_ARGS(&_rootContext->device));
					++i;

					if (_rootContext->device != nullptr)
					{
						std::wstring wAdapterName = { desc.Description };
						std::string adapterName(wAdapterName.length(), 0);
						std::transform(wAdapterName.cbegin(), wAdapterName.cend(), adapterName.begin(), [](wchar_t c) {
							return static_cast<char>(c);
							});
						_logger->Log(Feature::ELogType::Info, "D3D12 device created. adapter: ", adapterName);
						break;
					}
				}

				if (_rootContext->device != nullptr)
				{
					break;
				}
			}

			assert(_rootContext->device != nullptr);


			_rootContext->rtvDescPool = std::make_shared<Backend::Dx12::RTVDescriptorPool>(_rootContext->device, Core::Pool::Settings{});
			_rootContext->dsvDescPool = std::make_shared<Backend::Dx12::DSVDescriptorPool>(_rootContext->device, Core::Pool::Settings{});
			_rootContext->cbvSrvUavShaderInVisibleDescPool = std::make_shared<Backend::Dx12::CBVSRVUAVDescriptorPool>(_rootContext->device, Core::Pool::Settings{}, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
			_rootContext->cbvSrvUavShaderVisibleDescPool = std::make_shared<Backend::Dx12::CBVSRVUAVDescriptorPool>(_rootContext->device, Core::Pool::Settings{}, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
			_rootContext->constantBufferEntityHeap = std::make_shared<Backend::Dx12::ConstantBufferHeap>(
				Core::Heap::Settings {
					D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, 2
				}
			);

			Microsoft::WRL::ComPtr<ID3D12Fence> fence;
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> list;
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;

			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			_rootContext->device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(queue.GetAddressOf()));


			_rootContext->device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(fence.GetAddressOf()));

			_rootContext->device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(allocator.GetAddressOf()));

			_rootContext->device->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				allocator.Get(),
				nullptr,
				IID_PPV_ARGS(list.GetAddressOf()));

			list->Close();
			const auto streamContext = std::make_shared<Backend::Dx12::StreamContext>(queue, allocator, list, fence);

			_defaultStream = std::make_shared<Backend::Dx12::Stream>(_rootContext, streamContext);

			Feature::RootApp<Backend::Dx12::GlobalContext>::Initialize();
		}
	}
#endif

#ifdef MMPENGINE_BACKEND_VULKAN
	namespace Vulkan
	{
		RootApp::RootApp(const std::shared_ptr<Backend::Vulkan::GlobalContext>& context, const std::shared_ptr<BaseLogger>& logger)
			: Feature::RootApp<Backend::Vulkan::GlobalContext>(context, logger)
		{
		}

		void RootApp::Initialize()
		{
			VkApplicationInfo appInfo{};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "MMPEngine.Feature.Vulkan.RootApp";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "MMPEngine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_3;

			VkInstanceCreateInfo createInstanceInfo{};
			createInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInstanceInfo.pApplicationInfo = &appInfo;
			createInstanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

			std::vector<const char*> requiredExtensions {VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME};

			if(_rootContext->settings.isDebug)
			{
				requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			createInstanceInfo.enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensions.size());
			createInstanceInfo.ppEnabledExtensionNames = requiredExtensions.data();

			std::vector<const char*> requiredLayers {};

			if(_rootContext->settings.isDebug)
			{
				requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
			}

			createInstanceInfo.enabledLayerCount = static_cast<std::uint32_t>(requiredLayers.size());
			createInstanceInfo.ppEnabledLayerNames = requiredLayers.data();

			const auto instanceRes = vkCreateInstance(&createInstanceInfo, nullptr, &_rootContext->instance);

			assert(instanceRes == VkResult::VK_SUCCESS);

			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(_rootContext->instance, &deviceCount, nullptr);

			std::vector<VkPhysicalDevice> physicalDevices {deviceCount};
			vkEnumeratePhysicalDevices(_rootContext->instance, &deviceCount, physicalDevices.data());

			std::optional<std::pair<std::size_t, VkPhysicalDeviceProperties>> selectedDeviceProps;

			for(std::size_t i = 0; i < physicalDevices.size(); ++i)
			{
				const auto pd = physicalDevices[i];

				VkPhysicalDeviceProperties deviceProps;
				vkGetPhysicalDeviceProperties(pd, &deviceProps);

				if(!selectedDeviceProps.has_value() || selectedDeviceProps.value().second.apiVersion < deviceProps.apiVersion)
				{
					selectedDeviceProps = std::make_pair(i, deviceProps);
				}
			}

			assert(selectedDeviceProps.has_value());

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[selectedDeviceProps.value().first], &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies {queueFamilyCount};
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[selectedDeviceProps.value().first], &queueFamilyCount, queueFamilies.data());

			std::optional<std::size_t> queueFamilyIndex = std::nullopt;
			for(std::size_t i = 0; i < queueFamilies.size(); ++i)
			{
				if(queueFamilies[i].queueFlags & (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT))
				{
					queueFamilyIndex = i;
				}
			}

			assert(queueFamilyIndex.has_value());

			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = static_cast<std::uint32_t>(queueFamilyIndex.value());
			queueCreateInfo.queueCount = 1;

			constexpr auto queuePriority = 1.0f;
			queueCreateInfo.pQueuePriorities = &queuePriority;


			VkDeviceCreateInfo createDeviceInfo{};
			createDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createDeviceInfo.pQueueCreateInfos = &queueCreateInfo;
			createDeviceInfo.queueCreateInfoCount = 1;

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(physicalDevices[selectedDeviceProps.value().first], &deviceFeatures);
			createDeviceInfo.pEnabledFeatures = &deviceFeatures;

			requiredExtensions.clear();
			requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			createDeviceInfo.enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensions.size());
			createDeviceInfo.ppEnabledExtensionNames = requiredExtensions.data();

			const auto createDeviceRes = vkCreateDevice(physicalDevices[selectedDeviceProps.value().first], &createDeviceInfo, nullptr, &_rootContext->device);

			assert(createDeviceRes == VK_SUCCESS);

			VkQueue queue;
			vkGetDeviceQueue(_rootContext->device, static_cast<std::uint32_t>(queueFamilyIndex.value()), 0, &queue);

			assert(queue != nullptr);


			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = static_cast<std::uint32_t>(queueFamilyIndex.value());
			VkCommandPool commandPool;
			
			vkCreateCommandPool(_rootContext->device, &poolInfo, nullptr, &commandPool);

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			
			vkAllocateCommandBuffers(_rootContext->device, &allocInfo, &commandBuffer);

			const auto streamContext = std::make_shared<Backend::Vulkan::StreamContext>(_rootContext, commandPool, commandBuffer);
			_defaultStream = std::make_shared<Backend::Vulkan::Stream>(_rootContext, streamContext);


			Feature::RootApp<Backend::Vulkan::GlobalContext>::Initialize();
		}
	}
#endif

}