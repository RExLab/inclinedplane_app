var express = require('express');
var panel = require('./build/Release/panel.node');

var app = express();
var configured = false;

app.get('/:command', function (req, res) {

	console.log(req.params.command);


if(req.params.command == 'setup'){
	configured = panel.setup();
	res.send("1 " + configured);
	
}else if(req.params.command == 'run'){
	if (configured)
		panel.run();

    res.send("OK!");

}else if(req.params.command == 'exit'){
	if(configured){
		panel.exit();
		configured = false;
	}
	res.send("OK!");

}else if(req.params.command == 'pos'){
	if(configured){
		console.log(panel.weighingPosition(1));
	}
    res.send("OK!");

}else if(req.params.command == 'weigh'){
	if(configured){
		console.log(panel.weigh());
	}
    res.send("OK!");

}else if(req.params.command == 'posback'){
	if(configured){
		console.log(panel.weighingPosition(0));
	}
    res.send("OK!");

}else if(req.params.command == 'angle'){
	if(configured){
		console.log(panel.angle());
	}
    res.send("OK!");

}else if(req.params.command == 'cancel'){
	if(configured){
		console.log(panel.cancel());
	}
    res.send("OK!");

}else if(req.params.command == 'start'){
	if(configured){
		console.log(panel.start());
	}
    res.send("OK!");

}else if(req.params.command == 'getfalltime'){
	if(configured){
		console.log(panel.getfalltime());
	}
    res.send("OK!");

}else{
	if(configured && !isNaN(req.params.command)){
		panel.setAngle(parseInt(req.params.command));
	    res.send("OK");
	}else{
	    res.send("NOK!");

	}
}
	

});





var server = app.listen(80, function () {

  var host = server.address().address;
  var port = server.address().port;
  
  console.log('App listening at http://%s:%s', host, port);
//   panel.setup();
//   panel.run();

});

