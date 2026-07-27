#include <ibex3D/core/runtime_win32.h>
#include <ibex3D/core/application_win32.h>
#include <ibex3D/core/logger.h>

#include <chrono>

// ----------------------------------------------------------------------------------------------------

#define WNDCLASS_NAME "ibex3D Window Class"

// ----------------------------------------------------------------------------------------------------

static LRESULT CALLBACK wndProcWrapper(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto rtPtr = reinterpret_cast<i3D_runtime_win32*>(GetWindowLongPtrA(hWnd, GWLP_USERDATA));

	if (rtPtr != nullptr)
	{
		return rtPtr->wndProc(hWnd, msg, wParam, lParam);
	}
	
	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

// ----------------------------------------------------------------------------------------------------

bool i3D_runtime_win32::initWindow(int width, int height, const char* title)
{
	HINSTANCE hInstance = GetModuleHandleA(NULL);
	
	WNDCLASSEXA wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEXA);
	wndClass.lpszClassName = "ibex3D Window Class";
	wndClass.hInstance = hInstance;
	wndClass.lpfnWndProc = wndProcWrapper;
	
	if (RegisterClassExA(&wndClass) == 0)
	{
		i3D_logErrorMessage("RUNTIME ERROR: Failed to register the Win32 window class.\n");
		return false;
	}

	DWORD wndStyle = WS_OVERLAPPEDWINDOW;

	RECT wndRect = { 0, 0, width, height };
	AdjustWindowRect(&wndRect, wndStyle, FALSE);

	m_hWnd = CreateWindowExA
	(
		0, "ibex3D Window Class", title, wndStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, wndRect.right - wndRect.left, wndRect.bottom - wndRect.top,
		NULL, NULL, hInstance, NULL
	);

	if (m_hWnd == NULL)
	{
		i3D_logErrorMessage("RUNTIME ERROR: Failed to create the Win32 window.\n");
		return false;
	}

	SetWindowLongPtrA(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	ShowWindow(m_hWnd, SW_SHOW);
	SetFocus(m_hWnd);

	i3D_logInfoMessage("RUNTIME INFO: Successfully created the Win32 window.\n");
	return true;
}

void i3D_runtime_win32::updateWindow()
{
	MSG msg = {};

	while (PeekMessageA(&msg, m_hWnd, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void i3D_runtime_win32::cleanupWindow()
{
	if (m_hWnd != nullptr)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;

		i3D_logInfoMessage("RUNTIME INFO: Destroyed the Win32 window.\n");
	}

	HINSTANCE hInstance = GetModuleHandleA(NULL);
	UnregisterClassA(WNDCLASS_NAME, hInstance);

	i3D_logInfoMessage("RUNTIME INFO: Unregistered the Win32 window class.\n");
}

bool i3D_runtime_win32::initApplication()
{
	m_application = new i3D_application_win32;
	
	if (!m_application->initialize(this, m_hWnd))
	{
		i3D_logErrorMessage("RUNTIME INFO: Failed to initialize the application.\n");
		return false;
	}

	i3D_logInfoMessage("RUNTIME INFO: Successfully initialized the application.\n");
	return true;
}

void i3D_runtime_win32::updateApplication()
{
	auto startTime = std::chrono::high_resolution_clock::now();

	if (m_application != nullptr)
	{
		m_application->update(m_deltaTime);
		m_application->render(m_deltaTime);
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	m_deltaTime = std::chrono::duration<float>(endTime - startTime).count();
}

void i3D_runtime_win32::closeApplication()
{	
	if (m_application != nullptr)
	{
		m_application->onWindowCloseRequest();
	}

	m_keepRunning = false;
	i3D_logInfoMessage("RUNTIME INFO: Started to close the application.\n");
}

void i3D_runtime_win32::cleanupApplication()
{
	if (m_application != nullptr)
	{
		m_application->cleanup();
		delete m_application;
		m_application = nullptr;

		i3D_logInfoMessage("RUNTIME INFO: Cleaned up the application.\n");
	}
}

LRESULT i3D_runtime_win32::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	if (m_application != nullptr)
	{
		switch (msg)
		{
			case WM_PAINT:
			{
				updateApplication();
				return 0;
			}
			case WM_LBUTTONDOWN:
			{
				break;
			}
			case WM_LBUTTONUP:
			{
				break;
			}
			case WM_RBUTTONDOWN:
			{
				break;
			}
			case WM_RBUTTONUP:
			{
				break;
			}
			case WM_MBUTTONDOWN:
			{
				break;
			}
			case WM_MBUTTONUP:
			{
				break;
			}
			case WM_MOUSEWHEEL:
			{
				break;
			}
			case WM_KEYDOWN:
			{
				// Isolating the 30th bit from lParam tells us whether or not the message is from an auto-repeat or initial key press
				if ((lParam >> 30) & 1)
				{
					m_application->onKeyAutoRepeat(wParam);
				}
				else
				{
					m_application->onKeyDown(wParam);
				}

				break;
			}
			case WM_KEYUP:
			{
				m_application->onKeyUp(wParam);
				break;
			}
			case WM_SIZE:
			{
				m_application->onWindowResize(LOWORD(lParam), HIWORD(lParam));
				break;
			}
			case WM_SETFOCUS:
			{
				m_application->onWindowFocus();
				break;
			}
			case WM_KILLFOCUS:
			{
				m_application->onWindowUnfocus();
				break;
			}
			case WM_CLOSE:
			{
				closeApplication();
				break;
			}
		}
	}
	
	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

bool i3D_runtime_win32::initialize(int wndWidth, int wndHeight, const char* wndTitle)
{
	if (!initWindow(wndWidth, wndHeight, wndTitle))
	{
		i3D_logErrorMessage("RUNTIME ERROR: Couldn't initialize the runtime because the Win32 window failed to initialize.\n");
		return false;
	}

	if (!initApplication())
	{
		i3D_logErrorMessage("RUNTIME ERROR: Couldn't initialize the runtime because the application failed to initialize.\n");
		return false;
	}
	
	i3D_logInfoMessage("RUNTIME INFO: Successfully initialized the runtime.\n");
	return true;
}

void i3D_runtime_win32::startRunning()
{
	while (m_keepRunning)
	{
		updateWindow();
	}
}

void i3D_runtime_win32::cleanup()
{	
	cleanupApplication();
	cleanupWindow();

	i3D_logInfoMessage("RUNTIME INFO: Cleaned up the whole runtime.\n");
}

void i3D_runtime_win32::setWindowTitle(const char* value)
{
	SetWindowTextA(m_hWnd, value);
}

void i3D_runtime_win32::setCursorVisibility(bool value)
{	
	// No way to lock/constrain the cursor in ibex3D yet, unfortunately
	ShowCursor(value);
}