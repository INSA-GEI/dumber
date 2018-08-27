#!/usr/local/bin/node

const express = require('express');

const app = express();
const http = require('http').Server(app);
const net = require('net');
const io = require('socket.io')(http);
let numMes = 1;

app.use(express.static(__dirname + '/dist'));

const socket = new net.Socket();
socket.setEncoding('hex');

const ipSuperviseur = 'localhost';
const port = process.env.port || 3000;
let dataInReception = '';
let lastRequest = '';
let dateOrigin;
const connections = [];

/** DECLARATION FONCTIONS */

/*
 * Fonction qui retourne le temps à un message passé en paramétre
 */
function addTime(msg) {
  const a = Math.abs(new Date() - dateOrigin);
  const date = new Date(a);
  const message = `<${date.getMinutes()}:${date.getSeconds()}:${date.getMilliseconds()}>${msg}`;
  return message;
}

function sendConsole(msg) {
  io.emit('consoleIn', addTime(msg));
}

/*
 * Conversion chaine de caractére avec des informations hexa en chaine de caractére
 * avec les informations ascii.
 */

function sendNotification(typemes, text) {
  const myObj = {
    msg: `${numMes} ${text}`,
    type: typemes,
  };
  io.emit('notifications', myObj);
  numMes++;
}

function hex2a(hexx) {
  const hex = hexx.toString();
  let str = '';
  for (let i = 0; i < hex.length; i += 2) {
    str += String.fromCharCode(parseInt(hex.substr(i, 2), 16));
  }
  return str;
}

function disconnectTcp() {
  socket.write('STO: ');
  lastRequest = 'disconnection';
  sendConsole('Déconnecté de la cible');
}

function moveForward() {
  socket.write('DMB:F ');
  lastRequest = 'GoingForward';
  sendConsole('Avance');
}

function moveBack() {
  socket.write('DMB:B ');
  lastRequest = 'GoingBack';
  sendConsole('recule');
}

function moveRight() {
  socket.write('DMB:R ');
  lastRequest = 'TurnRight';
  sendConsole('pivot droite');
}

function moveLeft() {
  socket.write('DMB:L ');
  lastRequest = 'TurnLeft';
  sendConsole('pivotGauche');
}

function stopMove() {
  socket.write('DMB:S ');
  lastRequest = 'Stop';
  sendConsole('Stop');
}

function startWithWDDumby() {
  socket.write('DMB:W ');
  lastRequest = 'StartDumbyWithWD';
}

function startWithoutWDDumby() {
  socket.write('DMB:u ');
  lastRequest = 'StartDumbyWithoutWD';
}

function restartState() {
  socket.write('DMB:r ');
  lastRequest = 'idleDumby';
}

function sendPos(position) {
  socket.write(`POS:${position.x},${position.y} `);
  lastRequest = 'SendPosition';
  sendConsole(`Envoi position : ${position.x}, ${position.y}`);
}

function setDisplayPos(data){
  if(data === true){
    socket.write('CAM:p ');
    lastRequest='displayPos';
  }else{
    socket.write('CAM:s ');
    lastRequest='dontDisplayPos';
  }
}

function eventArena(data) {
  if(data === 'ask'){
    socket.write('CAM:y ');
    lastRequest = 'askArena';
  } else if(data ==='ok'){ 
    socket.write('CAM:x ');
    lastRequest = 'arenaConfirm';
  }else if(data ==='nok'){
    socket.write('CAM:z ');
    lastRequest = 'arenaInfirm';
  }
}
/*
 * Traite une trame de donnée reçu.
 */
function traitmentMessage(val) {
  let payload = val.substring(6);
  const header = hex2a(val.substring(0, 6));
  if (header !== 'IMG') {
    payload = hex2a(payload);
    sendConsole(`Reçu : ${header}:${payload}`);
  }
  if (header === 'LCD') {
    io.emit('lostSerial', false);
    sendNotification('error', 'Communication Xbee : Perdu');
  }
  if (header === 'ACK') {
    if (lastRequest === 'IdleDumby') {
      io.emit('dumberDisconected');
    }
    if (lastRequest === 'openSerial') {
      io.emit('serialOpen', true);
      sendNotification('success', 'Communication Xbee : OK');
    }
    if (lastRequest === 'closeSerial') {
      io.emit('serialOpen', false);
      sendNotification('warning', 'Communication Xbee : fermé');
    }
    if (lastRequest === 'cameraActive') {
      io.emit('cameraState', true);
      sendNotification('info', 'Camera activé');
    }
    if (lastRequest === 'cameraInactive') {
      io.emit('cameraState', false);
      sendNotification('info', 'Camera désactivé');
    }
    if (lastRequest === 'StartDumbyWithWD' || lastRequest === 'StartDumbyWithoutWD') {
      sendConsole('Dumby is now started');
      io.emit('dumbyStart', true);
    }
    if (lastRequest === 'idleDumby') {
      sendConsole('Dumby is now stoped');
      io.emit('dumbyStart', false);
    }
  } else if (header === 'NAK') {
    sendNotification('error', `Error with last request : ${lastRequest}`);
    if(lastRequest === 'askArena'){
      io.emit('detectionError');
    }
  } else {
    if(header === 'BAT'){
      io.emit(header, payload.toString());
    }else{
      io.emit(header, payload);
    }
  } 
}

function serial(data) {
  if (data === true) {
    socket.write('COM:o ');
    lastRequest = 'openSerial';
  } else {
    socket.write('COM:C ');
    lastRequest = 'closeSerial';
  }
}

function specialMes(data) {
  sendConsole(data);
  socket.write(`MSG:${data} `);
}

function camera(state) {
  if (state === true) {
    lastRequest = 'cameraActive';
    socket.write('CAM:A ');
  } else {
    lastRequest = 'cameraInactive';
    socket.write('CAM:I ');
  }
}

/*
 * Descriptions des fonctions de call back
 */

function connectTcp() {
  socket.connect(8080, ipSuperviseur, (err) => {
    if (!err) {
      dateOrigin = new Date();
      io.emit('superViseurConnection', true);
      sendNotification('success', 'Connecté au superviseur');
      //sendConsole('Connecté au superviseur');
    } else {
      console.log('erreurConnection');
      sendNotification('danger', 'Erreur lors de la connection');
      //sendConsole('Erreur survenue lors de la connection au superviseur');
    }
  });
  lastRequest = 'Connection';
}

/*
 * bind des evenenements aux fonctions de callback
 * Gestion des clients connecté dans la list client[]
 */

function handleConnection(client) {
  sendNotification('success', 'Votre interface est connecté au serveur node');
  connections.push(client);
  client.on('askConnection', connectTcp);
  client.on('askDisconnection', disconnectTcp);
  client.on('startWithWD', startWithWDDumby);
  client.on('startWitouthWD', startWithoutWDDumby);
  client.on('idle', restartState);
  client.on('sendPos', sendPos);
  client.on('displayPos', setDisplayPos);
  client.on('arena', eventArena);
  client.on('MOVEFORWARD', moveForward);
  client.on('MOVEBACK', moveBack);
  client.on('MOVERIGHT', moveRight);
  client.on('MOVELEFT', moveLeft);
  client.on('MOVESTOP', stopMove);
  client.on('openSerial', serial);
  client.on('camera', camera);
  client.on('console-out', specialMes);

  client.on('disconnect', () => {
    const index = connections.indexOf(client);
    connections.splice(index, 1);
  });
}

/** FIN DECLARATION FONCTION */
socket.on('error', (err) => {
  console.log(`${err}`);
  sendNotification('error', 'Erreur impossible de se connecter');
  io.emit('superViseurConnection', false);
  lastRequest = '';
});

io.on('connection', handleConnection);

socket.on('end', () => {
  console.log('superviseur déconnecté');
  sendNotification('error', ' Connection au superviseur perdu');
  io.emit('superViseurConnection', false);
  lastRequest = '';
});

/*
 * Evenement sur une donnée tcp reçu
 */
socket.on('data', (data) => {
  const trame = data.split('5452414d45'); // Trame sera le parser final ici écrit en hexa
  dataInReception += trame[0];
  if (trame.length > 1) {
    traitmentMessage(dataInReception);
    for (let i = 1; i < trame.length - 1; i++) {
      traitmentMessage(trame[i]);
    }
    dataInReception = trame[trame.length - 1];
  }
});

/* Gestions des routes
 * */
app.get('/', (req, res) => {
  res.sendFile('index.html');
});

http.listen(port); // Lancement du serveur web
console.log(`server is now running on port ${port}`);
