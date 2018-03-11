"use strict";

//TODO: let platform 'node' work one day :/
const platform = 'web';


define(['os-'+platform], function(os){
  return {quit:os.quit};
});
