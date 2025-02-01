#pragma once
#include <memory>
#include <Core/Context.hpp>
#include <Core/Stream.hpp>
#include <Feature/Logger.hpp>
#include <Feature/Input.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Context.hpp>
#endif

#ifdef MMPENGINE_BACKEND_VULKAN
#include <Backend/Vulkan/Context.hpp>
#endif

namespace MMPEngine::Feature
{
	class BaseRootApp;
	class UserApp;

	class App
	{
	protected:
		App(const std::shared_ptr<BaseLogger>& logger);
	public:
		App(const App&) = delete;
		App(App&&) noexcept = delete;
		App& operator=(const App&) = delete;
		App& operator=(App&&) noexcept = delete;
		virtual ~App();

		virtual void Initialize();
		virtual void OnNativeWindowUpdated();
		virtual void OnPause();
		virtual void OnResume();
		virtual void OnUpdate(std::float_t dt);
		virtual void OnRender();
		virtual std::shared_ptr<Core::GlobalContext> GetContext() const = 0;
		virtual std::shared_ptr<Feature::Input> GetInput() const = 0;
		virtual std::shared_ptr<Core::BaseStream> GetDefaultStream() const = 0;

		static std::unique_ptr<BaseRootApp> BuildRootApp(
			const Core::GlobalContext::Settings& globalContextSettings, 
			std::unique_ptr<UserApp>&& userApp, 
			std::unique_ptr<Core::Math>&& math, 
			const std::shared_ptr<BaseLogger>& logger
		);

	protected:
		std::shared_ptr<BaseLogger> _logger;
	};

	class BaseRootApp : public App
	{
	public:
		BaseRootApp(const BaseRootApp&) = delete;
		BaseRootApp(BaseRootApp&&) noexcept = delete;
		BaseRootApp& operator=(const BaseRootApp&) = delete;
		BaseRootApp& operator=(BaseRootApp&&) noexcept = delete;
		~BaseRootApp() override;
	protected:
		BaseRootApp(const std::shared_ptr<BaseLogger>& logger);
		void Initialize() override;
		void OnNativeWindowUpdated() override;
		void OnPause() override;
		void OnResume() override;
		void OnUpdate(std::float_t dt) override;
		void OnRender() override;
		std::unique_ptr<UserApp> _userApp;
	public:
		void Attach(std::unique_ptr<UserApp>&& userApp);
		std::shared_ptr<Feature::Input> GetInput() const override;
	private:
		std::shared_ptr<Input> _input;
	};

	class UserApp : public App
	{
		friend class BaseRootApp;
	public:
		UserApp(const std::shared_ptr<BaseLogger>& logger);
		std::shared_ptr<Core::GlobalContext> GetContext() const override;
		std::shared_ptr<Core::BaseStream> GetDefaultStream() const override;
		std::shared_ptr<Feature::Input> GetInput() const override;
	private:
		void JoinToRootApp(const BaseRootApp* root);
		void UnjoinFromRootApp();
		const BaseRootApp* _rootApp;
	};

	template<typename TRootContext>
	class RootApp : public BaseRootApp
	{
		static_assert(std::is_base_of_v<Core::GlobalContext, TRootContext>, "TRootContext must be inherited from Core::GlobalContext");
	protected:
		RootApp(const std::shared_ptr<TRootContext>& context, const std::shared_ptr<BaseLogger>& logger);
	public:
		RootApp(const RootApp&) = delete;
		RootApp(RootApp&&) noexcept = delete;
		RootApp& operator=(const RootApp&) = delete;
		RootApp& operator=(RootApp&&) noexcept = delete;
		void Initialize() override;
	protected:
		~RootApp() override;
	public:
		std::shared_ptr<Core::GlobalContext> GetContext() const override;
		std::shared_ptr<Core::BaseStream> GetDefaultStream() const override;
	protected:
		std::shared_ptr<TRootContext> _rootContext;
		std::shared_ptr<Core::BaseStream> _defaultStream;
	};

#ifdef MMPENGINE_BACKEND_DX12
	namespace Dx12
	{
		class RootApp final : public Feature::RootApp<Backend::Dx12::GlobalContext>
		{
		public:
			RootApp(const std::shared_ptr<Backend::Dx12::GlobalContext>& context, const std::shared_ptr<BaseLogger>& logger);
			void Initialize() override;
		};
	}
#endif

#ifdef MMPENGINE_BACKEND_VULKAN
	namespace Vulkan
	{
		class RootApp final : public Feature::RootApp<Backend::Vulkan::GlobalContext>
		{
		public:
			RootApp(const std::shared_ptr<Backend::Vulkan::GlobalContext>& context, const std::shared_ptr<BaseLogger>& logger);
			void Initialize() override;
		};
	}
#endif

	template<typename TRootContext>
	inline RootApp<TRootContext>::RootApp(const std::shared_ptr<TRootContext>& context, const std::shared_ptr<BaseLogger>& logger) : BaseRootApp(logger), _rootContext(context)
	{
	}

	template<typename TRootContext>
	inline std::shared_ptr<Core::GlobalContext> RootApp<TRootContext>::GetContext() const
	{
		return _rootContext;
	}

	template<typename TRootContext>
	inline void RootApp<TRootContext>::Initialize()
	{
		_defaultStream->Restart();
		BaseRootApp::Initialize();
	}

	template<typename TRootContext>
	inline RootApp<TRootContext>::~RootApp()
	{
		_defaultStream->SubmitAndWait();
	}

	template<typename TRootContext>
	inline std::shared_ptr<Core::BaseStream> RootApp<TRootContext>::GetDefaultStream() const
	{
		return _defaultStream;
	}
}