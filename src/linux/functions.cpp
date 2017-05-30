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

#include <sys/stat.h>
#include <sys/mount.h>
#include <mntent.h>
#include <errno.h>
#include "../mountutils.hpp"

MOUNTUTILS_RESULT unmount_disk(const char *device_path) {
  const char *mount_path = NULL;

  // Stat the device to make sure it exists
  struct stat stats;

  if (stat(device_path, &stats) != 0) {
    MountUtilsLog("Stat failed");

    // TODO(jhermsmeier): See TODO below
    // v8::Local<v8::Value> argv[1] = {
    //   Nan::ErrnoException(errno, "stat", NULL, device_path)
    // };
    // Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, 1, argv);
    return MOUNTUTILS_ERROR_GENERAL;
  } else if (S_ISDIR(stats.st_mode)) {
    MountUtilsLog("Device is a directory");

    // TODO(jhermsmeier): See TODO below
    // v8::Local<v8::Value> argv[1] = {
    //   Nan::Error("Invalid device, path is a directory")
    // };
    // Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, 1, argv);
    return MOUNTUTILS_ERROR_INVALID_DRIVE;
  }

  // Get mountpaths from the device path, as `umount(device)`
  // has been removed in Linux 2.3+
  struct mntent *mount_entity;
  FILE *proc_mounts;

  MountUtilsLog("Reading /proc/mounts");
  proc_mounts = setmntent("/proc/mounts", "r");

  if (proc_mounts == NULL) {
    MountUtilsLog("Couldn't read /proc/mounts");
    // TODO(jhermsmeier): Refactor MOUNTUTILS_RESULT into a struct
    // with error_msg, errno, error_code etc. and set the respective
    // values on the struct and move creation of proper errors with
    // the right errno messages etc. into the AsyncWorkers (even better:
    // create a function which creates the proper error from a
    // MOUNTUTILS_RESULT struct).
    // v8::Local<v8::Value> argv[1] = {
    //   Nan::ErrnoException(errno, "setmntent", NULL, "/proc/mounts")
    // };
    // Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, 1, argv);
    return MOUNTUTILS_ERROR_GENERAL;
  }

  while ((mount_entity = getmntent(proc_mounts)) != NULL) {
    mount_path = mount_entity->mnt_fsname;

    MountUtilsLog(std::string("Mount point found: ") + std::string(mount_path));

    if (strncmp(mount_path, device_path, strlen(device_path)) == 0) {
      MountUtilsLog("Mount point belongs to drive");

      // Use umount2() with the MNT_DETACH flag, which performs a lazy unmount;
      // making the mount point unavailable for new accesses,
      // and only actually unmounting when the mount point ceases to be busy
      if (umount2(mount_entity->mnt_dir, MNT_DETACH) != 0) {
        MountUtilsLog("Unmount failed");
        endmntent(proc_mounts);
        // TODO(jhermsmeier): See TODO above
        // v8::Local<v8::Value> argv[1] = {
        //   Nan::ErrnoException(errno, "umount2", NULL, mount_entity->mnt_dir)
        // };
        // v8::Local<v8::Object> ctx = Nan::GetCurrentContext()->Global();
        // Nan::MakeCallback(ctx, callback, 1, argv);
        return MOUNTUTILS_ERROR_GENERAL;
      }

      MountUtilsLog("Unmount success");
    }
  }

  MountUtilsLog("Closing /proc/mounts");
  endmntent(proc_mounts);

  return MOUNTUTILS_SUCCESS;
}

// FIXME: This is just a stub copy of `UnmountDisk()`,
// and needs implementation!
MOUNTUTILS_RESULT eject_disk(const char *device) {
  return unmount_disk(device);
}
