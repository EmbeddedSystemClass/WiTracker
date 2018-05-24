from tkinter import *           # Import tkinter
import simplekml                # Import simplekml
import paho.mqtt.client as mqtt # Import mqtt
import csv                      # Import csv

##import os                       # Import os
##import time as t                # Import time
##import socket                   # Import socket module
class GUI():
    def __init__(self, root):
        self.root = root
        self.root.title("GUI")
        self.root.geometry("600x300")
        self.menuBar = Menu(self.root)
        self.root.config(menu = self.menuBar)
        self.fileMenu = Menu(self.menuBar)
        self.frame = Frame(self.root)
        self.frame2 = Frame(self.root)
        self.frame.pack()
        self.frame2.pack(side = BOTTOM)
        self.fileMenu.add_command(label = "Connect MQTT", command = self.connectMQTT)
        self.fileMenu.add_command(label = "Disconnect MQTT", command = self.disconnectMQTT)
        self.fileMenu.add_command(label = "Import CSV", command = self.importFile)
        self.fileMenu.add_command(label = "Exit", command = self.onExit)
        self.menuBar.add_cascade(label = "File", menu = self.fileMenu)

        self.listbox = Listbox(self.frame, width = 65, height=14, font=("Helvetica", 12))
        self.listbox.pack(side = "left", fill = "y")
        self.scrollbar = Scrollbar(self.frame, orient="vertical")
        self.scrollbar.config(command = self.listbox.yview)
        self.scrollbar.pack(side = "right", fill="y")
        self.listbox.config(yscrollcommand = self.scrollbar.set)

        self.startButton = Button(self.frame2, text = "Start", command = self.startCollectData, width= 10,borderwidth=2)
        self.pauseButton = Button(self.frame2, text = "Pause", command = self.pauseCollectData, width= 10,borderwidth=2)
        self.saveButton = Button(self.frame2, text = "Save", command = self.genKML, width =10, borderwidth=2)
        self.startButton.grid(column=0, row=1)
        self.pauseButton.grid(column=1, row=1)
        self.saveButton.grid(column=2, row=1)
        
        self.dataList = []
        self.isCollect = True
        self.isConnect = False
        self.MQTT = MQTT()
        self.MQTT.client.on_message = self.on_message
        self.data = None
        
        self.root.mainloop()
        
    def onExit(self):
        quit()
        
    def connectMQTT(self):
        self.isConnect = True
        self.isCollect = True
        self.MQTT.connect()
        
    def disconnectMQTT(self):
        self.isConnect = False
        self.isCollect = False
        self.MQTT.disconnect()
        
    def on_message(self, client, userdata, message):
        if self.isCollect:
            print("message received " ,str(message.payload.decode("utf-8")))
            self.addData(str(message.payload.decode("utf-8")))

    def addData(self, packet):
        parsePacket = Parser(packet)
        
        if parsePacket.parsePacket()!= True:
           return False
        self.data = parsePacket.data
        self.dataList.append(self.data)
        if self.data == False:
            self.listbox.insert(END, "Incorrect, Skipped")
        elif self.data == "No New Data":
            self.listbox.insert(END, "Incorrect, Skipped")
        else:
            try:
                state, latitude, longtitude, time, temperature,humidity, outside, tone, voltage, networks = self.data
                string = "State: " + str(state) + "; Latitude: " + str(latitude) + "; Longtitude: " \
                        + str(longtitude) + "; Temp: " + str(temperature) + "; Humidity: " + str(humidity)
                self.listbox.insert(END, string)
            except Exception as e:
                print(e)
        return True
    
    def importFile(self):
        self.listbox.delete(0,'end')
        self.dataList = []
        try:
            filename = filedialog.askopenfilename(initialdir = "/",title = "Select file",
                                                  filetypes = (("csv file","*.csv"),("all files","*.*")))
            csvfile = open(filename, 'r')
        except Exception as e:
            print("No File Input")
        reader = csv.reader(csvfile)
        try:
            for i in reader:
                self.addData(i[2])
        except Exception as e :
            print("Wrong file format, " + str(e))
            
    def addToListBox(self, message):
        self.listbox.insert(END, message)
        
    def startCollectData(self):
        self.listbox.delete(0,'end')
        self.dataList = []
        self.isCollect = True

    def pauseCollectData(self):
        print(self.isCollect)
        if self.isConnect:
            if self.isCollect == True:
                self.isCollect = False
                self.pauseButton.config(text = "Resume")
            else:
                self.isCollect = True
                self.pauseButton.config(text = "Pause")
        else:
            """do nothing"""
        
    def genKML(self):
        if self.dataList == []:
            print("No data added")
            return False
        kml=simplekml.Kml()
        ls = kml.newlinestring(name="GPS path")
        ls.altitudemode = "absolute"
        altitude = 100
        for i in self.dataList:
            state, latitude, longtitude, time, temperature,humidity, outside, tone, voltage, networks = i
            point = kml.newpoint()
            point.coords = [(longtitude,latitude, altitude)]
            point.altitudemode = "absolute"
            
            string = "State: " + str(state) + ";\n Latitude: " + str(latitude) + ";\n Longtitude: " \
                    + str(longtitude) + ";\n Temp: " + str(temperature) + ";\n Humidity: " + str(humidity) \
                    + ";\n Tone: " + str(tone) + ";\n Outside: " + 'Yes;' if outside == 1 else 'No;' 
            point.description = string
            ls.coords.addcoordinates([(longtitude, latitude,altitude)])
            altitude = 100 + altitude
        kml.save("Path.kml")
        print("Success generate KML file")
        self.genCSV()

    def genCSV(self):
        if self.dataList == []:
            print("No data");
            return False
        with open("Points.csv", 'w') as myfile:
            writer = csv.writer(myfile, delimiter=',', lineterminator='\n')
            writer.writerow(("State", "Latitude", "Longtitude", "Time", "Temperature", "Humidity", "Outside", "Tone", "Voltage", "Networks"))
            for i in self.dataList:
                if (i == False) | ( i == "No New Data") :
                   writer.writerow(["SKIPPED"])
                else:
                    state, latitude, longtitude, time, temperature,humidity, outside, tone, voltage, networks = i
                    dummy = ""
                    for network in networks:
                        
                        dummy = dummy + "SSID: " + str(network[0]) + ", BSSID: " + str(network[1]) +  ", RSSID: " + str(network[2]) + " and "
                    
                    dummy = dummy[:-3]
                    writer.writerow((state, latitude, longtitude, time, temperature,humidity, outside, tone, voltage, dummy))
        print("Success generate CSV file")
        
class MQTT():
    def __init__(self):
        self.client = mqtt.Client("T03")
        self.client.on_connect = self.on_connect
        self.client.on_subscribe = self.on_subscribe
        self.client.on_publish = self.on_publish
        self.client.on_log = self.on_log
        self.client.username_pw_set("engg4810_2018", password = "blpc7n2DYExpBGY5BP7")

    def connect(self):
        self.client.connect("tp-mqtt.zones.eait.uq.edu.au", 1883, 60)
        print("Connect MQTT server")
        self.client.loop_start()
        
    def disconnect(self):
        print("Disconnect MQTT server")
        self.client.loop_stop()
        self.client.disconnect()

    def on_connect(self, client, userdata, flags, rc):
        print("Connected with result code "+str(rc))
        if rc == 0:
            client.subscribe("/engg4810_2018/G03")

    def on_log(self, client, userdata, level, buf):
        print("log: ",buf)

    def on_subscribe(self, client, userdata, mid, granted_qos):
        print(granted_qos)
        
    def on_publish(self, client, userdata, mid):
        print(mid)

class Parser():
    def __init__(self, packet):
        self.packet = packet
        self.data = None

    def parsePacket(self):
        if len(self.packet) == 0:
            print("Empty packet Ignore")
            self.data = "IGNORED"
            return False
        if self.packet[0] != "$":
            print("Incorrect packet Ignored")
            self.data = "IGNORED"
            return False
        stringSplit = self.packet.split(",")
        group_ID = stringSplit[0][1:]
        try:
            packet_number = int(stringSplit[1])
            new_data = int(stringSplit[2])
            checksum = int(stringSplit[-1][0:-len("<CR><LF>")])
        except Exception as e:
            print("Incorrect packet format, Exception: " + str(e))
            self.data = "IGNORED"
            return False
            
        dummy = ",".join(stringSplit[3:])
        dataString = ""
        for i in range(0, len(dummy)) :
            if dummy[i] == "*" :
                dataString = dummy[:i]
                break
        if new_data == 1 :
            self.data = self.parseData(dataString)
        else :
            self.data = "No New Data"
        return True

    def parseData(self, string):
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
            state = "SKIPPED"
            return None
        if latitude > 90.0 or latitude < -90.0:
            print("Incorrect format, latitude is range -90 to 90")
            return None
        if longtitude > 180.0 or latitude < -180.0:
            print("Incorrect format, longtitude is range -180 to 180")
            return None
        if outside not in [0,1]:
            print("Incorrect format, outside is 0 or 1")
            return None
        if tone not in [0,1]:
            print("Incorrect format, tone is 0 or 1")
            return None
        if voltage < 0:
            print("Incorrect format, rssi is positive value")
            return None
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
                return None
            bssid = network[1]
            dummy = bssid.split(":")
            if len(dummy) != 6:
                print("Incorrect bssid format") 
                return None
            for i in dummy:
                if len(i) != 2:
                    print("Incorrect bssid format") 
                    return None
            networks.append((ssid,bssid,rssi))
        data.append(networks)
        return data
def main():
    root = Tk()
    GUI(root)
    
if __name__== "__main__":
    main()

