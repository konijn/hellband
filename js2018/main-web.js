
function quit(why){
      //Where are we now?
  let location = window.location.href,
      //Split the URL up, we need to work with the last part
      locationParts = location.split('/'),
      //Change location from index.html (and possible arguments) to quit.html
      newLocation = location.chainedPop().chainedPush('quit.html?reason=' + why).join('/');
  //Actually go there
  window.location.href = newLocation;
}