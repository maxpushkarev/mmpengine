#include <thread>
#include <Feature/AppContainer.hpp>
#include <GLFW/glfw3.h>

#ifdef MMPENGINE_WIN
#define GLFW_EXPOSE_NATIVE_WIN32 1
#endif

#include <GLFW/glfw3native.h>

namespace MMPEngine::Feature
{
	void AppContainer::ClearAllInputs() const
	{
		_app->GetInput()->ClearAll();
	}

	void AppContainer::ClearInstantInputEvents() const
	{
		_app->GetInput()->ClearInstantEvents();
	}

	void AppContainer::UpdateMouseNormalizedPosition(const Core::Vector2Float& pos) const
	{
		_app->GetInput()->UpdateMouseNormalizedPosition(pos);
	}

	void AppContainer::SetButtonPressedStatus(KeyButton btn, bool status) const
	{
		_app->GetInput()->SetButtonPressedStatus(btn, status);
	}

	void AppContainer::SetButtonPressedStatus(MouseButton btn, bool status) const
	{
		_app->GetInput()->SetButtonPressedStatus(btn, status);
	}

	AppContainer::AppContainer(Settings&& settings, std::unique_ptr<Feature::BaseRootApp>&& app) : _settings(std::move(settings)), _app(std::move(app))
	{
	}

	void AppContainer::OnWindowChanged()
	{
		if (!_state.appInitialized)
		{
			return;
		}

		const auto appContext = _app->GetContext();
		appContext->screenRefreshRate = GetCurrentScreenRefreshRate();
		appContext->windowSize = GetCurrentWindowSize();

		_app->OnNativeWindowUpdated();
	}

	std::chrono::milliseconds AppContainer::NowMs()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
	}

	AppContainer::~AppContainer() = default;

	std::int32_t AppContainer::Run()
	{
		CreateNativeContainer();
		_app->Initialize();
		_state.appInitialized = true;
		OnWindowChanged();
		return RunInternal();
	}

	namespace Shared
	{
		std::unordered_map<std::int32_t, Feature::KeyButton> AppContainer::_keyMap {

			{ GLFW_KEY_W, Feature::KeyButton::W },
			{ GLFW_KEY_A, Feature::KeyButton::A },
			{ GLFW_KEY_S, Feature::KeyButton::S },
			{ GLFW_KEY_D, Feature::KeyButton::D },

			{ GLFW_KEY_LEFT, Feature::KeyButton::LeftArrow },
			{ GLFW_KEY_RIGHT, Feature::KeyButton::RightArrow },
			{ GLFW_KEY_UP, Feature::KeyButton::UpArrow },
			{ GLFW_KEY_DOWN, Feature::KeyButton::DownArrow },

			{ GLFW_KEY_Q, Feature::KeyButton::Q },
			{ GLFW_KEY_E, Feature::KeyButton::E },
			{ GLFW_KEY_R, Feature::KeyButton::R },
			{ GLFW_KEY_F, Feature::KeyButton::F },

			{ GLFW_KEY_PAGE_UP, Feature::KeyButton::PageUp },
			{ GLFW_KEY_PAGE_DOWN, Feature::KeyButton::PageDown },

			{ GLFW_KEY_ENTER, Feature::KeyButton::Enter },
			{ GLFW_KEY_SPACE, Feature::KeyButton::Space }
		};

		AppContainer::AppContainer(PlatformAppContainer::Settings&& settings, std::unique_ptr<Feature::BaseRootApp>&& app) :
			PlatformAppContainer<MMPEngine::Feature::Shared::AppContainerSetting>(std::move(settings), std::move(app)), _window(nullptr)
		{
		}

		void AppContainer::OnWindowFocusChanged(GLFWwindow* window, int focused)
		{
			const auto appContainerPtr = static_cast<AppContainer*>(glfwGetWindowUserPointer(window));
			appContainerPtr->_state.paused = !static_cast<bool>(focused);
		}

		void AppContainer::OnWindowSizeChanged(GLFWwindow* window, int width, int height)
		{
			const auto appContainerPtr = static_cast<AppContainer*>(glfwGetWindowUserPointer(window));
			appContainerPtr->OnWindowChanged();
		}

		void AppContainer::CreateNativeContainer()
		{
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			
			_window = glfwCreateWindow(_settings.initialWindowWidth, _settings.initialWindowHeight, _settings.windowCaption.c_str(), nullptr, nullptr);
			glfwSetWindowUserPointer(_window, this);

			glfwSetWindowFocusCallback(_window, OnWindowFocusChanged);
			glfwSetWindowSizeCallback(_window, OnWindowSizeChanged);

#ifdef MMPENGINE_WIN
			_app->GetContext()->nativeWindow = glfwGetWin32Window(_window);
#endif
		}

		Core::Vector2Uint AppContainer::GetCurrentWindowSize() const
		{
			int w = 0;
			int h = 0;
			glfwGetWindowSize(_window, &w, &h);

			const Core::Vector2Uint res {
				static_cast<std::uint32_t>(w),
				static_cast<std::uint32_t>(h)
			};
			return res;
		}


		std::uint32_t AppContainer::GetCurrentScreenRefreshRate() const
		{
			const auto monitorPtr = glfwGetPrimaryMonitor();
			const auto mode = glfwGetVideoMode(monitorPtr);
			return static_cast<std::uint32_t>(mode->refreshRate);
		}

		std::int32_t AppContainer::RunInternal()
		{
			const auto globalContext = _app->GetContext();
			_state.previousFrameMs.reset();

			const auto targetFrameMilliseconds = std::chrono::milliseconds(
				static_cast<std::uint64_t>(floor((1000.0 / static_cast<double_t>(_settings.targetFps))))
			);

			while (!glfwWindowShouldClose(_window))
			{
				glfwPollEvents();

				if (_state.prevPaused.value_or(_state.paused) != _state.paused)
				{
					if (_state.paused)
					{
						_app->OnPause();
					}
					else
					{
						_app->OnResume();
					}
				}

				_state.prevPaused = _state.paused;

				if (!_state.paused)
				{
					const auto beforeFrameMs = NowMs();
					const auto prevMs = _state.previousFrameMs.value_or(beforeFrameMs);
					_state.previousFrameMs.emplace(beforeFrameMs);

					const auto frameDiff = beforeFrameMs - prevMs;
					const auto dt = static_cast<float_t>(frameDiff.count()) * 0.001f;

					if (_settings.showFps)
					{
						glfwSetWindowTitle(_window, Core::Text::CombineToString(_settings.windowCaption, " | FPS: ", static_cast<std::uint32_t>(std::round(1.0f / dt))).c_str());
					}

					for (const auto& [glfwKey, coreKey] : _keyMap)
					{
						SetButtonPressedStatus(coreKey, glfwGetKey(_window, glfwKey) == GLFW_PRESS);
					}


					SetButtonPressedStatus(MouseButton::Left, glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
					SetButtonPressedStatus(MouseButton::Right, glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

					std::double_t x;
					std::double_t y;
					glfwGetCursorPos(_window, &x, &y);

					UpdateMouseNormalizedPosition({
						std::clamp(static_cast<std::float_t>(x) / static_cast<std::float_t>(globalContext->windowSize.x), 0.0f, 1.0f),
						std::clamp(static_cast<std::float_t>(y) / static_cast<std::float_t>(globalContext->windowSize.y), 0.0f, 1.0f)
					});

					_app->OnUpdate(dt);
					ClearInstantInputEvents();
					_app->OnRender();

					const auto afterFrame = NowMs();
					const auto diffMs = afterFrame - beforeFrameMs;

					if (diffMs >= targetFrameMilliseconds)
					{
						std::this_thread::yield();
					}
					else
					{
						const auto sleepTimeout = targetFrameMilliseconds - diffMs;
						while (NowMs() - _state.previousFrameMs.value() < sleepTimeout)
						{
							std::this_thread::yield();
						}
					}
				}
				else
				{
					ClearAllInputs();
					std::this_thread::sleep_for(std::chrono::milliseconds{_settings.pausedSleepTimeoutMs});
				}
			}

			_app->OnPause();
			_app.reset();
			return 0;
		}

		AppContainer::~AppContainer()
		{
			glfwDestroyWindow(_window);
			glfwTerminate();
		}
	}


#ifdef MMPENGINE_WIN
	namespace Win
	{
		AppContainer::AppContainer(PlatformAppContainer::Settings&& settings, std::unique_ptr<Feature::BaseRootApp>&& app) :
			PlatformAppContainer<MMPEngine::Feature::Win::AppContainerSetting>(std::move(settings), std::move(app))
		{

		}

		std::int32_t AppContainer::RunInternal()
		{
			MSG msg = {};
			const auto globalContext = _app->GetContext();
			_state.previousFrameMs.reset();

			const auto targetFrameMilliseconds = std::chrono::milliseconds(
				static_cast<std::uint64_t>(floor((1000.0 / static_cast<double_t>(_settings.targetFps))))
			);

			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					TRACKMOUSEEVENT tme;
					tme.cbSize = sizeof(TRACKMOUSEEVENT);
					tme.dwFlags = TME_LEAVE;
					tme.hwndTrack = globalContext->nativeWindow;
					TrackMouseEvent(&tme);

					if (_state.prevPaused.value_or(_state.paused) != _state.paused)
					{
						if (_state.paused)
						{
							_app->OnPause();
						}
						else
						{
							_app->OnResume();
						}
					}

					_state.prevPaused = _state.paused;

					if (!_state.paused)
					{
						const auto beforeFrameMs = NowMs();
						const auto prevMs = _state.previousFrameMs.value_or(beforeFrameMs);
						_state.previousFrameMs.emplace(beforeFrameMs);

						const auto frameDiff = beforeFrameMs - prevMs;
						const auto dt = static_cast<float_t>(frameDiff.count()) * 0.001f;

						if (_settings.showFps)
						{
							SetWindowTextA(globalContext->nativeWindow, Core::Text::CombineToString(_settings.windowCaption, " | FPS: ", static_cast<std::uint32_t>(std::round(1.0f / dt))).c_str());
						}

						_app->OnUpdate(dt);
						ClearInstantInputEvents();
						_app->OnRender();

						const auto afterFrame = NowMs();
						const auto diffMs = afterFrame - beforeFrameMs;

						if (diffMs >= targetFrameMilliseconds)
						{
							std::this_thread::yield();
						}
						else
						{
							const auto sleepTimeout = targetFrameMilliseconds - diffMs;
							while (NowMs() - _state.previousFrameMs.value() < sleepTimeout)
							{
								std::this_thread::yield();
							}
						}
					}
					else
					{
						ClearAllInputs();
						std::this_thread::sleep_for(std::chrono::milliseconds{_settings.pausedSleepTimeoutMs});
					}
				}
			}

			_app->OnPause();
			_app.reset();
			return static_cast<std::int32_t>(msg.wParam);
		}

		std::uint32_t AppContainer::GetCurrentScreenRefreshRate() const
		{
			const auto globalContext = _app->GetContext();
			const auto currentWindowMonitor = MonitorFromWindow(globalContext->nativeWindow, MONITOR_DEFAULTTONEAREST);

			MONITORINFOEX monitorInfo {};
			monitorInfo.cbSize = sizeof(MONITORINFOEX);

			DEVMODEA currentDisplaySettings {};
			GetMonitorInfo(currentWindowMonitor, &monitorInfo);
			EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &currentDisplaySettings);

			return static_cast<std::uint32_t>(currentDisplaySettings.dmDisplayFrequency);
		}

		Core::Vector2Uint AppContainer::GetCurrentWindowSize() const
		{
			RECT rect {};
			GetWindowRect(_app->GetContext()->nativeWindow, &rect);

			return Core::Vector2Uint {
				static_cast<std::uint32_t>(rect.right - rect.left),
					static_cast<std::uint32_t>(rect.bottom - rect.top)
			};
		}

		void AppContainer::CreateNativeContainer()
		{
			const auto globalContext = _app->GetContext();
			const auto className = TEXT(_platformSettings.windowClassName.c_str());
			WNDCLASS wc;
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc = MainWndProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = _platformSettings.currentWindowApplicationHandle;
			wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
			wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
			wc.lpszMenuName = nullptr;
			wc.lpszClassName = className;

			RegisterClass(&wc);

			RECT rect = { 0, 0, _settings.initialWindowWidth, _settings.initialWindowHeight };
			//AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
			const auto w = rect.right - rect.left;
			const auto h = rect.bottom - rect.top;

			globalContext->nativeWindow = CreateWindow(className, TEXT(_settings.windowCaption.c_str()),
				WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, nullptr, nullptr, _platformSettings.currentWindowApplicationHandle, this);


			ShowWindow(globalContext->nativeWindow, SW_SHOW);
			UpdateWindow(globalContext->nativeWindow);

		}

		std::unordered_map<WPARAM, Feature::KeyButton> AppContainer::_keyMap {

			{ 0x57, Feature::KeyButton::W },
			{ 0x41, Feature::KeyButton::A },
			{ 0x53, Feature::KeyButton::S },
			{ 0x44, Feature::KeyButton::D },

			{ 0x25, Feature::KeyButton::LeftArrow },
			{ 0x27, Feature::KeyButton::RightArrow },
			{ 0x26, Feature::KeyButton::UpArrow },
			{ 0x28, Feature::KeyButton::DownArrow },

			{ 0x51, Feature::KeyButton::Q },
			{ 0x45, Feature::KeyButton::E },
			{ 0x52, Feature::KeyButton::R },
			{ 0x46, Feature::KeyButton::F },

			{ 0x21, Feature::KeyButton::PageUp },
			{ 0x22, Feature::KeyButton::PageDown },

			{ 0x0D, Feature::KeyButton::Enter },
			{ 0x20, Feature::KeyButton::Space }
		};

		LRESULT AppContainer::MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			const auto container = reinterpret_cast<AppContainer*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

			switch (msg)
			{
			case WM_CREATE:
			{
				const auto pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
				const auto container = reinterpret_cast<AppContainer*>(pcs->lpCreateParams);
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(container));
			}
			return 0;
			case WM_ACTIVATE:
			{
				if (LOWORD(wParam) == WA_INACTIVE)
				{
					container->_state.paused = true;
				}
				else
				{
					container->_state.previousFrameMs.reset();
					container->_state.paused = false;
				}
			}
			return 0;
			case WM_ENTERSIZEMOVE:
			{
				container->_state.paused = true;
				container->_state.resizeInProgress = true;
			}
			return 0;
			case WM_EXITSIZEMOVE:
			{
				container->_state.resizeInProgress = false;
				container->_state.paused = false;
				container->_state.previousFrameMs.reset();
				container->OnWindowChanged();
			}
			return 0;
			case WM_SIZE:
			{
				const auto globalContext = container->_app->GetContext();

				if (wParam == SIZE_MINIMIZED)
				{
					container->_state.paused = true;
					container->_state.minimized = true;
					container->_state.maximized = false;
					container->OnWindowChanged();
				}
				else if (wParam == SIZE_MAXIMIZED)
				{
					container->_state.paused = false;
					container->_state.minimized = false;
					container->_state.maximized = true;
					container->_state.previousFrameMs.reset();
					container->OnWindowChanged();
				}
				else if (wParam == SIZE_RESTORED)
				{
					if (container->_state.minimized)
					{
						container->_state.paused = false;
						container->_state.minimized = false;
						container->_state.previousFrameMs.reset();
						container->OnWindowChanged();
					}
					else if (container->_state.maximized)
					{
						container->_state.paused = false;
						container->_state.maximized = false;
						container->_state.previousFrameMs.reset();
						container->OnWindowChanged();
					}
					else if (!container->_state.resizeInProgress)
					{
						container->OnWindowChanged();
					}
				}
			}
			return 0;
			case WM_DESTROY:
			{
				PostQuitMessage(0);
			}
			return 0;
			case WM_MENUCHAR:
				return MAKELRESULT(0, MNC_CLOSE);
			case WM_KEYDOWN:
				if (_keyMap.find(wParam) != _keyMap.cend())
				{
					container->SetButtonPressedStatus(_keyMap.at(wParam), true);
				}
				return 0;
			case WM_KEYUP:
				if (_keyMap.find(wParam) != _keyMap.cend())
				{
					container->SetButtonPressedStatus(_keyMap.at(wParam), false);
				}
				return 0;
			case WM_LBUTTONDOWN:
				container->SetButtonPressedStatus(Feature::MouseButton::Left, true);
				return 0;
			case WM_LBUTTONUP:
				container->SetButtonPressedStatus(Feature::MouseButton::Left, false);
				return 0;
			case WM_MBUTTONDOWN:
				container->SetButtonPressedStatus(Feature::MouseButton::Middle, true);
				return 0;
			case WM_MBUTTONUP:
				container->SetButtonPressedStatus(Feature::MouseButton::Middle, false);
				return 0;
			case WM_RBUTTONDOWN:
				container->SetButtonPressedStatus(Feature::MouseButton::Right, true);
				return 0;
			case WM_RBUTTONUP:
				container->SetButtonPressedStatus(Feature::MouseButton::Right, false);
				return 0;
			case WM_MOUSEMOVE:
			{
				const auto globalContext = container->_app->GetContext();
				const auto x = GET_X_LPARAM(lParam);
				const auto y = GET_Y_LPARAM(lParam);
				container->UpdateMouseNormalizedPosition({
					max(0.0f, min(static_cast<std::float_t>(x) / static_cast<std::float_t>(globalContext->windowSize.x), 1.0f)),
					max(0.0f, min(static_cast<std::float_t>(y) / static_cast<std::float_t>(globalContext->windowSize.y), 1.0f))
					});
			}
			return 0;
			case WM_MOUSELEAVE:
				container->ClearAllInputs();
				return 0;
			default:
				return DefWindowProc(hWnd, msg, wParam, lParam);
			}
		}
	}
#endif
}
