const express = require('express');
const { networkInterfaces } = require('os');
const path = require('path');

 
const app = express();
const nets = networkInterfaces();
 
// Server port
const PORT = 3000;
 
app.get('/', (request, response) => response.send('This is the ESP-Clock Update & services server'));

let downloadCounter = 1;
app.get('/firmware/httpUpdateNew.bin', (request, response) => {
    console.log('FW update require');
    response.download(path.join(__dirname, 'firmware/httpUpdateNew.bin'), 'httpUpdateNew.bin', (err)=>{
        if (err) {
            console.error("Problem on download firmware: ", err)
        }else{
            downloadCounter++;
        }
    });
    console.log('Your file has been downloaded '+downloadCounter+' times!')
})

app.get('/gettimediff',  (request, response) => {
    console.log('Time info');
    now = new Date();
    retObj = new Object();
    retObj.diff=now.getTimezoneOffset();
    retObj.epoch=now.getTime();
    retObj.local=now.toString();
    response.setHeader('Content-Type', 'application/json');
    response.send(retObj);
});


app.listen(PORT, () => {
    const results = {}; // Or just '{}', an empty object
 
    for (const name of Object.keys(nets)) {
        for (const net of nets[name]) {
            // Skip over non-IPv4 and internal (i.e. 127.0.0.1) addresses
            if (net.family === 'IPv4' && !net.internal) {
                if (!results[name]) {
                    results[name] = [];
                }
                results[name].push(net.address);
            }
        }
    }
 
    console.log('ESPClock server Listening on port '+PORT+'\n', results)
});
