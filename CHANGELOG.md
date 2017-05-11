# Change Log

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

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
