//
// Created by ilewis on 9/25/15.
//

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
