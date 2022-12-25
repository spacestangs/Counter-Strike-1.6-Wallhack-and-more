#include "../../../backend.hpp"
#include "../../../console/console.hpp"

#ifdef ENABLE_BACKEND_OPENGL
#include <Windows.h>

#include <memory>

#include "hook_opengl.hpp"

#include "../../../dependencies/imgui/imgui_impl_opengl3.h"
#include "../../../dependencies/imgui/imgui_impl_win32.h"
#include "../../../dependencies/minhook/MinHook.h"

#include "../../hooks.hpp"

#include "../../../dependencies/pch.h"
#include "../../../menu/menu.hpp"
#include <gl/GL.h>
#include <gl/GLU.h>

bool wallhack = false;
bool perfectwallhack = false;
bool smoke = false;
bool flash = false;
bool lambert = false;
bool blackskys = false;
namespace ig = ImGui;

namespace Menu {
    void InitializeContext(HWND hwnd) {
        if (ig::GetCurrentContext( ))
            return;

        ImGui::CreateContext( );
        ImGui_ImplWin32_Init(hwnd);

        ImGuiIO& io = ImGui::GetIO( );
        io.IniFilename = io.LogFilename = nullptr;
    }

    void Render( ) {
        if (!bShowMenu)
            return;

        static bool show_window = true;
        ImGui::SetNextWindowSize(ImVec2(600.000f, 400.000f), ImGuiCond_Once);
        ImGui::Begin("Counter-Strike 1.6 MultiCheat By SpaceStangs8 %.1f FPS", NULL, 64);
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO( ).Framerate, ImGui::GetIO( ).Framerate);
        //ImGui::Text("8 Counter-Strike 1.6 MultiCheat", ImVec2(0.000f, 0.000f));

        ImGui::Text("NUM 1 - Wall Hack");

        ImGui::Text("NUM 2 - Wall Hack 2");

        ImGui::Text("NUM 3 - Remove Smoke");

        ImGui::Text("NUM 4 - Black Sky");

        ImGui::Text("NUM 5 - Lambert");

        ImGui::Text("NUM 6 - Remove Flash");

        ImGui::Text("NUM 7 - Night Mode");

        ImGui::Text("NUM 8 - White walls");

        ImGui::Text("NUM 9 - Inverted Models");
        
        ImGui::End( );
    }

} // namespace Menu

typedef void(APIENTRY* glBegin_t)(GLenum);
typedef void(APIENTRY* glEnd_t)(void);
typedef void(APIENTRY* glVertex3fv_t)(const GLfloat* v);
typedef void(APIENTRY* glClear_t)(GLbitfield);

glBegin_t pglBegin = NULL;
glEnd_t pglEnd = NULL;
glVertex3fv_t pglVertex3fv = NULL;
glClear_t pglClear = NULL;

GLenum g_glMode;

static std::add_pointer_t<BOOL WINAPI(HDC)> oWglSwapBuffers;
static BOOL WINAPI hkWglSwapBuffers(HDC Hdc) {
    if (!H::bShuttingDown && ImGui::GetCurrentContext( )) {
        if (!ImGui::GetIO( ).BackendRendererUserData)
            ImGui_ImplOpenGL3_Init( );

        ImGui_ImplOpenGL3_NewFrame( );
        ImGui_ImplWin32_NewFrame( );
        ImGui::NewFrame( );

        Menu::Render( );

        ImGui::Render( );
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData( ));
    }

    return oWglSwapBuffers(Hdc);
}

bool bfl4sH;
static GLint bt[4];
int sky = 1;
bool nosky = false;
bool bSmoke = false;
BOOL bSky = FALSE;

void APIENTRY Hooked_glBegin(GLenum mode) {
    g_glMode = mode;


    // wallhack
    if (GetKeyState(VK_NUMPAD1)) wallhack = !wallhack;

    if (wallhack) {
        if (mode == GL_TRIANGLES || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN) {
            glDepthRange(0, 0.5);

        } else {
            wallhack = false;
        }
    }

    // perfect wallhack
    if (GetKeyState(VK_NUMPAD2)) perfectwallhack = !perfectwallhack;

    if (perfectwallhack) {

        if (!(mode == GL_TRIANGLES || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN || mode == GL_QUADS)) {
            GLfloat curcol[4];
            glGetFloatv(GL_CURRENT_COLOR, curcol);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(curcol[0], curcol[1], curcol[2], 0.5f);
            glClearColor(0.0f, 0.0f, 0.0f, 0.05f);
        } else {
            perfectwallhack = false;
        }
    }




    if (GetKeyState(VK_NUMPAD3)) smoke = !smoke;

    if (smoke)
    {

        // smoke
        if (mode == GL_QUADS) {
            GLfloat smokecol[4];
            glGetFloatv(GL_CURRENT_COLOR, smokecol);
            if ((smokecol[0] == smokecol[1]) && (smokecol[0] == smokecol[2]) && (smokecol[0] != 0.0) && (smokecol[0] != 1.0) && (smokecol[0] >= 0.6) && (smokecol[0] <= 0.7)) { // works fine with csnz, didnt test in 1.6 tho
                glColor4f(smokecol[0], smokecol[1], smokecol[2], 0.01);
                bSmoke = true;
            } else {
                bSmoke = false;
                smoke = false;
            }
        }
    }
    // black sky
    if (GetKeyState(VK_NUMPAD4))
        blackskys = !blackskys;

    if (blackskys) {
        bSky = (mode == GL_QUADS);
    } else {
        blackskys = false;
    }
    // LAMBERT

    if (GetKeyState(VK_NUMPAD5))
        lambert = !lambert;

    if (lambert) {

        if (mode == GL_TRIANGLE_STRIP) {
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        } else {
            lambert = false;
        }
    }
    // no flash
    if (GetKeyState(VK_NUMPAD6))
        flash = !flash;

    if (flash) {

        if (mode == GL_QUADS) {
            GLfloat curcol[4];
            glGetFloatv(GL_CURRENT_COLOR, (float*)&curcol);
            if ((curcol[0] = 1.0f) && (curcol[1] == 1.0f) && (curcol[2] == 1.0f)) {
                glColor4f(curcol[0], curcol[1], curcol[2], 0.05f);
            }
        } else {
            flash = false;
        }
    }

    // night mode
    if (GetKeyState(VK_NUMPAD7)) {

        if (mode == GL_POLYGON) {
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
            pglBegin(mode);
        }
    }
    // white walls
    if (GetKeyState(VK_NUMPAD8)) {

        if (mode == GL_TRIANGLES || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN || mode == GL_POLYGON) {
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        }
    }
    if (GetKeyState(VK_NUMPAD9)) {

        // Inverted Models
        if (mode == GL_TRIANGLE_STRIP)
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    }

    (*pglBegin)(mode);
}

void sys_glVertex2f(GLfloat x, GLfloat y) {
    if (bfl4sH) {
        if (y == bt[3]) {
            GLfloat fcurcol[4];
            glGetFloatv(GL_CURRENT_COLOR, fcurcol);
            glColor4f(fcurcol[0], fcurcol[1], fcurcol[2], 0.01f);
        }
    }
    (glVertex2f)(x, y);
}

void APIENTRY Hooked_glEnd(void) {

    (*pglEnd)( );
}

void APIENTRY Hooked_glVertex3fv(GLfloat* v) {
    if (bSky)
        return;
    if (bSmoke)
        return;
    (*pglVertex3fv)(v);
}

void APIENTRY Hooked_glClear(GLbitfield mask) {
    if (mask == GL_DEPTH_BUFFER_BIT)

    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        (*pglClear)(mask);
    }
    (*pglClear)(mask);
}




void* DetourFunc(BYTE* src, const BYTE* dst, const int len) {
    BYTE* jmp = (BYTE*)malloc(len + 5);
    DWORD dwback;
    VirtualProtect(src, len, PAGE_READWRITE, &dwback);
    memcpy(jmp, src, len);
    jmp += len;
    jmp[0] = 0xE9;
    *(DWORD*)(jmp + 1) = (DWORD)(src + len - jmp) - 5;
    src[0] = 0xE9;
    *(DWORD*)(src + 1) = (DWORD)(dst - src) - 5;
    VirtualProtect(src, len, dwback, &dwback);
    return (jmp - len);
}

namespace GL {

    void Hook(HWND hwnd) {
        HMODULE openGL32 = GetModuleHandleA("opengl32.dll");
        pglBegin = (glBegin_t)DetourFunc((LPBYTE)GetProcAddress(openGL32, "glBegin"), (LPBYTE)&Hooked_glBegin, 5);
        pglEnd = (glEnd_t)DetourFunc((LPBYTE)GetProcAddress(openGL32, "glEnd"), (LPBYTE)&Hooked_glEnd, 6);
        pglVertex3fv = (glVertex3fv_t)DetourFunc((LPBYTE)GetProcAddress(openGL32, "glVertex3fv"), (LPBYTE)&Hooked_glVertex3fv, 5);
        pglClear = (glClear_t)DetourFunc((LPBYTE)GetProcAddress(openGL32, "glClear"), (LPBYTE)&Hooked_glClear, 5);
        if (openGL32) {
            LOG("[+] OpenGL32: ImageBase: 0x%p\n", openGL32);

            void* fnWglSwapBuffers = reinterpret_cast<void*>(GetProcAddress(openGL32, "wglSwapBuffers"));
            if (fnWglSwapBuffers) {
                Menu::InitializeContext(hwnd);

                // Hook
                LOG("[+] OpenGL32: fnWglSwapBuffers: 0x%p\n", fnWglSwapBuffers);

                static MH_STATUS wsbStatus = MH_CreateHook(reinterpret_cast<void**>(fnWglSwapBuffers), &hkWglSwapBuffers, reinterpret_cast<void**>(&oWglSwapBuffers));

                MH_EnableHook(fnWglSwapBuffers);
            }
        }
    }

    void Unhook( ) {
        if (ImGui::GetCurrentContext( )) {
            if (ImGui::GetIO( ).BackendRendererUserData)
                ImGui_ImplOpenGL3_Shutdown( );

            if (ImGui::GetIO( ).BackendPlatformUserData)
                ImGui_ImplWin32_Shutdown( );

            ImGui::DestroyContext( );
        }
    }
} // namespace GL
#else
#include <Windows.h>
namespace GL {
    void Hook(HWND hwnd) { LOG("[!] OpenGL backend is not enabled!\n"); }
    void Unhook( ) { }
} // namespace GL
#endif
