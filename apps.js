// Setup basic express server
var panel = require('./build/Release/panel.node');
var express = require('express');
var app = express();

var server = require('http').createServer(app);
var io = require('socket.io')(server);
var port =  80;

app.use(express.static(__dirname + '/public'));

server.listen(port, function () {
  console.log('Server listening at port %d', port);
  
  
}); 
io.set("heartbeat timeout",30000);
io.set("heartbeat interval", 5000);

var hold = false;	
var running = false;

  function drop(socket){
	    panel.start();
		panel.weighingPosition(0);
		
		setTimeout(function(){
			var data = JSON.parse(panel.getfalltime());
			data.code = 3;
			data.angle=panel.angle();
			socket.emit('lab done', data);
			panel.cancel();
		},2000);	
	  
  }

  function sendMessage(socket){  
		var ret = {};
		ret.angle = panel.angle();
		
		ret.forcey = panel.weigh()*9.8;
		ret.forcex = '?';
		if(ret.angle == 90){
			ret.force = ret.forcey; 
			ret.forcex = Math.sqrt(Math.pow(ret.force,2)-Math.pow(ret.forcey,2));
		}
		ret.hold = false;
		ret.status = 'ready';
		if(!hold && ret.angle > 0){
			ret.error = 1;
		}else{
			ret.hold = true;
		}
		socket.emit('setup', ret);
  }
  
  function setupConn(socket){
	  if ( panel.setup() == 1){
			panel.run();
			panel.cancel();
			sendMessage(socket);
			running = socket.configured=true;
			hold = false;
		}else{
			//socket.emit('setup', 'error:'+ "something wrong is going on");
			console.log("nao foi possivel abrir uart");
		}	
	  
  }
  
  
io.on('connection', function (socket) {
  
   socket.configured = false;
   socket.authenticated = false;
   
  socket.on('new connection', function(data){
	// Autorização
	if( running ){
		panel.exit();
		setTimeout(function(){			
			setupConn(socket);
		}, 500);
		
	}else{
		setupConn(socket);	
	}
	
  });
  
  
 socket.on('new angle', function(data){
	 if(!socket.configured){	 
		 return;
	 }
	
	if(parseInt(data.angle) > 90){
		sendMessage(socket);
		return;
	}
	
	//console.log("set angle: "+panel.setAngle(parseInt(data.angle))+ " = "+ data.angle);	
	
	if(data.angle <= -15){
		setTimeout(function(){
			if(panel.angle()<=-15){
				panel.weighingPosition(1);
			}
			sendMessage(socket);
			hold = true;
		},6000+Math.abs((panel.angle()-data.angle)*165));
	
	}else{
		setTimeout(function(){
				sendMessage(socket);
				console.log("pronto para soltar");

		},6000+Math.abs((data.angle - panel.angle())*165));
	}
 });
 

  
	socket.on('drop', function (data) {
		if(!socket.configured){
			 
			 return;
		 }
		//console.log("drop: "+ data);
		
		if(hold && panel.angle() > 0){
			drop(socket);
			hold = false;
		}else{
			sendMessage(socket);

		}
		  
	});



  socket.on('disconnect', function () {
	 if(!socket.configured){
		 return;
	 }
	panel.setAngle(-3);
	console.log('disconnected');
	setTimeout(function(){
		
		socket.configured = running = false;
		panel.exit();
		
		
	}, 500);
  });
  
  
});
