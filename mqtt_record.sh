mosquitto_sub -h "broker.hivemq.com" -p 1883 -t water/level -u broker_user -P broker_password -F "%I %t %p" >> ./mqtt_data.txt &
tail -f ./mqtt_data.txt

