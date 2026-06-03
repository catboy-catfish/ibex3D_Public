#include <ibex3D/core/runtime.h>
#include <ibex3D/core/application.h>
#include <ibex3D/core/win32.h>

#include <ibex3D/utility/logger.h>
#include <ibex3D/utility/bitUtils.h>

#include <stdio.h>
#include <string>
#include <exception>

// ----------------------------------------------------------------------------------------------------

#define WINDOW_CLASS_NAME "ibex3D Window Class"

struct windowData_t
{
	HINSTANCE hInstance = nullptr;
	HWND hWnd = nullptr;
};

static LRESULT CALLBACK windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	// FIX: Exceptions thrown from this function are not caught by the try-catch block in runtime::run().

	LONG_PTR rtLongPtr = GetWindowLongPtrA(hWnd, GWLP_USERDATA);
	auto rtHandle = reinterpret_cast<runtime*>(GetWindowLongPtrA(hWnd, GWLP_USERDATA));

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
			// The 30th bit of lParam distinguishes between the initial key press (0) and subsequent auto-repeats while held (1).
			
			if (getNthBit(lParam, 30))
			{
				rtHandle->window_onKeyRepeatEvent(static_cast<unsigned int>(wParam));
			}
			else
			{
				rtHandle->window_onKeyDownEvent(static_cast<unsigned int>(wParam));
			}
			
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

bool runtime::initialize(unsigned int wndWidth, unsigned int wndHeight, const char* wndTitle)
{
	try
	{
		if (!initWindow(wndWidth, wndHeight, wndTitle))
		{
			return false;
		}

		if (!initApplication(wndWidth, wndHeight))
		{
			return false;
		}
	}
	catch (const std::exception& e)
	{
		auto descString = std::string("runtime::initialize(): Caught an exception during the initialization stage! Details: ") + e.what() + std::string(".\n");
		logger::logError(descString.c_str(), __FILE__, __LINE__ - 6);

		return false;
	}

	return true;
}

void runtime::run()
{
	try
	{
		if (!isSafeToStartRunning()) return;

		while (m_keepRunningFlag)
		{
			updateWindow();
		}
	}
	catch (const std::exception& e)
	{
		// FIX: This doesn't catch any exceptions thrown by windowProc or deeper in the call stack. Investigate this!
		
		auto descString = std::string("runtime::run(): Caught an exception during the runtime stage! Details: ") + e.what() + std::string(".\n");
		logger::logError(descString.c_str(), __FILE__, __LINE__ - 6);
		
		return;
	}
}

void runtime::update()
{	
	auto endTime = std::chrono::high_resolution_clock::now();

	float deltaTime = std::chrono::duration<float>(endTime - m_startTime).count();
	m_startTime = endTime;

	if (m_application != nullptr)
	{
		m_application->update(deltaTime);
		m_application->render(deltaTime);
	}

	if (m_application->input_isKeyDown(VK_ESCAPE))
	{
		forceClose();
	}
}

void runtime::forceClose()
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

void runtime::cleanup()
{
	cleanupApplication();
	cleanupWindow();
}

void runtime::window_onKeyDownEvent(unsigned int key)
{
	if (m_application != nullptr)
	{
		m_application->input_onKeyDownEvent(key);
	}
}

void runtime::window_onKeyRepeatEvent(unsigned int key)
{
	if (m_application != nullptr)
	{
		m_application->input_onKeyRepeatEvent(key);
	}
}

void runtime::window_onKeyUpEvent(unsigned int key)
{
	if (m_application != nullptr)
	{
		m_application->input_onKeyUpEvent(key);
	}
}

// ----------------------------------------------------------------------------------------------------

void runtime::window_onResizeEvent(unsigned int newWidth, unsigned int newHeight)
{	
	fprintf(stdout, "Window resized to dimensions { %i, %i }\n", newWidth, newHeight);
	
	if (m_windowData != nullptr)
	{
		if (m_application != nullptr)
		{
			m_application->window_onResizeEvent(newWidth, newHeight);
		}
	}
}

void runtime::window_onFocusEvent()
{
	if (m_application != nullptr)
	{
		m_application->window_onFocusEvent();
	}
}

void runtime::window_onUnfocusEvent()
{
	if (m_application != nullptr)
	{
		m_application->window_onUnfocusEvent();
	}
}

void runtime::window_onCloseRequestedEvent()
{
	if (m_application != nullptr)
	{
		m_application->window_onCloseRequestedEvent();
	}

	m_keepRunningFlag = false;
}

// ----------------------------------------------------------------------------------------------------

bool runtime::initWindow(unsigned int wndWidth, unsigned int wndHeight, const char* wndTitle)
{
	auto wndData = new windowData_t;
	m_windowData = wndData;

	wndData->hInstance = GetModuleHandleA(nullptr);

	WNDCLASSEXA wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEXA);
	wndClass.lpszClassName = WINDOW_CLASS_NAME;
	wndClass.hInstance = wndData->hInstance;
	wndClass.lpfnWndProc = windowProc;

	if (RegisterClassExA(&wndClass) == 0)
	{
		logger::logError("runtime::initWindow(): An error occurred while trying to register the window class.", __FILE__, __LINE__ - 2);
		return false;
	}

	DWORD wndStyle = WS_OVERLAPPEDWINDOW;

	RECT wndRect = { 0, 0, static_cast<LONG>(wndWidth), static_cast<LONG>(wndHeight)};
	AdjustWindowRect(&wndRect, wndStyle, FALSE);

	wndData->hWnd = CreateWindowExA
	(
		0,
		WINDOW_CLASS_NAME,
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
		logger::logError("runtime::initWindow(): An error occurred while trying to create the window.", __FILE__, __LINE__ - 18);
		return false;
	}

	ShowWindow(wndData->hWnd, SW_SHOW);
	SetFocus(wndData->hWnd);

	SetWindowLongPtrA(wndData->hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	return true;
}

void runtime::updateWindow()
{	
	MSG msg = {};

	while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void runtime::cleanupWindow()
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
			UnregisterClassA(WINDOW_CLASS_NAME, wndData->hInstance);
			wndData->hInstance = nullptr;
		}

		delete wndData;
		m_windowData = nullptr;
	}
}

// ----------------------------------------------------------------------------------------------------

bool runtime::initApplication(unsigned int wndWidth, unsigned int wndHeight)
{
	m_application = new application;

	auto wndData = static_cast<windowData_t*>(m_windowData);
	if (!m_application->initialize(this, wndData->hWnd))
	{
		logger::logError("runtime::initApplication(): The application failed to initialize.", __FILE__, __LINE__ - 2);
		return false;
	}

	return true;
}

void runtime::cleanupApplication()
{
	if (m_application != nullptr)
	{
		m_application->cleanup();
		delete m_application;
		m_application = nullptr;
	}
}

// ----------------------------------------------------------------------------------------------------

bool runtime::isSafeToStartRunning()
{
	if (m_windowData == nullptr)
	{
		logger::logError("runtime::isSafeToStartRunning(): Not safe to start running because m_windowData is nullptr.", __FILE__, __LINE__ - 2);
		return false;
	}
	
	auto wndData = static_cast<windowData_t*>(m_windowData);

	if (wndData->hInstance == nullptr)
	{
		logger::logError("runtime::isSafeToStartRunning(): Not safe to start running because m_windowData->hInstance is nullptr.", __FILE__, __LINE__ - 2);
		return false;
	}

	if (wndData->hWnd == nullptr)
	{
		logger::logError("runtime::isSafeToStartRunning(): Not safe to start running because m_windowData->hWnd is nullptr.", __FILE__, __LINE__ - 2); 
		return false;
	}

	if (m_application == nullptr)
	{
		logger::logError("runtime::isSafeToStartRunning(): Not safe to start running because m_application is nullptr.", __FILE__, __LINE__ - 2);
		return false;
	}
	else
	{
		if (!m_application->isSafeToStartRunning())
		{
			logger::logError("runtime::isSafeToStartRunning(): Not safe to start running because m_application->isSafeToStartRunning() failed.", __FILE__, __LINE__ - 2);
			return false;
		}
	}

	return true;
}