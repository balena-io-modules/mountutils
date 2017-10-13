# Change Log

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## v1.2.2 - 2017-08-28

## v1.3.0 - 2017-10-13

* Chore(ci): Add CI deployment scripts [Jonas Hermsmeier]
* Feat(package): Add prebuilds for native bindings [Jonas Hermsmeier]
* Chore(appveyor): Use VS2015 base image [Jonas Hermsmeier]
* Upgrade(package): Update dependencies [Jonas Hermsmeier]

### Changed

- Retry ejection various times before giving up on Windows

## v1.2.1 - 2017-07-23

### Changed

- Retry the unmount operation if the loop times out in macOS

## v1.2.0 - 2017-05-30

### Changed

- Use `Nan::AsyncWorker` to avoid blocking the main thread
- Fix global state on Mac OS causing improper return values

## v1.1.0 - 2017-05-11

### Changed

- Add `.eject()` method to unmount & eject a device

## v1.0.6 - 2017-04-21

### Changed

- Fix "Unmount failed" error on Windows when there are network drives.

## v1.0.5 - 2017-04-14

### Added

- Add extra logging when setting `MOUNTUTILS_DEBUG`.

## v1.0.4 - 2017-04-12

### Changed

- Don't throw "invalid drive" for Windows drives that have no logical volumes
  attached.

## v1.0.3 - 2017-04-07

### Changed

- Make `bindings` correctly resolve the node add-on location even when
  concatenated with Browserify.

## v1.0.2 - 2017-03-30

### Changed

- Don't include build directory in npm package

## v1.0.1 - 2017-03-27

### Changed

- Fix Windows build failures when installing from npm
