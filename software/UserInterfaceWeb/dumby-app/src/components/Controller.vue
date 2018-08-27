<template>
    <div class="command">
         <img id="up" src="/static/arrow-up.svg" class="direction" v-bind:class="{touched: keyup}">
        <div>
            <img id="left" src="/static/arrow-left.svg" class="direction" v-bind:class="{touched: keyleft}">
            <img id="down" src="/static/arrow-down.svg" class="direction" v-bind:class="{touched: keydown}">
            <img id="right" src="/static/arrow-right.svg" class="direction" v-bind:class="{touched: keyright}">
        </div>
    </div>
</template>


<script>
export default {
    data:()=>{
        return {
            keyup: false, //38
            keydown: false, // 40
            keyleft:false, // 37
            keyright:false // 39
        }
    },
    mounted:function(){
        window.addEventListener('keydown', (event)=> {
            if (event.keyCode == 37) {
                this.keyleft= true;
            }
            if (event.keyCode == 38) {
                this.keyup=true;
            }
            if (event.keyCode == 39) {
                this.keyright=true;
            }
            if (event.keyCode == 40) {
                this.keydown=true;
            }
        });
        window.addEventListener('keyup', (event)=> {
            if (event.keyCode == 37 ) {
                this.keyleft=false;
            }
            if (event.keyCode == 38 ) {
                this.keyup=false;
            }
            if (event.keyCode == 39 ) {
                this.keyright=false;
            }
            if (event.keyCode == 40 ) {
                this.keydown=false;
            }
        });
    },
    watch:{
        keyup:function(){
            if(this.keyup===true){
                this.$socket.emit('MOVEFORWARD'); 
            }
            else{
                this.$socket.emit('MOVESTOP');
            }
        },
        keyleft:function(){
            if(this.keyleft===true){
                this.$socket.emit('MOVERIGHT'); 
            }
            else{
                this.$socket.emit('MOVESTOP');
            }
        },
        keydown:function(){
            if(this.keydown===true){
                this.$socket.emit('MOVEBACK');
            }
            else{
                this.$socket.emit('MOVESTOP');
            }
        },
        keyright:function(){
            if(this.keyright===true){
                this.$socket.emit('MOVELEFT');
            }
            else{
                this.$socket.emit('MOVESTOP');
            }
        },
    }
}
</script>

<style>
    .direction{
        border:solid;
        border-color:lightgray;
        border-width: 1px;
        border-radius: 50%;        
    }
    .touched{
        background-color:lightblue;
    }
    .command{
        text-align: center;
    }
</style>
