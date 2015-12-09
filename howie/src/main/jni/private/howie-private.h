/*
 * Copyright 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */#ifndef HOWIE_PRIVATE_H
#define HOWIE_PRIVATE_H

#include <SLES/OpenSLES.h>
#include <type_traits>
#include "../howie.h"
#include <android/log.h>

constexpr const char * kLibName = "HOWIE";

#define HOWIE_TRACE_LEVEL_NONE        0
#define HOWIE_TRACE_LEVEL_CALLS       1
#define HOWIE_TRACE_LEVEL_DIAGNOSTIC  2
#define HOWIE_TRACE_LEVEL_REALTIME    3
#define HOWIE_TRACE_LEVEL_ALL         4

#ifndef HOWIE_TRACE_LEVEL
#define HOWIE_TRACE_LEVEL 0
#endif

#if HOWIE_TRACE_LEVEL > HOWIE_TRACE_LEVEL_NONE

#define HOWIE_TRACE(L, S, ...) if (L <= HOWIE_TRACE_LEVEL) { \
  __android_log_print( ANDROID_LOG_VERBOSE, kLibName, \
                    "In %s line %d: " S, __FILE__, __LINE__, __VA_ARGS__);}

#define HOWIE_TRACE_FN(L) if (L <= HOWIE_TRACE_LEVEL) { \
  __android_log_print( \
  ANDROID_LOG_VERBOSE, kLibName, __func__); }
#else
#define HOWIE_TRACE(L, S, ...)
#define HOWIE_TRACE_FN(L)
#endif

#define HOWIE_CHECK_L(op, L) {             \
  HOWIE_TRACE(L, "checking %s", #op); \
  auto op_result = howie::check((op));\
  if (!HOWIE_SUCCEEDED(op_result)) {  \
      HOWIE_TRACE(L, "check failed: %s ", #op); \
      __android_log_print(ANDROID_LOG_VERBOSE, kLibName, "%s failed " \
                          "with code %d", __func__, op_result); \
      return op_result;                 \
  }                                   \
}

#define HOWIE_CHECK(op) HOWIE_CHECK_L(op, HOWIE_TRACE_LEVEL_DIAGNOSTIC)
#define HOWIE_RTCHECK(op) HOWIE_CHECK_L(op, HOWIE_TRACE_LEVEL_REALTIME)

#define HOWIE_CHECK_NOT_NULL(o) {   \
  if(!(o)) { \
      __android_log_print(ANDROID_LOG_VERBOSE, kLibName, "%s failed " \
                          "null check at line %d", __func__, __LINE__); \
    return HOWIE_ERROR_NULL; \
  }\
}

#define HOWIE_CHECK_ENGINE_INITIALIZED() {      \
  if (!howie::EngineImpl::get()) {              \
      __android_log_print(ANDROID_LOG_VERBOSE, kLibName, "%s failed " \
                          "because engine not initialized at line %d", \
                          __func__, __LINE__); \
    return HOWIE_ERROR_ENGINE_NOT_INITIALIZED;  \
  }                                             \
}


namespace howie {
  HowieError check(SLresult code);
  HowieError check(HowieError err);

  template<typename derived_t>
  HowieError checkCast(const void* obj) {
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