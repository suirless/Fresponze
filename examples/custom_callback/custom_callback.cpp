/*********************************************************************
* Copyright (C) Anton Kovalev (vertver), 2019-2020. All rights reserved.
* Copyright (C) Suirless, 2020. All rights reserved.
* Fresponze - fast, simple and modern multimedia sound library
* Apache-2 License
**********************************************************************
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************/
#include "Fresponze.h"
#include "FresponzeWavFile.h"
#include "FresponzeListener.h"
#include "FresponzeMixer.h"
#include "FresponzeMasterEmitter.h"

IFresponze* pFresponze = nullptr;

#include "imgui.h"
#include "spectrum.h"
#ifdef WINDOWS_PLATFORM
#include <windows.h>
#include "FresponzeFileSystemWindows.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#elif defined(LINUX_PLATFORM)
#include "FresponzeAlsaEnumerator.h"
#include <stdio.h>
#include "imgui_impl_opengl3.h"

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
#elif defined(MACOS_PLATFORM)
#endif

class CCustomAudioCallback final : public IAudioCallback
{
protected:
	PcmFormat fmt = {};  

public:
	CCustomAudioCallback()
	{
		AddRef();
	}

	fr_err FlushCallback() override
	{
		return 0;
	}

	fr_err FormatCallback(PcmFormat* fmtToSwitch) override
	{
		if (!fmtToSwitch) return -1;
		memcpy(&fmt, fmtToSwitch, sizeof(PcmFormat));
	}

	fr_err EndpointCallback(fr_f32* pData, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate, fr_i32 CurrentEndpointType) override
	{
		if (CurrentEndpointType == RenderType) {
			/* #TODO: Your custom callback update and render code here */
			static bool state = false;
			static fr_f32 phase = 0.f;
			static fr_f32 freq = 150.f;
			fr_f32* pBuf = (fr_f32*)pData;
			for (size_t i = 0; i < (size_t)Frames * (size_t)Channels; i++) {
				if (freq >= 600.f / Channels) state = !state;
				pBuf[i] = sinf(phase * 6.283185307179586476925286766559005f) * 0.1f;
				phase = fmodf(phase + freq / SampleRate, 1.0f);
				freq = !state ? freq + 0.001f : freq - 0.001f;
				if (freq <= 300.f / Channels) state = !state;
			}
		}

		return 0;
	}

	fr_err RenderCallback(fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate)
	{
		return 0;
	}
};

fr_i32 OutputCount = 0;
fr_i32 InputCount = 0;
ListenersNode* listNode = nullptr;
PcmFormat format = {};
EndpointInformation OutputsLists = {};
EndpointInformation InputsLists = {};
IBaseEmitter* pBaseEmitter = nullptr;
IBaseEmitter* pBaseEmitterSecond = nullptr;
IAudioHardware* pAudioHardware = nullptr;
IAdvancedMixer* pAdvancedMixer = nullptr;
IAudioCallback* pAudioCallback = nullptr;

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

#ifdef WINDOWS_PLATFORM
// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

void DrawImGui()
{
#ifdef WINDOWS_PLATFORM
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
#else
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
#endif
	ImGui::NewFrame();

	{
		static int current_delay = 1;
		static int current_item = 0;
		static float volume = 1.0f;
		static int counter = 0;
		static bool is_already_runned = false;
		static float session_volume = 0.5f;

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::Begin("Fresponze custom callback", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		if (ImGui::Button("Press me", ImVec2(ImGui::GetIO().DisplaySize.x - 20, ImGui::GetIO().DisplaySize.y - 100))) {
			if (is_already_runned) {
				pAudioHardware->Close();
				is_already_runned = false;
			} else {
				pAudioHardware->Open(RenderType, 100.f);
				is_already_runned = true;
			}
		}

		/*
			This value function provides to system API to control audio
			sessions. You can change it to your custom or use software volume level
		*/
		if (ImGui::SliderFloat("Session volume", &session_volume, 0.f, 1.f)) {
			pAudioHardware->SetVolume(session_volume);
		}

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	ImGui::Render();

#ifdef WINDOWS_PLATFORM
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	g_pSwapChain->Present(1, 0);
#endif
}

// Main code
int main()
{
#ifdef WINDOWS_PLATFORM
	// Create application window
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Fresponze device enumerating"), NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Fresponze custom callback"), WS_OVERLAPPEDWINDOW, 100, 100, 500, 300, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd)) {
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);
#else
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }
#endif

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::Spectrum::StyleColorsSpectrum();

#ifdef WINDOWS_PLATFORM
	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
#else
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
#endif

	ImFontConfig font_config;
	font_config.OversampleH = 1; //or 2 is the same
	font_config.OversampleV = 1;
	font_config.PixelSnapH = true;
	ImFont* Fonts[16] = {};

	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x052F, // Cyrillic
		0,
	};

	Fonts[0] = io.Fonts->AddFontDefault();//io.Fonts->AddFontFromFileTTF("Montserrat-Medium.ttf", 18.0f, &font_config, ranges);
	io.Fonts->Build();

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;

	/* Initialize internal instance of library */
	if (FrInitializeInstance((void**)&pFresponze) != 0) {
		return -1;
	}

	/* 
		#WARNING:
		In this case, we can use custom callback with your handler, but on Windows you
		must process buffer equals or smaller device buffer length, because system
		buffer padding can't be always 0 or max buffer size.
	*/
	pAudioCallback = new CCustomAudioCallback();

	/* Create our system dependent hardware */
	if constexpr ((SUPPORTED_HOSTS & eWindowsCoreHost)) {
		pFresponze->GetHardwareInterface(eEndpointWASAPIType, pAudioCallback, (void**)&pAudioHardware);
	}
	else if constexpr ((SUPPORTED_HOSTS & eLinuxPlatform)) {
		pFresponze->GetHardwareInterface(eEndpointAlsaType, pAudioCallback, (void**)&pAudioHardware);
	}

	IAudioEndpoint* pAudioEndpoint = nullptr;
	EndpointInformation* pEndpointInfo = nullptr;

#ifdef WINDOWS_PLATFORM
	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		DrawImGui();
	}

	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	DestroyWindow(hwnd);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
#else
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        DrawImGui();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
#endif

	FrDestroyInstance(pFresponze);
	return 0;
}

// Helper functions
#ifdef WINDOWS_PLATFORM
bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool Window_Flag_Resizeing = false;

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ENTERSIZEMOVE:
		/*
			HACK: with this timer, we can draw window with new size
			while this window in resizing state
		*/
		SetTimer(hWnd, 2, 4, NULL);
		Window_Flag_Resizeing = true;
		return 0;
	case WM_EXITSIZEMOVE:
		KillTimer(hWnd, 2);
		Window_Flag_Resizeing = false;
		return 0;
	case WM_PAINT:
		if (g_pd3dDevice != NULL && Window_Flag_Resizeing) {
			DrawImGui();
		}
		break;
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
			CleanupRenderTarget();

			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
		} else {
			Sleep(1);
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}


	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

#endif

