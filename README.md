# Tarea 1 Sistemas Operativos
- Disclaimer: Uso preferente en Sistemas con base Linux
## Integrantes
- Nicolás Soto
- Alejandro Neira
- Erick Saldívar
## Como ejecutar
- Primero se debe instalar la libreria readline, en caso de no tenerla instalada.
- Para instalar readline medienta la linea de comandos, se debe correr el siguiente comando:
```sudo apt-get install libreadline-dev```
- Para ejecutar el archivo, primero se debe compilar usando en el root del archivo main.cpp:
```g++ -o shell main.cpp -lreadline```
- Luego, para ejecutar el programa, se debe correr:
```./shell```
- Se vera un prompt que indica que el programa esta corriendo, y se puede ingresar comandos.
- Para salir del programa, se debe ingresar el comando ```exit```