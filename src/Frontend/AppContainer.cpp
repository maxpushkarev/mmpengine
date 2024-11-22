#include <thread>
#include <Frontend/AppContainer.hpp>

namespace MMPEngine::Frontend
{
	AppContainer::AppContainer(Settings&& settings, const std::shared_ptr<Core::BaseRootApp>& app) : _settings(std::move(settings)), _app(app)
	{
		_inputController = std::make_shared<Core::AppInputController>(app);
	}

	void AppContainer::OnWindowChanged()
	{
		if(!_state.appInitialized)
		{
			return;
		}

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
		_app->OnNativeWindowUpdated();
		return RunInternal();
	}

#ifdef MMPENGINE_WIN
	namespace Win
	{
		AppContainer::AppContainer(PlatformAppContainer::Settings&& settings, const std::shared_ptr<Core::BaseRootApp>& app) :
			PlatformAppContainer<MMPEngine::Frontend::Win::AppContainerSetting>(std::move(settings), app)
		{
			
		}

		std::int32_t AppContainer::RunInternal()
		{
			MSG msg = {};
			const auto appContext = _app->GetContext();
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
					tme.hwndTrack = appContext->nativeWindow;
					TrackMouseEvent(&tme);

					if(_state.prevPaused.value_or(_state.paused) != _state.paused)
					{
						if(_state.paused)
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
							SetWindowTextA(appContext->nativeWindow, Core::Text::CombineToString(_settings.windowCaption, " | FPS: ", static_cast<std::uint32_t>(std::round(1.0f / dt))).c_str());
						}

						_app->OnUpdate(dt);
						_inputController->ClearInstantEvents();
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
						_inputController->ClearAll();
						std::this_thread::sleep_for(std::chrono::milliseconds{_settings.pausedSleepTimeoutMs});
					}
				}
			}

			_app->OnPause();
			_app.reset();
			return static_cast<std::int32_t>(msg.wParam);
		}


		void AppContainer::CreateNativeContainer()
		{
			const auto appContext = _app->GetContext();
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
			AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
			appContext->windowSize.x = rect.right - rect.left;
			appContext->windowSize.y = rect.bottom - rect.top;

			appContext->nativeWindow = CreateWindow(className, TEXT(_settings.windowCaption.c_str()),
				WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, appContext->windowSize.x, appContext->windowSize.y, nullptr, nullptr, _platformSettings.currentWindowApplicationHandle, this);


			ShowWindow(appContext->nativeWindow, SW_SHOW);
			UpdateWindow(appContext->nativeWindow);

		}

		std::unordered_map<WPARAM, Core::KeyButton> AppContainer::_keyMap {

			{ 0x57, Core::KeyButton::W },
			{ 0x41, Core::KeyButton::A },
			{ 0x53, Core::KeyButton::S },
			{ 0x44, Core::KeyButton::D },

			{ 0x25, Core::KeyButton::LeftArrow },
			{ 0x27, Core::KeyButton::RightArrow },
			{ 0x26, Core::KeyButton::UpArrow },
			{ 0x28, Core::KeyButton::DownArrow },

			{ 0x51, Core::KeyButton::Q },
			{ 0x45, Core::KeyButton::E },
			{ 0x52, Core::KeyButton::R },
			{ 0x46, Core::KeyButton::F },

			{ 0x21, Core::KeyButton::PageUp },
			{ 0x22, Core::KeyButton::PageDown },

			{ 0x0D, Core::KeyButton::Enter },
			{ 0x20, Core::KeyButton::Space }
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
					const auto appContext = container->_app->GetContext();
					appContext->windowSize.x = LOWORD(lParam);
					appContext->windowSize.y = HIWORD(lParam);

					if (wParam == SIZE_MINIMIZED)
					{
						container->_state.paused = true;
						container->_state.minimized = true;
						container->_state.maximized = false;
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
				if(_keyMap.find(wParam) != _keyMap.cend())
				{
					container->_inputController->SetButtonPressedStatus(_keyMap.at(wParam), true);	
				}
				return 0;
			case WM_KEYUP:
				if (_keyMap.find(wParam) != _keyMap.cend())
				{
					container->_inputController->SetButtonPressedStatus(_keyMap.at(wParam), false);
				}
				return 0;
			case WM_LBUTTONDOWN:
				container->_inputController->SetButtonPressedStatus(Core::MouseButton::Left, true);
				return 0;
			case WM_LBUTTONUP:
				container->_inputController->SetButtonPressedStatus(Core::MouseButton::Left, false);
				return 0;
			case WM_MBUTTONDOWN:
				container->_inputController->SetButtonPressedStatus(Core::MouseButton::Middle, true);
				return 0;
			case WM_MBUTTONUP:
				container->_inputController->SetButtonPressedStatus(Core::MouseButton::Middle, false);
				return 0;
			case WM_RBUTTONDOWN:
				container->_inputController->SetButtonPressedStatus(Core::MouseButton::Right, true);
				return 0;
			case WM_RBUTTONUP:
				container->_inputController->SetButtonPressedStatus(Core::MouseButton::Right, false);
				return 0;
			case WM_MOUSEMOVE:
				{
					const auto appContext = container->_app->GetContext();
					const auto x = GET_X_LPARAM(lParam);
					const auto y = GET_Y_LPARAM(lParam);
					container->_inputController->UpdateMouseNormalizedPosition({
						max(0.0f, min(static_cast<std::float_t>(x) / static_cast<std::float_t>(appContext->windowSize.x), 1.0f)),
						max(0.0f, min(static_cast<std::float_t>(y) / static_cast<std::float_t>(appContext->windowSize.y), 1.0f))
					});
				}
				return 0;
			case WM_MOUSELEAVE:
				container->_inputController->ClearAll();
				return 0;
			default:
				return DefWindowProc(hWnd, msg, wParam, lParam);
			}
		}
	}
#endif

}
