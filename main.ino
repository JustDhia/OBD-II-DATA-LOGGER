#include <OBD2UART.h>
#include <WiFi.h>
#include <SD.h>
#include <FTP.h>

import processing.serial.*;

// Engine
const int PID_RPM = 12;                        // Engine RPM (rpm) [0, 16383.75] en 2 octets
const int PID_ENGINE_LOAD = 4;                 // Calculated engine load (%) [0,100] en 1 octet
const int PID_COOLANT_TEMP = 5;                // Engine coolant temperature (°C) [-40, 215] en 1 octet
const int PID_TIMING_ADVANCE = 14;             // Ignition timing advance (°) [-64, 63.5] en 1 octet
const int PID_ENGINE_OIL_TEMP = 92;            // Engine oil temperature (°C) 
const int PID_ENGINE_TORQUE_PERCENTAGE = 98;   // Engine torque percentage (%) [-122, 130] en 1 octet
const int PID_ENGINE_REF_TORQUE = 99;          // Engine reference torque (Nm) [0, 65535] en 2 octets

// Intake/Exhaust
const int PID_INTAKE_TEMP = 15;                // Intake temperature (°C) [-40, 215]
const int PID_INTAKE_PRESSURE = 11;            // Intake manifold absolute pressure (kPa) [0, 255] en 1 octets
const int PID_MAF_FLOW = 16;                   // MAF flow pressure (grams/s) [0, 655.35] en 2 octets
const int PID_BAROMETRIC = 51;                 // Barometric pressure (kPa) [0, 255] en 1 octet

// Speed/Time
const int PID_SPEED = 13;                      // Vehicle speed (km/h) [0, 255] en 1 octet
const int PID_RUNTIME = 127;                   // Engine running time (second) en 13 octets
const int PID_DISTANCE = 49;                   // Vehicle running distance (km) [0, 65535] en 2 octets

// Driver
const int PID_THROTTLE = 17;                   // Throttle position (%) [0, 100] en 1 octet
const int PID_AMBIENT_TEMP = 70;               // Ambient temperature (°C) [-40, 215] en 1 octets

// Electric Systems
const int PID_CONTROL_MODULE_VOLTAGE = 66;     // vehicle control module voltage (V) [0, 65535] en 2 octets
const int PID_HYBRID_BATTERY_PERCENTAGE = 91;  // Hybrid battery pack remaining life (%) [0, 100] en 1 octet

COBD obd;
const int obdPin = 13;

char ssid[] = "SSID";               // SSID du reseau
char pass[] = "Password";           // mot de passe de reseau
int status = WL_IDLE_STATUS;        // l'état de Wifi

const int buttonPin = 2;            // le numéro de la broche du bouton-poussoir
int buttonState = 0;                // variable pour lire l'état du bouton poussoir

const sdPin = 10;
bool sdState = true;                // variable pour stocker l'etat du stockage externe

// configuration FTP
uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 10, 10, 6);
IPAddress namesServer(8, 8, 8, 8);
IPAddress gateway(10, 10, 10, 1);
IPAddress netmask(255, 255, 255, 0);

IPAddress server(192, 168, 1, 220);
const char *user = "USER";
const char *pass = "PASSWORD";
const char *fileName = "data.csv";

EthernetClient ftpControl;
EthernetClient ftpData;

FTP ftp(ftpControl, ftpData);
// Fin configuration FTP

int RPM;
int ENGINE_LOAD;
int COOLANT_TEMP;
int TIMING_ADVANCE;
int ENGINE_OIL_TEMP;
int ENGINE_TORQUE_PERCENTAGE;
int ENGINE_REF_TORQUE;
int INTAKE_TEMP;
int INTAKE_PRESSURE;
int MAF_FLOW;
int BAROMETRIC;
int SPEED;
int RUNTIME;
int DISTANCE;
int THROTTLE;
int AMBIENT_TEMP;
int CONTROL_MODULE_VOLTAGE;
int HYBRID_BATTERY_PERCENTAGE;

Table dataTable;
File dataFile;

int frequency = 10;

void setup() {
    // initialiser la broche (pin) du bouton-poussoir comme entrée.
    pinMode(buttonPin, INPUT);
    // initialiser la broche (pin) du carte sd comme sortie.
    pinMode(sdPin, OUTPUT);
    if (!SD.begin(4)) {
        // initialisation échouée
        // stocker les données en intern sur arduino
        sdState = false;
        frequency = frequency * 4;
    }
    
    pinMode(obdPin, OUTPUT);
    obd.begin();
    // initier la connexion OBD-II jusqu'au succès
    while (!obd.init());

    dataTable = new Table();
    dataTable.addColumn("RPM");
    dataTable.addColumn("ENGINE_LOAD");
    dataTable.addColumn("COOLANT_TEMP");
    dataTable.addColumn("TIMING_ADVANCE");
    dataTable.addColumn("ENGINE_OIL_TEMP");
    dataTable.addColumn("ENGINE_TORQUE_PERCENTAGE");
    dataTable.addColumn("ENGINE_REF_TORQUE");
    dataTable.addColumn("INTAKE_TEMP");
    dataTable.addColumn("INTAKE_PRESSURE");
    dataTable.addColumn("MAF_FLOW");
    dataTable.addColumn("BAROMETRIC");
    dataTable.addColumn("SPEED");
    dataTable.addColumn("RUNTIME");
    dataTable.addColumn("DISTANCE");
    dataTable.addColumn("THROTTLE");
    dataTable.addColumn("AMBIENT_TEMP");
    dataTable.addColumn("CONTROL_MODULE_VOLTAGE");
    dataTable.addColumn("HYBRID_BATTERY_PERCENTAGE");

    if (sdState) {
        dataFile = SD.open("data.csv", FILE_WRITE);
    }
}

void WIFIsetup() {
    // vérifier la présence du shield
    if (WiFi.status() == WL_NO_SHIELD) {
        // ne continue pas
        while (true);
    }

    String fv = WiFi.firmwareVersion();

    // tenter de se connecter au réseau Wifi
    while (status != WL_CONNECTED) {
        
        // Tentative de connexion au SSID WPA
        status = WiFi.begin(ssid, pass);

        // attendez 2 secondes pour la connexion
        delay(2000);
    }
}

void FTPSetup() {
    Ethernet.begin(mac, ip, namesServer, gateway, netmask);
    if (!ftp.connect(server, user, pass)) {
        while (true);
    }
}

void transferFile() {
    obd.readPID(PID_DISTANCE, DISTANCE)
    TableRow newRow = table.addRow();
    newRow.setString("DISTANCE", DISTANCE);

    if (sdState) {
        saveTable(dataTable, dataFile); 
        ftp.store(fileName, dataFile, strlen(dataFile));
    } else {
        saveTable(dataTable, "tmp.csv");
        ftp.store(fileName, "tmp.csv", strlen(dataFile));
    }
}


void loop() {
    // lire l'état de la valeur du bouton poussoir
    i=0;

    while (i<frequency) {
        buttonState = digitalRead(buttonPin);
        if (buttonState == HIGH) {
            WIFIsetup();
            FTPSetup();
            // transfer les données
            transferFile();
        }
        i++;
        sleep(1000);  
    }
    buttonState = digitalRead(buttonPin);

    // vérifier si le bouton poussoir est enfoncé
    // Si c'est le cas, le buttonState est HIGH.
    if (buttonState == HIGH) {
        WIFIsetup();
        FTPSetup();
        // transfer les données
        transferFile();
    }
    else {
        obd.readPID(PID_RPM, RPM) 
        obd.readPID(PID_COOLANT_TEMP, COOLANT_TEMP)
        obd.readPID(PID_TIMING_ADVANCE, TIMING_ADVANCE)
        obd.readPID(PID_ENGINE_OIL_TEMP, ENGINE_OIL_TEMP)
        obd.readPID(PID_ENGINE_TORQUE_PERCENTAGE, EGINE_TORQUE_PERCENTAGENG)
        obd.readPID(PID_ENGINE_REF_TORQUE, ENGINE_REF_TORQUE)
        obd.readPID(PID_INTAKE_TEMP, INTAKE_TEMP)
        obd.readPID(PID_INTAKE_PRESSURE, INTAKE_PRESSURE)
        obd.readPID(PID_MAF_FLOW, MAF_FLOW)
        obd.readPID(PID_BAROMETRIC, BAROMETRIC)
        obd.readPID(PID_SPEED, SPEED)
        obd.readPID(PID_RUNTIME, RUNTIME)
        // obd.readPID(PID_DISTANCE, DISTANCE)
        obd.readPID(PID_THROTTLE, THROTTLE)
        obd.readPID(PID_AMBIENT_TEMP, AMBIENT_TEMP)
        obd.readPID(PID_CONTROL_MODULE_VOLTAGE, CONTROL_MODULE_VOLTAGE)
        obd.readPID(PID_HYBRID_BATTERY_PERCENTAGE, HYBRID_BATTERY_PERCENTAGE)

        TableRow newRow = table.addRow();

        newRow.setString("RPM", RPM);
        newRow.setString("ENGINE_LOAD", ENGINE_LOAD);
        newRow.setString("COOLANT_TEMP", COOLANT_TEMP);
        newRow.setString("TIMING_ADVANCE", TIMING_ADVANCE);
        newRow.setString("ENGINE_OIL_TEMP", ENGINE_OIL_TEMP);
        newRow.setString("ENGINE_TORQUE_PERCENTAGE", ENGINE_TORQUE_PERCENTAGE);
        newRow.setString("ENGINE_REF_TORQUE", ENGINE_REF_TORQUE);
        newRow.setString("INTAKE_TEMP", INTAKE_TEMP);
        newRow.setString("INTAKE_PRESSURE", INTAKE_PRESSURE);
        newRow.setString("MAF_FLOW", MAF_FLOW);
        newRow.setString("BAROMETRIC", BAROMETRIC);
        newRow.setString("SPEED", SPEED);
        newRow.setString("RUNTIME", RUNTIME);
        // il est mieux d'interroger la distance seulement une fois a la fin du trajet
        // newRow.setString("DISTANCE", DISTANCE);
        newRow.setString("THROTTLE", THROTTLE);
        newRow.setString("AMBIENT_TEMP", AMBIENT_TEMP);
        newRow.setString("CONTROL_MODULE_VOLTAGE", CONTROL_MODULE_VOLTAGE);
        newRow.setString("HYBRID_BATTERY_PERCENTAGE", HYBRID_BATTERY_PERCENTAGE);

        if (sdState) {
            // si la carte sd est disponible
            saveTable(dataTable, dataFile); 
        } else {
            saveTable(dataTable, "tmp.csv");
        }
        
    }
}