/* NOTE: Definitions for jdk17u */

#ifndef JVM_HPP
#define JVM_HPP

#include <stdint.h>

typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;
typedef unsigned long u8;
typedef void *address;

// Dummy types (they are only used as pointers)
typedef void outputStream, MetaspaceClosure, ConstantPool, MethodData, MethodCounters, ImmutableOopMapSet, ExceptionCache, PcDesc, xmlStream, mtCode, AdapterFingerPrint;
template <typename T>
class Array {  };

class MetaspaceObj {
public:
	typedef void Type;
};

class Metadata : public MetaspaceObj {
public:
	
	virtual bool is_metadata() const;
	virtual bool is_klass() const;
	virtual bool is_method() const;
	virtual bool is_methodData() const;
	virtual bool is_constantPool() const;
	virtual bool is_methodCounters() const;
	virtual int  size() const;
	virtual MetaspaceObj::Type type() const;
	virtual const char *internal_name() const;
	virtual void metaspace_pointers_do(MetaspaceClosure *iter);
	virtual void print_on(outputStream *st) const;
	virtual void print_value_on(outputStream *st) const;
	virtual bool on_stack() const;
	virtual void set_on_stack(const bool value);
};

class AccessFlags {
public:
	jint _flags;
};

class ConstMethod {
public:
	typedef enum { NORMAL, OVERPASS } MethodType;
	uint64_t _fingerprint;
	
	ConstantPool *_constants;
	
	Array<u1> *_stackmap_data;
	
	int _constMethod_size;
	u2 _flags;
	u1 _result_type;
	
	u2 _code_size;
	u2 _name_index;
	u2 _signature_index;
	u2 _method_idnum;

	u2 _max_stack;
	u2 _max_locals;
	u2 _size_of_parameters;
	u2 _orig_method_idnum;
};

enum CompilerType {
	compiler_none,
	compiler_c1,
	compiler_c2,
	compiler_jvmci,
	compiler_number_of_types
};

class CodeBlob {
public:
	const CompilerType _type;
	int _size;
	int _header_size;
	int _frame_complete_offset;

	int _data_offset;
	int _frame_offset;

	address _code_begin;
	address _code_end;
	address _content_begin;

	address _data_end;
	address _relocation_begin;
	address _relocation_end;

	ImmutableOopMapSet *_oop_maps;
	bool _caller_must_gc_arguments;

	const char *name;

	virtual bool is_buffer_blob() const;
	virtual bool is_nmethod() const;
	virtual bool is_runtime_stub() const;
	virtual bool is_deoptimization_stub() const;
	virtual bool is_uncommon_trap_stub() const;
	virtual bool is_exception_stub() const;
	virtual bool is_safepoint_stub() const;
	virtual bool is_adapter_blob() const;
	virtual bool is_vtable_blob() const;
	virtual bool is_method_handles_adapter_blob() const { return false; }
	virtual bool is_compiled() const;
	virtual bool is_optimized_entry_blob() const;
	virtual bool is_zombie() const;
	virtual bool is_locked_by_vm() const;
	virtual bool is_unloaded() const;
	virtual bool is_not_entrant() const;
	virtual bool is_alive() const;
	virtual void preserve_callee_argument_oops(/* <removed args> */) = 0;
	virtual void verify() = 0;
	virtual void print() const;
	virtual void print_on(outputStream* st) const;
	virtual void print_value_on(outputStream* st) const;
	virtual void print_block_comment(outputStream* stream, address block_begin) const;
};

class PcDescCache {
public:
	enum { cache_size = 4 };
	typedef PcDesc *PcDescPtr;
	volatile PcDescPtr _pc_descs[cache_size];
};

class PcDescContainer {
public:
	PcDescCache _pc_desc_cache;
};

class Method;
class CompiledMethod : public CodeBlob {
public:
	enum MarkForDeoptimizationStatus {
		not_marked,
		deoptimize,
		deoptimize_noupdate
	};

	MarkForDeoptimizationStatus _mark_for_deoptimization_status;
	unsigned int _has_unsafe_access;
	unsigned int _has_method_handle_invokes;
	unsigned int _has_wide_vectors;

	Method *_method;
	address _scopes_data_begin;
	address _deopt_handler_begin;
	address _deopt_mh_handler_begin;

	PcDescContainer _pc_desc_container;
	ExceptionCache *_exception_cache;

	void *_gc_data;

	virtual void flush() = 0;
	virtual bool is_compiled() const;
	virtual bool  is_in_use() const = 0;
	virtual int   comp_level() const = 0;
	virtual int   compile_id() const = 0;
	virtual address verified_entry_point() const = 0;
	virtual void log_identity(xmlStream* log) const = 0;
	virtual void log_state_change() const = 0;
	virtual bool make_not_used() = 0;
	virtual bool make_not_entrant() = 0;
	virtual bool make_entrant() = 0;
	virtual address entry_point() const = 0;
	virtual bool make_zombie() = 0;
	virtual bool is_osr_method() const = 0;
	virtual int osr_entry_bci() const = 0;
	virtual void print_pcs() = 0;

	// NOTE: Missing more virtual methods!
};

template <typename T>
class BasicHashtableEntry {
public:
	unsigned int _hash;
	BasicHashtableEntry<T> *_next;
};

class AdapterHandlerEntry : public BasicHashtableEntry<mtCode> {
public:
	AdapterFingerPrint *_fingerprint;
	address _i2c_entry;
	address _c2i_entry;
	address _c2i_unverified_entry;
	address _c2i_no_clinit_check_entry;
};

class Method : public Metadata {
public:
	ConstMethod *_constMethod;
	MethodData *_method_data;
	MethodCounters *_method_counters;
	AdapterHandlerEntry *_adapter;
	AccessFlags _access_flags;
	int _vtable_index;

	u2 _intrinsic_id;

	u2 _flags;
	
	address _i2i_entry;
	
	address _from_compiled_entry;
	
	CompiledMethod *_code;
	address _from_interpreted_entry;

	virtual bool is_method() const;
};

#endif
