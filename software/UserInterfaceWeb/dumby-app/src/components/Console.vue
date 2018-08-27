<template>
    <div>
        <v-layout row wrap>
            <v-flex d-flex xs12>
                <div id="console-input" class="ui fluid consoleMes"></div>
            </v-flex>
            <v-flex d-flex xs12>
                <v-text-field :disabled="!tcpState" @keyup.enter="consoleOut" label="Envoi Message Console" v-model="output"> </v-text-field>
                <v-btn :disabled="!tcpState" icon @click="consoleOut" color="primary" dark><v-icon>send</v-icon></v-btn>
            </v-flex>
        </v-layout>
    </div> 
</template>

<script>
export default {
  props:['tcpState', 'serialState'],
  data:()=>{
      return {
          output: '',
      }
  },
  sockets:{
      consoleIn(message){
          if(this.tcpState === true){
            let balise = document.createElement('div');
            balise.textContent = message;
            let console = document.getElementById('console-input').prepend(balise);
          }
          
      }
  },
  methods:{
       consoleOut(){
           if(this.output !==''){
               this.$socket.emit('console-out',this.output.replace(/\s/g, "")); 
               this.output = '';
           }
          
      }
  }
}
</script>

<style>
    .consoleMes{
       text-align: left; 
       border:solid;
       border-width:1px;
       border-color:lightgray;
       border-radius:5px;
       padding-left: 10px;
       padding-top:20px;
       padding-bottom:20px;
       color:white;
       background-color:rgb(36, 29, 29);
       font: 0.8em 'Andale Mono', Consolas, 'Courier New';
       font-weight: bold;
       font-size: 1.1em;
       min-height: 100px;
       overflow-y: scroll;
    }
    #console-input{
        max-height: 300px;
    }
</style>
