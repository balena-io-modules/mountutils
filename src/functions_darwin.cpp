/*
 * The MIT License
 *
 * Copyright (c) 2016 Juan Cruz Viotti. https://github.com/jviotti
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
