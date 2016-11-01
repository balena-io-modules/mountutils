const mountutils = require('.');

mountutils.unmount('/dev/sdb', (error) => {
  if (error) {
    throw error;
  }

  console.log('Hello');
});
