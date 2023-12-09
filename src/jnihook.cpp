#include <jnihook.h>
#include <libmem/libmem.hpp>
#include <iostream>
#include <stdarg.h>
#include <unordered_map>
#include "jvm.hpp"

extern "C" void jnihook_gateway();
extern "C" void jnihook_comp_gateway();
extern "C" void jnihook_gateway_common();

enum CompilerActivity {
	stop_compilation = 0,
	run_compilation = 1,
	shutdown_compilation = 2
};

static lm_module_t libjvm = { .base = LM_ADDRESS_BAD };
static CompilerActivity *_should_compile_new_jobs;
static CompilerActivity _should_compile_new_jobs_orig;

#define JNIHOOK_DEBUG
#ifdef JNIHOOK_DEBUG
void LOG(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);

	printf("[JNIHook] ");
	vprintf(fmt, va);

	va_end(va);
}
#else
void LOG(const char *fmt, ...) {
	
}
#endif

struct hook_info {
	address orig_i2c;
	address orig_c2i;
	int (*callback)(jmethodID, void *, void *);
};

static auto hook_table = std::unordered_map<Method *, hook_info>();

extern "C"
address JNIHook_CallHandler(Method *method, void *senderSP, void *thread, uintptr_t is_compiled)
{
#	ifdef JNIHOOK_DEBUG
	std::cout << "[JNIHook] CallHandler called from gateway" << std::endl;
	std::cout << "[JNIHook] method: " << method << std::endl;
	std::cout << "[JNIHook] senderSP: " << senderSP << std::endl;
	std::cout << "[JNIHook] thread: " << thread << std::endl;
	std::cout << "[JNIHook] is_compiled? " << (is_compiled ? "true" : "false") << std::endl;
#	endif

	// TODO: actually lookup method in hook table
	auto callback = hook_table.begin()->second.callback;
	address orig = is_compiled ? hook_table.begin()->second.orig_c2i : hook_table.begin()->second.orig_i2c;
#	ifdef JNIHOOK_DEBUG
	std::cout << "[JNIHook] Callback: " << (void *)callback << std::endl;
	std::cout << "[JNIHook] JmpBack: " << (void *)orig << std::endl;
#	endif

	if (is_compiled) {
		size_t nargs = method->_constMethod->_size_of_parameters;
		int extraspace = nargs * sizeof(address) /* + sizeof(address) */;
		// TODO: align_up
		senderSP = (void *)((lm_address_t)senderSP - extraspace);
	}

	int result = callback((jmethodID)&method, senderSP, thread);

	return orig;
}

int
JNIHook_Init()
{
	int ret = -1;
	
	if (LM_FindModule((lm_string_t)"libjvm.so", &libjvm) != LM_TRUE) {
		LOG("Unable to find JVM module!\n");
		return ret;
	}

	LOG("Found libjvm module: %s @ %p\n", libjvm.name, libjvm.base);

	// TODO: Add pattern scan instead of offset
	_should_compile_new_jobs = (CompilerActivity *)(libjvm.base + 0x11172a8);
	LOG("_should_compile_new_jobs: %p\n", _should_compile_new_jobs);

	_should_compile_new_jobs_orig = *_should_compile_new_jobs;
	LOG("original _should_compile_new_jobs: %d\n", _should_compile_new_jobs_orig);

	*_should_compile_new_jobs = shutdown_compilation;
	LOG("Compiler shut down\n");

	ret = 0;
	return ret;
}

int
JNIHook_Place(jmethodID mID, int (*callback)(jmethodID mID, void *senderSP, void *thread))
{
	int ret = -1;
	Method *method;

	LOG("jmethodID: %p\n", (void *)mID);

	method = *(Method **)mID;
	LOG("Method: %p\n", method);

	LOG("Method _from_interpreted_entry: %p\n", method->_from_interpreted_entry);
	hook_table[method] = hook_info {
		.orig_i2c = (address)method->_from_interpreted_entry,
		.orig_c2i = (address)method->_adapter->_c2i_entry,
		.callback = callback
	};

	LOG("Method _from_compiled_entry: %p\n", method->_from_compiled_entry);

	LOG("Method _code: %p\n", method->_code);

	LOG("Method _adapter: %p\n", method->_adapter);

	LOG("Method _adapter->_c2i_entry: %p\n", method->_adapter->_c2i_entry);
	
	method->_from_interpreted_entry = (address)jnihook_gateway;

	if (method->_code) {
		LOG("_code entry point: %p\n", method->_code->entry_point());
		// TODO: Implement
	} else {
		method->_adapter->_c2i_entry = (address)jnihook_comp_gateway;
	}

	LOG("Hooks placed for method: %p\n", mID);

	ret = 0;
	return ret;
}
