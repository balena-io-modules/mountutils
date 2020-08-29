mountutils
==========

[![npm](https://img.shields.io/npm/v/mountutils.svg?style=flat-square)](https://npmjs.com/package/mountutils)
[![npm license](https://img.shields.io/npm/l/mountutils.svg?style=flat-square)](https://npmjs.com/package/mountutils)
[![npm downloads](https://img.shields.io/npm/dm/mountutils.svg?style=flat-square)](https://npmjs.com/package/mountutils)
[![Travis CI status](https://img.shields.io/travis/resin-io-modules/mountutils/master.svg?style=flat-square&label=linux%20|%20mac)](https://travis-ci.org/resin-io-modules/mountutils/branches)
[![AppVeyor status](https://img.shields.io/appveyor/ci/resin-io/mountutils/master.svg?style=flat-square&label=windows)](https://ci.appveyor.com/project/resin-io/mountutils/branch/master)

> Cross platform mount related utilities

Installation
------------

Install `mountutils` by running:

```sh
$ npm install --save mountutils
```

Mounting and Unmounting Block Devices Requires Elevated Permissions
-------------------------------------------------------------------

For more information on how Balena Etcher forks a priveleged process to write SD cards, see https://github.com/balena-io-modules/mountutils/issues/78


Debug mode
----------

You can enable debug mode by setting the `MOUNTUTILS_DEBUG` environment
variable.

Documentation
-------------

<a name="module_mountutils.unmountDisk"></a>

### mountutils.unmountDisk(device, callback)
**Kind**: static method of <code>[mountutils](#module_mountutils)</code>  
**Summary**: Unmount a whole disk  
**Access**: public  

| Param | Type | Description |
| --- | --- | --- |
| device | <code>String</code> | device |
| callback | <code>function</code> | callback (error) |

**Example**  
```js
// macOS
const drive = '/dev/disk2';

// GNU/Linux
const drive = '/dev/sdb';

// Windows
const drive = '\\\\.\\PHYSICALDRIVE2';

mountutils.unmountDisk(drive, (error) => {
  if (error) {
    throw error;
  }

  console.log('Done!');
});
```

Support
-------

If you're having any problem, please [raise an issue][newissue] on GitHub and
the Resin.io team will be happy to help.

Tests
-----

Run the test suite by doing:

```sh
$ npm test
```

Troubleshooting
---------------

### `error C2373: '__pfnDliNotifyHook2': redefinition`

This error indicates that the version of npm you're running is too old. Upgrade
by running:

```sh
npm install -g npm@latest
```

See the [following `node-gyp` issue](https://github.com/nodejs/node-gyp/issues/972) for more details.

Contribute
----------

- Issue Tracker: [github.com/resin-io-modules/mountutils/issues][issues]
- Source Code: [github.com/resin-io-modules/mountutils][source]

### Dependencies

- [cpplint][cpplint]

Before submitting a PR, please make sure that you include tests, and that
linters run without any warning:

```sh
$ npm run lint
```

License
-------

The project is licensed under the Apache 2.0 license.

[issues]: https://github.com/resin-io-modules/mountutils/issues
[newissue]: https://github.com/resin-io-modules/mountutils/issues/new
[source]: https://github.com/resin-io-modules/mountutils
[cpplint]: https://github.com/cpplint/cpplint
