var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function (e) {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};


// getting the location 
function locationSuccess(pos) {
  // Construct URL
  //var url = "http://open.live.bbc.co.uk/weather/feeds/en/2643123/observations.rss"; 
  console.log("latitude" + pos.coords.latitude + "and lon=" + pos.coords.longitude);
  
  //get geoname id and location name by calling geonames endpoint
  //var old_url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
  //    pos.coords.latitude + "&lon=" + pos.coords.longitude;
  //console.log(url);
  
  var url = "http://api.geonames.org/findNearbyPlaceNameJSON?lat=" + pos.coords.latitude + "&lng=" + pos.coords.longitude+"&username=demo";
  console.log(url);
  
  xhrRequest(url, 'GET',
    function(responseText){
      var json = JSON.parse(responseText);
      var geoname = json.geonames[0];
      console.log(geoname);
      var location_name = geoname.name;
      var geoname_id = geoname.geonameId;
      console.log(location_name);
      console.log(geoname_id);
      
      var bbc_url = "http://open.live.bbc.co.uk/weather/feeds/en/" + geoname_id + "/observations.rss";
      console.log(bbc_url);
      
      // Send request to OpenWeatherMap
      xhrRequest(bbc_url, 'GET', 
        function(responseText) {
          // responseText contains an XML object with weather info
          console.log(responseText);
          var item = responseText.substring(responseText.indexOf('<item>'), responseText.indexOf('</item>'));
          var description = item.substring(item.indexOf('<description>'), item.indexOf('</description>'));
          var temperature = description.substring(description.indexOf("Temperature: ")+13, description.indexOf("C (")+1);
          var conditions = item.substring(item.indexOf("BST: ")+5, item.indexOf(", ")); 
          var imageid = 6;
          
          console.log(imageid); 
          console.log(conditions);
          if(conditions.toLowerCase().indexOf("light cloud") != -1){
             imageid = 5;
          }
          else if(conditions.toLowerCase().indexOf("cloudy") != -1){
            imageid = 4;
          }
          else if(conditions.toLowerCase().indexOf("sunny intervals") != -1){
            imageid = 6;
          }
          else if (conditions.toLowerCase().indexOf("sunny") != -1){
            imageid = 3;
          }
          else if (conditions.toLowerCase().indexOf("clear sky") != -1){
            imageid = 7;
          }
          else if (conditions.toLowerCase().indexOf("partly cloudy") != -1){
            imageid = 8;
          }
          
          
          console.log(imageid);  
          console.log(location_name);
         //var json = JSON.parse(responseText);
        
          // Assemble dictionary using our keys
          var dictionary = {
            "KEY_TEMPERATURE": temperature,
            "KEY_CONDITIONS": conditions,
            "KEY_IMAGE": imageid,
            "LOCATION_NAME": location_name
          };
          
          
          console.log(dictionary);
    
          // Send to Pebble
          Pebble.sendAppMessage(dictionary,
            function(e) {
              console.log("Weather info sent to Pebble successfully!");
            },
            function(e) {
              console.log("Error sending weather info to Pebble!");
            }
          );
        }      
      );
    });
  
  
}

function geoloc(latitude, longitude)
{
    this.coords = {};
    this.coords.latitude=latitude;
    this.coords.longitude=longitude;
}


function locationError(err) {
  console.log("Error requesting location!");
  console.log(err);
  console.log("Setting up default location: Manchester, 53.48095, -2.23743");
  var pos = new geoloc(53.48095, -2.23743);
  locationSuccess(pos);
}

function getWeather() {
  console.log("inside getWeather..");
  var id = navigator.geolocation.getCurrentPosition(locationSuccess, locationError, {enableHighAccuracy: true, 
     maximumAge: 0, 
     timeout: 5000});
}


// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    getWeather();
    // Get the initial weather
    //locationSuccess();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    //Get current location 
    getWeather();
  }                     
);
