Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('http://cleggo.co.uk/snowtime2-config-page.html');
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode and parse config data as JSON
  var config_data = JSON.parse(decodeURIComponent(e.response));

  // Prepare AppMessage payload
  var dict = {
    'DISABLE_SHAKE': config_data.disableShake
  };

  console.log('Sending: ', JSON.stringify(dict));    
    
  // Send settings to Pebble watchapp
  Pebble.sendAppMessage(dict, function(){
    console.log('Sent config data to Pebble');  
  }, function() {
    console.log('Failed to send config data!');
  });
});