#include <jnihook.h>
#include <jvmti.h>
#include <iostream>
#include <stdarg.h>
#include <unordered_map>
#include "jvm.hpp"

extern "C" void jnihook_gateway();

extern "C" VMStructEntry *gHotSpotVMStructs;

struct hook_info {
	int (*callback)(jmethodID mID, void *senderSP, void *thread);
	address orig;
};

std::unordered_map<Method *, hook_info> hook_table;

extern "C"
void *JNIHook_CallHandler(Method *method, void *senderSP, void *thread)
{
	printf("[*] JNI hook call handler\n");

	hook_table[method].callback((jmethodID)&method, senderSP, thread);
	
	return hook_table[method].orig;
}

int
JNIHook_Attach(JavaVM *jvm, jmethodID mID, int (*callback)(jmethodID mID, void *senderSP, void *thread))
{
	int ret = -1;

	JNIEnv *jni;
	jvmtiEnv *jvmti;

	std::cout << "gHotSpotVMStructs: " << gHotSpotVMStructs << std::endl;

	std::cout << "Method fields: " << std::endl;
	for (int i = 0; gHotSpotVMStructs[i].typeName != NULL; ++i) {
		if (strcmp(gHotSpotVMStructs[i].typeName, "Method"))
			continue;

		// std::cout << "typeName: " << gHotSpotVMStructs[i].typeName << std::endl;
		std::cout << "  " << gHotSpotVMStructs[i].typeString << " " << gHotSpotVMStructs[i].fieldName << " @ " << gHotSpotVMStructs[i].offset << std::endl;
	}

	std::cout << "attaching hook to: " << mID << std::endl;
	if (jvm->GetEnv((void **)&jni, JNI_VERSION_1_6) != JNI_OK)
		return ret;

	std::cout << "jni: " << jni << std::endl;
	
	if (jvm->GetEnv((void **)&jvmti, JVMTI_VERSION_1_0) != JNI_OK)
		return ret;

	std::cout << "jvmti: " << jvmti << std::endl;

	jvmtiCapabilities capabilities = { .can_retransform_classes = JVMTI_ENABLE };
	jvmti->AddCapabilities(&capabilities);
	std::cout << "added capabilities to jvmti" << std::endl;

	/* Force class methods to be interpreted */
	jclass klass;
	if (jvmti->GetMethodDeclaringClass(mID, &klass) != JNI_OK)
		return ret;
	jvmti->RetransformClasses(1, &klass);
	jni->DeleteLocalRef(klass);

	Method *method = *(Method **)mID;

	// Prevent method from being compiled
	method->_access_flags._flags |= JVM_ACC_NOT_C2_COMPILABLE | JVM_ACC_NOT_C1_COMPILABLE | JVM_ACC_NOT_C2_OSR_COMPILABLE;

	hook_table[method].orig = method->_from_interpreted_entry;
	hook_table[method].callback = callback;

	method->_from_interpreted_entry = (address)jnihook_gateway;
	method->_i2i_entry = (address)jnihook_gateway;
	method->_code = NULL;
	method->_from_compiled_entry = method->_adapter->_c2i_entry;

	ret = 0;
	return ret;
}
