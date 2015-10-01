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
 */
#include "slutils.h"

HowieError howie::check(SLresult code) {
  HowieError result = HOWIE_ERROR_UNKNOWN;
  switch (code) {
    case SL_RESULT_SUCCESS:
      result = HOWIE_SUCCESS;
      break;
    case SL_RESULT_PARAMETER_INVALID:
      result = HOWIE_ERROR_INVALID_PARAMETER;
      break;
    case SL_RESULT_IO_ERROR:
      result = HOWIE_ERROR_IO;
      break;
  }
  return result;
}

HowieError howie::check(HowieError err){
  return err;
}
