#include <ibex3D/core/appRuntime.h>
#include <ibex3D/utility/windowsUtils.h>

#include <sampleApp/appInterface.h>

#include <stdio.h>

// ----------------------------------------------------------------------------------------------------
// - Win32-specific stuff -----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

struct windowData_t
{
	const char* className = "";
	HINSTANCE hInstance = nullptr;
	HWND hWnd = nullptr;
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
		case WM_SIZE:
		{
			rtHandle->windowEvent_onResize();
			break;
		}
		case WM_SETFOCUS:
		{
			rtHandle->windowEvent_onFocus();
			break;
		}
		case WM_KILLFOCUS:
		{
			rtHandle->windowEvent_onUnfocus();
			break;
		}
		case WM_CLOSE:
		{
			rtHandle->windowEvent_onClose();
			break;
		}
	}
	
	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

// ----------------------------------------------------------------------------------------------------
// - Main public functions ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool appRuntime::initialize(int wndWidth, int wndHeight, const char* wndTitle)
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

void appRuntime::startRunning()
{
	if (!isSafeToStartRunning()) return;

	while (m_keepRunningFlag)
	{
		updateWindow();
	}
}

void appRuntime::update()
{
	float deltaTime = m_timer.getTimeSinceCheckpoint();
	m_timer.updateCheckpoint();

	if (pAppInterface != nullptr)
	{
		pAppInterface->update(deltaTime);
		pAppInterface->render(deltaTime);
	}
}

void appRuntime::forceClose()
{
	m_keepRunningFlag = false;

	if (pWindowData != nullptr)
	{
		auto wndData = static_cast<windowData_t*>(pWindowData);

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

// ----------------------------------------------------------------------------------------------------
// - Window event functions ---------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

void appRuntime::windowEvent_onResize()
{	
	if (pWindowData != nullptr)
	{
		auto wndData = static_cast<windowData_t*>(pWindowData);

		int wndWidth, wndHeight;
		if (!win32Utils::getWindowDimensions(wndData->hWnd, wndWidth, wndHeight))
		{
			return;
		}

		if (pAppInterface != nullptr)
		{
			pAppInterface->windowEvent_onResize(wndWidth, wndHeight);
		}
	}
}

void appRuntime::windowEvent_onFocus()
{
	if (pAppInterface != nullptr)
	{
		pAppInterface->windowEvent_onFocus();
	}
}

void appRuntime::windowEvent_onUnfocus()
{
	if (pAppInterface != nullptr)
	{
		pAppInterface->windowEvent_onUnfocus();
	}
}

void appRuntime::windowEvent_onClose()
{
	if (pAppInterface != nullptr)
	{
		pAppInterface->windowEvent_onClose();
	}

	m_keepRunningFlag = false;
}

// ----------------------------------------------------------------------------------------------------
// - Window creation functions ------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool appRuntime::initWindow(int wndWidth, int wndHeight, const char* wndTitle)
{
	auto wndData = new windowData_t;
	pWindowData = static_cast<void*>(wndData);

	wndData->className = "ibex3D Window Class";
	wndData->hInstance = GetModuleHandleA(nullptr);

	WNDCLASSEXA wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEXA);
	wndClass.lpszClassName = wndData->className;
	wndClass.hInstance = wndData->hInstance;
	wndClass.lpfnWndProc = windowProc;

	if (RegisterClassExA(&wndClass) == 0)
	{
		printf("RUNTIME ERROR - appRuntime::initWindow(): RegisterClassExA() failed.\n");
		return false;
	}

	DWORD wndStyle = WS_OVERLAPPEDWINDOW;

	RECT wndRect = { 0, 0, wndWidth, wndHeight };
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
		printf("RUNTIME ERROR - appRuntime::initWindow(): CreateWindowExA() failed.\n");
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
	if (pWindowData != nullptr)
	{
		auto wndData = static_cast<windowData_t*>(pWindowData);

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
		pWindowData = nullptr;
	}
}

// ----------------------------------------------------------------------------------------------------
// - Application functions ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool appRuntime::initApplication(int wndWidth, int wndHeight)
{
	pAppInterface = new appInterface;

	auto wndData = static_cast<windowData_t*>(pWindowData);
	if (!pAppInterface->initialize(this, static_cast<void*>(wndData->hWnd)))
	{
		printf("RUNTIME ERROR - appRuntime::initApplication(): appInterface::initialize() failed.\n");
		return false;
	}

	return true;
}

void appRuntime::cleanupApplication()
{
	if (pAppInterface != nullptr)
	{
		pAppInterface->cleanup();
		delete pAppInterface;
		pAppInterface = nullptr;
	}
}

// ----------------------------------------------------------------------------------------------------
// - Helper functions ---------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool appRuntime::isSafeToStartRunning()
{
	if (pWindowData == nullptr)
	{
		printf("RUNTIME ERROR - appRuntime::isSafeToStartRunning(): pWindowData is nullptr.\n");
		return false;
	}
	
	auto wndData = static_cast<windowData_t*>(pWindowData);

	if (wndData->hInstance == nullptr)
	{
		printf("RUNTIME ERROR - appRuntime::isSafeToStartRunning(): pWindowData->hInstance is nullptr.\n");
		return false;
	}

	if (wndData->hWnd == nullptr)
	{
		printf("RUNTIME ERROR - appRuntime::isSafeToStartRunning(): pWindowData->hWnd is nullptr.\n");
		return false;
	}

	if (pAppInterface == nullptr)
	{
		printf("RUNTIME ERROR - appRuntime::isSafeToStartRunning(): pAppInterface is nullptr.\n");
		return false;
	}
	else
	{
		if (!pAppInterface->isSafeToStartRunning())
		{
			printf("RUNTIME ERROR - appRuntime::isSafeToStartRunning(): pAppInterface->isSafeToStartRunning() returned false.\n");
			return false;
		}
	}

	return true;
}