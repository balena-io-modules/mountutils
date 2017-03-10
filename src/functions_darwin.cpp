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

static int exit_code = 0;
static bool unmount_done = false;
static MOUNTUTILS_ERROR error = UNKNOWN;

void unmount_callback(DADiskRef disk, DADissenterRef dissenter, void *context) {
  unmount_done = true;
  CFRunLoopRef loop = (CFRunLoopRef)context;

  if (dissenter != NULL) {
    DAReturn code = DADissenterGetStatus(dissenter);

    if (code == kDAReturnBadArgument || code == kDAReturnNotFound) {
      error = INVALID_DRIVE;
    }

    if (code == kDAReturnNotPermitted || code == kDAReturnNotPrivileged) {
      error = ACCESS_DENIED;
    }

    exit_code = 1;
  }

  CFRunLoopStop(loop);
}

NAN_METHOD(UnmountDisk) {
  v8::Local<v8::Function> callback = info[1].As<v8::Function>();

  if (!info[0]->IsString()) {
    YIELD_ERROR(callback, "Invalid device");
  }

  v8::String::Utf8Value device(info[0]->ToString());

  DASessionRef session = DASessionCreate(kCFAllocatorDefault);

  if (session == NULL) {
    YIELD_ERROR(callback, "Couldn't create DiskArbitration session");
  }

  CFRunLoopRef loop = CFRunLoopGetCurrent();
  DASessionScheduleWithRunLoop(session, loop, kCFRunLoopDefaultMode);
  DADiskRef disk = DADiskCreateFromBSDName(kCFAllocatorDefault,
                                           session,
                                           reinterpret_cast<char *>(*device));

  DADiskUnmount(disk,
                kDADiskUnmountOptionWhole | kDADiskUnmountOptionForce,
                unmount_callback,
                reinterpret_cast<void *>(loop));

  if (!unmount_done) {
    CFRunLoopRun();
    DASessionUnscheduleFromRunLoop(session, loop, kCFRunLoopDefaultMode);
    CFRelease(session);
  }

  if (exit_code == 0) {
    YIELD_NOTHING(callback);
  } else {
    if (error == ACCESS_DENIED) {
      YIELD_ERROR(callback, "Unmount failed, access denied");
    } else if (error == INVALID_DRIVE) {
      YIELD_ERROR(callback, "Unmount failed, invalid drive");
    } else {
      YIELD_ERROR(callback, "Unmount failed");
    }
  }
}
