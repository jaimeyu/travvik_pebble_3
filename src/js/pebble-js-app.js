function error_fetching(route, station, direction){
  console.log("Error in response.");
  stop_eta = 42;
  //route_destination = "(╯°□°）╯︵ ┻━┻";
  route_destination = "(J*_*)J^|_|";
  stop_name = "Sry. No data.";
 
  Pebble.sendAppMessage({
    "KEY_ROUTE" : parseInt(route),    
    "KEY_STOP_NUM" : parseInt(station),   
    "KEY_ETA" : parseInt(stop_eta),  
    "KEY_DST" : route_destination.substring(0,24),
    "KEY_STATION_STR" : stop_name.substring(0,24),
    "KEY_DIRECTION" : parseInt(direction)
  });


}

function parseTravvikData(response, route, station, direction){
  var stop_eta = null, route_destination = null, stop_name = null;

  console.log("Parsing downloaded data for:" + JSON.stringify(direction));
  /*{"station":"TUNNEY PASTURE","route":"97","stop_eta":"14","destination0":"Airport \/ A\u00e9roport","arrival1":"2","destination1":"Bayshore"}*/
  try {
    stop_name = response.station;
    if (response.route === ""){
      error_fetching(route, station, direction);
      return;
    }
    else if (direction === 0 || direction === "0" || response.arrival1 === ""){
      console.log("Grabbing from direction 0");
      stop_eta = response.arrival0;
      route_destination = response.destination0;
      direction = 0;
    }
    else {
      console.log("Grabbing from direction 1");
      stop_eta = response.arrival1;
      route_destination = response.destination1;
      direction = 1;
    }
  }
  catch (e) {
    console.log("Something went wrong trying to parse data:" + JSON.stringify(e));
  }

  console.log(("Arrival of " + route + " from " + stop_name + "to " + route_destination + " in " + stop_eta + " mins."));

  Pebble.sendAppMessage({
    "KEY_ROUTE" : parseInt(route),    
    "KEY_STOP_NUM" : parseInt(station),   
    "KEY_ETA" : parseInt(stop_eta),  
    "KEY_DST" : route_destination.substring(0,24),
    "KEY_STATION_STR" : stop_name.substring(0,24),
    "KEY_DIRECTION" : parseInt(direction)
  });

  // No error detector, save the values.
  console.log("Saving data:" + station + " " + route + " " + direction);  
  localStorage.setItem("last_station", station);
  localStorage.setItem("last_route", route);
  localStorage.setItem("last_direction", direction);


}


function fetch_next_bus(route, station, direction) {
  var response;
  var req = new XMLHttpRequest();
  var uri = "http://ottawa.travvik.com/beta/pebble.php?" +
      "routeno=" + route + "&stopno=" + station + "&src=pebble";

  console.log("Fetching from:" + uri);
  req.timeout = (1000*5); // 5 second timeout
  req.ontimeout = error_fetching(route, station, direction);
  req.open('GET', uri , true);
  req.onload = function(e) {
  if (req.readyState === 4) {
    if (req.status === 200) {
      // Caution, unicode errors may happen because of accent e.
      //console.log(escape(req.responseText));
      console.log(req.responseText);
      response = JSON.parse(req.responseText);
      parseTravvikData(response, route, station, direction);
      console.log("Sent data to pebble");
    }
    else {
      error_fetching(route, station, direction);
      }
  } else {
    error_fetching(route, station, direction);
    }
};
req.send(null);

}


Pebble.addEventListener("ready",
    function(e) {
      console.log("connect!" + e.ready);
      console.log(e.type);
      
      // Boot up, fetch old data, send it down!
      last_station = localStorage.getItem("last_station");
      last_route =   localStorage.getItem("last_route");
      last_direction = localStorage.getItem("last_direction");
      if ( last_station === null || last_route === null || last_direction === null) {
        console.log("Can't find old data to sync");
        last_station = -1;
        last_route = -1;
        last_direction = 0;
      }
      console.log("Old data:" + last_station + " " + last_route + " " + last_direction);  
      fetch_next_bus(last_route, last_station, last_direction);

    });

Pebble.addEventListener("appmessage",
    function(e) {
      console.log(e.type);
      console.log("rcvd msg frm pbl");
      fetch_next_bus( e.payload.KEY_ROUTE, 
                      e.payload.KEY_STOP_NUM,
                      e.payload.KEY_DIRECTION);
    });

Pebble.addEventListener("webviewclosed",
    function(e) {
      console.log("webview closed");
      console.log(e.type);
      console.log(e.response);
    });


