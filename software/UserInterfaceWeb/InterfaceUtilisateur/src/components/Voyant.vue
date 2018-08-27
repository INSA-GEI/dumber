<template>
    <div class="led" :class="{light:ledOn}"></div>
</template>

<script>
export default {
  props:['ledOn'],
  data: ()=>{
      return{
          light:false,
      }
  }
}
</script>


<style>
.led{
    border-radius: 50%;
    background-color: red;
    border:solid;
    border-width: 2px;
}

.light{
    background-color: rgb(0, 255, 0);
}
</style>
