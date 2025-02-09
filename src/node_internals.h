// Copyright & License details are available under JXCORE_LICENSE file

#ifndef SRC_NODE_INTERNALS_H_
#define SRC_NODE_INTERNALS_H_

#include <stdlib.h>
#include "jx/Proxy/JSEngine.h"
#include "jx/commons.h"

namespace node {

class commons;

NODE_EXTERN void EmitExit(JS_HANDLE_OBJECT process_l);
NODE_EXTERN void EmitReset(JS_HANDLE_OBJECT process_l, const int code);

NODE_EXTERN JS_HANDLE_VALUE
    MakeDomainCallback(node::commons *com, const JS_HANDLE_OBJECT_REF object,
                       const JS_HANDLE_FUNCTION_REF callback, int argc,
                       JS_HANDLE_VALUE argv[]);

NODE_EXTERN JS_HANDLE_VALUE MakeCallback(node::commons *com,
                                         const JS_HANDLE_OBJECT_REF object,
                                         const JS_HANDLE_FUNCTION_REF callback,
                                         int argc, JS_HANDLE_VALUE argv[]);

NODE_EXTERN JS_HANDLE_VALUE MakeCallback(node::commons *com,
                                         const JS_HANDLE_OBJECT_REF object,
                                         const JS_HANDLE_STRING symbol,
                                         int argc, JS_HANDLE_VALUE argv[]);

NODE_EXTERN JS_HANDLE_VALUE
    MakeCallback(node::commons *com, const JS_HANDLE_OBJECT_REF object,
                 const char *symbol, int argc, JS_HANDLE_VALUE argv[]);

#if defined(JS_ENGINE_MOZJS)
JS_HANDLE_VALUE
MakeCallback(node::commons *com, JS_HANDLE_OBJECT_REF host, const char *name,
             int argc, jsval argv[]);
#endif

void EnableDebug(bool wait_connect, node::commons *node);

#ifdef _WIN32
int RegisterDebugSignalHandler();
// emulate snprintf() on windows, _snprintf() doesn't zero-terminate the buffer
// on overflow...
#include <stdarg.h>
inline static int snprintf(char *buf, unsigned int len, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int n = _vsprintf_p(buf, len, fmt, ap);
  if (len) buf[len - 1] = '\0';
  va_end(ap);
  return n;
}
#elif defined(__POSIX__)
void EnableDebugSignalHandler(uv_signal_t *handle, int);
#endif

#if defined(__x86_64__)
#define BITS_PER_LONG 64
#else
#define BITS_PER_LONG 32
#endif

#ifndef offset_of
// g++ in strict mode complains loudly about the system offsetof() macro
// because it uses NULL as the base address.
#define offset_of(type, member) ((intptr_t)((char *)&(((type *)8)->member) - 8))
#endif

#ifndef container_of
#define container_of(ptr, type, member) \
  ((type *)((char *)(ptr)-offset_of(type, member)))
#endif

#define CONTAINER_OF(Pointer, TypeName, Field)                        \
  reinterpret_cast<TypeName *>(reinterpret_cast<uintptr_t>(Pointer) - \
                               OFFSET_OF(TypeName, Field))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))
#endif

#ifndef ROUND_UP
#define ROUND_UP(a, b) ((a) % (b) ? ((a) + (b)) - ((a) % (b)) : (a))
#endif

#define UNWRAP(type)                                                     \
  assert(!args.Holder().IsEmpty());                                      \
  assert(args.Holder()->InternalFieldCount() > 0);                       \
  type *wrap = static_cast<type *>(JS_GET_POINTER_DATA(args.Holder()));  \
  if (!wrap) {                                                           \
    fprintf(stderr, #type ": Aborting due to unwrap failure at %s:%d\n", \
            __FILE__, __LINE__);                                         \
    abort();                                                             \
  }                                                                      \
  node::commons *com = wrap->com

JS_HANDLE_VALUE FromConstructorTemplateX(JS_HANDLE_FUNCTION_TEMPLATE t,
                                         jxcore::PArguments &args);

}  // namespace node

#endif  // SRC_NODE_INTERNALS_H_
