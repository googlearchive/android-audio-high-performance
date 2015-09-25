#ifndef HOWIE_PRIVATE_H
#define HOWIE_PRIVATE_H

#include <SLES/OpenSLES.h>
#include <type_traits>
#include "../howie.h"
#include <android/log.h>


#define HOWIE_CHECK(op) {             \
  auto op_result = howie::check((op));\
  if (!HOWIE_SUCCEEDED(op_result)) {  \
      __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", "%s failed " \
                          "with code %d", __func__, op_result); \
      return op_result;                 \
  }                                   \
}

#define HOWIE_CHECK_NOT_NULL(o) {   \
  if(!(o)) return HOWIE_ERROR_NULL; \
}

#define HOWIE_CHECK_ENGINE_INITIALIZED() {      \
  if (!howie::EngineImpl::get()) {              \
    return HOWIE_ERROR_ENGINE_NOT_INITIALIZED;  \
  }                                             \
}

namespace howie {
  HowieError check(SLresult code);
  HowieError check(HowieError err);

  template<typename derived_t>
  HowieError checkCast(void* obj) {
    HowieError result = HOWIE_SUCCESS;
    if (!obj) {
      result = HOWIE_ERROR_NULL;
    } else if (reinterpret_cast<derived_t>(obj)->version
               != sizeof(typename std::remove_pointer<derived_t>::type)) {
      result = HOWIE_ERROR_INVALID_OBJECT;
    }
    return result;
  }
}

#endif // HOWIE_PRIVATE_H