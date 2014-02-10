/*
 * Copyright © 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file egl_without_glx.c
 *
 * Tries to test operation of the library on a system with a GLES
 * installed but no GLX.  This test is varied by the GLES_VERSION
 * defined at compile time to test either a GLES1-only or a GLES2-only
 * system.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <err.h>
#include <dlfcn.h>
#include "epoxy/gl.h"
#include "epoxy/egl.h"

#include "egl_common.h"

/**
 * Wraps the system dlopen(), which libepoxy will end up calling when
 * it tries to dlopen() the API libraries, and errors out the
 * libraries we're trying to simulate not being installed on the
 * system.
 */
void *
dlopen(const char *filename, int flag)
{
    void * (*dlopen_unwrapped)(const char *filename, int flag);

    if (!strcmp(filename, "libGL.so.1"))
        return NULL;
#if GLES_VERSION == 2
    if (!strcmp(filename, "libGLESv1_CM.so.1"))
        return NULL;
#else
    if (!strcmp(filename, "libGLESv2.so.2"))
        return NULL;
#endif

    dlopen_unwrapped = dlsym(RTLD_NEXT, "dlopen");
    assert(dlopen_unwrapped);

    return dlopen_unwrapped(filename, flag);
}

int
main(int argc, char **argv)
{
    bool pass = true;
    EGLDisplay *dpy = get_egl_display_or_skip();
    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, GLES_VERSION,
        EGL_NONE
    };
    EGLConfig cfg;
    EGLint config_attribs[] = {
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_RED_SIZE, 1,
	EGL_GREEN_SIZE, 1,
	EGL_BLUE_SIZE, 1,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
	EGL_NONE
    };
    EGLint count;
    EGLContext ctx;
    const unsigned char *string;

    if (!epoxy_has_egl_extension(dpy, "EGL_KHR_surfaceless_context"))
        errx(77, "Test requires EGL_KHR_surfaceless_context");

    eglBindAPI(EGL_OPENGL_ES_API);

    if (!eglChooseConfig(dpy, config_attribs, &cfg, 1, &count))
        errx(77, "Couldn't get an EGLConfig\n");

    ctx = eglCreateContext(dpy, cfg, NULL, context_attribs);
    if (!ctx)
        errx(77, "Couldn't create a GLES%d context\n", GLES_VERSION);

    eglMakeCurrent(dpy, NULL, NULL, ctx);

    string = glGetString(GL_VERSION);
    printf("GL_VERSION: %s\n", string);

    return pass != true;
}