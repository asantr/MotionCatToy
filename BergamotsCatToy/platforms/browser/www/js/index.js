console.log("starting index.js")

var SCAN_TIME = 7000;           // Scan for 7 seconds
var CONNECTION_TIMEOUT = 7000;  // Wait for 7 seconds for a valid connection

// *********   Global variables used for all IDs that are used in multiple functions
var refreshDevicesModal = null;
var connectingModal = null;
var deviceList = null;
var deviceObjectMap = null;
var pageNavigator = null;
var connectingDevice = null;
var connectionTimeout = null;

var defaultRed = 255;
var defaultGreen = 255;
var defaultBlue  = 255;  

// UUID's that are used to identify different characteristics
var simpleCustomService = "cd914837-b0d8-4cf7-9b11-f3f985a03c2d";
var onoff = "FF01";
var slowC = "FF02";
var fastC = "FF03";
var weirdC = "FF04";
var leftC = "FF05";
var rightC = "FF06";
var circleC = "FF07";
var catInRoom = "FF08"



// *********   Functions for scanning and scan related events

function scanFailed() {
    refreshDevicesModal.hide();
}

function scanStop() {
    ble.stopScan();
    refreshDevicesModal.hide();
}

function deviceDiscovered(device) {
    // Debugging: Console log of details of item
    // console.log(JSON.stringify(device));

    if(deviceObjectMap.get(device.id) == undefined ) {
        // New Device. Add it to the collection and to the window
        deviceObjectMap.set(device.id, device);

        // Identify the name or use a default
        var name = "(none)";
        if (device.name != undefined) {
            name = device.name;
        }

        // Create the Onsen List Item
        var item = ons._util.createElement('<ons-list-item modifier="chevron" tappable> ' +
            '<ons-row><ons-col><span class="list-item__title" style="font-size: 4vw;">' + device.id + '</span></ons-col></ons-row>' +
            '<ons-row><ons-col><span class="list-item__subtitle" style="font-size: 2vw;">RSSI:' + device.rssi + '</span></ons-col><ons-col><span style="font-size: 2vw;">Name: ' + name + '</span></ons-col></ons-row>' +
            '</ons-list-item>');

        // Set the callback function
        item.addEventListener('click', deviceSelected, false);

        // Associate the element in the list with the object
        item.device = device;

        // Iterate through the list and add item in place by RSSI
        var descendants = deviceList.getElementsByTagName('ons-list-item');
        var i;
        for(i=0;i<descendants.length;i++) {
            if(device.rssi > descendants[i].device.rssi) {
                descendants[i].parentNode.insertBefore(item, descendants[i]);
                return;
            }
        }
        // If it hasn't already returned, it wasn't yet inserted.
        deviceList.append(item);
    }
}

function startScan() {
    // Disable the window
    refreshDevicesModal.show();

    // Empty the list (on screen and Map)
    deviceList.innerHTML = "";
    deviceObjectMap = new Map();

    // Start the scan
    ble.scan([], SCAN_TIME, deviceDiscovered, scanFailed);

    // Re-enable the window when scan done
    setTimeout(scanStop, SCAN_TIME);
}


var messageCounter = 0;

// ***** Button Related Functions ********
function buttonData(buffer) {
    var array = new Uint8Array(buffer);
    var buttonValue = document.getElementById("buttonValue");
    buttonValue.checked =  (array[0] != 0);
    messageCounter++;

    console.log("Total Messages: " + messageCounter);
}
function catData(buffer) {
    var array = new Uint8Array(buffer);
 
     if(array[0] != 1){
        document.getElementById("catStatCol").innerHTML = "Bergamot is not here";
    }else{
        document.getElementById("catStatCol").innerHTML = "Bergamot is in the room";
    }
}


function buttonDataFailed() {
    console.log("Button Read Failed");
}


//This function is for reading the values in characteristics

function readButton() {
    ble.read(connectingDevice.id, simpleCustomService, catInRoom, catData);
    ble.read(connectingDevice.id, buttonService, buttonCharacteristic, buttonData, buttonDataFailed); 
}



// ********   Functions for device connection related events


function deviceConnectionSuccess(device) {
    clearTimeout(connectionTimeout);
    connectingModal.hide();
    connectingDevice = device;

    // Studio 11:  Now that the device is connected, request any data that's needed
//    readButton();
    // Set up a timer to repeatedly "poll" for data
    //our set up reads the characteristics every second. 
    connectingDevice.pollingTimer = setInterval(readButton, 1000);

}

function deviceConnectionFailure(device) {
    console.log("Device Disconnected");
    pageNavigator.popPage();
    refreshDevicesModal.hide();
    connectingDevice = null;
}

function deviceConnectionTimeout() {
    // Device connection failure
    connectingModal.hide();
    pageNavigator.popPage();
    refreshDevicesModal.hide();
    if(connectingDevice != null) {
        clearInterval(connectingDevice.pollingTimer);
        ble.disconnect(connectingDevice.id);
    }
}

function disconnectDevice() {
    console.log("Disconnecting");
    if(connectingDevice !== null) {
        clearInterval(connectingDevice.pollingTimer);
        ble.disconnect(connectingDevice.id);
    }
}


// ***** Function for user-interface selection of a device
function deviceSelected(evt) {
    var device = evt.currentTarget.device;
    // Initiate a connection and switch screens; Pass in the "device" object
    pageNavigator.pushPage('deviceDetails.html', {data: {device: evt.currentTarget.device}});
    connectingDevice = device;
    ble.connect(device.id, deviceConnectionSuccess, deviceConnectionFailure);
    connectionTimeout = setTimeout(deviceConnectionTimeout, CONNECTION_TIMEOUT);
}

// *****  Function for initial startup
ons.ready(function() {
    console.log("Ready");

    // Initialize global variables
    refreshDevicesModal = document.getElementById('refreshDevicesModal');
    pageNavigator = document.querySelector('#pageNavigator');
    pageNavigator.addEventListener('postpop', disconnectDevice);

    var refreshButton = document.getElementById('refreshButton');
    refreshButton.addEventListener('click',  function() {
            console.log("Refresh; Showing modal");
            startScan();
    } );

    deviceList = document.getElementById('deviceList');

    // Add a "disconnect" when app auto-updates
    if(typeof window.phonegap !== 'undefined') {
        // Works for iOS (not Android)
        var tmp = window.phonegap.app.downloadZip;
        window.phonegap.app.downloadZip = function (options) {
            disconnectDevice();
            tmp(options);
        }
    }

    var pullHook = document.getElementById('pull-hook');
    pullHook.onAction = function(done) {
        startScan();
        // Call the "done" function in to hide the "Pull to Refresh" message (but delay just a little)
        setTimeout(done, 500);
    };
});


// *** Functions for page navigation (page change) events
document.addEventListener('init', function(event) {
    var page = event.target;

    if (page.id === 'deviceDetails') {
        // Enable the modal window
        connectingModal = document.getElementById('connectingModal');
        connectingModal.show();

        // Update the page's title bar
        page.querySelector('ons-toolbar .center').innerHTML = "Bergamot's Cat Toy";
        document.getElementById("buttonValue").addEventListener("change", function(event) {
                alert("Don't change the switch!")
                event.switch.checked = !event.switch.checked;
            });
    }
    //start page
    //displays the time and current color
    // has buttons for on off and fade on and off
    if(page.id === 'status'){
        var onOff = new Uint8Array(1);
        onOff[0] = 0;
        //if on is clicked write 1 to characteristic to trigger LED on
        page.querySelector('#onbutton').onclick = function(){
            onOff[0] = 1;
            ble.write(connectingDevice.id, simpleCustomService, onoff, onOff.buffer);
            document.getElementById("statCol").innerHTML = "Cat toy is on";
        } 
        //if off is clicked write 0 to characteristic to trigger LED off
        page.querySelector('#offbutton').onclick = function(){
            onOff[0] = 0;
            ble.write(connectingDevice.id, simpleCustomService, onoff, onOff.buffer);
            document.getElementById("statCol").innerHTML = "Cat toy is off";
        }
    }

    //page to set the default color
    if(page.id === 'modes'){
       var modeSet = new Uint8Array(1);
       modeSet[0] = 0;

        // on set color button click we write the values to color characteristic
        page.querySelector('#slowButton').onclick = function(){
            modeSet[0] = 1;
            ble.write(connectingDevice.id, simpleCustomService, slowC, modeSet.buffer);
        } 
        //if off is clicked write 0 to characteristic to trigger LED off
        page.querySelector('#fastButton').onclick = function(){
            modeSet[0] = 1;
            ble.write(connectingDevice.id, simpleCustomService, fastC, modeSet.buffer);
        }
        page.querySelector('#randomButton').onclick = function(){
            modeSet[0] = 1;
            ble.write(connectingDevice.id, simpleCustomService, weirdC, modeSet.buffer);
        } 
       
    }
    //page to set on in or off in 
    if(page.id === 'freeplay'){
       var moveSet = new Uint8Array(1);
       moveSet[0] = 0;

        // on set color button click we write the values to color characteristic
        page.querySelector('#leftButton').onclick = function(){
            moveSet[0] = 1;
            ble.write(connectingDevice.id, simpleCustomService, leftC, moveSet.buffer);
        }
        //if off is clicked write 0 to characteristic to trigger LED off
        page.querySelector('#rightButton').onclick = function(){
            moveSet[0] = 1;
            ble.write(connectingDevice.id, simpleCustomService, rightC, moveSet.buffer);
        }
        page.querySelector('#circleButton').onclick = function(){
            moveSet[0] = 1;
            ble.write(connectingDevice.id, simpleCustomService, circleC, moveSet.buffer);
        } 
    }
    
});
console.log("loaded index.js");
