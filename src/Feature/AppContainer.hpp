#pragma once
#include <chrono>
#include <memory>
#include <optional>
#include <unordered_map>
#include <list>
#include <Feature/App.hpp>
#include <Feature/Input.hpp>

#ifdef MMPENGINE_WIN
#include <Windows.h>
#include <windowsx.h>
#endif

struct GLFWwindow;

namespace MMPEngine::Feature
{
	class AppContainer
	{
	public:
		struct Settings final
		{
			struct FPS final
			{
				std::uint32_t frameCount = 10;
				std::float_t updateFpsSec = 0.5f;
				bool show = true;
			};

			std::string windowCaption;
			std::int32_t initialWindowWidth = 1280;
			std::int32_t initialWindowHeight = 720;
			std::int32_t targetFps = 60;
			std::int32_t pausedSleepTimeoutMs = 42;
			FPS fps{};
		};
	protected:
		struct State final
		{
			bool appInitialized = false;
			std::optional<bool> prevPaused = std::nullopt;
			bool paused = false;
			bool maximized = false;
			bool minimized = false;
			bool resizeInProgress = false;
			std::optional<std::chrono::milliseconds> previousFrameMs = std::nullopt;
			std::list<std::float_t> deltaTimeFrames;
			std::float_t fpsUpdateTimer = 0.0f;
		};

		AppContainer(Settings&& settings, std::unique_ptr<Feature::BaseRootApp>&& app);
		void OnWindowChanged();
		virtual void CreateNativeContainer() = 0;
		virtual std::uint32_t GetCurrentScreenRefreshRate() const = 0;
		virtual Core::Vector2Uint GetCurrentWindowSize() const = 0;
		virtual std::int32_t RunInternal() = 0;
		static std::chrono::milliseconds NowMs();

		void ClearFPSData();
		void AddFPSData(std::float_t dt);
		std::uint32_t CalculateSmoothFPS() const;

		void ClearAllInputs() const;
		void ClearInstantInputEvents() const;
		void SetButtonPressedStatus(KeyButton, bool) const;
		void SetButtonPressedStatus(MouseButton, bool) const;
		void UpdateMouseNormalizedPosition(const Core::Vector2Float&) const;
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
		std::unique_ptr<Feature::App> _app;
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
		PlatformAppContainer(Settings&& settings, std::unique_ptr<Feature::BaseRootApp>&& app);
	protected:
		TPlatformAppContainerSettings _platformSettings;
	};

	template<typename TPlatformAppContainerSettings>
	inline PlatformAppContainer<TPlatformAppContainerSettings>::PlatformAppContainer(Settings&& settings, std::unique_ptr<Feature::BaseRootApp>&& app)
		: AppContainer(std::move(settings.base), std::move(app)), _platformSettings(std::move(settings.platform))
	{
	}

	namespace Shared
	{
		struct AppContainerSetting final
		{
		};

		class AppContainer final : public PlatformAppContainer<AppContainerSetting>
		{
		public:
			AppContainer(PlatformAppContainer::Settings&& settings, std::unique_ptr<Feature::BaseRootApp>&& app);
			AppContainer(const AppContainer&) = delete;
			AppContainer(AppContainer&&) noexcept = delete;
			AppContainer& operator=(const AppContainer&) = delete;
			AppContainer& operator=(AppContainer&&) noexcept = delete;
			~AppContainer() override;
		protected:
			std::int32_t RunInternal() override;
			void CreateNativeContainer() override;
			Core::Vector2Uint GetCurrentWindowSize() const override;
			std::uint32_t GetCurrentScreenRefreshRate() const override;
		private:
			static void OnWindowFocusChanged(GLFWwindow* window, int focused);
			static void OnWindowSizeChanged(GLFWwindow* window, int width, int height);
		private:
			static std::unordered_map<std::int32_t, Feature::KeyButton> _keyMap;
			GLFWwindow* _window;
		};
	}

#ifdef MMPENGINE_WIN
	namespace Win
	{
		struct AppContainerSetting final
		{
			HINSTANCE currentWindowApplicationHandle;
			std::string windowClassName = "MMPEngine::Feature::Win::AppContainer";
		};

		class AppContainer : public PlatformAppContainer<AppContainerSetting>
		{
		public:
			AppContainer(PlatformAppContainer::Settings&& settings, std::unique_ptr<Feature::BaseRootApp>&& app);
		protected:
			std::int32_t RunInternal() override;
			void CreateNativeContainer() override;
			std::uint32_t GetCurrentScreenRefreshRate() const override;
			Core::Vector2Uint GetCurrentWindowSize() const override;
		private:
			static std::unordered_map<WPARAM, Feature::KeyButton> _keyMap;
			static LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
		};
	}
#endif
}