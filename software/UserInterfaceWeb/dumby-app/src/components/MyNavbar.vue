<template>
  <v-toolbar dark color="pink darken-3">
    <v-toolbar-title  class="white-text hidden-sm-and-down">Dumby User Interface</v-toolbar-title>
    <v-menu :nudge-width="100">
        <img class="logo" id="logo-connection" src="static/logo_connection.svg" slot="activator">
      <v-list>
          <v-list-tile @click="tcpManageRq">
            <v-list-tile-title> <div v-if="!tcpState">Connexion au Superviseur</div> <div v-else>Deconnection au Superviseur</div></v-list-tile-title>
          </v-list-tile>  
          <v-list-tile :disabled="!tcpState" @click="manageSerial" class="hidden-sm-and-up">
            <v-list-tile-title> <div v-if="!serialState"> Connexion au Robot</div> <div v-else> Deconnexion au Robot </div></v-list-tile-title>
          </v-list-tile>
      </v-list>
    </v-menu>
    <v-menu :nudge-width="100" class="hidden-sm-and-up">
        <img class="logo" id="logo-robot" src="static/logo_dumby_white.svg" slot="activator">
      <v-list>
        <v-list-tile :disabled="!serialState" @click="runWithWDRq">
          <v-list-tile-title>Demarrer avec Watchdog</v-list-tile-title>
        </v-list-tile>
        <v-list-tile :disabled="!serialState" @click="runWithoutWDRq">
          <v-list-tile-title> Demarrer Sans Watchdog</v-list-tile-title>
        </v-list-tile>
        <v-list-tile :disabled="!serialState" @click="idleDumby">
          <v-list-tile-title> Stoper le robot</v-list-tile-title>
        </v-list-tile>
      </v-list>
    </v-menu>
    <v-menu :nudge-width="100" class="hidden-sm-and-up">
        <v-icon class="logo" slot="activator">camera_alt </v-icon>
      <v-list>
          <v-list-tile :disabled="!tcpState" @click="manageCamera">
            <v-list-tile-title>Activer / Désactiver Camera</v-list-tile-title>
          </v-list-tile>
        <v-list-tile :disabled="!tcpState" @click="arenaDetectionRq">
          <v-list-tile-title>Detection Arene</v-list-tile-title>
        </v-list-tile>
      </v-list>
    </v-menu>
    <v-spacer></v-spacer>
    <v-tooltip>
        <Voyant id="led-connection" slot="activator" :ledOn="tcpState"/>
        <span>Etat de connection au Superviseur</span>
    </v-tooltip>
    <v-tooltip>
    <img id="logo-battery" :src="batterie_src" slot="activator" alt="">
    <span>Niveau de batterie</span>
    </v-tooltip>
  </v-toolbar>
</template>

<script>
import Voyant from './Voyant'
export default {
  components:{
    Voyant,
  },
  props:['tcpState', 'serialState'],
  data:()=>{
    return{
      robotRun: false,
      battery: 0,
      batterie_src : '',
    }
  },
  sockets:{
    BAT: function(data){
      this.battery = parseInt(data);
     /* if(data === '2'){
        this.batterie_src = "static/battery_full_white.svg";
      }else if (data === '1'){
        this.batterie_src = "static/battery_medium_white.svg";
      }else{
        this.batterie_src = "static/battery_low_white.svg";
      }*/
    },
    robotState:function(data){
      
    }

  },
 watch: {
    battery: function(){
      switch (this.battery) {
        case 0:
          this.batterie_src = "static/battery_low_white.svg";
          break;
        case 1:
          this.batterie_src = "static/battery_medium_white.svg";
          break;
        case 2:
          this.batterie_src = "static/battery_full_white.svg";
          break;
        default:
          this.batterie_src= ""
          break;
      }
    }
  },
  methods:{
    tcpManageRq: function(){
      if(this.tcpState==false){
        this.$socket.emit('askConnection');
        console.log('demande de connection');
      }
      else{
        this.$socket.emit('askDisconnection'); 
      }
      
    },
    manageSerial: function(){
      if(this.tcpState){
          this.$socket.emit('openSerial',!this.serialState);
      }
    },
    runWithWDRq: function(){
    },
    runWithoutWDRq: function(){
    },
    idleDumby: function(){
    },
    manageCamera: function(){
    },
    arenaDetectionRq: function(){
    }
  }
}
</script>


<style>
  .logo{
    margin-left:12px;
  }
  #logo-robot{
    width: 35px;
    margin-top:7px;
    margin-left:16px;
  }   
  #logo-connection{
    width: 16px;
    margin-top:5px;
    margin-left:30px;
  }
  #logo-battery{
    width:50px;
    margin-left: 30px;
  }
  #led-connection{
    width:20px;
    height: 20px;
    padding-left:10px;
    margin-left:10px;
  }
</style>
