#include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h> // Para rastrear procesos hijos
#include <sys/reg.h> // RAX, ORIG_RAX
#include <sys/wait.h> // KILL, SIGSTOP, waitpid

int table_lenght = 0; // Largo de la tabla de syscalls
int mtx[2][1000]; // [] PID, [] Cantidad de syscalls con ese PID
int PIDS = sizeof(mtx[0]) / sizeof(mtx[0][0]); // Cantidad de columnas de la matriz
int flag = -1; // 0 -v , 1 -V

static GtkWidget* window;
static GtkWidget* imagen;
static GtkBuilder *builder;
GtkWidget *fixed1;
GtkWidget *grid1; 
GtkWidget *label[1000];
GtkWidget *button[1000];
GtkWidget *view1;
GtkWidget *siguiente;

void on_destroy(); 
void on_row(GtkButton *);

char tmp[1024]; // general use
int	row;

char** s;
int frec[250];	
int lst_idx = 0;

/*
Funcion para leer los syscalls del txt tras haber ejecutado el comando
Los lee, capta el nombre y los mete a un arreglo contando cuantas veces aparece para crear la tabla para el grafico
*/
void actualizar(){
	while(gtk_events_pending()){
		gtk_main_iteration_do(1);
	}
}
void crear_csv(){
	FILE *fp;
	fp=fopen("tabla.csv","w+");
	fprintf(fp,"Nombre, Frecuencia");
	for(int i=0;i<lst_idx-1;i++){
		fprintf(fp,"\n%s , %i",s[i], frec[i]);
	}
	
	fclose(fp);
}


void mostrar_imagen(){
	system("gnuplot pastel.gnuplot");
	const gchar * filename = "piechart.png";
	gtk_image_set_from_file(GTK_IMAGE(imagen), filename);
	gtk_widget_show_all(window);
	actualizar();

}
void crear_tabla(){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

	s = (char **)calloc(250,sizeof(char **));
	for(int i = 0; i < 250; i++){
        s[i] = (char *)malloc(20);
		s[i] = "";
    }
    fp = fopen("syscalls.txt", "r"); 
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
		char *ptr = NULL;
		ptr = strtok(line, "(");
		int flag = 0;

		for(int k = 0; k<=lst_idx;k++){
			if(strcmp(s[k], ptr) == 0){
				frec[k]+=1;
				flag=1;
			}
		}
		if(flag==0){
			s[lst_idx] = strdup(ptr);
			frec[lst_idx]=1;
			lst_idx+=1;
		}
		
    }
	//printf("lst idx %i\n",lst_idx);

    fclose(fp);
    if (line)
        free(line);
}
void ejecutar_comando(GtkButton *ejecutar, gpointer data){
    const char *text = gtk_entry_get_text(data);
	char strace[50];
	strcpy(strace, "strace -o syscalls.txt "); //Guardar en txt sus syscalls del comando
	strcat(strace,text);
	system(strace);
	crear_tabla();
	crear_csv();
		
	int ind = 0;
	char secuencia[10000];
	char id[10000];
	char frecuencia[10000];
	
	char *array[10];
	int i=1;
	
	array[0] = "./interfaz";
	array[i] = strtok(text," ");

	while(array[i]!=NULL){
		array[++i] = strtok(NULL," ");
	}
	
	otroMain(array,i);

	row = 0;
	for (int i = 0; i < table_lenght; i++)
    {
    	printf("\n\t %d\t %d\t      %d\n", i, mtx[0][i], mtx[1][i]);
    	printf("\t----------------------------");
    	gtk_grid_insert_row (GTK_GRID(grid1), row);
    
	
		
		snprintf(secuencia, 10000, "%d", i);
		snprintf(id, 10000, "%d", mtx[0][i]);
		snprintf(frecuencia, 10000, "%d", mtx[1][i]);
		char* string0 = malloc(strlen(s[ind])+strlen(frecuencia)+1);
		char* string1 = malloc(strlen(s[ind])+strlen(frecuencia)+1);
		char* string2 = malloc(strlen(s[ind])+strlen(frecuencia)+1);
		char* string3 = malloc(strlen(s[ind])+strlen(frecuencia)+1);
		char* string4 = malloc(strlen(s[ind])+strlen(frecuencia)+1);
		char* string5 = malloc(strlen(s[ind])+strlen(frecuencia)+1);

		string0 = strcat(secuencia, "                   ");
		string1 = strcat(string0, id);
		string2 = strcat(string1, "                   ");
		string3 = strcat(string2, "Nombre");
		string4 = strcat(string3, "                   ");
		string5 = strcat(string4, "Descripción");

		button[row] = gtk_button_new_with_label (string5);
		gtk_button_set_alignment (GTK_BUTTON(button[row]), 0.0, 0.5); // hor left, ver center
		gtk_grid_attach (GTK_GRID(grid1), button[row], 1, row, 1, 1);
		//g_signal_connect(button[row], "clicked", G_CALLBACK(on_row), NULL);
		row ++;
	}
		
	mostrar_imagen();
}	

void ejecutar_pausado(GtkButton *ejecutarPausado, gpointer data){
	
	flag = 1;
	if(data != NULL)
		gtk_widget_set_sensitive (ejecutarPausado, FALSE);
	
	const char *text = gtk_entry_get_text(data);
	char strace[50];
	strcpy(strace, "strace -o syscalls.txt "); //Guardar en txt sus syscalls del comando
	strcat(strace,text);
	system(strace);
	crear_tabla();
	crear_csv();
		
	int ind = 0;
	char secuencia[10000];
	char id[10000];
	char frecuencia[10000];
	
	char *array[10];
	int i=2;
	
	array[0] = "./interfaz";
	array[1] = "-V";
	array[i] = strtok(text," ");

	while(array[i]!=NULL){
		array[++i] = strtok(NULL," ");
	}
	
	otroMain(array,i);
	
}

void ejecutar_siguiente_False(GtkWidget *widget){
	gtk_widget_set_sensitive (widget, FALSE);

}

void ejecutar_siguiente_True(GtkWidget *widget){
	gtk_widget_set_sensitive (widget, TRUE);

}

//Esto falta, no sé aún cómo reiniciar la ventana
/**
void reiniciar_ejecucion(GtkButton *reiniciar){
	gtk_main_quit();
	
	interfaz();
}
*/

void interfaz()
{
	window   = 0;
	
	GtkButton *ejecutar;
    GtkEntry *comando;
    
	builder = gtk_builder_new();
	gtk_builder_add_from_file (builder, "glade/interfaz.glade", NULL);
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);
	comando = GTK_ENTRY(gtk_builder_get_object(builder, "comando"));
	ejecutar = GTK_BUTTON(gtk_builder_get_object(builder, "ejecutar"));
	imagen = GTK_WIDGET(gtk_builder_get_object(builder,"imagen"));
	fixed1 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed1"));
	view1 = GTK_WIDGET(gtk_builder_get_object(builder, "view1"));
	grid1 = GTK_WIDGET(gtk_builder_get_object(builder, "grid1"));
	g_signal_connect (siguiente, "clicked", G_CALLBACK (ejecutar_siguiente_False), NULL);
		
    gtk_builder_connect_signals(builder, NULL);
  
	g_object_unref(builder);

	gtk_widget_show(window);


	gtk_main();
	return EXIT_SUCCESS;
}

void	on_row(GtkButton *b) {
	printf("Selecció: %s\n", gtk_button_get_label (b));
}

void	on_destroy() { 
		gtk_main_quit();
}


//Realmente funciona únicamente con ENTER
void any_key()
{
	if (flag == 1){
		printf("Presione Enter para continuar: \n");  
		getchar();//pause(2);//getchar();
	}
}

// Imprime la tabla de syscalls acumulados
void print_table()
{

	printf("\n\t       Tabla Acumulada\t\n");
	printf("\t----------------------------");
	printf("\n\t i\tSysCall\t   Cantidad\n");
	printf("\t----------------------------");

    for (int i = 0; i < table_lenght; i++)
    {
    	printf("\n\t %d\t %d\t      %d\n", i, mtx[0][i], mtx[1][i]);
    	printf("\t----------------------------");
    }
    printf("\n");
}

// Busca un lugar en mtx para alojar un nuevo syscall o incrementar sus usos
int find_place(int pid)
{

	int i = 0;

	for (i; i < PIDS; i++){

		if (mtx[0][i] == pid || mtx[0][i] == -1) {
			mtx[1][i]++;
			break;
		}
	}

	if (i >= table_lenght) table_lenght = i+1;

	return i;
}

int create_child(int argc, char **argv) {

    char *args [argc+1];
    
    memcpy(args, argv, argc * sizeof(char*));
    args[argc] = NULL;
    ptrace(PTRACE_TRACEME);
    kill(getpid(), SIGSTOP);

    return execvp(args[0], args);
}


int wait_for_syscall(pid_t child) {
    int status;

    while (1) {

        ptrace(PTRACE_SYSCALL, child, 0, 0);
        waitpid(child, &status, 0);

        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80) return 0;
        
        if (WIFEXITED(status)) return 1;
    }
}

int trace(pid_t child) 
{

    int status, syscall, retval;
    waitpid(child, &status, 0);
    ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD);

    while(1) 
    {
    	if (wait_for_syscall(child) != 0) break;

        syscall = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*ORIG_RAX);

        int place = find_place(syscall); //Busca un ligar en mtx[0] y lo ubica o suma
        mtx[0][place] = syscall;		 // la cantidad de PIDS del contador en mtx[1]

        if (flag != -1) fprintf(stderr, "Syscall(%d) = ", syscall); // Si es -v imprima datos del syscall

        if (wait_for_syscall(child) != 0) break;

        retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*RAX); // Si es -v imprima valor de retorno del syscall

        if (flag != -1) fprintf(stderr, "%d\n", retval);

        //any_key();
        gboolean button_state;
        
        button_state = gtk_toggle_button_get_active(siguiente); //Aquí quiero saber cómo agarrar el botón como global y modificrlo
        while(button_state){
			
		}
		
		ejecutar_siguiente_True(siguiente);
    }

    return 0;
}

int otroMain(char **argv,int argc) {
	
    if (argc < 2) { // Debe haber argumentos en el comando de ejecución
        fprintf(stderr, "Flatan argumentos: %s program arg1 arg2 arg3...\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1],"-v") == 0) flag = 0; // -v

    if (strcmp(argv[1],"-V") == 0) flag = 1; // -V


    for (int i=0; i < PIDS; i++){ // Inicializa mtx
    	mtx[0][i] = -1;
    	mtx[1][i] = 0;
    }

    pid_t child = fork(); // Obtengo el PID del proceso hijo
    if (child == 0) {

    	if (flag >= 0) create_child(argc-2, argv+2); // Si hay -v o -V, se corrigen argv y argc
    	else create_child(argc-1, argv+1); // Se ignora ./rastreador

    } else {
        trace(child);
    }
    printf(" \n \n");

    print_table();
    
    return 0;
}

int main (int argc, char *argv[]) {
	gtk_init(&argc, &argv);
	interfaz();
	return 0;

}
