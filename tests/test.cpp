#include <jnihook.h>
#include <iostream>

int hkMyFunction(jmethodID mID, void *senderSP, void *thread)
{
	jint *mynumber = (jint *)&((void **)senderSP)[1];
	std::cout << "[*] mynumber address: " << mynumber << std::endl;
	std::cout << "[*] mynumber value: " << *mynumber << std::endl;
	*mynumber = 1337;
	return 0;
}

static void start(JavaVM *jvm, JNIEnv *jni)
{
	jclass dummyClass;
	jmethodID myFunctionID;
	
	dummyClass = jni->FindClass("dummy/Dummy");
	if (!dummyClass) {
		std::cout << "[!] Failed to find dummy class" << std::endl;
		return;
	}
	std::cout << "[*] Dummy class: " << dummyClass << std::endl;

	myFunctionID = jni->GetStaticMethodID(dummyClass, "myFunction", "(ILjava/lang/String;)I");
	if (!myFunctionID) {
		std::cout << "[!] Failed to find 'myFunction'" << std::endl;
		return;
	}
	std::cout << "[*] myFunction ID: " << myFunctionID << std::endl;

	JNIHook_Attach(jvm, myFunctionID, hkMyFunction);
}

void __attribute__((constructor))
dl_entry()
{
	JavaVM *jvm;
	JNIEnv *jni;

	std::cout << "[*] Library loaded!" << std::endl;

	if (JNI_GetCreatedJavaVMs(&jvm, 1, NULL) != JNI_OK) {
		std::cout << "[!] Failed to retrieve JavaVM pointer" << std::endl;
		return;
	}

	std::cout << "[*] JavaVM: " << jvm << std::endl;

	if (jvm->AttachCurrentThread((void **)&jni, NULL) != JNI_OK) {
		std::cout << "[!] Failed to retrieve JNI environment" << std::endl;
		return;
	}

	std::cout << "[*] JNIEnv: " << jni << std::endl;

	start(jvm, jni);
}
