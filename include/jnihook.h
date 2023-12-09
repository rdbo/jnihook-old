#ifndef JNIHOOK_H
#define JNIHOOK_H

#include <jni.h>

int
JNIHook_Init();

int
JNIHook_Place(jmethodID mID, int (*callback)(jmethodID mID, void *senderSP, void *thread));

int
JNIHook_Shutdown();

#endif
