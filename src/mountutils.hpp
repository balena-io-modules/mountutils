#ifndef SRC_MOUNTUTILS_HPP_
#define SRC_MOUNTUTILS_HPP_

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

#include <nan.h>
#include <string>

#define YIELD_ERROR(CALLBACK, ERROR) \
  v8::Local<v8::Value> argv[1] = { Nan::Error((ERROR)) }; \
  Nan::MakeCallback(Nan::GetCurrentContext()->Global(), (CALLBACK), 1, argv); \
  return;

#define YIELD_NOTHING(CALLBACK) \
  Nan::MakeCallback(Nan::GetCurrentContext()->Global(), (CALLBACK), 0, 0);

enum MOUNTUTILS_RESULT {
  MOUNTUTILS_SUCCESS,
  MOUNTUTILS_ERROR_INVALID_DRIVE,
  MOUNTUTILS_ERROR_ACCESS_DENIED,
  MOUNTUTILS_ERROR_GENERAL
};

void MountUtilsLog(std::string string);

NAN_METHOD(UnmountDisk);

#endif  // SRC_MOUNTUTILS_HPP_
