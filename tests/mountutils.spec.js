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

// migrating those to ES6 modules is needed for tests to pass on node 20
// weirdly enough node 22 didn't care and was happy with the requires
import { expect } from 'chai';
import mountutils from '../index.js';
import { fileURLToPath } from 'node:url';
import { dirname } from 'node:path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

describe('MountUtils', function() {

  describe('.unmountDisk()', function() {

    it('should be a function', function() {
      expect(mountutils.unmountDisk).to.be.a('function');
    });

    context('missing / wrong arguments', function() {

      specify('throws on missing device', function() {
        expect(function() {
          mountutils.unmountDisk(null, function() {});
        }).to.throw(/must be a string/i);
      });

      specify('throws on missing callback', function() {
        expect(function() {
          mountutils.unmountDisk('novalue');
        }).to.throw(/must be a function/i);
      });

    });

    context('invalid device', function() {

      specify('device is a directory', function( done ) {
        mountutils.unmountDisk( __dirname, function( error ) {
          expect(error.message).to.match(/Unmount failed/);
          done();
        });
      });

      specify('device is an empty string', function( done ) {
        mountutils.unmountDisk( '', function( error ) {
          expect(error.message).to.match(/Unmount failed/);
          done();
        });
      });

    });

  });

  describe('.eject()', function() {

    it('should be a function', function() {
      expect(mountutils.eject).to.be.a('function');
    });

    context('missing / wrong arguments', function() {

      specify('throws on missing device', function() {
        expect(function() {
          mountutils.eject(null, function() {});
        }).to.throw(/must be a string/i);
      });

      specify('throws on missing callback', function() {
        expect(function() {
          mountutils.eject('novalue');
        }).to.throw(/must be a function/i);
      });

    });

    context('invalid device', function() {

      specify('device is a directory', function( done ) {
        mountutils.eject( __dirname, function( error ) {
          expect(error.message).to.match(/Eject failed/);
          done();
        });
      });

      specify('device is an empty string', function( done ) {
        mountutils.eject( '', function( error ) {
          expect(error.message).to.match(/Eject failed/);
          done();
        });
      });

    });

  });

});
