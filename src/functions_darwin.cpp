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

void unmount_callback(DADiskRef disk, DADissenterRef dissenter, void *context) {
  CFRunLoopRef loop = (CFRunLoopRef)context;

  // TODO: Get the actual dissenter result code
  if (dissenter != NULL) {
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
  DADiskRef disk = DADiskCreateFromBSDName(kCFAllocatorDefault, session, (char *)(*device));
  DADiskUnmount(disk, kDADiskUnmountOptionWhole | kDADiskUnmountOptionForce, unmount_callback, (void *)loop);
  CFRunLoopRun();
  DASessionUnscheduleFromRunLoop(session, loop, kCFRunLoopDefaultMode);
  CFRelease(session);

  if (exit_code == 0) {
    YIELD_NOTHING(callback);
  } else {
    YIELD_ERROR(callback, "Unmount failed");
  }
}
