#ifndef JNIHOOK_H
#define JNIHOOK_H

#include <jni.h>

int
JNIHook_Attach(JavaVM *jvm, jmethodID mID, int (*callback)(jmethodID mID, void *senderSP, void *thread));

int
JNIHook_Detach(jmethodID mID);

#endif
