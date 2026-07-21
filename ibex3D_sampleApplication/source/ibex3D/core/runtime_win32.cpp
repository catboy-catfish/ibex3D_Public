#include <ibex3D/core/runtime_win32.h>
#include <ibex3D/core/application_win32.h>

#include <chrono>
#include <stdio.h>

#define WNDCLASS_NAME "ibex3D Window Class"

static LRESULT CALLBACK wndProcWrapper(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto rtPtr = reinterpret_cast<i3D_runtime_win32*>(GetWindowLongPtrA(hWnd, GWLP_USERDATA));

	if (rtPtr != nullptr)
	{
		return rtPtr->wndProc(hWnd, msg, wParam, lParam);
	}
	
	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

bool i3D_runtime_win32::initWindow(int width, int height, const char* title)
{
	HINSTANCE hInstance = GetModuleHandleA(NULL);
	
	WNDCLASSEXA wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEXA);
	wndClass.lpszClassName = WNDCLASS_NAME;
	wndClass.hInstance = hInstance;
	wndClass.lpfnWndProc = wndProcWrapper;
	
	if (RegisterClassExA(&wndClass) == 0)
	{
		fprintf(stderr, "WIN32 ERROR: Failed to register the window class.\n");
		return false;
	}

	DWORD wndStyle = WS_OVERLAPPEDWINDOW;

	RECT wndRect = { 0, 0, width, height };
	AdjustWindowRect(&wndRect, wndStyle, FALSE);

	m_hWnd = CreateWindowExA
	(
		0,
		WNDCLASS_NAME, title,
		wndStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wndRect.right - wndRect.left, wndRect.bottom - wndRect.top,
		NULL, NULL, hInstance, NULL
	);

	if (m_hWnd == NULL)
	{
		fprintf(stderr, "WIN32 ERROR: Failed to create the window.\n");
		return false;
	}

	SetWindowLongPtrA(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	ShowWindow(m_hWnd, SW_SHOW);
	SetFocus(m_hWnd);

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
	}

	HINSTANCE hInstance = GetModuleHandleA(NULL);
	UnregisterClassA(WNDCLASS_NAME, hInstance);
}

bool i3D_runtime_win32::initApplication()
{
	m_application = new i3D_application_win32;
	
	if (!m_application->initialize(this, m_hWnd))
	{
		return false;
	}

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

void i3D_runtime_win32::cleanupApplication()
{
	if (m_application != nullptr)
	{
		m_application->cleanup();

		delete m_application;
		m_application = nullptr;
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
			case WM_KEYDOWN:
			{
				// Isolating the 30th bit from lParam tells us whether or not the message is from an initial key press or auto-repeat
				if ((lParam >> 30) & 1) {
					m_application->onKeyAutoRepeat(wParam);
				}
				else {
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
				close();
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
		return false;
	}

	if (!initApplication())
	{
		return false;
	}
	
	return true;
}

void i3D_runtime_win32::startRunning()
{
	while (m_keepRunning)
	{
		updateWindow();
	}
}

void i3D_runtime_win32::close()
{
	if (m_application != nullptr)
	{
		m_application->onWindowCloseRequest();
	}
	
	m_keepRunning = false;
}

void i3D_runtime_win32::cleanup()
{
	cleanupApplication();
	cleanupWindow();
}
