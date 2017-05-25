/*
 * Copyright 2017 The Android Open Source Project
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
 */

#ifndef SIMPLESYNTH_OPENSLES_ANDROID_API24_H
#define SIMPLESYNTH_OPENSLES_ANDROID_API24_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <jni.h>

/** Android Configuration interface methods */
struct SLAndroidConfigurationItfAPI24_;
typedef const struct SLAndroidConfigurationItfAPI24_ * const * SLAndroidConfigurationItfAPI24;
/*
 * Java Proxy Type IDs
 */
#define SL_ANDROID_JAVA_PROXY_ROUTING   0x0001

struct SLAndroidConfigurationItfAPI24_ {
  SLresult (*SetConfiguration) (SLAndroidConfigurationItfAPI24 self,
                                const SLchar *configKey,
                                const void *pConfigValue,
                                SLuint32 valueSize);
  SLresult (*GetConfiguration) (SLAndroidConfigurationItfAPI24 self,
                                const SLchar *configKey,
                                SLuint32 *pValueSize,
                                void *pConfigValue
  );
  SLresult (*AcquireJavaProxy) (SLAndroidConfigurationItfAPI24 self,
                                SLuint32 proxyType,
                                jobject *pProxyObj);
  SLresult (*ReleaseJavaProxy) (SLAndroidConfigurationItfAPI24 self,
                                SLuint32 proxyType);
};

#endif //SIMPLESYNTH_OPENSLES_ANDROID_API24_H
