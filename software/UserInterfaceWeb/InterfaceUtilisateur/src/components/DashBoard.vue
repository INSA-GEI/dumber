<template>
    <v-card color="grey lighten-4" flat>
        <v-card-title primary class="title hidden-xs">
        <div class="center">
            <h3 class="display-1 mb-0">Robot Dashboard</h3>
            <div class="title">
                Demande d'ouverture du serial port <v-btn @click="manageSerial" :disabled="!tcpState" color="primary" flat slot="activator"><div v-if="!serialState">Ouverture Serial</div><div v-else>Fermeture Serial</div></v-btn>
            </div>            
            <div class="title">
                 Faire changer le robot d'état
                 <v-menu :disabled="!serialState" offset-y>
                 <v-btn :disabled="!serialState" color="primary" flat slot="activator">RUN ou IDLE</v-btn>
                 <v-list>
                    <v-list-tile @click="startWithWD">
                        <v-list-tile-title> Demarer avec watchdog </v-list-tile-title>
                    </v-list-tile>
                    <v-list-tile @click="startWithoutWD">
                        <v-list-tile-title> Demarer sans watchdog </v-list-tile-title>
                    </v-list-tile>
                    <v-list-tile @click="stopIdle">
                        <v-list-tile-title> Robot à l'état IDLE </v-list-tile-title>
                    </v-list-tile>
                </v-list>
                </v-menu>
            </div>
        </div>
        </v-card-title>
        <Controller v-if="dumbyStarted"/>
    </v-card>
</template>

<script>
import Voyant from './Voyant'
import Controller from './Controller'
export default {
    components:{
        Controller,      
    },
    data:()=>{
        return {dumbyStarted:false}
    },
    props:['tcpState', 'serialState'],
    methods:{
        manageSerial: function(state){
            if(this.tcpState){
                this.$socket.emit('openSerial',!this.serialState);
            }
            else{
                this.serialState = false;
            }

            if(state !== undefined){
                this.serialState = state;
            }
        },
        startWithWD: function(){
            this.$socket.emit('startWithWD');
        },
        startWithoutWD: function(){
            this.$socket.emit('startWitouthWD');
        },
        stopIdle: function(){
            this.$socket.emit('idle');
        }
    },
    sockets:{
        dumbyStart:function(state){
            if(this.serialState === true)
                this.dumbyStarted=state;
            else this.dumbyStarted=false;
        },
        lostSerial: function(){
            this.dumbyStarted = false;
            this.serialState = false;
        },

    },
    watch:{
       serialState: function(){
           if(this.serialState===false){
               this.dumbyStarted =false;
               this.manageSerial(false);
           }
       },
    }
}
</script>

<style>
    .center{
        text-align: center;

        width: 100%;
    }
</style>
