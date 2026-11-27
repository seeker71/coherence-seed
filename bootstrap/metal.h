/* Public Metal compute ABI from C. One completed dispatch owns all its host objects. */
#ifdef __APPLE__
#include <objc/runtime.h>
#include <objc/message.h>
extern id MTLCreateSystemDefaultDevice(void);
typedef struct { size_t width, height, depth; } MetalSize;
#define METAL_SEND(result, ...) ((result (*)(id, SEL, ##__VA_ARGS__))objc_msgSend)
#define MSEL(s) sel_registerName(s)
static id metal_get(id object, const char *selector) { return METAL_SEND(id)(object, MSEL(selector)); }
static size_t metal_size(id object, const char *selector) { return METAL_SEND(size_t)(object, MSEL(selector)); }
static void metal_void(id object, const char *selector) { METAL_SEND(void)(object, MSEL(selector)); }
/* Apple GPU family 3 and Metal family 3 advertise 64-bit integer arithmetic. */
static int metal_family(id device, long family) { return METAL_SEND(BOOL, long)(device, MSEL("supportsFamily:"), family); }
static int metal_words(id device) {
  if (__builtin_available(macOS 13.0, *)) return metal_family(device, 1003) || metal_family(device, 5001);
  if (__builtin_available(macOS 10.15, *)) return metal_family(device, 1003);
  return 0;
}
static int metal_available(void) {
  id pool = metal_get((id)objc_getClass("NSAutoreleasePool"), "new"), device = MTLCreateSystemDefaultDevice();
  int available = metal_words(device); metal_void(device, "release"); metal_void(pool, "drain"); return available;
}
static int metal_map(const uint8_t *library, size_t length, const uint8_t *a, const uint8_t *b, size_t size, uint8_t *out) {
  id pool = metal_get((id)objc_getClass("NSAutoreleasePool"), "new"), device = MTLCreateSystemDefaultDevice();
  id lib = nil, function = nil, pipeline = nil, queue = nil, buffer = nil;
  int ok = 0;
  if (!metal_words(device) || size > metal_size(device, "maxBufferLength") / 3) goto done;
  size_t working = metal_size(device, "recommendedMaxWorkingSetSize"), used = metal_size(device, "currentAllocatedSize");
  if (working && (working <= used || size > (working - used) / 3)) goto done;
  dispatch_data_t data = dispatch_data_create(library, length, NULL, NULL);
  if (!data) goto done;
  lib = METAL_SEND(id, dispatch_data_t, id *)(device, MSEL("newLibraryWithData:error:"), data, NULL);
  dispatch_release(data);
  id names = metal_get(lib, "functionNames");
  if (metal_size(names, "count") != 1) goto done;
  function = METAL_SEND(id, id)(lib, MSEL("newFunctionWithName:"), metal_get(names, "firstObject"));
  if (metal_size(function, "functionType") != 3) goto done; /* MTLFunctionTypeKernel */
  pipeline = METAL_SEND(id, id, id *)(device, MSEL("newComputePipelineStateWithFunction:error:"), function, NULL);
  if (!pipeline) goto done;
  if (!size) { ok = 1; goto done; }
  queue = metal_get(device, "newCommandQueue");
  id command = metal_get(queue, "commandBuffer");
  if (!command) goto done;
  buffer = METAL_SEND(id, size_t, size_t)(device, MSEL("newBufferWithLength:options:"), size * 3, 0);
  uint8_t *words = METAL_SEND(void *)(buffer, MSEL("contents"));
  if (!words) goto done;
  memcpy(words, a, size); memcpy(words + size, b, size);
  id encoder = metal_get(command, "computeCommandEncoder");
  if (!encoder) goto done;
  METAL_SEND(void, id)(encoder, MSEL("setComputePipelineState:"), pipeline);
  METAL_SEND(void, id, size_t, size_t)(encoder, MSEL("setBuffer:offset:atIndex:"), buffer, 0, 0);
  uint32_t lanes = (uint32_t)(size / sizeof(uint64_t));
  METAL_SEND(void, const void *, size_t, size_t)(encoder, MSEL("setBytes:length:atIndex:"), &lanes, sizeof lanes, 1);
  size_t width = metal_size(pipeline, "threadExecutionWidth");
  if (!width) { metal_void(encoder, "endEncoding"); goto done; }
  MetalSize group = {width, 1, 1}, grid = {(lanes + width - 1) / width, 1, 1};
  METAL_SEND(void, MetalSize, MetalSize)(encoder, MSEL("dispatchThreadgroups:threadsPerThreadgroup:"), grid, group);
  metal_void(encoder, "endEncoding"); metal_void(command, "commit"); metal_void(command, "waitUntilCompleted");
  if (metal_size(command, "status") == 4) { /* MTLCommandBufferStatusCompleted */
    memcpy(out, words + size * 2, size); ok = 1;
  }
done:
  metal_void(buffer, "release");
  metal_void(queue, "release"); metal_void(pipeline, "release"); metal_void(function, "release");
  metal_void(lib, "release"); metal_void(device, "release"); metal_void(pool, "drain"); return ok;
}
#undef YES
#undef NO
#undef MSEL
#undef METAL_SEND
#else
static int metal_available(void) { return 0; }
#endif
