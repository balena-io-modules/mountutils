const mountutils = require('.');

mountutils.unmount('F', (error) => {
  if (error) {
    throw error;
  }

  console.log('Hello');
});
