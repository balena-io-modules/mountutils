/*
 * Copyright 2017 resin.io
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <DiskArbitration/DiskArbitration.h>
#include "functions.hpp"
#include "utils.hpp"

static bool unmount_done = false;
static MOUNTUTILS_RESULT code = MOUNTUTILS_SUCCESS;

void unmount_callback(DADiskRef disk, DADissenterRef dissenter, void *context) {
  unmount_done = true;
  CFRunLoopRef loop = (CFRunLoopRef)context;

  if (dissenter != NULL) {
    DAReturn status = DADissenterGetStatus(dissenter);

    if (status == kDAReturnBadArgument ||
        status == kDAReturnNotFound) {
      code = MOUNTUTILS_ERROR_INVALID_DRIVE;
    } else if (status == kDAReturnNotPermitted ||
               status == kDAReturnNotPrivileged) {
      code = MOUNTUTILS_ERROR_ACCESS_DENIED;
    } else {
      code = MOUNTUTILS_ERROR_GENERAL;
    }
  }

  CFRunLoopStop(loop);
}

MOUNTUTILS_RESULT unmount_whole_disk(const char *device) {
  DASessionRef session = DASessionCreate(kCFAllocatorDefault);
  if (session == NULL) {
    return MOUNTUTILS_ERROR_GENERAL;
  }

  CFRunLoopRef loop = CFRunLoopGetCurrent();
  DASessionScheduleWithRunLoop(session, loop, kCFRunLoopDefaultMode);
  DADiskRef disk = DADiskCreateFromBSDName(kCFAllocatorDefault,
                                           session,
                                           device);

  DADiskUnmount(disk,
                kDADiskUnmountOptionWhole | kDADiskUnmountOptionForce,
                unmount_callback,
                reinterpret_cast<void *>(loop));

  if (!unmount_done) {
    CFRunLoopRun();
    DASessionUnscheduleFromRunLoop(session, loop, kCFRunLoopDefaultMode);
    CFRelease(session);
  }

  return code;
}

NAN_METHOD(UnmountDisk) {
  if (!info[1]->IsFunction()) {
    return Nan::ThrowError("Callback must be a function");
  }

  v8::Local<v8::Function> callback = info[1].As<v8::Function>();

  if (!info[0]->IsString()) {
    return Nan::ThrowError("Device argument must be a string");
  }

  v8::String::Utf8Value device(info[0]->ToString());

  MOUNTUTILS_RESULT result =
    unmount_whole_disk(reinterpret_cast<char *>(*device));

  if (result == MOUNTUTILS_SUCCESS) {
    YIELD_NOTHING(callback);
  } else if (result == MOUNTUTILS_ERROR_ACCESS_DENIED) {
    YIELD_ERROR(callback, "Unmount failed, access denied");
  } else if (result == MOUNTUTILS_ERROR_INVALID_DRIVE) {
    YIELD_ERROR(callback, "Unmount failed, invalid drive");
  } else {
    YIELD_ERROR(callback, "Unmount failed");
  }
}
