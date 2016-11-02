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

// Adapted from https://support.microsoft.com/en-us/kb/165721

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winioctl.h>
#include <tchar.h>
#include <stdio.h>
#include "functions.hpp"
#include "utils.hpp"

DWORD GetAccessFlags(TCHAR driveLetter) {
  TCHAR rootName[5];
  wsprintf(rootName, TEXT("%c:\\"), driveLetter);

  switch(GetDriveType(rootName)) {
    case DRIVE_REMOVABLE:
      return GENERIC_READ | GENERIC_WRITE;
    case DRIVE_CDROM:
      return GENERIC_READ;
    default:
      return FALSE;
  }
}

HANDLE OpenVolume(TCHAR driveLetter) {
  DWORD dwAccessFlags = GetAccessFlags(driveLetter);
  ATTEMPT(dwAccessFlags, INVALID_HANDLE_VALUE);
  TCHAR szVolumeName[8];
  wsprintf(szVolumeName, TEXT("\\\\.\\%c:"), driveLetter);

  return CreateFile(szVolumeName,
                    dwAccessFlags,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);
 }

BOOL LockVolume(HANDLE volume) {
  DWORD bytesReturned;

  for (int tries = 0; tries < 20; tries++) {
    if (DeviceIoControl(volume, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytesReturned, NULL)) {
      return TRUE;
    }

    Sleep(500);
  }

  return FALSE;
}

BOOL DismountVolume(HANDLE volume) {
  DWORD bytesReturned;

  return DeviceIoControl(volume,
                         FSCTL_DISMOUNT_VOLUME,
                         NULL, 0,
                         NULL, 0,
                         &bytesReturned,
                         NULL);
}

BOOL AllowRemovalOfVolume(HANDLE volume) {
  DWORD bytesReturned;
  PREVENT_MEDIA_REMOVAL buffer;
  buffer.PreventMediaRemoval = FALSE;

  return DeviceIoControl(volume,
                         IOCTL_STORAGE_MEDIA_REMOVAL,
                         &buffer, sizeof(PREVENT_MEDIA_REMOVAL),
                         NULL, 0,
                         &bytesReturned,
                         NULL);
 }

BOOL AutoEjectVolume(HANDLE volume) {
  DWORD bytesReturned;

  return DeviceIoControl(volume,
                         IOCTL_STORAGE_EJECT_MEDIA,
                         NULL, 0,
                         NULL, 0,
                         &bytesReturned,
                         NULL);
}

BOOL EjectVolume(TCHAR driveLetter) {

  // Open the volume.
  HANDLE volume = OpenVolume(driveLetter);
  if (volume == INVALID_HANDLE_VALUE) {
    return FALSE;
  }

  ATTEMPT(LockVolume(volume), FALSE);
  ATTEMPT(DismountVolume(volume), FALSE);

  // Set prevent removal to false and eject the volume.
  if (AllowRemovalOfVolume(volume)) {
    AutoEjectVolume(volume);
  }

  ATTEMPT(CloseHandle(volume), FALSE);

  return TRUE;
}

NAN_METHOD(unmount) {
  v8::Local<v8::Function> callback = info[1].As<v8::Function>();

  if (!info[0]->IsString()) {
    YIELD_ERROR(callback, "Invalid device");
  }

  v8::String::Utf8Value device(info[0]->ToString());
  char driveLetter = (*device)[0];

  if (!EjectVolume(driveLetter)) {
    YIELD_ERROR(callback, "Unmount failed");
  }

  YIELD_NOTHING(callback);
}
