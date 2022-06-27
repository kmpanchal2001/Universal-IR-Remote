import mysql.connector

mydb = mysql.connector.connect(host="localhost", user="root", passwd="krishna", database="remotevalue1")

mycursor = mydb.cursor()

mycursor.execute("select * from hexdata1")

result = mycursor.fetchall()
while(1):
    mqttQuery = input("key ")

    for i in result:
        if i[0] == mqttQuery:
            print("data is found")
            print(i[1])