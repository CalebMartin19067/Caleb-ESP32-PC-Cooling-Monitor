// TODO What are these lines doing?
// at the bottom is says about responsive to stack links vertically
function handleNavBar() {
    var x = document.getElementById("myTopnav");  // Get the element with the id "myTopnav" (the top navigation bar) 
    if (x.className === "topnav") {               // Check if it only has the class "topnav"
        x.className += " responsive";             // If yes, add " responsive" to its class name (becomes "topnav responsive")
    } else {
        x.className = "topnav";                   // If it already has "responsive", remove it (reset back to just "topnav")
    }
}

//---------------------------------------------------------------
//What do these variable and functions have to do with the temperature reading?
window.addEventListener('load', getReadings);
// Function to get current readings on the web page when it loads for the first time
function getReadings() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) { //4 and 200 mean ready and ok
            var myObj = JSON.parse(this.responseText);
            console.log(myObj);
            document.getElementById("pres").innerHTML = myObj.temperature;
        }
    };
    xhr.open("GET", "/readings", true); 
    xhr.send();
}
// Create an Event Source to listen for events
if (!!window.EventSource) {
    var source = new EventSource('/events');
    source.addEventListener('open', function (e) {
        console.log("Events Connected");
    }, false);
    source.addEventListener('error', function (e) {
        if (e.target.readyState != EventSource.OPEN) {
            console.log("Events Disconnected");
        }
    }, false);
    source.addEventListener('new_readings', function (e) {
        console.log("new_readings", e.data);
        var obj = JSON.parse(e.data);
        document.getElementById("pres").innerHTML = obj.temperature;
    }, false);
}
