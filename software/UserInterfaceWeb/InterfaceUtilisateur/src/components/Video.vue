<template>
    <v-card>
      <img draggable="false" id="video" @click="sendPos" :src="source" alt="Flux Video">
      <p id="fps"></p>
        <v-card-actions>
          <div v-if="!detectionMode">
            <v-btn @click="manageCamera" :disabled="!tcpState" flat color="orange"> <div v-if="!cameraActive">Activer Camera</div> <div v-else>Désactiver Camera</div> </v-btn>
            <v-btn @click="askArena" :disabled="!cameraActive" flat color="orange">Detection Arene</v-btn>
            <v-btn @click="displayPos" :disabled="!cameraActive" flat color="orange"><div v-if="!positionMode">Afficher la position</div> <div v-else>Ne plus afficher la position</div> </v-btn>
          </div>
          <div v-else>
            <v-btn @click="confirmArena" :disabled="!cameraActive" flat color="orange">L'arene me plait</v-btn>
            <v-btn @click="infirmArena" :disabled="!cameraActive" flat color="orange">L'arene ne me plait pas</v-btn>
          </div>
          <div v-if="positionMode" class="subheadingfont position_place">
            <span v-if="positionMode"> Position :  {{robot.x}}, {{robot.y}}</span>
          </div>
          <div  v-if="positionMode && this.cursor.x !== null" class="subheadingfont position_place">
            <span> Ordre :  {{cursor.x}}, {{cursor.y}}</span>
          </div>
        </v-card-actions>
      </v-card>
</template>

<script>
export default {
  props:['tcpState'],
  data:()=>{
    return{
      source:'/static/neige.jpg',
      fps:1,
      cameraActive: false,
      positionState: false,
      detectionMode: false,
      positionMode: false,
      robot:{x: -1,y: -1},
      cursor:{x: null, y: null},
    }
  },
  methods:{
    hexToBase64 : function(data){
      return btoa(String.fromCharCode.apply(null, data.replace(/\r|\n/g, "").replace(/([\da-fA-F]{2}) ?/g, "0x$1 ").replace(/ +$/, "").split(" ")));
    },
    manageCamera: function(){
      if(this.tcpState === true)
        this.$socket.emit('camera', !this.cameraActive);
    },
    askArena: function(){
      if(this.cameraActive === true)
        this.$socket.emit('arena', 'ask');
        this.detectionMode = true;
    },
    displayPos:function(){
      if(this.cameraActive === true){
        this.$socket.emit('displayPos', !positionMode);
        this.positionMode= !this.positionMode;
      }
    },
    confirmArena: function(){
      if(this.cameraActive === true){
        this.$socket.emit('arena', 'ok');
        this.detectionMode = false;
      }
    },
    infirmArena: function(){
      if(this.cameraActive === true){
        this.$socket.emit('arena', 'nok');
        this.detectionMode = false;
      }
    },
    displayPos : function(){
      if(this.cameraActive === true){
        this.$socket.emit('displayPos', !this.positionMode);
        this.positionMode = !this.positionMode;
          
      }
    },
    sendPos: function(e){
      if(this.positionMode === true){
        let rect = document.getElementById('video').getBoundingClientRect();
        let robotX = (e.pageX - rect.x)|0, robotY=e.pageY - rect.y;
        this.cursor.x = parseInt((480/rect.width)*robotX);
        this.cursor.y = parseInt((360/rect.height)*robotY);
        this.$socket.emit('sendPos', this.cursor);
      }
    }
  },
  sockets:{
    IMG: function(data){
      if(this.cameraActive){
        let frame;
        this.source = "data:image/jpeg;base64,";
        this.source += this.hexToBase64(data);
        //frame = document.getElementById("video");
        //frame.src = this.source;
        this.fps++;
      }

    },
    POS: function(data){
      let a = data.substr(11);
      a = a.split(';');
      let x = Number(a[0]);
      let y = a[1].split('|');
      y = Number(y[0]);
      let robot2 = {
        x,
        y
      }
      this.robot = robot2;     
    },
    cameraState: function(data){
      this.cameraActive = data;
    },
    detectionError: function(){
      this.detectionMode=false;
    }
  },
  mounted: function (){
    setInterval(function(){
        document.getElementById('fps').innerText = `${this.fps} fps`;
        this.fps = 0;
    }.bind(this), 1000);
  },
  watch:{
    tcpState: function(){
      if(this.tcpState == false){
        this.cameraActive = false;
        this.source ="/static/neige.jpg"

      }
    }
  }
}
</script>

<style>
  #fps{
    position: absolute;
    top:10px;
    left:20px;
    color:red;
  }

  #video{
    width: 100%;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -o-user-select: none;
    user-select: none;
  }
  #video:hover
  {
    cursor:crosshair;
  }

  .clikable:hover
  {
    cursor:pointer;
  }
 .position_place{
   border:solid;
   border-color: lightblue;
   padding : 10px;
   border-width: 1px;
 }
</style>

