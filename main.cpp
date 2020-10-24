#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "libmem/libmem.h"
#include "sdk/sdk.hpp"

ILauncherMgr* launcher_mgr;
VMTHook*      launcher_vmt;
PseudoGLContextPtr user_context = NULL;

typedef void (*ShowPixels_t) (ILauncherMgr*, CShowPixelsParams*);

void hkShowPixels(ILauncherMgr* thisptr, CShowPixelsParams* params)
{
    ShowPixels_t oShowPixels = launcher_vmt->GetOriginalFunction<ShowPixels_t>(29);

    if (!params->m_noBlit)
		return oShowPixels(thisptr, params);

    SDL_Window* window = reinterpret_cast<SDL_Window*>(launcher_mgr->GetWindowRef());
    PseudoGLContextPtr original_context = launcher_mgr->GetMainContext();

    if(!user_context)
    {
        user_context = launcher_mgr->CreateExtraContext();
    }

    launcher_mgr->MakeContextCurrent(user_context);
    
    //Do your stuff

    glClearColor(1.f, 0, 0, 1.f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.5, 0.5);
    glVertex2f(-0.5, -0.5);
    glVertex2f(0.5, 0.5);
    glVertex2f(0.5, 0.5);
    glVertex2f(-0.5, -0.5);
    glVertex2f(0.5, -0.5);
    glEnd();

    //Restore old context
    launcher_mgr->MakeContextCurrent(original_context);

    //Call original ShowPixels
    oShowPixels(thisptr, params);
}

void* main_thread(void* args)
{
    mem_string_t holder;

    holder = mem_string_new("/launcher.so\n");
    mem_module_t m_launcher = mem_in_get_module(holder);
    mem_string_free(&holder);

    holder = mem_string_new("xxx????");
    mem_byte_t pattern[] = { 0x24, 0x8B, 0x1D, 0x0, 0x0, 0x0, 0x0 };
    launcher_mgr = **reinterpret_cast<ILauncherMgr***>(
        mem_in_pattern_scan(pattern, holder, m_launcher.base, m_launcher.end) + 3
	);
    mem_string_free(&holder);

    printf("Launcher Manager: %p\n", (void*)launcher_mgr);

    launcher_vmt = new VMTHook(launcher_mgr);

    launcher_vmt->HookFunction(reinterpret_cast<void*>(hkShowPixels), 29);

    return args;
}

__attribute__((constructor))
void css_hook_entry()
{
    pthread_t thread;
    pthread_create(&thread, NULL, (void*(*)(void*))main_thread, (void*)NULL);
}