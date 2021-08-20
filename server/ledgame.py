#!/usr/bin/python
# -*-coding:utf-8 -*-
import json
import os
import time
import threading
import eventlet
from flask_mqtt import Mqtt
from flask_socketio import SocketIO
from flask_cors import CORS

eventlet.monkey_patch()
os.environ["LANG"] = "en_US.UTF-8"
import requests
from flask import Flask, request, render_template

app = Flask(__name__, template_folder='templates')
app.config['MQTT_BROKER_URL'] = 't.xxx.com'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_KEEPALIVE'] = 60
app.config['MQTT_USERNAME'] = 'xxx'
app.config['MQTT_PASSWORD'] = 'xxx'
CORS(app)
mqtt = Mqtt(app)
socketio = SocketIO(app)
tempdata = None

@socketio.on('publish')
def handle_publish(json_str):
    data = json.loads(json_str)
    # mqtt_ws.publish(data['topic'], data['message'])


@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    mqtt.subscribe('/home/s/ledgame/#')


@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    global tempdata
    try:
        tempdata = message.payload.decode()
        print(tempdata)
    except Exception as e:
        print(e)


@app.route('/', methods=['GET'])
def index():
    # return render_template('weather.html')
    # key = request.args.get('uuid')
    return render_template('index.html')


@app.route('/ajax', methods=['GET'])
def ajax():
    runtime = request.args.get('runtime')
    print(runtime)
    mqtt.publish('/home/r/ledgame/ESP8266Client-13843913',
                 json.dumps({"OptType":"gameStart","Command":"","ExtData":"1"}))
    return json.dumps({'code': 0})
@app.route('/message', methods=['GET'])
def message():
    global tempdata
    msg = tempdata
    tempdata = None
    return json.dumps({'code': 0,'message':msg})

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=89, use_reloader=True, debug=True)
