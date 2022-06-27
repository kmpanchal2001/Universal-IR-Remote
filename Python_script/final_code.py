import mysql.connector #import library mysql.connector
import paho.mqtt.client as mqttclient #import library paho.mqtt.client as mqttclient
import time

mydb = mysql.connector.connect(host="localhost", user="root", passwd="krishna", database="remotevalue1")# provide host name, user name, password and name of database and connect to it

mycursor = mydb.cursor()
# database is created in mysql workbench sofware. Name of database is remotevalvue1 and table of this database is named as hexdata1


def on_connect(client, usedata, flags, rc): #function defined to connect to provided database 
    if rc==0: # check the value of rc variable if it is 0 it means client is connected so enter to the loop
        print("client is connected")# print the message "client is connected"
        global connected
        connected = True # set connected variable to true
    else:
        print("connection failed") # if failed to connect to the data base then print mesage that connection failed

connected = False
broker_address = "broker.mqtt-dashboard.com" # domain name of mqtt broker is provided to which we have to connect

port = 1883 # 1833 is the port number on which our mqtt broker is running so we have to provide it
user = "krishna" # user name of mqtt broker is provided
password = "asutosh" #password is provide

client = mqttclient.Client("MQTT") # initiate mqtt as client
client.username_pw_set(user, password=password) #use provided user name and password to connect with client
client.on_connect = on_connect
client.connect(broker_address, port=port) # connect to client through provide details
client.loop_start()
msg=0
while connected != True: #if client is not connected then wait for 0.2 second
    time.sleep(0.2)
mycursor.execute("select * from hexdata1")# fetch the data from "hexdata1" table and execute it
result = mycursor.fetchall()# fetch all the content from the database

while(1):
    found = True #variable named as "found" is set as True
    mqttQuery = input("enter button name ") #ask user to enter the name of remote button which they have to press
    mqttQuery = mqttQuery.upper()
    for i in result:
        if i[0] == mqttQuery: #if entered button name of remote is avalable the enter to the loop
            print("data is found")# print the message that the enterd data is found 
            msg=i[1]#store the hex value fetched from database to the variable named as "msg"
            print(msg)  #print the hex value fetched from database 
            found = False #set "found" variable as false
            client.publish("mqtt/ka", msg) #publish the hex data to the connected client
            client.loop_stop()
            
    if found == True: # if the entered button value is not velid or not present in the database then print the message "data is not found"
        print("data is not found") # print the message "data is not found"
   