# Description
Iot project with esp32 nodes that gather busyness data which is pushed to a MQTT bus. A cloud will gather this data in a time database and futher inference is done by python clients running ML. A UI is also provided for insight.

# Hierarchy
-  Cloud / Backend
    - Machine Learning (Python client)
    - GUI (Serve a webpage)
-  Gateway
-  Sensor Node (ESP)

## Cloud / Backend
(Implemented on the raspberry pi.)
This will host a database, which we can access through an API. This is a webserver.
A webpage is served to the end user, here they can select a room and get insight into busyness overtime.

## Machine Learning
(offloaded to a laptop, is no different to running it on the cloud or on the raspi)
A python client will fetch data point from the database by calling the API. The model will run a classification model to infer the busyness.
For training labeled data is needed. This will be added when sensor data is being gathered.

## Gateway
This will be the MQTT broker that hosts the bus.

## ESP nodes
These gather data, and prepare it for sending to the MQTT bus. They store their room location internally. And send data on a fixed interval, which is configurable as well.

# Data structure
See dummy_data.json for this.

# Guide
open this subfolder as your main folder in vscode. Download the platformio extension. Do not download the arduino_pio extension.

In the bottom left of vscode window there is the verify and the compile buttons.

platform.ini defines the board and dependencies. Please update properly.

# Sensors
BME680 co2 + hum + temp sensor. -> I2C
https://www.espboards.dev/sensors/bme680/

