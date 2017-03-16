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

#include <sys/mount.h>
#include <mntent.h>
#include <errno.h>
#include "../mountutils.hpp"

NAN_METHOD(UnmountDisk) {
  if (!info[1]->IsFunction()) {
    return Nan::ThrowTypeError("Callback must be a function");
  }

  v8::Local<v8::Function> callback = info[1].As<v8::Function>();

  if (!info[0]->IsString()) {
    return Nan::ThrowTypeError("Device argument must be a string");
  }

  v8::String::Utf8Value device(info[0]->ToString());

  const char *device_path = reinterpret_cast<char *>(*device);
  const char *mount_path = NULL;

  // Get mountpaths from the device path, as `umount(device)`
  // has been removed in Linux 2.3+
  struct mntent *mount_entity;
  FILE *proc_mounts;

  proc_mounts = setmntent("/proc/mounts", "r");

  if (proc_mounts == NULL) {
    v8::Local<v8::Value> argv[1] = {
      Nan::ErrnoException(errno, "setmntent", NULL, "/proc/mounts")
    };
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, 1, argv);
    return;
  }

  // TODO(jhermsmeier): Support únmounting multiple mountpoints (?)
  // NOTE: Might not be necessary, as MNT_DETACH will cause
  // every mount in the mount namespace to be lazily unmounted
  while ((mount_entity = getmntent(proc_mounts)) != NULL) {
    mount_path = mount_entity->mnt_dir;
    if (strncmp(mount_path, device_path, strlen(device_path)) == 0) {
      break;
    }
  }

  endmntent(proc_mounts);

  if (mount_path == NULL) {
    v8::Local<v8::Value> argv[1] = { Nan::Error("Couldn't find mount paths") };
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, 1, argv);
    return;
  }

  // Use umount2() with the MNT_DETACH flag, which performs a lazy unmount;
  // makíng the mount point unavailable for new accesses,
  // and only actually unmounting when the mount point ceases to be busy
  int result = umount2(mount_path, MNT_DETACH);

  if (result == 0) {
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, 0, 0);
  } else {
    v8::Local<v8::Value> argv[1] = {
      Nan::ErrnoException(errno, "umount", NULL, mount_path)
    };
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, 1, argv);
  }
}
