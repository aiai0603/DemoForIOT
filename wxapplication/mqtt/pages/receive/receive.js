//index.js
//获取应用实例
const app = getApp()
var mqtt = require('../../utils/mqtt.js');

Page({
  data: {
    humidity:0,
    temperature:0,
    date:null
  },
  onLoad: function() {
    this.initSocket();
  },
  //接收信息函数
  initSocket: function() {
   
    var that=this;
    app.globalData.client.subscribe('CJP/RE');
    app.globalData.client.on('message', function(topic, payload) {
      var receive=JSON.parse(payload);
      var time=new Date();
      //赋值
       that.setData({
          temperature:receive.temperature,
          humidity:receive.humidity,
          date:time.getHours()+":"+time.getMinutes()+":"+time.getSeconds()
       })
    })
    }


  
  
})

