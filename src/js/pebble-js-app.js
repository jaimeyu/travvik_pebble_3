
function iconFromWeatherId(weatherId) {
  if (weatherId < 600) {
    return 2;
  } else if (weatherId < 700) {
    return 3;
  } else if (weatherId > 800) {
    return 1;
  } else {
    return 0;
  }
}

function fetchWeather() {
console.log("Fecthing transit");
var direction=0, route=96, station=3011
  var response;
    var req = new XMLHttpRequest();
    console.log("Fetching transit data for " + route, station);
    console.log("http://ottawa.travvik.com/json.php?" +
            "routeno=" + route + "&stopno=" + station + "&src=pebble");
    req.open('GET', "http://ottawa.travvik.com/json.php?" +
            "routeno=" + route + "&stopno=" + station + "&src=pebble", true);
    req.onload = function(e) {
        if (req.readyState === 4) {
            if (req.status === 200) {
                //console.log(req.responseText);
                response = JSON.parse(req.responseText);
                var temperature, icon, city;
                var number, stopLabel, destination, arrival;
                    console.log("***SUCCESS in getting data!");
                    var arrival0 = null, dst0 = null, stoplabel = null;

                    console.log("Executing direction: " + direction);
try{
  arrival0 =  response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection[direction].Trips.Trip.AdjustedScheduleTime;
  dst0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection[direction].Trips.Trip.TripDestination;
}catch(e){
  console.log("Failed to get data");
  }
  try {
    arrival0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection[direction].Trips.Trip.AdjustedScheduleTime;
    dst0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection[direction].Trips.Trip.TripDestination;
  }
  catch (e) {
    console.log(e);
  }

  try {
    arrival0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection.Trips.Trip.AdjustedScheduleTime;
    dst0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection.Trips.Trip.TripDestination;
  }
  catch (e) {
    console.log(e);
  }

  try {
    arrival0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection[direction].Trips.Trip[0].AdjustedScheduleTime;
    dst0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection[direction].Trips.Trip[0].TripDestination;
  }
  catch (e) {
    console.log(e);
  }

  try {
    arrival0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection.Trips.Trip[0].AdjustedScheduleTime;
    dst0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection.Trips.Trip[0].TripDestination;
  }
  catch (e) {
    console.log(e);
  }
if (arrival0 === null) {
  console.log("FAIL arrival");
  Pebble.sendAppMessage({
    "REQ_BUS_NB" : 97,    
    "REQ_STOP_NB" : 3011,   
    "TRIP_ARRIVAL" : -5,  
    "TRIP_DESTINATION" : "FAIL NULLED"
  });

  return;
}
console.log("Arrival in " + arrival0);
console.log("Arrival in " + parseInt(arrival0, 10));

stoplabel = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.StopLabel;


Pebble.sendAppMessage({
  "REQ_BUS_NB" : 97,    
  "REQ_STOP_NB" : 3011,   
  "TRIP_ARRIVAL" : arrival0,  
  "TRIP_DESTINATION" : dst0
});
// No error detector, save the values.
localStorage.setItem("lastStop", station);
localStorage.setItem("lastBus", route);
console.log("Sent data to pebble");
}
} else {
  console.log("Error");
}
};
req.send(null);

/* 
   Pebble.sendAppMessage({
   "REQ_BUS_NB" : 97,    
   "REQ_STOP_NB" : 3011,   
   "TRIP_ARRIVAL" : 5,  
   "TRIP_DESTINATION" : "Bayshore"
   });
   console.log("Sent the messasge to pebble");
   */
}


Pebble.addEventListener("ready",
    function(e) {
      console.log("connect!" + e.ready);
      console.log(e.type);
    });

Pebble.addEventListener("appmessage",
    function(e) {
      console.log(e.type);
      console.log(JSON.stringify(e.payload));
      console.log(e.payload.TRIP_ARRIVAL);
      console.log(e.payload.TRIP_DESTINATION);
      console.log(e.payload.REQ_BUS_NB);
      console.log(e.payload.REQ_STOP_NB);
      console.log("message!");
      fetchWeather();
    });

Pebble.addEventListener("webviewclosed",
    function(e) {
      console.log("webview closed");
      console.log(e.type);
      console.log(e.response);
    });


