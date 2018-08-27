<template>
<div>
    <v-alert icon="info" v-for="(notify, index) in list" :id="index" :key="index" 
    :color="notify.type" value="true" transition="slide-y-transition" @click="deleteNotes">
      {{notify.msg}} 
    </v-alert>
</div>
</template>

<script>
export default {
  data:()=>{
      return {
          list:[],
      }
  },
  sockets:{
      notifications(data){
          this.list.push(data);
      }
  },
  methods:{
      deleteNotes(e){
         const noteSelected = e.target.parentElement.id;
         this.list = this.list.filter((info, index) => noteSelected != index);   
      }
  }
}
</script>

<style>

</style>
