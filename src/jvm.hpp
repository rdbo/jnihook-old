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

enum CompilerActivity {
	stop_compilation = 0,
	run_compilation = 1,
	shutdown_compilation = 2
};

enum {
	// flags actually put in .class file
	JVM_ACC_WRITTEN_FLAGS           = 0x00007FFF,

	// Method* flags
	JVM_ACC_MONITOR_MATCH           = 0x10000000,     // True if we know that monitorenter/monitorexit bytecodes match
	JVM_ACC_HAS_MONITOR_BYTECODES   = 0x20000000,     // Method contains monitorenter/monitorexit bytecodes
	JVM_ACC_HAS_LOOPS               = 0x40000000,     // Method has loops
	JVM_ACC_LOOPS_FLAG_INIT         = (int)0x80000000,// The loop flag has been initialized
	JVM_ACC_QUEUED                  = 0x01000000,     // Queued for compilation
	JVM_ACC_NOT_C2_COMPILABLE       = 0x02000000,
	JVM_ACC_NOT_C1_COMPILABLE       = 0x04000000,
	JVM_ACC_NOT_C2_OSR_COMPILABLE   = 0x08000000,
	JVM_ACC_HAS_LINE_NUMBER_TABLE   = 0x00100000,
	JVM_ACC_HAS_CHECKED_EXCEPTIONS  = 0x00400000,
	JVM_ACC_HAS_JSRS                = 0x00800000,
	JVM_ACC_IS_OLD                  = 0x00010000,     // RedefineClasses() has replaced this method
	JVM_ACC_IS_OBSOLETE             = 0x00020000,     // RedefineClasses() has made method obsolete
	JVM_ACC_IS_PREFIXED_NATIVE      = 0x00040000,     // JVMTI has prefixed this native method
	JVM_ACC_ON_STACK                = 0x00080000,     // RedefineClasses() was used on the stack
	JVM_ACC_IS_DELETED              = 0x00008000,     // RedefineClasses() has deleted this method

	// Klass* flags
	JVM_ACC_HAS_MIRANDA_METHODS     = 0x10000000,     // True if this class has miranda methods in it's vtable
	JVM_ACC_HAS_VANILLA_CONSTRUCTOR = 0x20000000,     // True if klass has a vanilla default constructor
	JVM_ACC_HAS_FINALIZER           = 0x40000000,     // True if klass has a non-empty finalize() method
	JVM_ACC_IS_CLONEABLE_FAST       = (int)0x80000000,// True if klass implements the Cloneable interface and can be optimized in generated code
	JVM_ACC_HAS_FINAL_METHOD        = 0x01000000,     // True if klass has final method
	JVM_ACC_IS_SHARED_CLASS         = 0x02000000,     // True if klass is shared
	JVM_ACC_IS_HIDDEN_CLASS         = 0x04000000,     // True if klass is hidden
	JVM_ACC_IS_VALUE_BASED_CLASS    = 0x08000000,     // True if klass is marked as a ValueBased class
	JVM_ACC_IS_BEING_REDEFINED      = 0x00100000,     // True if the klass is being redefined.

	// Klass* and Method* flags
	JVM_ACC_HAS_LOCAL_VARIABLE_TABLE= 0x00200000,

	JVM_ACC_PROMOTED_FLAGS          = 0x00200000,     // flags promoted from methods to the holding klass

	// field flags
	// Note: these flags must be defined in the low order 16 bits because
	// InstanceKlass only stores a ushort worth of information from the
	// AccessFlags value.
	// These bits must not conflict with any other field-related access flags
	// (e.g., ACC_ENUM).
	// Note that the class-related ACC_ANNOTATION bit conflicts with these flags.
	JVM_ACC_FIELD_ACCESS_WATCHED            = 0x00002000, // field access is watched by JVMTI
	JVM_ACC_FIELD_MODIFICATION_WATCHED      = 0x00008000, // field modification is watched by JVMTI
	JVM_ACC_FIELD_INTERNAL                  = 0x00000400, // internal field, same as JVM_ACC_ABSTRACT
	JVM_ACC_FIELD_STABLE                    = 0x00000020, // @Stable field, same as JVM_ACC_SYNCHRONIZED and JVM_ACC_SUPER
	JVM_ACC_FIELD_INITIALIZED_FINAL_UPDATE  = 0x00000100, // (static) final field updated outside (class) initializer, same as JVM_ACC_NATIVE
	JVM_ACC_FIELD_HAS_GENERIC_SIGNATURE     = 0x00000800, // field has generic signature

	JVM_ACC_FIELD_INTERNAL_FLAGS       = JVM_ACC_FIELD_ACCESS_WATCHED |
	                               JVM_ACC_FIELD_MODIFICATION_WATCHED |
	                               JVM_ACC_FIELD_INTERNAL |
	                               JVM_ACC_FIELD_STABLE |
	                               JVM_ACC_FIELD_HAS_GENERIC_SIGNATURE,

	                                            // flags accepted by set_field_flags()
	// JVM_ACC_FIELD_FLAGS                = JVM_RECOGNIZED_FIELD_MODIFIERS | JVM_ACC_FIELD_INTERNAL_FLAGS
};


typedef struct {
  const char* typeName;            // The type name containing the given field (example: "Klass")
  const char* fieldName;           // The field name within the type           (example: "_name")
  const char* typeString;          // Quoted name of the type of this field (example: "Symbol*";
                                   // parsed in Java to ensure type correctness
  int32_t  isStatic;               // Indicates whether following field is an offset or an address
  uint64_t offset;                 // Offset of field within structure; only used for nonstatic fields
  void* address;                   // Address of field; only used for static fields
                                   // ("offset" can not be reused because of apparent solstudio compiler bug
                                   // in generation of initializer data)
} VMStructEntry;


#endif
