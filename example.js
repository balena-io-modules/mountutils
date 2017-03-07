const mountutils = require('.');

mountutils.unmountDisk('F', (error) => {
  if (error) {
    throw error;
  }

  console.log('Hello');
});
