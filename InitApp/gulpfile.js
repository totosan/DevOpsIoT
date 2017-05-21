/*
 * IoT Hub ESP8266 - Copiied and modified from Microsoft Sample Code - Copyright (c) 2016 - Licensed MIT
 */
'use strict';

var gulp = require('gulp');
var runSequence = require('run-sequence');
var path=require('path');
var folder = path.join(process.env[(process.platform == 'win32') ? 'USERPROFILE' : 'HOME'], '.iot-hub-getting-started');
console.log(folder);


/**
 * Setup common gulp tasks: init, install-tools, deploy, run
 */
require('gulp-common')(require('gulp'), 'arduino-esp8266-nodemcuv2', {
  appName: 'devopsiot',
  //board: {parameters: 'CpuFrequency=80,UploadSpeed=115200,FlashSize=4M3M' },
  configTemplate:
  {
    iot_hub_connection_string: '[IoT hub connection string]',
    iot_device_connection_string: '[IoT device connection string]',
    wifi_ssid: '[Wi-Fi SSID]',
    wifi_password: '[Wi-Fi password]',
    iot_hub_consumer_group_name: 'cg1',
    azure_storage_connection_string: '[Azure storage connection string]'
  },
  configPostfix: 'arduino',
  app: ['app.ino', 'config.h']
});

var config = gulp.config;

/**
 * Gulp task to send cloud-to-device messages from host machine
 */
var test1 = require('./gulp_test.js').Tests;
gulp.task('test-flash-command', false, function () { test1.UpdateFwTest(config); });

/**
 * Override 'run' task with customized behavior
 */
gulp.task('run', 'Runs deployed SW on the board', function (cb) {
  runSequence('deploy', cb);
  /*  runSequence('deploy', 'test-flash-command', cb);*/
});
