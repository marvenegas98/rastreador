## Qué hace el programa
Este proyecto rastrea las llamadas del sistema (syscalls) y recupera el id, orden secuencial, nombre y descripción de cada uno.
Asimismo, permite ejecutar el rastreador de manera pausada o continua. 

El rastreador recibe un programa a ejecutar e inspecciona los syscalls de los procesos hijos mediante la biblioteca ptrace. Se espera que este proceso hijo termine de ejecutarse, para recuperar su PID. 
Al obtener este dato, se hace uso de ausyscall para obtener su nombre, enviando el PID por parámetro, el cual es mapeado para obtener su nombre corresponiente. Esto se realiza mediante la función get_syscall_name().

Para obtener la descripción de cada Syscall, fue necesario crear un archivo en formato texto donde se guardara cada descripción de los syscall. Dentro del archivo se podrá encontrar, por cada línea, el ID del Syscall seguido por su descripción, separado por una coma. De este archivo se obtiene la descripción de los Syscalls mediante la función get_syscall_desc(), la cual recibe como parámetro el id del syscall.

Una vez que el id, nombre y descripción del syscall son obtenidos, se imprimen en la interfaz y se continua con el mismo proceso para el siguiente syscall (hijo).

Todo el trabajo se llevo a cabo en el sistema operativo GNU/LINUX y se utilizó el lenguaje de progra-
mación C. 

Se despliega una interfaz mostrando los datos de distinta manera (bitácora, tabla acumulada, gráfico de pastel).


## Uso del programa
Para ejecutar este proyecto, se deben ejecutar los siguientes comandos:

Para instalar la biblioteca ausyscall:
       
       apt-get install auditd

Para limpiar el proyecto de ejecutables y objetos
    
        make clean 
        
Para construir el proyecto:
        
        make   
        
Para correr el proyecto:

        ./interfaz 

Realizado esto, se despliega la interfaz del proyecto. 
Aquí, en el campo de "PROG" se debe escribir el programa a ejecutar, por ejemplo:

        ls -al

## Qué no funciona

Al intentar correr el comando "cd", sin importar los parámetros utilizados en el mismo, el programa parece enciclarse, y al final de la ejecución retorna el siguiente error:

      [xcb] Unknown sequence number while processing queue
      [xcb] Most likely this is a multi-threaded client and XInitThreads has not been called
      [xcb] Aborting, sorry about that.
      
Indagando acerca del error, no dimos cuenta que el error parece venir de la librería "libx11" apartir de una actualización realizada a la misma. Esto es discutido en el siguiente foro: https://bugs.launchpad.net/ubuntu/+source/gtk+2.0/+bug/1808710 

Lo único que se ha encontrado que no funciona es el trabajo extra opcional del proyecto (Mostrar de manera formateada los argumentos de por lo menos 5 tipos diferentes de
system call.), ya que no se implementó esta característica.

## Bibliotecas utilizadas
### Biblioteca ptrace
Se trata de un syscall que permite a un proceso padre rastrear o inspeccionar los syscalls
de los procesos hijos.
### Biblioteca sys/reg
 Nos permite utilizar Rax y ORGI RAX que nos ayudarán ubicar el id de un syscall
y su valor de retorno respectivamente.
### Biblioteca unistd
Es realmente un header que proporciona acceso a la API del sistema operativo POSIX.
Nos permite obtener el PID de un syscall y ejecutar fork() para crear procesos hijos.
### Biblioteca ausyscall
Esta biblioteca extra debe ser instalada para el correcto funcionamiento del programa. 
Se encarga de recibir el id de un Syscall y mapearlo con su respectivo nombre.
Tambien permite realizar la función inversa, recibiendo el nombre de un Syscall y devolviendo su
respectivo ID.

## Estructuras de datos usadas y funciones:

### ptrace: Función que permite que nuestro programa (proceso padre) rastree los syscalls de los procesos hijos.

### wait for syscall: Esta función simplemente nos ayudará a saber cuando se terminó de ejecutar un syscall
(recibiendo como parámentro un proceso hijo), en dicho caso retorna 1. Si no se ha terminado de ejecutar el
syscall, retorna 0.
### trace: Se trata de la función que rastrea los syscalls, toma su PID y su valor de retorno. 
Esta función hace uso de wait for syscall para saber cuando es prudente tomar valores sin crear errores, en caso contrario, corta
la ejecución.
### create child: Ejecuta ptrace para dar los permisos necesario al programa.

### split_description: Función para separar el id de la descripción.
La descripción recupera del archivo de texto con las descripciones tiene el siguiente formato "id,descripción".
Esta función recibe toda la cadena y devulve solo la descripción.

### get_syscall_desc: Función que recupera la descripción del syscall de acuerdo a su id.
Esta función recibe un id de un syscall y busca su descripción dentro de un txt creado previamente que contiene la 
descripción de cada uno de los 314 syscalls existentes.

### ordenar: Función que ordena una lista en orden ascendetne o descendente.
La función recibe dos listas, una de ids y otra de nombres, recibe tambien dos banderas que indican cual lista
debe ser ordenada y en que orden (ascendente o descendente). Recibe las dos listas porque, aunque el criterio de
ordenamiento se aplique directamente solo a una, la otra debe ser ordenada tambien para mantener la correspondencia entre ambas.

### split_command: Función que separa todos los componentes de un comando.
Esta función recibe un comando en formato string y devuelve todos sus componentes en una lista. La función separa por espacios.

### get_syscall_name: Función que recupera el nombre de un syscall.
Esta función utiliza la biblioteca ausyscall para mapear un id con su respectivo nombre. La función recibe justo el id a buscar y
se encarga de recuperar solo el nombre del valor devuelto por la función ausyscall.
    
