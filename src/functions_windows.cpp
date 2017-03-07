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
#include <cfgmgr32.h>
#include <setupapi.h>
#include "functions.hpp"
#include "utils.hpp"

HANDLE CreateVolumeHandleFromDriveLetter(TCHAR driveLetter) {
  TCHAR volumeName[8];
  wsprintf(volumeName, TEXT("\\\\.\\%c:"), driveLetter);

  return CreateFile(volumeName,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);
}

ULONG GetDeviceNumberFromVolumeHandle(HANDLE volume) {
  STORAGE_DEVICE_NUMBER storageDeviceNumber;
  DWORD bytesReturned;

  BOOL result = DeviceIoControl(volume,
                                IOCTL_STORAGE_GET_DEVICE_NUMBER,
                                NULL, 0, 
                                &storageDeviceNumber,
                                sizeof(storageDeviceNumber),
                                &bytesReturned,
                                NULL);

  if (!result) {
    return -1;
  }

  return storageDeviceNumber.DeviceNumber;
}

BOOL IsDriveFixed(TCHAR driveLetter) {
  TCHAR rootName[5];
  wsprintf(rootName, TEXT("%c:\\"), driveLetter); 
  return GetDriveType(rootName) == DRIVE_FIXED;
}

BOOL LockVolume(HANDLE volume) {
  DWORD bytesReturned;

  for (size_t tries = 0; tries < 20; tries++) {
    if (DeviceIoControl(volume,
                        FSCTL_LOCK_VOLUME,
                        NULL, 0,
                        NULL, 0,
                        &bytesReturned,
                        NULL)) {
      return TRUE;
    }

    Sleep(500);
  }

  return FALSE;
}

DEVINST GetDeviceInstanceFromDeviceNumber(ULONG DeviceNumber) {
  GUID* guid = (GUID*)&GUID_DEVINTERFACE_DISK;

  // Get device interface info set handle
  // for all devices attached to system
  HDEVINFO deviceInformation = SetupDiGetClassDevs(guid, NULL, NULL,
                    DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

  if (deviceInformation == INVALID_HANDLE_VALUE)  {
    return 0;
  }

  // Retrieve a context structure for a device interface
  // of a device information set.
  DWORD dwIndex = 0;

  BYTE Buf[1024];
  PSP_DEVICE_INTERFACE_DETAIL_DATA pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)Buf;
  SP_DEVICE_INTERFACE_DATA spdid;
  SP_DEVINFO_DATA spdd;
  DWORD dwSize;

  spdid.cbSize = sizeof(spdid);

  while (true)  {
    if (!SetupDiEnumDeviceInterfaces(deviceInformation, NULL, guid, dwIndex, &spdid)) {
      break;
    }

    dwSize = 0;
    SetupDiGetDeviceInterfaceDetail(deviceInformation,
      &spdid, NULL, 0, &dwSize, NULL);

    if ( dwSize!=0 && dwSize<=sizeof(Buf) ) {
      pspdidd->cbSize = sizeof(*pspdidd); // 5 Bytes!

      ZeroMemory((PVOID)&spdd, sizeof(spdd));
      spdd.cbSize = sizeof(spdd);

      long res =
        SetupDiGetDeviceInterfaceDetail(deviceInformation, &
                                        spdid, pspdidd,
                                        dwSize, &dwSize,
                                        &spdd);
      if ( res ) {
        HANDLE hDrive = CreateFile(pspdidd->DevicePath,0,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL, OPEN_EXISTING, 0, NULL);
        if ( hDrive != INVALID_HANDLE_VALUE ) {
          STORAGE_DEVICE_NUMBER sdn;
          DWORD dwBytesReturned = 0;
          res = DeviceIoControl(hDrive,
                        IOCTL_STORAGE_GET_DEVICE_NUMBER,
                        NULL, 0, &sdn, sizeof(sdn),
                        &dwBytesReturned, NULL);
          if ( res ) {
            if ( DeviceNumber == (long)sdn.DeviceNumber ) {
              CloseHandle(hDrive);
              SetupDiDestroyDeviceInfoList(deviceInformation);
              return spdd.DevInst;
            }
          }
          CloseHandle(hDrive);
        }
      }
    }
    dwIndex++;
  }

  SetupDiDestroyDeviceInfoList(deviceInformation);

  return 0;
}

BOOL UnlockVolume(HANDLE volume) {
  DWORD bytesReturned;

  return DeviceIoControl(volume, 
                         FSCTL_UNLOCK_VOLUME,
                         NULL, 0,
                         NULL, 0,
                         &bytesReturned,
                         NULL);
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

BOOL IsVolumeMounted(HANDLE volume) {
  DWORD bytesReturned;

  return DeviceIoControl(volume, 
                         FSCTL_IS_VOLUME_MOUNTED,
                         NULL, 0,
                         NULL, 0,
                         &bytesReturned,
                         NULL);
}

BOOL EjectRemovableVolume(HANDLE volume) {
  DWORD bytesReturned;
  PREVENT_MEDIA_REMOVAL buffer;
  buffer.PreventMediaRemoval = FALSE;
  BOOL result = DeviceIoControl(volume,
                                IOCTL_STORAGE_MEDIA_REMOVAL,
                                &buffer, sizeof(PREVENT_MEDIA_REMOVAL),
                                NULL, 0,
                                &bytesReturned,
                                NULL);

  if (!result) {
    return FALSE;
  }

  return DeviceIoControl(volume,
                         IOCTL_STORAGE_EJECT_MEDIA,
                         NULL, 0,
                         NULL, 0,
                         &bytesReturned,
                         NULL);
}

BOOL EjectFixedDriveByDeviceNumber(ULONG deviceNumber) {
  DEVINST deviceInstance = GetDeviceInstanceFromDeviceNumber(deviceNumber);
  if (!deviceInstance) {
    return FALSE;
  }

  CONFIGRET status;
  PNP_VETO_TYPE vetoType = PNP_VetoTypeUnknown;
  char vetoName[MAX_PATH];

  // It's often seen that the removal fails on the first attempt but works on the second attempt.
  // See https://www.codeproject.com/articles/13839/how-to-prepare-a-usb-drive-for-safe-removal
  for (size_t tries = 0; tries < 3; tries++) {
    status = CM_Request_Device_Eject(deviceInstance, &vetoType, vetoName, MAX_PATH, 0);
    if (status == CR_SUCCESS) {
      return TRUE;
    }

    // We use this as an indicator that the device driver
    // is not setting the `SurpriseRemovalOK` capability.
    // See https://msdn.microsoft.com/en-us/library/windows/hardware/ff539722(v=vs.85).aspx
    if (status == CR_REMOVE_VETOED && vetoType == PNP_VetoIllegalDeviceRequest) {

      // We have to add the `CM_REMOVE_NO_RESTART` flag because
      // otherwise the just-removed device may be immediately
      // redetected, which might happen on XP and Vista.
      // See https://www.codeproject.com/articles/13839/how-to-prepare-a-usb-drive-for-safe-removal
      status = CM_Query_And_Remove_SubTree(deviceInstance, &vetoType, vetoName, MAX_PATH, CM_REMOVE_NO_RESTART);

      return status == CR_SUCCESS;
    }

    Sleep(500);
  }

  return FALSE;
}

BOOL Eject(TCHAR driveLetter) {
  HANDLE volumeHandle = CreateVolumeHandleFromDriveLetter(driveLetter);
  if (volumeHandle == INVALID_HANDLE_VALUE) {
    return FALSE;
  }

  // Don't proceed if the volume is not mounted
  if (!IsVolumeMounted(volumeHandle)) {
    return CloseHandle(volumeHandle);
  }

  if (IsDriveFixed(driveLetter)) {
    ULONG deviceNumber = GetDeviceNumberFromVolumeHandle(volumeHandle);
    if (deviceNumber < 0) {
      CloseHandle(volumeHandle);
      return FALSE;
    }

    if (!CloseHandle(volumeHandle)) {
      return FALSE;
    }

    return EjectFixedDriveByDeviceNumber(deviceNumber);
  }

  if (!LockVolume(volumeHandle)) {
    CloseHandle(volumeHandle);
    return FALSE;
  }

  if (!DismountVolume(volumeHandle)) {
    CloseHandle(volumeHandle);
    return FALSE;
  }

  if (!EjectRemovableVolume(volumeHandle)) {
    CloseHandle(volumeHandle);
    return FALSE;
  }

  if (!UnlockVolume(volumeHandle)) {
    CloseHandle(volumeHandle);
    return FALSE;
  }

  return CloseHandle(volumeHandle);
}

NAN_METHOD(unmount) {
  v8::Local<v8::Function> callback = info[1].As<v8::Function>();

  if (!info[0]->IsString()) {
    YIELD_ERROR(callback, "Invalid device");
  }

  v8::String::Utf8Value device(info[0]->ToString());
  char driveLetter = (*device)[0];

  if (!Eject(driveLetter)) {
    YIELD_ERROR(callback, "Unmount failed");
  }

  YIELD_NOTHING(callback);
}
