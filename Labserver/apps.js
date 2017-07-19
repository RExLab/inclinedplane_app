// Setup basic express server
var panel = require('./build/Release/panel.node');
var express = require('express');
var app = express();
var fs = require('fs')
var server = require('http').createServer(app);
var io = require('socket.io')(server);
var port = 80;
var cors = require('cors');


app.get('/', cors(), function (req, res, next) {
    var data = fs.readFileSync('/home/inclinedplane/public/metadata.json', 'utf8');
    res.send(data);
});


var hold = false;
var running = false;
var configured = false;

function drop(socket) {
    panel.start();
    panel.weighingPosition(0);

    setTimeout(function () {
        var data = JSON.parse(panel.getfalltime());
        data.code = 3;
        data.angle = panel.angle();
        socket.emit('lab done', data);
        panel.cancel();
    }, 2000);

}

function sendMessage(socket) {
    var ret = {};
    ret.angle = panel.angle();

    ret.forcey = panel.weigh() * 9.8;
    ret.forcex = '?';
    if (ret.angle == 0) {
        ret.force = ret.forcey;
        ret.forcex = Math.sqrt(Math.pow(ret.force, 2) - Math.pow(ret.forcey, 2));
    }
    ret.hold = false;
    ret.status = 'ready';
    if (!hold && ret.angle > 0) {
        ret.error = 1;
    } else {
        ret.hold = true;
    }
    socket.emit('setup', ret);
}

function setupConn(socket) {
    if (panel.setup() == 1) {
        panel.run();
        panel.cancel();
        sendMessage(socket);
        running = configured = true;
        hold = false;
    } else {
        //socket.emit('setup', 'error:'+ "something wrong is going on");
        console.log("nao foi possivel abrir uart");
    }

}


io.on('connection', function (socket) {

    var authenticated = false;

    socket.on('new connection', function (data) {
      console.log(new Date());

        authenticated = true;
        if (running) {
            panel.exit();
            setTimeout(function () {
                setupConn(socket);
            }, 500);

        } else {
            setupConn(socket);
        }

    });


    socket.on('new angle', function (data) {

      console.log(new Date());

        if (!configured && !authenticated) {
            console.log('attempt not authenticated')
            console.log(data)
            return;
        }


        if (parseInt(data.angle) > 90) {
            sendMessage(socket);
            return;
        }
        var ret = 0;
        if ((ret = panel.setAngle(parseInt(data.angle))) > 0) {
            console.log("ajustando angulo " + data.angle);
        } else {
            console.log("erro ao ajustar angulo", ret);
            socket.emit('erro', {code: ret, message: "An error ocurred while attempting to set a new angle."})
            return;
        }

        if (data.angle <= -15) {
            setTimeout(function () {

                if (panel.angle() <= -15) {
                    panel.weighingPosition(1);
                    hold = true;
                    sendMessage(socket);

                } else {
                    setTimeout(function () {

                        if (panel.angle() <= -15) {
                            panel.weighingPosition(1);
                            hold = true;
                        }
                        sendMessage(socket);

                    }, 6000 + Math.abs((panel.angle() - data.angle) * 165));
                }

            }, 6000 + Math.abs((panel.angle() - data.angle) * 165));

        } else {
            setTimeout(function () {
                sendMessage(socket);
                console.log("pronto para soltar");

            }, 6000 + Math.abs((data.angle - panel.angle()) * 165));
        }
    });



    socket.on('drop', function (data) {
        if (!configured && !authenticated) {
            console.log('attempt not authenticated')
            console.log(data)
            return;
        }

        if (hold && panel.angle() > 0) {
            drop(socket);
            hold = false;
        } else {
            sendMessage(socket);

        }

    });



    socket.on('disconnect', function () {
        if (!authenticated) {
            console.log('attempt not authenticated')
            return;
        }
      console.log(new Date());

        if (configured) {
            panel.setAngle(-3);
            console.log('disconnected');
            setTimeout(function () {
                configured = running = authenticated = false;
                panel.exit();
            }, 500);
        } else {
            panel.setup();
            panel.run();
            panel.exit();
        }

    });


});


server.listen(port, function () {
    console.log('Server listening at port %d', port);
    console.log(new Date());
});
