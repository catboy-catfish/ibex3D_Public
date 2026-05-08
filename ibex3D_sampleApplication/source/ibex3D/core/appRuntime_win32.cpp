#include <ibex3D/core/appRuntime.h>
#include <ibex3D/core/win32.h>

#include <ibex3D/utility/logger.h>

#include <sampleApp/appInterface.h>

#include <stdio.h>

// ----------------------------------------------------------------------------------------------------
// - Win32-specific stuff -----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

struct windowData_t
{
	HINSTANCE hInstance = nullptr;
	HWND hWnd = nullptr;
	const char* className = "";
};

static LRESULT CALLBACK windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR rtLongPtr = GetWindowLongPtrA(hWnd, GWLP_USERDATA);
	auto rtHandle = reinterpret_cast<appRuntime*>(GetWindowLongPtrA(hWnd, GWLP_USERDATA));

	if (rtHandle == nullptr)
	{
		return DefWindowProcA(hWnd, msg, wParam, lParam);
	}

	switch (msg)
	{
		case WM_PAINT:
		{
			rtHandle->update();
			return 0;
		}
		case WM_KEYDOWN:
		{
			rtHandle->window_onKeyDownEvent(static_cast<unsigned int>(wParam));
			break;
		}
		case WM_KEYUP:
		{
			rtHandle->window_onKeyUpEvent(static_cast<unsigned int>(wParam));
			break;
		}
		case WM_SIZE:
		{
			auto newWidth = static_cast<unsigned int>(LOWORD(lParam));
			auto newHeight = static_cast<unsigned int>(HIWORD(lParam));
			
			rtHandle->window_onResizeEvent(newWidth, newHeight);
			break;
		}
		case WM_SETFOCUS:
		{
			rtHandle->window_onFocusEvent();
			break;
		}
		case WM_KILLFOCUS:
		{
			rtHandle->window_onUnfocusEvent();
			break;
		}
		case WM_CLOSE:
		{
			rtHandle->window_onCloseRequestedEvent();
			break;
		}
	}
	
	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

// ----------------------------------------------------------------------------------------------------
// - Main public functions ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool appRuntime::initialize(unsigned int wndWidth, unsigned int wndHeight, const char* wndTitle)
{
	if (!initWindow(wndWidth, wndHeight, wndTitle))
	{
		return false;
	}

	if (!initApplication(wndWidth, wndHeight))
	{
	 	return false;
	}

	return true;
}

void appRuntime::run()
{
	if (!isSafeToStartRunning()) return;

	while (m_keepRunningFlag)
	{
		updateWindow();
	}
}

void appRuntime::update()
{	
	auto endTime = std::chrono::high_resolution_clock::now();

	float deltaTime = std::chrono::duration<float>(endTime - m_startTime).count();
	m_startTime = endTime;

	if (m_appInterface != nullptr)
	{
		m_appInterface->update(deltaTime);
		m_appInterface->render(deltaTime);
	}

	if (m_appInterface->input_isKeyDown(VK_ESCAPE))
	{
		forceClose();
	}
}

void appRuntime::forceClose()
{
	m_keepRunningFlag = false;

	if (m_windowData != nullptr)
	{
		auto wndData = static_cast<windowData_t*>(m_windowData);

		if (wndData->hWnd != nullptr)
		{
			DestroyWindow(wndData->hWnd);
			wndData->hWnd = nullptr;
		}
	}
}

void appRuntime::cleanup()
{
	cleanupApplication();
	cleanupWindow();
}

void appRuntime::window_onKeyDownEvent(unsigned int key)
{
	if (m_appInterface != nullptr)
	{
		m_appInterface->input_onKeyDownEvent(key);
	}
}

void appRuntime::window_onKeyUpEvent(unsigned int key)
{
	if (m_appInterface != nullptr)
	{
		m_appInterface->input_onKeyUpEvent(key);
	}
}

// ----------------------------------------------------------------------------------------------------
// - Window event functions ---------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

void appRuntime::window_onResizeEvent(unsigned int newWidth, unsigned int newHeight)
{	
	fprintf(stdout, "Window resized to dimensions { %i, %i }\n", newWidth, newHeight);
	
	if (m_windowData != nullptr)
	{
		if (m_appInterface != nullptr)
		{
			m_appInterface->window_onResizeEvent(newWidth, newHeight);
		}
	}
}

void appRuntime::window_onFocusEvent()
{
	if (m_appInterface != nullptr)
	{
		m_appInterface->window_onFocusEvent();
	}
}

void appRuntime::window_onUnfocusEvent()
{
	if (m_appInterface != nullptr)
	{
		m_appInterface->window_onUnfocusEvent();
	}
}

void appRuntime::window_onCloseRequestedEvent()
{
	if (m_appInterface != nullptr)
	{
		m_appInterface->window_onCloseRequestedEvent();
	}

	m_keepRunningFlag = false;
}

// ----------------------------------------------------------------------------------------------------
// - Window creation functions ------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool appRuntime::initWindow(unsigned int wndWidth, unsigned int wndHeight, const char* wndTitle)
{
	auto wndData = new windowData_t;
	m_windowData = static_cast<void*>(wndData);

	wndData->className = "ibex3D Window Class";
	wndData->hInstance = GetModuleHandleA(nullptr);

	WNDCLASSEXA wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEXA);
	wndClass.lpszClassName = wndData->className;
	wndClass.hInstance = wndData->hInstance;
	wndClass.lpfnWndProc = windowProc;

	if (RegisterClassExA(&wndClass) == 0)
	{
		logger::logError("appRuntime::initWindow(): An error occurred while trying to register the window class.", __FILE__, __LINE__ - 2);
		return false;
	}

	DWORD wndStyle = WS_OVERLAPPEDWINDOW;

	RECT wndRect = { 0, 0, static_cast<LONG>(wndWidth), static_cast<LONG>(wndHeight)};
	AdjustWindowRect(&wndRect, wndStyle, FALSE);

	wndData->hWnd = CreateWindowExA
	(
		0,
		wndData->className,
		wndTitle,
		wndStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wndRect.right - wndRect.left,
		wndRect.bottom - wndRect.top,
		nullptr,
		nullptr,
		wndData->hInstance,
		nullptr
	);

	if (wndData->hWnd == nullptr)
	{
		logger::logError("appRuntime::initWindow(): An error occurred while trying to create the window.", __FILE__, __LINE__ - 18);
		return false;
	}

	ShowWindow(wndData->hWnd, SW_SHOW);
	SetFocus(wndData->hWnd);

	SetWindowLongPtrA(wndData->hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	return true;
}

void appRuntime::updateWindow()
{
	MSG msg = {};

	while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void appRuntime::cleanupWindow()
{
	if (m_windowData != nullptr)
	{
		auto wndData = static_cast<windowData_t*>(m_windowData);

		if (wndData->hWnd != nullptr)
		{
			DestroyWindow(wndData->hWnd);
			wndData->hWnd = nullptr;
		}

		if (wndData->hInstance != nullptr)
		{
			UnregisterClassA(wndData->className, wndData->hInstance);
			wndData->hInstance = nullptr;
		}

		delete wndData;
		m_windowData = nullptr;
	}
}

// ----------------------------------------------------------------------------------------------------
// - Application functions ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool appRuntime::initApplication(unsigned int wndWidth, unsigned int wndHeight)
{
	m_appInterface = new appInterface;

	auto wndData = static_cast<windowData_t*>(m_windowData);
	if (!m_appInterface->initialize(this, static_cast<void*>(wndData->hWnd)))
	{
		logger::logError("appRuntime::initApplication(): The appInterface failed to initialize.", __FILE__, __LINE__ - 2);
		return false;
	}

	return true;
}

void appRuntime::cleanupApplication()
{
	if (m_appInterface != nullptr)
	{
		m_appInterface->cleanup();
		delete m_appInterface;
		m_appInterface = nullptr;
	}
}

// ----------------------------------------------------------------------------------------------------
// - Helper functions ---------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool appRuntime::isSafeToStartRunning()
{
	if (m_windowData == nullptr)
	{
		logger::logError("appRuntime::isSafeToStartRunning(): Not safe to start running because m_windowData is nullptr.", __FILE__, __LINE__ - 2);
		return false;
	}
	
	auto wndData = static_cast<windowData_t*>(m_windowData);

	if (wndData->hInstance == nullptr)
	{
		logger::logError("appRuntime::isSafeToStartRunning(): Not safe to start running because m_windowData->hInstance is nullptr.", __FILE__, __LINE__ - 2);
		return false;
	}

	if (wndData->hWnd == nullptr)
	{
		logger::logError("appRuntime::isSafeToStartRunning(): Not safe to start running because m_windowData->hWnd is nullptr.", __FILE__, __LINE__ - 2); 
		return false;
	}

	if (m_appInterface == nullptr)
	{
		logger::logError("appRuntime::isSafeToStartRunning(): Not safe to start running because m_appInterface is nullptr.", __FILE__, __LINE__ - 2);
		return false;
	}
	else
	{
		if (!m_appInterface->isSafeToStartRunning())
		{
			logger::logError("appRuntime::isSafeToStartRunning(): Not safe to start running because m_appInterface->isSafeToStartRunning() failed.", __FILE__, __LINE__ - 2);
			return false;
		}
	}

	return true;
}