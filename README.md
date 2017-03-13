mountutils
==========

> Cross platform mount related utilities

Installation
------------

Install `mountutils` by running:

```sh
$ npm install --save resin-io-modules/mountutils
```

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
mountutils.unmountDisk('/dev/disk2', (error) => {
  if (error) {
    throw error;
  }

  console.log('Done!');
});
```
**Example**  
```js
mountutils.unmountDisk('/dev/sdb', (error) => {
  if (error) {
    throw error;
  }

  console.log('Done!');
});
```
**Example**  
```js
mountutils.unmountDisk('\\\\.\\PHYSICALDRIVE2', (error) => {
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
