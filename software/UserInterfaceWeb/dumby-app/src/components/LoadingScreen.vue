<template>
<div class="loading">
    <v-progress-circular indeterminate v-bind:size="70" v-bind:width="7" color="purple"></v-progress-circular>
    <h1>Non Connecté ! Veuillez vérifier que le serveur node a bien été lancé. Puis rafraichissez la page !</h1>
</div>  
</template>

<style>
.loading{
    padding-top: 10%;
    position:fixed;
    z-index: 10;
    width:100%;
    height: 100%;
    background-color: rgb(46, 37, 37);
    color: rgba(255, 255, 255, 0.788);
}   
</style>