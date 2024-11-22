#pragma once
#include <chrono>
#include <memory>
#include <optional>
#include <Core/App.hpp>

#ifdef MMPENGINE_WIN
#include <Windows.h>
#include <windowsx.h>
#endif

namespace MMPEngine::Frontend
{
	class AppContainer
	{
	public:
		struct Settings final
		{
			std::string windowCaption;
			std::int32_t initialWindowWidth = 1280;
			std::int32_t initialWindowHeight = 720;
			std::int32_t targetFps = 60;
			std::int32_t pausedSleepTimeoutMs = 42;
			bool showFps = true;
		};
	protected:
		struct State final
		{
			bool appInitialized = false;
			bool paused = false;
			bool maximized = false;
			bool minimized = false;
			bool resizeInProgress = false;
			std::optional<std::chrono::milliseconds> previousFrameMs = {};
		};

		AppContainer(Settings&& settings, const std::shared_ptr<Core::BaseRootApp>& app);
		void OnWindowChanged();
		virtual void CreateNativeContainer() = 0;
		virtual std::int32_t RunInternal() = 0;
		static std::chrono::milliseconds NowMs();
	public:
		AppContainer(const AppContainer&) = delete;
		AppContainer(AppContainer&&) noexcept = delete;
		AppContainer& operator=(const AppContainer&) = delete;
		AppContainer& operator=(AppContainer&&) noexcept = delete;
		virtual ~AppContainer();

		std::int32_t Run();
	protected:
		Settings _settings;
		State _state;
		std::shared_ptr<Core::App> _app;
		std::shared_ptr<Core::AppInputController> _inputController;
	};

	template<typename TPlatformAppContainerSettings>
	class PlatformAppContainer : public AppContainer
	{
		static_assert(std::is_final_v<TPlatformAppContainerSettings>, "TPlatformAppContainerSettings must be final");
	public:
		struct Settings final
		{
			AppContainer::Settings base;
			TPlatformAppContainerSettings platform;
		};
		PlatformAppContainer(Settings&& settings, const std::shared_ptr<Core::BaseRootApp>& app);
	protected:
		TPlatformAppContainerSettings _platformSettings;
	};

	template<typename TPlatformAppContainerSettings>
	inline PlatformAppContainer<TPlatformAppContainerSettings>::PlatformAppContainer(Settings&& settings, const std::shared_ptr<Core::BaseRootApp>& app)
		: AppContainer(std::move(settings.base), app), _platformSettings(std::move(settings.platform))
	{
	}

#ifdef MMPENGINE_WIN
	namespace Win
	{
		struct AppContainerSetting final
		{
			HINSTANCE currentWindowApplicationHandle;
			std::string windowClassName = "MMPEngine::Frontend::Win::AppContainer";
		};

		class AppContainer : public PlatformAppContainer<AppContainerSetting>
		{
		public:
			AppContainer(PlatformAppContainer::Settings&& settings, const std::shared_ptr<Core::BaseRootApp>& app);
		protected:
			std::int32_t RunInternal() override;
			void CreateNativeContainer() override;
		private:
			static LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
		};
	}
#endif
}
