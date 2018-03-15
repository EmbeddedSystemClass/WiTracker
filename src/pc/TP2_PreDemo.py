from tkinter import *       # Import tkinter
import socket               # Import socket module
import simplekml            # Import simplekml
import paho.mqtt.client as mqtt
import time
kml=simplekml.Kml()
COORDS = []
client = mqtt.Client("T03")

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    if rc == 0:
        client.subscribe("/engg4810_2018/G03")

        
    
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    print("message topic=",message.topic)
    print("message qos=",message.qos)
    print("message retain flag=",message.retain)
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
    name = nameEntry.get()
    try:
        long = float(longEntry.get())
        lat = float(latEntry.get())
    except ValueError:
        print("Incorrect format, longtitude or latitude must be float")
        return 0
    if name == '' or long == '' or lat == '' :
        print("Incorrect format, entry field/s empty")
        return 0
    kml.newpoint(name=name, coords=[(long,lat)])

def saveCoords():
    kml.save('paths.kml')
        
def sendMes():
    buffer = message.get("1.0","end-1c")
    client.publish("/engg4810_2018/G03",buffer)

    
root = Tk()
root.title("GUI")
root.geometry("200x150")

menuBar = Menu(root)
root.config(menu = menuBar)
fileMenu = Menu(menuBar)
fileMenu.add_command(label="Connect MQTT", command = connect)
fileMenu.add_command(label="Disconnect MQTT", command = disconnect)

fileMenu.add_command(label="Exit", command = onExit)

menuBar.add_cascade(label="File", menu=fileMenu)

##canvas = Canvas(root, width = 200, height = 200, borderwidth=1, relief="ridge", bg="white")
##canvas.pack(side=LEFT, fill=Y)
frame = Frame(root)
frame.pack(side = TOP)
Label(frame, text="Name: ").grid(row = 0)
Label(frame, text="Longitude: ").grid(row = 1)
Label(frame, text="Latitude: ").grid(row = 2)
nameEntry = Entry(frame)
longEntry = Entry(frame)
latEntry = Entry(frame)
message = Text(root)
sendButton = Button(root, text="Send to server", command = sendMes)
nameEntry.grid(row = 0, column = 1)
longEntry.grid(row = 1, column = 1)
latEntry.grid(row = 2, column = 1)
addButton = Button(frame, text = "Add", command=addCoord)
saveButton = Button(frame, text = "Save", command=saveCoords)
addButton.grid(row = 3)
saveButton.grid(row = 3, column = 1)
sendButton.pack(side=BOTTOM)
message.pack(side=BOTTOM)

root.mainloop()
    
    
