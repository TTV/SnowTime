Pebble.addEventListener('showConfiguration', function(e) {
  // Retrieve settings from Pebble watchapp
  var msg = {
    'APPMSG_ACTION': 0 // get settings
  };
  Pebble.sendAppMessage(msg, function(data){
    // console.log('data: ', JSON.stringify(data));
    // Show config page
    var config_data = {
        "disableShake": 0,
        "dayOffset": 0
    };
    // todo : extend config_data with data received in data
    Pebble.openURL('http://cleggo.co.uk/pebble/snowtime2-config-page.html?settings=' + JSON.stringify(config_data));
  }, function() {
    console.log('Failed to retrieve config data!');
  });
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode and parse config data as JSON
  var config_data = JSON.parse(decodeURIComponent(e.response));

  // Prepare AppMessage payload
  var dict = {
    'APPMSG_ACTION': 1, // set settings
    'DISABLE_SHAKE': config_data.disableShake,
    'DAY_OFFSET': config_data.dayOffset
  };

  // console.log('Sending: ', JSON.stringify(dict));    
    
  // Send settings to Pebble watchapp
  Pebble.sendAppMessage(dict, function(){
    // console.log('Sent config data to Pebble');  
  }, function() {
    console.log('Failed to send config data!');
  });
});