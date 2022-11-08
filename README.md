# Modbus communication 🪐

This design allow on communication two devices by means of communication protocol modbus. The project uses platform esp32 and library esp-idf. Library esp -idf allow on simple communicatie through protocol modbus and implemented by RS485. Main device gets information about temperature outdoor through modbus and processed the  data to web server. The web server allow controls the diode LED in indoor and outdoor through modbus.

# Screenshots 🎲

![Zrzut ekranu_20221107_095811](https://user-images.githubusercontent.com/90866275/200576812-feb7a899-d6af-4a34-a737-9bf369c436f4.png)

![Zrzut ekranu_20221107_100000k](https://user-images.githubusercontent.com/90866275/200576845-63077f38-5fdf-4c0b-976b-7906a599ba0c.png)

# Installation

```bash

git clone https://github.com/bartosz1998/Modbus-communication.git

idf.py build

idf.py -p PORT flash

idf.py monitor 

```

# Technologies 💡

+ c
+ ESP - IDF 
+ MODBUS
+ HTML
