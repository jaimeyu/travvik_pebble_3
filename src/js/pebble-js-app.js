
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
/*
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
                console.log(req.responseText);
                response = JSON.parse(req.responseText);
                var temperature, icon, city;
                var number, stopLabel, destination, arrival;
                    console.log("***SUCCESS in getting data!");
                    var arrival0 = null, dst0 = null, stoplabel = null;

                    console.log("Executing direction: " + direction);

                    try {
                        arrival0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection[direction].Trips.Trip.                 AdjustedScheduleTime;
                        dst0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection[direction].Trips.Trip.TripDestination;
                    }
                    catch (e) {
                        console.log(e);
                    try {
                        arrival0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection.Trips.Trip.AdjustedScheduleTime;
                        dst0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection.Trips.Trip.TripDestination;
                    }
                    catch (e) {
                        console.log(e);
                    }

                    try {
                        arrival0 = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.Route.RouteDirection[direction].Trips.Trip[0].              AdjustedScheduleTime;
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
                        Pebble.sendAppMessage({
                            "arrival": 0,
                            "origin": "No data",
                            "destination": "be found",
                            "direction": direction,
                            "busnb": parseInt(route, 10),
                            "stopnb": parseInt(station, 10)
                        });
										return;
                    }
                    console.log("Arrival in " + arrival0);
                    console.log("Arrival in " + parseInt(arrival0, 10));

                    stoplabel = response.GetNextTripsForStopResponse.GetNextTripsForStopResult.StopLabel;


  Pebble.sendAppMessage({
		"REQ_BUS_NB" : 97,    
    "REQ_STOP_NB" : 3011,   
    "TRIP_ARRIVAL" : 5,  
    "TRIP_DESTINATION" : "Bayshore"
        });
                    // No error detector, save the values.
                    localStorage.setItem("lastStop", station);
                    localStorage.setItem("lastBus", route);
                    console.log("Sent data to pebble");
                }
            } else {
                console.log("Error");
            }
        }
    };
    req.send(null);
*/
    
  Pebble.sendAppMessage({
		"REQ_BUS_NB" : 97,    
    "REQ_STOP_NB" : 3011,   
    "TRIP_ARRIVAL" : 5,  
    "TRIP_DESTINATION" : "Bayshore"
        });
  console.log("Sent the messasge to pebble");
  /*req.open('GET', "http://api.openweathermap.org/data/2.1/find/city?" +
    "lat=" + latitude + "&lon=" + longitude + "&cnt=1", true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        var temperature, icon, city;
        if (response && response.list && response.list.length > 0) {
          var weatherResult = response.list[0];
          temperature = Math.round(weatherResult.main.temp - 273.15);
          icon = iconFromWeatherId(weatherResult.weather[0].id);
          city = weatherResult.name;
          console.log(temperature);
          console.log(icon);
          console.log(city);
          Pebble.sendAppMessage({
            "icon":icon,
            "temperature":temperature + "\u00B0C",
            "city":city});
        }

      } else {
        console.log("Error");
      }
    }
  }
  req.send(null);
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
                          console.log(e.payload.TRIP_ARRIVAL);
                          console.log("message!");
												  fetchWeather();
                        });

Pebble.addEventListener("webviewclosed",
                                     function(e) {
                                     console.log("webview closed");
                                     console.log(e.type);
                                     console.log(e.response);
                                     });


