## Qué hace el programa
Este proyecto rastrea las llamadas del sistema (syscalls) y recupera el id, orden secuencial, nombre y descripción de cada uno.
Asimismo, permite ejecutar el rastreador de manera pausada o continua. 

El rastreador recibe un programa a ejecutar e inspecciona los syscalls de los procesos hijos mediante la biblioteca ptrace. Se espera que este proceso hijo termine de ejecutarse, para 
recuperar su PID. 
Al obtener este dato, se hace uso de ausyscall para obtener su nombre, enviando el PID por parámetro. Asimismo, se busca en el txt de descripciones la descripción correspondiente al syscall según
el id. 

Todo el trabajo se llevo a cabo en el sistema operativo GNU/LINUX y se utilizó el lenguaje de progra-
mación C. 

Se despliega una interfaz mostrando los datos de distinta manera (bitácora, tabla acumulada, gráfico de pastel).

### Bibliotecas utilizadas
Biblioteca ptrace: Se trata de un syscall que permite a un proceso padre rastrear o inspeccionar los syscalls
de los procesos hijos.
Biblioteca sys/reg: Nos permite utilizar Rax y ORGI RAX que nos ayudarán ubicar el id de un syscall
y su valor de retorno respectivamente.
Biblioteca unistd: Es realmente un header que proporciona acceso a la API del sistema operativo POSIX.
Nos permite obtener el PID de un syscall y ejecutar fork() para crear procesos hijos.

### Estructuras de datos usadas y funciones:
**ARGV: Puntero al espacio en memoria que C utiliza por defecto para almacenar los parámetros que el
usuario asigne al programa en cuestión.
ptrace: Función que permite que nuestro programa (proceso padre) rastree los syscalls de los procesos hijos.
wait for syscall: Esta función simplemente nos ayudará a saber cuando se terminó de ejecutar un syscall
(recibiendo como parámentro un proceso hijo), en dicho caso retorna 1. Si no se ha terminado de ejecutar el
syscall, retorna 0.
trace: Se trata de la función que rastrea los syscalls, toma su PID y su valor de retorno. Esta función hace
uso de wait for syscall para saber cuando es prudente tomar valores sin crear errores, en caso contrario, corta
la ejecución.
create child: Eejecuta ptrace para dar los permisos necesario al pro




## Uso del programa
Para ejecutar este proyecto, se deben ejecutar los siguientes comandos:

Para limpiar el proyecto de ejecutables y objetos
    
        make clean 
        
Para construir el proyecto:
        
        make   
        
Para correr el proyecto:

        ./interfaz 

Realizado esto, se despliega la interfaz del proyecto. 
Aquí, en el campo de "PROG" se debe escribir el programa a ejecutar, por ejemplo:

        ls
    
## Qué no funciona
Lo único que se ha encontrado que no funciona es el trabajo extra opcional del proyecto (Mostrar de manera formateada los argumentos de por lo menos 5 tipos diferentes de
system call.), ya que no se implementó esta característica.
    