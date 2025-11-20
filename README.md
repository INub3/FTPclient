# FTPclient

> Este proyecto implementa un **cliente FTP** básico utilizando sockets TCP en lenguaje C. Permite conectarse a un servidor FTP, enviar comandos estándar y realizar transferencias de archivos en modo pasivo (PASV) y activo (PORT). Tiene la intención de soportar **descargas** y **subidas** concurrentes mediante procesos hijo `(fork())`.

---

### Requerimientos

Este proyecto implementa un cliente FTP; en consecuencia, necesitaremos instalar un servidor FTP concurrente.
**Instalación y configuración:**
Para entornos Unix basados en Debian o Ubuntu la instalación se realiza con el siguiente comando:

```bash
$ sudo apt install vsftpd
```

Para hacer uso del servidor necesitamos iniciar su `daemon` de la siguiente manera:

```bash
$ sudo /etc/init.d/vsftpd start
```

### Instalar cliente

Para instalar el cliente debemos seguir los siguientes pasos:

1. Clonar el proyecto:

```bash
$ git clone https://github.com/INub3/TCPclient.git && cd TCPclient
```

2. Ejecutar el archivo de configuración **Makefile** (por cada cambio que queramos realizar en el código `TCPftp.c`):

```bash
$ make
```

### Ejecutar el cliente

El cliente requiere ser iniciado con la dirección IP y el puerto del servicio (valores predeterminados => localhost:21):

```bash
$ ./TCPftp <host> <puerto>
```

---

## Problema

Este cliente intenta implementar concurrencia por parte del cliente, ejecutando la consola de comandos desde el socket de control y realizando cargas o descargas concurrentes mediante el uso de subprocesos en el socket de datos.
Sin embargo, se presenta el problema de que no se pueden realizar varias acciones de descarga y carga al mismo tiempo, puesto que un solo socket de datos no puede realizar la operación concurrente.

Este problema será detallado en el informe.
