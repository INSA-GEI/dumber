<template>
    <v-footer class="pa-3" color="pink darken-3">
        <div style="color:white;">L.Senaneuch</div>
        <v-spacer></v-spacer>
        <img class="logo-vue" src="static/logo.png" alt="logo-vue">
        <img class="logo-vue" src="static/logo-node.png" alt="logo-node">
        <v-spacer></v-spacer>
        <div style="color:white;">© INSA - Toulouse {{ new Date().getFullYear() }}</div>
    </v-footer>
</template>

<style>
    .logo-vue{
        width: 30px;
        margin-left:15px;
    }
</style>
