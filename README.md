mountutils
==========

[![Build Status](https://travis-ci.org/resin-io-modules/mountutils.svg?branch=master)](https://travis-ci.org/resin-io-modules/mountutils)
[![Build status](https://ci.appveyor.com/api/projects/status/c8elon3elh7tw2s0/branch/master?svg=true)](https://ci.appveyor.com/project/resin-io/mountutils/branch/master)

> Cross platform mount related utilities

Installation
------------

Install `mountutils` by running:

```sh
$ npm install --save mountutils
```

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
