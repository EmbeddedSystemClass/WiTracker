from tkinter import *           # Import tkinter
from tkinter import filedialog
import simplekml                # Import simplekml
import paho.mqtt.client as mqtt  # Import mqtt
import csv                      # Import csv
import base64                   # Import base64
import datetime                 # Import datetime


class GUI():
    """
    A GUI and behaviours class
    Allow user to input CSV (encode or not). Comunicate with MQTT,
    and retrieve data as input data. Output kml and csv base on input data
    """

    def __init__(self, root):
        # Set up GUI
        self.root = root
        self.root.title("GUI")
        self.root.geometry("600x300")
        self.menuBar = Menu(self.root)
        self.root.config(menu=self.menuBar)
        self.fileMenu = Menu(self.menuBar)
        self.frame = Frame(self.root)
        self.frame2 = Frame(self.root)
        self.frame.pack()
        self.frame2.pack(side=BOTTOM)
        self.fileMenu.add_command(
            label="Connect MQTT", command=self.connectMQTT)
        self.fileMenu.add_command(
            label="Disconnect MQTT", command=self.disconnectMQTT)
        self.fileMenu.add_command(label="Import CSV", command=self.importFile)
        self.fileMenu.add_command(
            label="Import encoded CSV", command=lambda: self.importFile(True))
        self.fileMenu.add_command(label="Exit", command=self.onExit)
        self.menuBar.add_cascade(label="File", menu=self.fileMenu)

        self.listbox = Listbox(self.frame, width=65,
                               height=14, font=("Helvetica", 12))
        self.listbox.pack(side="left", fill="y")
        self.scrollbar = Scrollbar(self.frame, orient="vertical")
        self.scrollbar.config(command=self.listbox.yview)
        self.scrollbar.pack(side="right", fill="y")
        self.listbox.config(yscrollcommand=self.scrollbar.set)

        self.startButton = Button(
            self.frame2, text="Start", command=self.startCollectData, width=10, borderwidth=2)
        self.pauseButton = Button(
            self.frame2, text="Pause", command=self.pauseCollectData, width=10, borderwidth=2)
        self.saveButton = Button(
            self.frame2, text="Save", command=self.genKML, width=10, borderwidth=2)
        self.startButton.grid(column=0, row=1)
        self.pauseButton.grid(column=1, row=1)
        self.saveButton.grid(column=2, row=1)
        # Set up variables
        self.data = None
        self.dataList = []
        self.skippedPacket = []
        self.isCollect = True
        self.isConnect = False
        self.MQTT = MQTT()
        self.MQTT.client.on_message = self.on_message

        self.root.mainloop()

    def onExit(self):
        """
        Exit program
        """
        quit()

    def connectMQTT(self):
        """
        Connect to MQTT server using MQTT class
        """
        self.isConnect = True
        self.isCollect = True
        self.MQTT.connect()

    def disconnectMQTT(self):
        """
        Disconnect from MQTT server
        """
        self.isConnect = False
        self.isCollect = False
        self.MQTT.disconnect()

    def on_message(self, client, userdata, message):
        """
        Recieve message when, new message is publish
        """
        if self.isCollect:
            print("message received ", str(message.payload.decode("utf-8")))
            self.addData(str(message.payload.decode("utf-8")))

    def addData(self, packet):
        """
        Receive packet and parse it through Parser class
        then add the data from the packet to data list
        addData(string) -> void;
        """
        parsePacket = Parser(packet)

        if parsePacket.parsePacket() == False:
            return False
        self.data = parsePacket.data
        if self.data[0] == "WRONG FORMAT":
            print("Data recieved is in wrong format, ignored package")
            return False
        self.dataList.append(self.data)
        self.dataList.sort(key=lambda x: x[1])

        if self.data[0] == "OK":
            self.insertToListBox()
        elif self.data[0] == "SKIP":
            self.insertToListBox()

    def insertToListBox(self):
        """
        Add all the data from data list to list box
        """
        self.listbox.delete(0, 'end')
        for data in self.dataList:
            if data[0] == "SKIP":
                self.listbox.insert(END, "|SKIPPED| Number: " + str(data[1]))
            else:
                try:
                    status, packet_number, state, latitude, longtitude, time, temperature, humidity, outside, tone, voltage, network, skip = data
                    string = "|  ADDED | " + "Number: " + str(packet_number) + " State: " + str(state) + "; Latitude: " + str(latitude) + "; Longtitude: " \
                        + str(longtitude) + "; Temp: " + \
                        str(temperature) + "; Humidity: " + str(humidity)
                    self.listbox.insert(END, string)
                except Exception as e:
                    print(e)

    def importFile(self, encode=False):
        """
        Import CSV file as input data
        """
        self.listbox.delete(0, 'end')
        self.dataList = []
        try:
            filename = filedialog.askopenfilename(initialdir="/", title="Select file",
                                                  filetypes=(("csv file", "*.csv"), ("all files", "*.*")))

            csvfile = open(filename, 'r')
        except Exception as e:
            print(e)
            print("No File Input")
            return False

        reader = csv.reader(csvfile)
        for i in reader:
            print(i[2])
            if encode == False:
                data = i[2]
            else:
                data = base64.b64decode(i[2]).decode('utf-8')
            data = data.strip('\n')
            data = data.strip('\t')
            data = data.strip('\r')
            self.addData(data)

    def startCollectData(self):
        """
        Start collect data from MQTT
        """
        self.listbox.delete(0, 'end')
        self.dataList = []
        self.isCollect = True

    def pauseCollectData(self):
        """
        Stop collect data from MQTT
        """
        if self.isConnect:
            if self.isCollect == True:
                self.isCollect = False
                self.pauseButton.config(text="Resume")
            else:
                self.isCollect = True
                self.pauseButton.config(text="Pause")
        else:
            """do nothing"""

    def genKML(self):
        """
        Generate KML file from all the collected data (or input from CSV)
        """
        if self.dataList == []:
            print("No data added")
            return False
        # Setup kml
        kml = simplekml.Kml(name="ENGG4810", open=1)
        now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        doc = kml.newdocument(
            name='ENGG4810', snippet=simplekml.Snippet("Created on : " + now))
        doc.lookat.latitude = self.dataList[0][3]
        doc.lookat.longitude = self.dataList[0][4]
        doc.lookat.range = 1300.000000

        # Create folders
        pathFolder = doc.newfolder(name='Paths')
        pointFolder = doc.newfolder(name='Points')

        # Create linestrings
        pathLine = pathFolder.newlinestring(name="Path")
        pathLine.altitudemode = "absolute"
        humidLine = pathFolder.newlinestring(name="Humidity")
        humidLine.altitudemode = "absolute"
        humidLine.style.linestyle.color = simplekml.Color.blue
        humidLine.extrude = 1
        tempLine = pathFolder.newlinestring(name="Temperature")
        tempLine.altitudemode = "absolute"
        tempLine.style.linestyle.color = simplekml.Color.red
        tempLine.extrude = 1
        uvLine = pathFolder.newlinestring(name="Outside/Inside")
        uvLine.altitudemode = "absolute"
        uvLine.style.linestyle.color = simplekml.Color.yellow
        uvLine.extrude = 1
        skippedLine = pathFolder.newlinestring(name="Skipped measurements")
        skippedLine.altitudemode = "absolute"
        skippedLine.style.linestyle.color = simplekml.Color.purple
        skippedLine.extrude = 1
        voltageLine = pathFolder.newlinestring(name="Voltage")
        voltageLine.altitudemode = "absolute"
        voltageLine.style.linestyle.color = simplekml.Color.orange
        voltageLine.extrude = 1

        # Add all SSID to dictionary and calculate skipped meassurements at 1
        # location
        SSID_dict = {}
        for data in self.dataList:
            if data[0] == "SKIP":
                index = self.dataList.index(data)
                found = False
                while index > 0:
                    index -= 1
                    if self.dataList[index][0] == "OK":
                        found = True
                        break
                if found == True:
                    self.dataList[index][-1] += 1
                else:
                    print("No packets before skipped packet")
            else:
                SSID_dict[data[11][0]] = []

        # Add data to kml
        for data in self.dataList:
            if data[0] == "SKIP":
                continue
            status, packet_number, state, latitude, longtitude, time, temperature, humidity, outside, tone, voltage, network, skip = data
            # Create points / placemarks
            point = pointFolder.newpoint()
            point.coords = [(longtitude, latitude)]
            # Create string for description
            tone_string = ""
            if tone == 0:
                tone_string = "No tone detected"
            elif tone == 1:
                tone_string = "1kHz detected"
            elif tone == 2:
                tone_string = "4kHz detected"
            elif tone == 3:
                tone_string = "1kHz and 4kHz detected"
            string = "State: " + str(state) + ";\n Latitude: " + str(latitude) + ";\n Longtitude: " \
                + str(longtitude) + ";\n Temp: " + str(temperature) + ";\n Humidity: " + str(humidity) \
                + ";\n Tone: " + tone_string + ";\n Outside: "
            if outside == 1:
                string = string + "Yes;\n"
            else:
                string = string + "No;\n"
            string = string + "Voltage: " + str(voltage) + "V;\n"
            string = string + "No. of skipped measurements: " + \
                str(skip) + ";\nReceived By: " + \
                time.strftime("%Y-%m-%d %H:%M:%S")
            string = string + ";\n Received from SSID: " + \
                str(network[0]) + " RSSI: " + str(network[2])
            point.description = string
            # Add coords to paths
            try:
                pathLine.coords.addcoordinates([(longtitude, latitude)])
            except Exception as e:
                print(e)

            try:
                tempLine.coords.addcoordinates(
                    [(longtitude, latitude, abs(temperature))])
            except Exception as e:
                print(e)

            try:
                humidLine.coords.addcoordinates(
                    [(longtitude, latitude, humidity)])
            except Exception as e:
                print(e)

            try:
                voltageLine.coords.addcoordinates(
                    [(longtitude, latitude, abs(voltage))])
            except Exception as e:
                print(e)

            try:
                uvLine.coords.addcoordinates(
                    [(longtitude, latitude, 25 if outside == 1 else 0)])
            except Exception as e:
                print(e)

            try:
                skippedLine.coords.addcoordinates(
                    [(longtitude, latitude, skip * 10)])
            except Exception as e:
                print(e)

            # Add data to SSID dictionary
            SSID_dict[network[0]].append([latitude, longtitude, network])

        # Create paths for each unique SSID
        for SSID, dataList in SSID_dict.items():
            RSSILine = pathFolder.newlinestring(name="SSID: " + str(SSID))
            RSSILine.altitudemode = "absolute"
            RSSILine.extrude = 1
            altitude = 0
            for data in dataList:
                latitude, longtitude, network = data
                RSSILine.coords.addcoordinates(
                    [(longtitude, latitude, abs(network[2]))])
        # Save KML and generate human-readable CSV
        kml.save("Path.kml")
        print("Success generate KML file")
        self.genCSV()

    def genCSV(self):
        """
        Generate CSV file from all the collected data (or input from CSV)
        """
        # Create csv file
        with open("Points.csv", 'w') as myfile:
            # Setup csv file
            writer = csv.writer(myfile, delimiter=',', lineterminator='\n')
            writer.writerow(("Status", "Number", "State", "Latitude", "Longtitude", "Time",
                             "Temperature", "Humidity", "Outside", "Tone", "Voltage", "Networks", "Skipped Packages"))
            # Input data to csv
            for data in self.dataList:
                if data[0] == "SKIP":
                    writer.writerow((data[0], data[1]))
                    continue
                status, packet_number, state, latitude, longtitude, time, temperature, humidity, outside, tone, voltage, network, skip = data
                tone_string = ""
                if tone == 0:
                    tone_string = "No tone detected"
                elif tone == 1:
                    tone_string = "1kHz detected"
                elif tone == 2:
                    tone_string = "4kHz detected"
                elif tone == 3:
                    tone_string = "1kHz and 4kHz detected"
                dummy = "SSID: " + \
                    str(network[0]) + ", BSSID: " + \
                    str(network[1]) + ", RSSID: " + str(network[2])
                writer.writerow((status, packet_number, state, latitude, longtitude, time,
                                 temperature, humidity, outside, tone_string, voltage, dummy, skip))
        print("Success generate CSV file")


class MQTT():
    """
    A Class responsible for MQTT server communication
    """

    def __init__(self):
        self.client = mqtt.Client("T03-1")
        self.client.on_connect = self.on_connect
        self.client.on_subscribe = self.on_subscribe
        self.client.on_publish = self.on_publish
        self.client.on_log = self.on_log
        self.client.username_pw_set(
            "engg4810_2018", password="blpc7n2DYExpBGY5BP7")

    def connect(self):
        """
        Connect to MQTT server
        """
        self.client.connect("tp-mqtt.zones.eait.uq.edu.au", 1883, 60)
        print("Connect MQTT server")
        self.client.loop_start()

    def disconnect(self):
        """
        Disconnect from MQTT server
        """
        print("Disconnect MQTT server")
        self.client.loop_stop()
        self.client.disconnect()

    def on_connect(self, client, userdata, flags, rc):
        """
        Function response after succeed to connect to MQTT 
        """
        print("Connected with result code " + str(rc))
        if rc == 0:
            client.subscribe("/engg4810_2018/G03")

    def on_log(self, client, userdata, level, buf):
        """
        Use for debug
        """
        print("log: ", buf)

    def on_subscribe(self, client, userdata, mid, granted_qos):
        """
        Function response after subscribed to topic
        """
        print(granted_qos)

    def on_publish(self, client, userdata, mid):
        """
        Function response after publish to topic
        """
        print(mid)


class Parser():
    """
    A class responsible to parsing packet from GPS device or CSV file
    """

    def __init__(self, packet):
        self.packet = packet
        # data will contains: data status, order number, and data such as
        # network, temperature, skipped measurements
        self.data = None

    def parsePacket(self):
        """
        A function parsing packet
        This function will ignore all incorrect format packet
        this function return False if packet is in wrong format, otherwise it return True
        """
        # Ignore empty packet or packet didn't start with $
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
        # Check packet values format
        try:
            packet_number = int(stringSplit[1])
            new_data = int(stringSplit[2])
            checksum = int(stringSplit[-1])
        except Exception as e:
            print("Incorrect packet format, Exception: " + str(e))
            self.data = "IGNORED"
            return False
        # If encounter no new data (or shorted packet), assign data to "skip
        # data"
        if new_data == 0:
            self.data = ["SKIP", packet_number]
            return True
        # Extract data from packet and parsing it
        dummy = ",".join(stringSplit[3:])
        dataString = ""
        for i in range(0, len(dummy)):
            if dummy[i] == "*":
                dataString = dummy[:i]
                break
        self.data = self.parseData(dataString)
        # Insert ordered number to data, as well as the status
        self.data.insert(0, packet_number)
        self.data.insert(0, 'OK')
        self.data.append(0)
        print(self.data)
        for i in self.data:
            if i == "SKIPPED":
                self.data.insert(0, 'WRONG FORMAT')
                break
        return True

    def parseData(self, string, new_data=True):
        """
        A function parsing data in the packet
        This will skip the incorrect data from packet
        """
        # Split data to individual elements and check format for each of
        # element
        dataSplit = string.split(",")
        if new_data == True:
            try:
                state = int(dataSplit[0])
                if state not in [0, 1, 2]:
                    print("Incorrect format, state is 0, 1 or 2")
                    state = "SKIPPED"
            except:
                state = "SKIPPED"

            try:
                latitude = float(dataSplit[1])
                if latitude > 90.0 or latitude < -90.0:
                    print("Incorrect format, latitude is range -90 to 90")
                    latitude = "SKIPPED"
            except:
                latitude = "SKIPPED"

            try:
                longtitude = float(dataSplit[2])
                if longtitude > 180.0 or longtitude < -180.0:
                    print("Incorrect format, longtitude is range -180 to 180")
                    longtitude = "SKIPPED"
            except:
                longtitude = "SKIPPED"

            try:

                time = datetime.datetime.strptime(
                    dataSplit[3], '%Y-%m-%dT%H-%M-%SZ')
            except:
                time = "SKIPPED"

            try:
                temperature = float(dataSplit[4])
            except:
                temperature = "SKIPPED"

            try:
                humidity = float(dataSplit[5])
            except:
                humidity = "SKIPPED"

            try:
                outside = int(dataSplit[6])
                if outside not in [0, 1]:
                    print("Incorrect format, outside is 0 or 1")
                    outside = "SKIPPED"
            except:
                outside = "SKIPPED"

            try:
                tone = int(dataSplit[7])
                if tone not in [0, 1, 2, 3]:
                    print("Incorrect format, tone is 0, 1, 2 or 3")
                    tone = "SKIPPED"
            except:
                tone = "SKIPPED"

            try:
                voltage = int(dataSplit[8])
                if voltage < 0:
                    print("Incorrect format, voltage should be positive value")
                    voltage = "SKIPPED"
            except:
                voltage = "SKIPPED"

            networksData = dataSplit[9]
            networksData = networksData.split("%")
            networks = []
            data = [state, latitude, longtitude, time, temperature,
                    humidity, outside, tone, voltage]
            # Choose the strongest signal network from all network
            network = "SKIPPED"
            start = True
            for i in networksData:
                dummy = i.split(";")
                ssid = dummy[0]
                bssid = dummy[1]
                rssi = dummy[2]
                try:
                    rssi = float(dummy[2])
                except Exception as e:
                    continue
                dummy2 = bssid.split(" ")
                if len(dummy2) != 6:
                    continue
                for i in dummy2:
                    if len(i) != 2:
                        continue
                if rssi > rssi or start == True:
                    start = False
                    network = (ssid, bssid, rssi)
            data.append(network)
        return data


def main():
    root = Tk()
    GUI(root)


if __name__ == "__main__":
    main()
