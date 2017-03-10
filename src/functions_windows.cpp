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

HANDLE CreateVolumeHandleFromDevicePath(LPCTSTR devicePath, DWORD flags) {
  return CreateFile(devicePath,
                    flags,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);
}

HANDLE CreateVolumeHandleFromDriveLetter(TCHAR driveLetter, DWORD flags) {
  TCHAR devicePath[8];
  wsprintf(devicePath, TEXT("\\\\.\\%c:"), driveLetter);
  return CreateVolumeHandleFromDevicePath(devicePath, flags);
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
    return 0;
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

// Adapted from https://www.codeproject.com/articles/13839/how-to-prepare-a-usb-drive-for-safe-removal
// which is licensed under "The Code Project Open License (CPOL) 1.02"
// https://www.codeproject.com/info/cpol10.aspx
DEVINST GetDeviceInstanceFromDeviceNumber(ULONG deviceNumber) {
  GUID* guid = reinterpret_cast<GUID *>(&GUID_DEVINTERFACE_DISK);

  // Get device interface info set handle for all devices attached to system
  DWORD deviceInformationFlags = DIGCF_PRESENT | DIGCF_DEVICEINTERFACE;
  HDEVINFO deviceInformation = SetupDiGetClassDevs(guid,
                                                   NULL, NULL,
                                                   deviceInformationFlags);

  if (deviceInformation == INVALID_HANDLE_VALUE)  {
    return 0;
  }

  DWORD memberIndex = 0;
  BYTE buffer[1024];

  PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData =
    (PSP_DEVICE_INTERFACE_DETAIL_DATA)buffer;

  SP_DEVINFO_DATA deviceInformationData;
  DWORD requiredSize;

  SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
  deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

  while (true) {
    if (!SetupDiEnumDeviceInterfaces(deviceInformation,
                                     NULL,
                                     guid,
                                     memberIndex,
                                     &deviceInterfaceData)) {
      break;
    }

    requiredSize = 0;
    SetupDiGetDeviceInterfaceDetail(deviceInformation,
                                    &deviceInterfaceData,
                                    NULL, 0,
                                    &requiredSize, NULL);

    if (requiredSize == 0 || requiredSize > sizeof(buffer)) {
      memberIndex++;
      continue;
    }

    deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    ZeroMemory((PVOID)&deviceInformationData, sizeof(SP_DEVINFO_DATA));
    deviceInformationData.cbSize = sizeof(SP_DEVINFO_DATA);

    BOOL result = SetupDiGetDeviceInterfaceDetail(deviceInformation,
                                                  &deviceInterfaceData,
                                                  deviceInterfaceDetailData,
                                                  sizeof(buffer),
                                                  &requiredSize,
                                                  &deviceInformationData);

    if (!result) {
      memberIndex++;
      continue;
    }

    LPCTSTR devicePath = deviceInterfaceDetailData->DevicePath;
    HANDLE driveHandle = CreateVolumeHandleFromDevicePath(devicePath, 0);

    if (driveHandle == INVALID_HANDLE_VALUE) {
      memberIndex++;
      continue;
    }

    ULONG currentDriveDeviceNumber =
      GetDeviceNumberFromVolumeHandle(driveHandle);

    CloseHandle(driveHandle);

    if (!currentDriveDeviceNumber) {
      memberIndex++;
      continue;
    }

    if (deviceNumber == currentDriveDeviceNumber) {
      SetupDiDestroyDeviceInfoList(deviceInformation);
      return deviceInformationData.DevInst;
    }

    memberIndex++;
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

MOUNTUTILS_RESULT EjectFixedDriveByDeviceNumber(ULONG deviceNumber) {
  DEVINST deviceInstance = GetDeviceInstanceFromDeviceNumber(deviceNumber);
  if (!deviceInstance) {
    return ERROR_GENERAL;
  }

  CONFIGRET status;
  PNP_VETO_TYPE vetoType = PNP_VetoTypeUnknown;
  char vetoName[MAX_PATH];

  // It's often seen that the removal fails on the first
  // attempt but works on the second attempt.
  // See https://www.codeproject.com/articles/13839/how-to-prepare-a-usb-drive-for-safe-removal
  for (size_t tries = 0; tries < 3; tries++) {
    status = CM_Request_Device_Eject(deviceInstance,
                                     &vetoType,
                                     vetoName,
                                     MAX_PATH,
                                     0);

    if (status == CR_SUCCESS) {
      return SUCCESS;
    }

    // We use this as an indicator that the device driver
    // is not setting the `SurpriseRemovalOK` capability.
    // See https://msdn.microsoft.com/en-us/library/windows/hardware/ff539722(v=vs.85).aspx
    if (status == CR_REMOVE_VETOED &&
        vetoType == PNP_VetoIllegalDeviceRequest) {
      status = CM_Query_And_Remove_SubTree(deviceInstance,
                                           &vetoType,
                                           vetoName,
                                           MAX_PATH,

      // We have to add the `CM_REMOVE_NO_RESTART` flag because
      // otherwise the just-removed device may be immediately
      // redetected, which might happen on XP and Vista.
      // See https://www.codeproject.com/articles/13839/how-to-prepare-a-usb-drive-for-safe-removal

                                           CM_REMOVE_NO_RESTART);

      if (status == CR_ACCESS_DENIED) {
        return ERROR_ACCESS_DENIED;
      }

      if (status == CR_SUCCESS) {
        return SUCCESS;
      }

      return ERROR_GENERAL;
    }

    Sleep(500);
  }

  return ERROR_GENERAL;
}

MOUNTUTILS_RESULT EjectDriveLetter(TCHAR driveLetter) {
  DWORD volumeFlags = GENERIC_READ | GENERIC_WRITE;
  HANDLE volumeHandle = CreateVolumeHandleFromDriveLetter(driveLetter,
                                                          volumeFlags);

  if (volumeHandle == INVALID_HANDLE_VALUE) {
    return ERROR_INVALID_DRIVE;
  }

  // Don't proceed if the volume is not mounted
  if (!IsVolumeMounted(volumeHandle)) {
    if (!CloseHandle(volumeHandle)) {
      return ERROR_GENERAL;
    }
  }

  if (IsDriveFixed(driveLetter)) {
    ULONG deviceNumber = GetDeviceNumberFromVolumeHandle(volumeHandle);
    if (!deviceNumber) {
      CloseHandle(volumeHandle);
      return ERROR_GENERAL;
    }

    if (!CloseHandle(volumeHandle)) {
      return ERROR_GENERAL;
    }

    return EjectFixedDriveByDeviceNumber(deviceNumber);
  }

  if (!LockVolume(volumeHandle)) {
    CloseHandle(volumeHandle);
    return ERROR_GENERAL;
  }

  if (!DismountVolume(volumeHandle)) {
    CloseHandle(volumeHandle);
    return ERROR_GENERAL;
  }

  if (!EjectRemovableVolume(volumeHandle)) {
    CloseHandle(volumeHandle);
    return ERROR_GENERAL;
  }

  if (!UnlockVolume(volumeHandle)) {
    CloseHandle(volumeHandle);
    return ERROR_GENERAL;
  }

  if (!CloseHandle(volumeHandle)) {
    return ERROR_GENERAL;
  }

  return SUCCESS;
}

MOUNTUTILS_RESULT Eject(ULONG deviceNumber) {
  DWORD logicalDrivesMask = GetLogicalDrives();
  TCHAR currentDriveLetter = 'A';

  if (logicalDrivesMask == 0) {
    return ERROR_GENERAL;
  }

  while (logicalDrivesMask) {
    if (logicalDrivesMask & 1) {
      HANDLE driveHandle =
        CreateVolumeHandleFromDriveLetter(currentDriveLetter, 0);

      if (driveHandle == INVALID_HANDLE_VALUE) {
        return ERROR_GENERAL;
      }

      ULONG currentDeviceNumber = GetDeviceNumberFromVolumeHandle(driveHandle);

      if (!CloseHandle(driveHandle)) {
        return ERROR_GENERAL;
      }

      if (currentDeviceNumber == deviceNumber) {
        MOUNTUTILS_RESULT result = EjectDriveLetter(currentDriveLetter);
        if (result != SUCCESS) {
          return result;
        }
      }
    }

    currentDriveLetter++;
    logicalDrivesMask >>= 1;
  }

  return SUCCESS;
}

NAN_METHOD(UnmountDisk) {
  v8::Local<v8::Function> callback = info[1].As<v8::Function>();

  if (!info[0]->IsNumber()) {
    YIELD_ERROR(callback, "Invalid device");
  }

  unsigned int deviceId = info[0]->Uint32Value();
  MOUNTUTILS_RESULT result = Eject(deviceId);

  if (result == SUCCESS) {
    YIELD_NOTHING(callback);
  } else if (result == ERROR_ACCESS_DENIED) {
    YIELD_ERROR(callback, "Unmount failed, access denied");
  } else if (result == ERROR_INVALID_DRIVE) {
    YIELD_ERROR(callback, "Unmount failed, invalid drive");
  } else {
    YIELD_ERROR(callback, "Unmount failed");
  }
}
