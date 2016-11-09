var Client = require('node-rest-client').Client;
var events = require('events');

var current_token = null;

function Auth(ssi_address, secret, lab_id) {
    this.ssi = ssi_address;
    this.timeout_timestamp = new Date().getTime();
    this.allowed_delay = 2000;
    this.token = null;
    this.secret = secret;
    this.lab_id = lab_id;
    
    this.HTTPauth = function (data, cb) {
        var args = {
            requestConfig: {
                timeout: 1000, //request timeout in milliseconds
            },
            responseConfig: {
                timeout: 5000 //response timeout
            },
            headers: {"Content-Type": "application/json"},
            data: data
        };
        // adicionar SSL 

        var client = new Client();
        console.log(args)

        var req = client.post('http://' + this.ssi + "/auth", args, function (data, response) {

            console.log('http' + response.statusCode, data);
            
            if (response.statusCode === 200 && typeof (cb) === 'function'){
                cb(data);

            }else if (typeof (cb) === 'function'){
                console.log('SSI request returned ',response.statusCode, new  Date())
                cb(null);
            }
                

        });

        req.on('requestTimeout', function (req) {
            console.log("request has expired");
            req.abort();
            cb(null);
        });

        req.on('responseTimeout', function (res) {
            console.log("response has expired");
            cb(null);
        });

    }
}

Auth.prototype.Authorize = function (token) {
    this.token = token;
    var eventEmitter = new events.EventEmitter();
    var that = this;
    
    this.HTTPauth({token: token,instance_secret: this.secret, lab_id: this.lab_id}, function (data) {
                
        if(data === null){
            eventEmitter.emit('not authorized');
        }else if(data.code === 200){
            console.log(data,new Date())
            that.timeout_timestamp =  parseInt(new Date().getTime()) + parseInt(data.duration*60*1000) - that.allowed_delay;
            current_token = that.token;
            eventEmitter.emit('authorized');
        }else {
            console.log(data, new Date());
            eventEmitter.emit('not authorized');
        }
        
    });

    return eventEmitter;
}  

Auth.prototype.isAuthorized = function () {
    
    return (new Date().getTime() <= this.timeout_timestamp && this.token === current_token) 
}

module.exports = Auth;