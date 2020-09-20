rastreador


    gcc `pkg-config --cflags gtk+-3.0` -o main main.c `pkg-config --libs gtk+-3.0` -export-dynamic -rdynamic
    
Para correr el makefile, realizan los siguientes comandos en el directorio en donde se encuentra el makefile:
    
    make clean //para limpiar el proyecto de ejecutables y objetos
    
    make   //para construir el proyecto
    
    ./interfaz //para correr la ventana
    
    
    
    