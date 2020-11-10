# OBD II data logger
Exploiter le standard OBD/OBII pour collecter les données du véhicule, stocker ses informations sur un support de stockage interne pour les transmettre après via WIFI.

## Table des matières
1. [Composants](#Composants)
2. [Compatibilité](#Compatibilité)
3. [Bibliothèques requises](#lib) 
4. [Communication avec la véhicule](#com)
5. [Interroger l'OBD](#int)
6. [Configuration de fichier de données](#file_conf)
7. [Configuration WIFI](#wifi_conf)
8. [Transfer de données (côté client)](#file_transfer)
9. [Fréquence d'interrogation des données](#freq)
10. [Schémas](#schemas)

## Composants
* [Arduino UNO rev3](https://store.arduino.cc/arduino-uno-rev3)
* [Freematics OBD-II UART Adapter V2.1](https://freematics.com/pages/products/freematics-obd-ii-uart-adapter-mk2/)
* [Arduino WIFI shield](https://www.newark.com/arduino/a000058/add-on-card-wifi-shield-r3-int/dp/45W6204?COM=ref_hackster)
* Micro SD Card Shield Module
* SD Card

## Compatibilité
Les véhicules utilisant les protocoles de véhicule suivants sont pris en charge:

- CAN 500Kbps/29bit
- CAN 500Kbps/11bit
- CAN 250Kbps/29bit
- CAN 250Kbps/11bit
- KWP2000 Fast
- KWP2000 5Kbps
- ISO9141-2 (partial)

## <a name="lib"></a> Bibliothèques requises
- ArduinoOBD (https://github.com/stanleyhuangyc/ArduinoOBD/tree/master/libraries/OBD2UART)
- WiFi library (https://www.arduino.cc/en/Reference/WiFi)
- SD Library (https://www.arduino.cc/en/reference/SD)
- Arduino-FTP (https://github.com/IndustrialShields/arduino-FTP)

## <a name="com"></a> Communication avec la véhicule
### le standard OBD-II

Les systèmes OBD permettent d'accéder à l'état des différents sous-systèmes du véhicule. Les implémentations OBD modernes utilisent un port de communication numérique normalisé pour fournir des données en temps réel en plus d'une série normalisée de codes de diagnostic, ou DTC, qui permettent à une personne d'identifier rapidement et de remédier aux dysfonctionnements dans le véhicule

### L’adaptateur Freematics OBD-II
Ce produit fonctionne comme un pont de données OBD-II pour Arduino, fournissant un accès aux données OBD-II avec la bibliothèque Arduino ainsi qu'une alimentation (convertie et régulée à partir du port OBD-II) pour Arduino et ses appareils connectés.

[Une bibliothèque Arduino](https://github.com/stanleyhuangyc/ArduinoOBD/tree/master/libraries/OBD2UART) dédiée est fournie pour un accès facile à toutes les fonctionnalités avec la carte Arduino en utilisant des API.

Certaines API couramment utilisées sont les suivantes:

- setBaudRate - définir la vitesse de transmission série de l'adaptateur
- readPID - lire le PID OBD-II spécifié et renvoyer la valeur analysée
- clearDTC - effacer le code de diagnostic
- getVoltage - mesure la tension de la batterie de la voiture
- getVIN - récupérer le numéro d'identification du véhicule
- getTemperature - récupère la température de l'appareil
- readAccel - lire l'accéléromètre X

L’adaptateur fournit une sortie 5 V régulée pour alimenter Arduino et les autres appareils, donc aucun cordon d'alimentation supplémentaire n'est nécessaire.

## <a name="int"></a> Interroger l'OBD
Les commandes OBD sont composées de codes hexadécimaux écrits en caractères ASCII. Les deux premiers nombres hexadécimaux font référence au mode de service à utiliser. Il existe 10 services de diagnostic décrits dans la dernière norme OBD-II SAE J1979. Puisque nous sommes intéressés par la surveillance en de l'etat de véhicule, nous n'utiliserons que le code 01.

![Modes](https://www.dropbox.com/s/ndszulj3xjc5ynm/Screen%20Shot%202020-07-09%20at%2010.04.59%20AM.png?dl=0&raw=1)

Tout numéro hexadécimal après le mode de service représente l'ID de paramètre (PID) pour obtenir des fonctions spéciales. Ci-dessous, la capture d'écran des PID en mode de service 01 . Plus d'informations peuvent être trouvées sur [Wikipedia](https://en.wikipedia.org/wiki/OBD-II_PIDs).

![PIDs](https://www.dropbox.com/s/019j7wz3fwfla9g/Screen%20Shot%202020-07-09%20at%2010.08.28%20AM.png?dl=0&raw=1)

### Les informations a extraire
```
const int PID_RPM = 12;                        
// Engine RPM (rpm) [0, 16383.75] en 2 octets
const int PID_ENGINE_LOAD = 4;                 
// Calculated engine load (%) [0,100] en 1 octet
const int PID_COOLANT_TEMP = 5;                
// Engine coolant temperature (°C) [-40, 215] en 1 octet
const int PID_TIMING_ADVANCE = 14;             
// Ignition timing advance (°) [-64, 63.5] en 1 octet
const int PID_ENGINE_OIL_TEMP = 92;            
// Engine oil temperature (°C) 
const int PID_ENGINE_TORQUE_PERCENTAGE = 98;   
// Engine torque percentage (%) [-122, 130] en 1 octet
const int PID_ENGINE_REF_TORQUE = 99;          
// Engine reference torque (Nm) [0, 65535] en 2 octets
const int PID_INTAKE_TEMP = 15;                
// Intake temperature (°C) [-40, 215]
const int PID_INTAKE_PRESSURE = 11;            
// Intake manifold absolute pressure (kPa) [0, 255] en 1 octets
const int PID_MAF_FLOW = 16;                   
// MAF flow pressure (grams/s) [0, 655.35] en 2 octets
const int PID_BAROMETRIC = 51;                 
// Barometric pressure (kPa) [0, 255] en 1 octet
const int PID_SPEED = 13;                      
// Vehicle speed (km/h) [0, 255] en 1 octet
const int PID_RUNTIME = 127;                   
// Engine running time (second) en 13 octets
const int PID_DISTANCE = 49;                   
// Vehicle running distance (km) [0, 65535] en 2 octets
const int PID_THROTTLE = 17;                   
// Throttle position (%) [0, 100] en 1 octet
const int PID_AMBIENT_TEMP = 70;               
// Ambient temperature (°C) [-40, 215] en 1 octets
const int PID_CONTROL_MODULE_VOLTAGE = 66;     
// vehicle control module voltage (V) [0, 65535] en 2 octets
const int PID_HYBRID_BATTERY_PERCENTAGE = 91;  
// Hybrid battery pack remaining life (%) [0, 100] en 1 octet
```

## <a name="file_conf"></a> Configuration de fichier de données
les données vont être stockées dans un fichier dans la carte SD.
```
#include <SD.h>
const sdPin = 10;
bool sdState = true;
Table dataTable;
File dataFile; 
void setup() {
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
```

## <a name="wifi_conf"></a> Configuration WIFI
```
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
```

## <a name="file_transfer"></a> Transfer de données (côté client)
```
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
```

## <a name="freq"></a> Fréquence d'interrogation des données
Les données sont interrogées toutes les 30 secondes si la carte SD externe est disponible, sinon elles seront interrogées toutes les 2 minutes.

## <a name="schemas"></a> Schémas
### Cablage OBD
![OBD](https://www.dropbox.com/s/gbeipt04fn1nkyc/schema0_0FjW3h1TtB.PNG?dl=0&raw=1)
### Cablage SD/WIFI
![WIFI](https://www.dropbox.com/s/jgjwmtiovntn7zn/schem4-2_a6eW9LZQ85.png?dl=0&raw=1)


License
----

MIT

[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)


   [dill]: <https://github.com/joemccann/dillinger>
   [git-repo-url]: <https://github.com/joemccann/dillinger.git>
   [john gruber]: <http://daringfireball.net>
   [df1]: <http://daringfireball.net/projects/markdown/>
   [markdown-it]: <https://github.com/markdown-it/markdown-it>
   [Ace Editor]: <http://ace.ajax.org>
   [node.js]: <http://nodejs.org>
   [Twitter Bootstrap]: <http://twitter.github.com/bootstrap/>
   [jQuery]: <http://jquery.com>
   [@tjholowaychuk]: <http://twitter.com/tjholowaychuk>
   [express]: <http://expressjs.com>
   [AngularJS]: <http://angularjs.org>
   [Gulp]: <http://gulpjs.com>

   [PlDb]: <https://github.com/joemccann/dillinger/tree/master/plugins/dropbox/README.md>
   [PlGh]: <https://github.com/joemccann/dillinger/tree/master/plugins/github/README.md>
   [PlGd]: <https://github.com/joemccann/dillinger/tree/master/plugins/googledrive/README.md>
   [PlOd]: <https://github.com/joemccann/dillinger/tree/master/plugins/onedrive/README.md>
   [PlMe]: <https://github.com/joemccann/dillinger/tree/master/plugins/medium/README.md>
   [PlGa]: <https://github.com/RahulHP/dillinger/blob/master/plugins/googleanalytics/README.md>
