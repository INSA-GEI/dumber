<template>
  <div id="app">
    <v-app v-if="wsConnected">
     <NotesList class="notif"/>
      <v-container grid-list-md>
        <Navbar :tcpState="tcpConnected" :serialState="serialOpened"/>
        <v-layout row wrap>
          <v-flex d-flex xs12 sm6 md6>
            <Video :tcpState="tcpConnected"/>
          </v-flex>
          <v-flex d-flex xs12 sm6 md6>
            <v-layout row wrap xs12>
              <v-flex d-flex>
                    <Dashboard :tcpState="tcpConnected" :serialState="serialOpened"/>
              </v-flex>
            </v-layout>
          </v-flex>
          <v-flex d-flex xs12 sm12 md12 class="hidden-sm-and-down">
              <Console :tcpState="tcpConnected" :serialState="serialOpened"/> 
          </v-flex>
        </v-layout>
        <FooterBar/>
      </v-container>
      </v-app>
      <v-app v-else>
        <Loading class="center"/>
      </v-app>

  </div>
  
</template>

<script>
import NotesList from './components/NotificationList'
import Console from './components/Console'
import Controller from './components/Controller'
import FooterBar from './components/Footerbar'
import Navbar from './components/MyNavbar'
import Video from './components/Video'
import Loading from './components/LoadingScreen'
import Dashboard from './components/DashBoard'

export default {
  name: 'App',
  data:()=>{
    return{
      wsConnected: false,
      tcpConnected:false,
      serialOpened:false,
    }
  },
  components: {
    NotesList,
    Console,
    Controller,
    Navbar,
    FooterBar,
    Video,
    Loading,
    Dashboard,
  },
  sockets:{
      connect: function(){
      return this.wsConnected=true;
    },
    disconnect: function(){
      this.wsConnected=false;
      this.tcpConnected = false;
      this.tcpSerial = false;

    },
    superViseurConnection: function(state){
      if(typeof(state) === 'boolean')
        this.tcpConnected = state;
      if(state === false)
        this.serialOpened = false;
    },
    serialOpen: function(state){
      if(typeof(state) === 'boolean')
        this.serialOpened = state;
    }
  },
} 
</script>
<style>
  .notif{
    margin-left: 50%;
    transform: translate(-50%);
    position: absolute;
    z-index: 10;
  }

  .center{
    text-align: center;
  }
</style>
