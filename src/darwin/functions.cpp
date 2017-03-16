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
#include "../mountutils.hpp"

static bool unmount_done = false;
static MOUNTUTILS_RESULT code = MOUNTUTILS_SUCCESS;

MOUNTUTILS_RESULT translate_dissenter(DADissenterRef dissenter) {
  if (dissenter) {
    DAReturn status = DADissenterGetStatus(dissenter);
    if (status == kDAReturnBadArgument ||
        status == kDAReturnNotFound) {
      return MOUNTUTILS_ERROR_INVALID_DRIVE;
    } else if (status == kDAReturnNotPermitted ||
               status == kDAReturnNotPrivileged) {
      return MOUNTUTILS_ERROR_ACCESS_DENIED;
    } else {
      MountUtilsLog("Unknown dissenter status");
      return MOUNTUTILS_ERROR_GENERAL;
    }
  } else {
    return MOUNTUTILS_SUCCESS;
  }
}

void unmount_callback(DADiskRef disk, DADissenterRef dissenter, void *context) {
  MountUtilsLog("Unmount callback called");
  unmount_done = true;
  CFRunLoopRef loop = (CFRunLoopRef)context;

  if (dissenter != NULL) {
    MountUtilsLog("Dissenter returned");
    code = translate_dissenter(dissenter);
  }

  MountUtilsLog("Stopping run loop from callback");
  CFRunLoopStop(loop);
}

MOUNTUTILS_RESULT unmount_whole_disk(const char *device) {
  MountUtilsLog("Creating session");

  DASessionRef session = DASessionCreate(kCFAllocatorDefault);
  if (session == NULL) {
    MountUtilsLog("Session couldn't be created");
    return MOUNTUTILS_ERROR_GENERAL;
  }

  MountUtilsLog("Starting run loop");
  CFRunLoopRef loop = CFRunLoopGetCurrent();
  DASessionScheduleWithRunLoop(session, loop, kCFRunLoopDefaultMode);
  DADiskRef disk = DADiskCreateFromBSDName(kCFAllocatorDefault,
                                           session,
                                           device);

  MountUtilsLog("Unmounting disk");

  DADiskUnmount(disk,
                kDADiskUnmountOptionWhole | kDADiskUnmountOptionForce,
                unmount_callback,
                reinterpret_cast<void *>(loop));

  if (!unmount_done) {
    MountUtilsLog("Stopping run loop");
    CFRunLoopRun();
    DASessionUnscheduleFromRunLoop(session, loop, kCFRunLoopDefaultMode);
    CFRelease(session);
  }

  return code;
}

void _eject_cb(DADiskRef disk, DADissenterRef dissenter, void *context) {
  MountUtilsLog("[eject]: Eject callback");
  if (dissenter) {
    MountUtilsLog("[eject]: Eject dissenter");
    code = translate_dissenter(dissenter);
  } else {
    MountUtilsLog("[eject]: Eject success");
    code = MOUNTUTILS_SUCCESS;
  }
  CFRunLoopRef run_loop = (CFRunLoopRef)context;
  CFRunLoopStop(run_loop);
}

void _unmount_cb(DADiskRef disk, DADissenterRef dissenter, void *context) {
  MountUtilsLog("[eject]: Unmount callback");
  CFRunLoopRef run_loop = (CFRunLoopRef) context;
  if (dissenter) {
    MountUtilsLog("[eject]: Unmount dissenter");
    code = translate_dissenter(dissenter);
    CFRunLoopRef run_loop = (CFRunLoopRef)context;
    CFRunLoopStop(run_loop);
  } else {
    MountUtilsLog("[eject]: Unmount success");
    MountUtilsLog("[eject]: Ejecting...");
    DADiskEject(disk,
      kDADiskEjectOptionDefault,
      _eject_cb,
      reinterpret_cast<void *>(run_loop));
  }
}

MOUNTUTILS_RESULT eject_disk(const char* device) {
  // Reset the result code
  code = MOUNTUTILS_SUCCESS;

  // Create a session object
  MountUtilsLog("[eject]: Creating DA session");
  DASessionRef session = DASessionCreate(kCFAllocatorDefault);

  if (session == NULL) {
    MountUtilsLog("[eject]: Session couldn't be created");
    return MOUNTUTILS_ERROR_GENERAL;
  }

  // Get a reference to the current thread's runloop
  CFRunLoopRef run_loop = CFRunLoopGetCurrent();

  // Get a disk object from the disk path
  MountUtilsLog("[eject]: Getting disk object");
  DADiskRef disk = DADiskCreateFromBSDName(kCFAllocatorDefault,
    session, device);

  // Unmount, and then eject from the unmount callback
  MountUtilsLog("[eject]: Unmounting");
  DADiskUnmount(disk,
                kDADiskUnmountOptionWhole | kDADiskUnmountOptionForce,
                _unmount_cb,
                reinterpret_cast<void *>(run_loop));

  // Schedule a disk arbitration session
  MountUtilsLog("[eject]: Schedule session on run loop");
  DASessionScheduleWithRunLoop(session, run_loop, kCFRunLoopDefaultMode);

  // Start the run loop: Run with a timeout of 500ms (0.5s),
  // and don't terminate after only handling one resource.
  // NOTE: As the unmount callback gets called *before* the runloop can
  // be started here when there's no device to be unmounted or
  // the device has already been unmounted, the loop would
  // hang indefinitely until stopped manually otherwise.
  // Here we repeatedly run the loop for a given time, and stop
  // it at some point if it hasn't gotten anywhere, or if there's
  // nothing to be unmounted, or a dissent has been caught before the run.
  // This way we don't have to manage state across callbacks.
  MountUtilsLog("[eject]: Starting run loop");

  bool done = false;
  unsigned int loop_count = 0;

  while (!done) {
    loop_count++;
    // See https://developer.apple.com/reference/corefoundation/1541988-cfrunloopruninmode
    SInt32 status = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.5, false);
    // Stop starting the runloop once it's been manually stopped
    if ((status == kCFRunLoopRunStopped) || (status == kCFRunLoopRunFinished)) {
      done = true;
    }
    // Bail out if DADiskUnmount caught a dissent and
    // thus returned before the runloop even started
    if (code != MOUNTUTILS_SUCCESS) {
      MountUtilsLog("[eject]: Runloop dry");
      done = true;
    }
    // Bail out if the runloop is timing out, but not getting anywhere
    if (loop_count > 10) {
      MountUtilsLog("[eject]: Runloop stall");
      code = MOUNTUTILS_ERROR_GENERAL;
      done = true;
    }
  }

  // Clean up the session
  MountUtilsLog("[eject]: Releasing session & disk object");
  DASessionUnscheduleFromRunLoop(session, run_loop, kCFRunLoopDefaultMode);
  CFRelease(session);

  return code;
}

NAN_METHOD(UnmountDisk) {
  if (!info[1]->IsFunction()) {
    return Nan::ThrowTypeError("Callback must be a function");
  }

  v8::Local<v8::Function> callback = info[1].As<v8::Function>();

  if (!info[0]->IsString()) {
    return Nan::ThrowTypeError("Device argument must be a string");
  }

  v8::String::Utf8Value device(info[0]->ToString());

  MOUNTUTILS_RESULT result =
    unmount_whole_disk(reinterpret_cast<char *>(*device));

  MountUtilsLog("Unmount complete");

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

NAN_METHOD(EjectDisk) {
  if (!info[1]->IsFunction()) {
    return Nan::ThrowError("Callback must be a function");
  }

  v8::Local<v8::Function> callback = info[1].As<v8::Function>();

  if (!info[0]->IsString()) {
    return Nan::ThrowError("Device argument must be a string");
  }

  v8::String::Utf8Value device(info[0]->ToString());

  MOUNTUTILS_RESULT result =
    eject_disk(reinterpret_cast<char *>(*device));

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
