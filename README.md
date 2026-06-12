# Laboratorio 3 - Sistemas Computacionales

## Integrantes

- Ronal Jesus Condor Blas
- Camila Araceli Alfaro Chuquino

## Descripción general

En este laboratorio se desarrollaron dos programas en lenguaje C relacionados con conceptos de sistemas operativos.

El primer ejercicio implementa un registro de accesos usando llamadas al sistema. El programa escribe en un archivo de texto el usuario que ejecutó el programa, la fecha y hora del acceso, y el PID del proceso que realizó la escritura.

El segundo ejercicio implementa un mini-shell básico. Este programa permite ingresar comandos desde un prompt, separarlos en argumentos, crear un proceso hijo y ejecutar el comando solicitado mediante el sistema operativo.

## Archivos del proyecto

```text
lab3_ejercicio1/
├── ejercicio1.c
├── ejercicio2.c
├── Makefile
├── access_log.txt
├── strace_ejercicio1.txt
└── README.md
```

## Ejercicio 1: Registro de accesos con syscalls

### Objetivo

Implementar un programa en C que registre accesos de usuarios en un archivo `access_log.txt`, utilizando llamadas al sistema en lugar de funciones de biblioteca estándar como `printf`, `fopen`, `fprintf` o `ctime`.

### Funcionamiento del programa

El programa recibe el nombre de usuario como argumento:

```bash
./ejercicio1 ana
```

Luego genera una línea de registro con el siguiente formato:

```text
[YYYY-MM-DD HH:MM:SS] Usuario: ana (PID=1105)
```

Cada vez que el programa se ejecuta, se agrega una nueva línea al final del archivo `access_log.txt`.

### Syscalls utilizadas

| Syscall | Uso en el programa |
|---|---|
| `SYS_time` | Obtiene el tiempo actual del sistema en segundos desde 1970. |
| `SYS_open` | Abre o crea el archivo `access_log.txt`. |
| `SYS_write` | Escribe datos en el archivo o en la terminal. |
| `SYS_close` | Cierra el descriptor del archivo después de escribir. |
| `SYS_getpid` | Obtiene el identificador del proceso que ejecuta el programa. |

### Explicación técnica

El programa usa `SYS_time` para obtener el tiempo actual en formato epoch. Como no se usa `ctime`, el código convierte manualmente ese valor a una fecha legible en formato `YYYY-MM-DD HH:MM:SS`.

Después, se usa `SYS_open` para abrir el archivo `access_log.txt`. Los flags utilizados son:

```c
O_WRONLY | O_CREAT | O_APPEND
```

Esto significa que el archivo se abre solo para escritura, se crea si no existe y cada nueva línea se agrega al final sin borrar los registros anteriores.

La escritura del registro se realiza mediante `SYS_write`. Finalmente, `SYS_close` cierra el descriptor del archivo para liberar el recurso usado por el sistema operativo.

También se agregó `SYS_getpid` para registrar el PID del proceso que realizó la escritura. Esto permite comprobar que cada ejecución del programa corresponde a una instancia distinta del proceso.

### Compilación

```bash
gcc ejercicio1.c -o ejercicio1
```

También puede compilarse usando el Makefile:

```bash
make
```

### Ejecución

```bash
./ejercicio1 ana
./ejercicio1 carlos
cat access_log.txt
```

### Resultado obtenido

```text
[2026-06-11 23:01:46] Usuario: ana (PID=1105)
[2026-06-11 23:01:46] Usuario: carlos (PID=1106)
[2026-06-11 23:02:00] Usuario: ana (PID=1116)
[2026-06-11 23:05:26] Usuario: ana (PID=1191)
```

### Análisis del resultado

El archivo `access_log.txt` muestra que cada ejecución del programa genera una nueva entrada. Esto confirma que el flag `O_APPEND` funciona correctamente, ya que los registros anteriores no se eliminan.

También se observa que cada línea tiene un PID diferente. Esto ocurre porque cada ejecución del programa crea un nuevo proceso, y el sistema operativo asigna un identificador distinto a cada proceso.

## Verificación con strace

Para observar las llamadas al sistema realizadas por el programa, se ejecutó:

```bash
strace ./ejercicio1 ana 2> strace_ejercicio1.txt
```

Luego se filtraron las llamadas más importantes:

```bash
grep -E "time|getpid|open|write|close" strace_ejercicio1.txt
```

En la salida se observaron llamadas como:

```text
time(NULL)
open("access_log.txt", O_WRONLY|O_CREAT|O_APPEND, 0644)
getpid()
write(...)
close(...)
```

Esto demuestra que el programa realmente interactúa con el sistema operativo mediante syscalls. En particular, `open` solicita acceso al archivo, `write` envía los datos al archivo, `getpid` obtiene el identificador del proceso y `close` libera el descriptor usado.

## Ejercicio 2: Mini-shell

### Objetivo

Implementar un intérprete de comandos básico en C que permita ejecutar comandos del sistema desde un prompt propio.

### Funcionamiento del programa

El programa muestra el siguiente prompt:

```text
mini-shell>
```

Desde ahí, el usuario puede escribir comandos como:

```bash
pwd
ls -l
echo Prueba
date
```

El programa también acepta los comandos internos:

```bash
exit
salir
```

Estos comandos finalizan la ejecución del mini-shell.

### Componentes principales

| Elemento | Función |
|---|---|
| `fgets` | Lee la entrada escrita por el usuario. |
| `strtok` | Divide la entrada en comando y argumentos. |
| `fork` | Crea un proceso hijo. |
| `execvp` | Ejecuta el comando solicitado en el proceso hijo. |
| `waitpid` | Hace que el proceso padre espere a que termine el hijo. |
| `exit` / `salir` | Finaliza el mini-shell de forma limpia. |

### Explicación técnica

El mini-shell trabaja dentro de un bucle principal. En cada iteración muestra el prompt y espera que el usuario escriba un comando.

La entrada se separa en tokens usando `strtok`. Por ejemplo, si el usuario escribe:

```bash
ls -l
```

El programa lo interpreta como:

```text
args[0] = "ls"
args[1] = "-l"
args[2] = NULL
```

Después, el programa verifica si el comando ingresado es `exit` o `salir`. Si es así, rompe el bucle y termina.

Si se ingresa un comando externo, se usa `fork` para crear un proceso hijo. El hijo ejecuta el comando con `execvp`, mientras que el padre espera su finalización mediante `waitpid`.

Este modelo evita que el mini-shell desaparezca cuando se ejecuta un comando, porque el proceso que cambia su imagen con `execvp` es el hijo y no el proceso principal.

### Compilación

```bash
gcc ejercicio2.c -o ejercicio2
```

También puede compilarse usando:

```bash
make
```

### Ejecución

```bash
./ejercicio2
```

### Pruebas realizadas

Dentro del mini-shell se ejecutaron los siguientes comandos:

```bash
pwd
ls -l
echo Prueba
date
exit
```

### Resultado de las pruebas

| Comando | Resultado |
|---|---|
| `pwd` | Muestra la ruta actual del proyecto. |
| `ls -l` | Lista los archivos del directorio. |
| `echo Prueba` | Imprime el texto `Prueba`. |
| `date` | Muestra la fecha y hora actual del sistema. |
| `exit` | Finaliza el mini-shell y regresa a la terminal normal. |

### Análisis del mini-shell

El mini-shell cumple con el flujo básico de ejecución de comandos en sistemas UNIX. El proceso principal conserva el control del intérprete, mientras que cada comando externo se ejecuta en un proceso hijo.

El uso de `fork`, `execvp` y `waitpid` permite reproducir el comportamiento esencial de una shell simple. El padre no ejecuta directamente el comando externo; solo crea el hijo y espera su finalización. El hijo, en cambio, reemplaza su imagen de proceso por el programa solicitado.

## Makefile

Se implementó un archivo `Makefile` para automatizar la compilación de ambos ejercicios.

### Compilar ambos programas

```bash
make
```

### Limpiar ejecutables

```bash
make clean
```

### Explicación

El comando `make` genera los ejecutables `ejercicio1` y `ejercicio2` a partir de los archivos fuente `ejercicio1.c` y `ejercicio2.c`.

Si los ejecutables ya están actualizados, `make` puede mostrar:

```text
make: Nothing to be done for 'all'.
```

Esto indica que no hubo cambios recientes en los archivos fuente y que no es necesario recompilar.

## Conclusiones

En el primer ejercicio se comprobó que un programa en C puede comunicarse directamente con el sistema operativo mediante llamadas al sistema. El uso de `SYS_time`, `SYS_open`, `SYS_write`, `SYS_close` y `SYS_getpid` permitió registrar accesos en un archivo sin depender de funciones de biblioteca de alto nivel.

En el segundo ejercicio se implementó un mini-shell funcional. Este programa demuestra cómo se crean procesos hijos con `fork`, cómo se ejecutan comandos externos con `execvp` y cómo el proceso padre espera la finalización del hijo mediante `waitpid`.

Ambos ejercicios permiten observar cómo los programas de usuario solicitan servicios al sistema operativo y cómo el kernel interviene en operaciones como manejo de archivos, identificación de procesos y ejecución de comandos.