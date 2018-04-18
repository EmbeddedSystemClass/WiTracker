from tkinter import *       # Import tkinter
import socket               # Import socket module
import simplekml            # Import simplekml
import paho.mqtt.client as mqtt
import time
import csv
import os
COORDS = []
client = mqtt.Client("T03")
new = True
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    if rc == 0:
        client.subscribe("/engg4810_2018/G03")
  
    
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    addData(str(message.payload.decode("utf-8")))
    
def on_log(client, userdata, level, buf):
    print("log: ",buf)

def on_subscribe(client, userdata, mid, granted_qos):
    print(granted_qos)
    
def on_publish(client, userdata, mid):
    print(mid)

    
def onExit():
    quit()

def connect():
    client.on_connect = on_connect
    client.on_message = on_message
    client.on_subscribe = on_subscribe
    client.on_publish = on_publish
    client.on_log=on_log

    client.username_pw_set("engg4810_2018", password = "blpc7n2DYExpBGY5BP7")
    client.connect("tp-mqtt.zones.eait.uq.edu.au", 1883, 60)
    client.loop_start()

def disconnect():
    print("Disconnect MQTT server")
    client.loop_stop()

def addCoord():
    listbox.delete(0,'end')
    global new
    new = True

def saveCoords():
    kml=simplekml.Kml()
    file2 = open('csvfile.csv','r')
    reader = csv.reader(file2)
    ls = kml.newlinestring(name="GPS path")
    header = []
    firstLine = True
    for row in reader:
        print(row)
        if firstLine == True:
            header = row
            firstLine = False
            continue
        if row[0] == "SKIPPED":
            continue
        point = kml.newpoint()
        point.coords = [(row[2],row[1])]
        string = ""
        for i in range(len(row)):
            if header[i] != "networks":
                string = string + header[i] + ": " + row[i] + "\n"
        point.description = string
        ls.coords.addcoordinates([(row[2],row[1])]) #<-- IMPORTANT! Longitude first, Latitude second.
    file2.close()
    kml.save('Path.kml');

        


Data = "$GP03,1,1,1,90,90.52,20180319T105701Z,23,50,1,0,5,abc;00:1b:2g:a8:e5:21;-71%abc;00:1b:2g:a8:e5:21;-71,*,77<CR><LF>"
def parseData(string):
    dataSplit = string.split(",")
    try:
        state = int(dataSplit[0])
        latitude = float(dataSplit[1])
        longtitude = float(dataSplit[2])
        temperature = int(dataSplit[4])
        humidity = int(dataSplit[5])
        outside = int(dataSplit[6])
        tone = int(dataSplit[7])
        voltage = float(dataSplit[8])
    except Exception as e:
        print("Incorrect data format, Exception: " + str(e))
        return string
    
    time = dataSplit[3]
    if state not in [0,1,2]:
        print("Incorrect format, state is 0, 1 or 2")
        return False
    if latitude > 90.0 or latitude < -90.0:
        print("Incorrect format, latitude is range -90 to 90")
        return False
    if longtitude > 180.0 or latitude < -180.0:
        print("Incorrect format, longtitude is range -180 to 180")
        return False
    if outside not in [0,1]:
        print("Incorrect format, outside is 0 or 1")
        return False
    if tone not in [0,1]:
        print("Incorrect format, tone is 0 or 1")
        return False
    if voltage < 0:
        print("Incorrect format, rssi is positive value")
        return False
    networksData = dataSplit[9]
    networksData = networksData.split("%")
    networks = []
    data = [state, latitude, longtitude, time, temperature,
            humidity, outside, tone, voltage]
    
    for i in networksData:
        network = i.split(";")
        ssid = network[0]
        try:
            rssi = float(network[2])
        except Exception as e:
            print("Incorrect data format, Exception: " + str(e))
            return False
        bssid = network[1]
        dummy = bssid.split(":")
        if len(dummy) != 6:
            print("Incorrect bssid format") 
            return False
        for i in dummy:
            if len(i) != 2:
                print("Incorrect bssid format") 
                return False
        networks.append((ssid,bssid,rssi))
    data.append(networks)
    return data
def parsePacket(string):
    if len(string) == 0:
        print("Empty packet Ignore")
        return False
    if string[0] != "$":
        print("Incorrect packet Ignored")
        return False
    stringSplit = string.split(",")
    group_ID = stringSplit[0][1:]
    try:
        packet_number = int(stringSplit[1])
        new_data = int(stringSplit[2])
        checksum = int(stringSplit[-1][0:-len("<CR><LF>")])
    except Exception as e:
        print("Incorrect packet format, Exception: " + str(e))
        return False
        
    dummy = ",".join(stringSplit[3:])
    dataString = ""
    for i in range(0, len(dummy)) :
        if dummy[i] == "*" :
            dataString = dummy[:i]
            break
    data = None
    if new_data == 1 :
        data = parseData(dataString)
    else :
        data = False
    packet = (group_ID,packet_number,new_data,data,checksum)
    return packet
def addData(message):
    file = openFile()
    writer = csv.writer(file, delimiter=',', lineterminator='\n')
    packet = parsePacket(message)
    if packet != False:
        group_ID,packet_number,new_data,data,checksum = packet
        if data == False:
            listbox.insert(END, "Incorrect or Old Data, Skipped")
            writer.writerow(["SKIPPED"])
        else:
            state, latitude, longtitude, time, temperature,humidity, outside, tone, voltage, networks = data
            listbox.insert(END, "State: " + str(state) + "; Latitude: " + str(latitude) + "; Longtitude: " +
                           str(longtitude) + "; Temp: " + str(temperature) + "; Humidity: " + str(humidity))
            writer.writerow((state, latitude, longtitude, time, temperature,humidity, outside, tone, voltage, networks))
    file.close()
def openFile():
    global new
    file = None
    if new == True:
        file = open('csvfile.csv','w')
        writer = csv.writer(file, delimiter=',', lineterminator='\n')
        writer.writerow(("State", "Latitude", "Longtitude", "Time", "Temperature", "Humidity", "Outside", "Tone", "Voltage", "Networks"))
        new = False
    else:
        file = open('csvfile.csv','a')
    return file
        
        
root = Tk()
root.title("GUI")
root.geometry("550x300")

menuBar = Menu(root)
root.config(menu = menuBar)
fileMenu = Menu(menuBar)
fileMenu.add_command(label="Connect MQTT", command = connect)
fileMenu.add_command(label="Disconnect MQTT", command = disconnect)
fileMenu.add_command(label="Exit", command = onExit)
menuBar.add_cascade(label="File", menu = fileMenu)

frame = Frame(root)
frame.pack()
listbox = Listbox(frame,width = 59, height=14, font=("Helvetica", 12))
listbox.pack(side = "left", fill="y")

scrollbar = Scrollbar(frame, orient="vertical")
scrollbar.config(command=listbox.yview)
scrollbar.pack(side="right", fill="y")
listbox.config(yscrollcommand=scrollbar.set)

frame2 = Frame(root)
addButton = Button(frame2, text = "New", command=addCoord, width= 10,borderwidth=2)
saveButton = Button(frame2, text = "Save", command=saveCoords, width =10, borderwidth=2)
frame2.pack(side = BOTTOM)
addButton.grid(column=0, row=1)
saveButton.grid(column=1, row=1)


root.mainloop()
    
    
