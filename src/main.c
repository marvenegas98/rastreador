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
int global_idx=0;
int elementos=0;
static GtkWidget* window;
static GtkWidget* imagen;
static GtkBuilder *builder;
GtkWidget *fixed1;
GtkWidget *grid1;
GtkWidget *grid2; 
GtkWidget *label[1000];
GtkWidget *button[1000];
GtkWidget *view1;
GtkButton *siguiente;
GtkButton *pausado;
GtkButton *ejecutar;
GtkButton *reiniciar;
GtkEntry *comando;
pid_t son;
int returnval;
int llamada;

void on_destroy(); 
void on_row(GtkButton *);

char tmp[1024]; // general use
int	row=0;

char** s;
char** syscalls;
char** sysnames;
int frec[250];	
int lst_idx = 0;

void actualizar(){
	while(gtk_events_pending()){
		gtk_main_iteration_do(1);
	}
}
void crear_csv(){
	FILE *fp;
	fp=fopen("tabla.csv","w+");
	fprintf(fp,"Nombre, Frecuencia\n");
	for(int i=0;i<table_lenght;i++){
		fprintf(fp,"%s , %d\n",sysnames[i], frec[i]);
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

// Va a ingresar una cadena a partir, dividida por espacios.
char** split_command(char* cadena) {

	char pivote[50]; //Variable para contar la cantidad de elementos.

	strcpy(pivote, cadena);
	char * token = strtok(pivote, " ");
	// Esto es solo para contar y sacar el largo de la lista final.
	while( token != NULL ) {
		token = strtok(NULL, " ");
		elementos++;
	}
	elementos+=1;
	int i = 0;
    char *elemento = strtok (cadena, " ");
    char **lista;
    lista = malloc(elementos * sizeof(char*));
	lista[0] = "./interfaz";
	i++;

    while (elemento != NULL)
    {
        lista[i++] = elemento;
        elemento = strtok (NULL, " ");
    }
        
        
   return lista;
}

void ejecutar_comando(GtkButton *ejecutar, gpointer data){
	flag=0;
	const char *text;
    text = gtk_entry_get_text(data);
	char** args = split_command(text);
	sysnames = (char **)calloc(250,sizeof(char **));
	for(int i = 0; i < 250; i++){
        sysnames[i] = (char *)malloc(20);
		sysnames[i] = "";
    }
	//crear_tabla();
	otroMain(args,elementos);
	//llenar_bitacora();
	crear_csv();
	mostrar_imagen();
}	

void ejecutar_pausado(GtkButton *ejecutarPausado, gpointer data){
	flag=1;
	const char *text;
    text = gtk_entry_get_text(data);
	char** args = split_command(text);
	sysnames = (char **)calloc(250,sizeof(char **));
	for(int i = 0; i < 250; i++){
        sysnames[i] = (char *)malloc(20);
		sysnames[i] = "";
    }
	//crear_tabla();
	otroMain(args,elementos);
	//llenar_bitacora();
	//crear_csv();
	//mostrar_imagen();
	
}


void reiniciar_ejecucion(GtkButton *reiniciar){
	system("rm piechart.png");
	system("rm tabla.csv");
	for(int i=0;i<row;i++){
		if(i<table_lenght){
			gtk_grid_remove_row(grid2,i);
		}
		gtk_grid_remove_row(grid1,i);
	}
	gtk_entry_set_text(comando, "");
	gtk_widget_show_all(window);
	actualizar();


}


void interfaz()
{
	window   = 0;

    
	builder = gtk_builder_new();
	gtk_builder_add_from_file (builder, "glade/interfaz.glade", NULL);
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);
	comando = GTK_ENTRY(gtk_builder_get_object(builder, "comando"));
	ejecutar = GTK_BUTTON(gtk_builder_get_object(builder, "ejecutar"));
	siguiente = GTK_BUTTON(gtk_builder_get_object(builder, "siguiente"));
	pausado = GTK_BUTTON(gtk_builder_get_object(builder, "pausado"));
	imagen = GTK_WIDGET(gtk_builder_get_object(builder,"imagen"));
	fixed1 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed1"));
	view1 = GTK_WIDGET(gtk_builder_get_object(builder, "view1"));
	grid1 = GTK_WIDGET(gtk_builder_get_object(builder, "grid1"));
	grid2 = GTK_WIDGET(gtk_builder_get_object(builder, "grid2"));
	reiniciar = GTK_WIDGET(gtk_builder_get_object(builder, "reiniciar"));
		
    gtk_builder_connect_signals(builder, NULL);
  
	g_object_unref(builder);

	gtk_widget_show(window);


	gtk_main();
	return EXIT_SUCCESS;
}

void on_destroy() { 
		gtk_main_quit();
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

void insertar_tabla(char* sysname,int idx, int freq){
	static GtkWidget* fila[2];
	for( int j = 0; j <2; j++)
		{
			fila[j] = gtk_entry_new();
			
		}

	if(freq!=1){
		gtk_grid_remove_row(grid2,idx);
	}
	sysnames[idx]=sysname;
	gtk_grid_insert_row(GTK_GRID(grid2), idx);
	//insertar entry con nombre
	gtk_entry_set_max_length(GTK_ENTRY(fila[0]),15);
	gtk_widget_set_size_request(fila[0],1,1);

	gtk_entry_set_text(GTK_ENTRY(fila[0]),sysname);
	gtk_grid_attach(GTK_GRID(grid2),fila[0],1,idx,1,1);
	//insertar entry con frec
	gtk_entry_set_max_length(GTK_ENTRY(fila[1]),4);
	gtk_widget_set_size_request(fila[1],1,1);
	char frequ[4];
	sprintf(frequ,"%d",freq);
	gtk_entry_set_text(GTK_ENTRY(fila[1]),frequ);
	gtk_grid_attach (GTK_GRID(grid2),fila[1],2,idx,1,1);

	gtk_widget_show_all(window);
	actualizar();
	//sleep(0.75);
}

// Busca un lugar en mtx para alojar un nuevo syscall o incrementar sus usos
int find_place(int pid, char* sysname)
{

	int i = 0;

	for (i; i < PIDS; i++){

		if ((mtx[0][i] == pid)||(mtx[0][i] == -1)){
			mtx[1][i]++;
			frec[i]+=1;
			insertar_tabla(sysname,i,mtx[1][i]);
			break;
		}
	}

	if (i >= table_lenght) table_lenght = i+1;

	return i;
}
char * get_syscall_name(int syscall_id){
	
	// Variables para guardar el nombre.
	char *name;
    name = malloc(50 * sizeof(char*));
    // Variables para castaer el syscall_id
	char id_str[50];
	sprintf(id_str, "%d", syscall_id);
	// Variables y construccion del comando a ejecutar
	char cmd[20];
	strcpy(cmd, "ausyscall ");
	strcat(cmd, id_str);
	//Variables para almacenar el resultado del comando
	FILE *fp;
	char ruta[1035];

	fp = popen(cmd, "r"); // Abre el archivo creado con el resultado del comando
	
	// Lee el archivo y guarda el nombre
	while (fgets(ruta, sizeof(ruta), fp) != NULL) {
		//ruta[sizeof(ruta)]='p';
		strcpy(name, ruta);
		int sz=sizeof(name)-2;
		name[strlen(name)-1]=' ';
		
    }
    
    pclose(fp);
	printf(name);
    printf("aqui %i\n",sizeof(name));
    return name;
}
int ktrace(pid_t hijo, int syscall, int retval){
	if (wait_for_syscall(hijo) != 0){
		crear_csv();
		mostrar_imagen();
	}

	syscall = ptrace(PTRACE_PEEKUSER, hijo, sizeof(long)*ORIG_RAX);
	char* sysname = get_syscall_name(syscall);
	insertar_bitacora(syscall,sysname);
	int place = find_place(syscall,sysname); //Busca un ligar en mtx[0] y lo ubica o suma
	mtx[0][place] = syscall;		 // la cantidad de PIDS del contador en mtx[1]

	if (flag != -1) fprintf(stderr, "Syscall(%d) = ", syscall); //  imprima datos del syscall

	if (wait_for_syscall(hijo) != 0){
		crear_csv();
		mostrar_imagen();
	}

	retval = ptrace(PTRACE_PEEKUSER, hijo, sizeof(long)*RAX); // imprima valor de retorno del syscall

	if (flag != -1) fprintf(stderr, "%d\n", retval);
}
void ejecutar_siguiente(GtkButton *ejecutar, gpointer data){
	ktrace(son,llamada,returnval);
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

void insertar_bitacora(int syscall, char* sysname) {
	static GtkWidget* fila[4];
	for( int j = 0; j <4; j++)
		{
			fila[j] = gtk_entry_new();
			
		}
	gtk_grid_insert_row(GTK_GRID(grid1), row);

	//insertar entry con secuencia
	gtk_entry_set_max_length(GTK_ENTRY(fila[0]),4);
	gtk_widget_set_size_request(fila[0],1,1);
	char global_index[4];
	sprintf(global_index,"%d",row);
	gtk_entry_set_text(GTK_ENTRY(fila[0]),global_index);
	gtk_grid_attach(GTK_GRID(grid1),fila[0],1,row,1,1);

	//insertar entry con id
	gtk_entry_set_max_length(GTK_ENTRY(fila[1]),4);
	gtk_widget_set_size_request(fila[1],1,1);
	char sysid[4];
	sprintf(sysid,"%d", syscall);
	gtk_entry_set_text(GTK_ENTRY(fila[1]),sysid);
	gtk_grid_attach (GTK_GRID(grid1),fila[1],2,row,1,1);

	//insertar entry con nombre
	gtk_entry_set_max_length(GTK_ENTRY(fila[2]),10);
	gtk_widget_set_size_request(fila[2],1,5);
	gtk_entry_set_text(GTK_ENTRY(fila[2]),sysname);
	gtk_grid_attach (GTK_GRID(grid1),fila[2],3,row,1,1);

	//insertar entry con descripcion
	gtk_entry_set_max_length(GTK_ENTRY(fila[3]),19);
	gtk_widget_set_size_request(fila[3],1,5);
	gtk_entry_set_text(GTK_ENTRY(fila[3]),"descripcion");
	gtk_grid_attach (GTK_GRID(grid1),fila[3],4,row,1,1);

	gtk_widget_show_all(window);
	actualizar();
	sleep(0.5);
	row ++;
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
		char* sysname = get_syscall_name(syscall);
		insertar_bitacora(syscall,sysname);
        int place = find_place(syscall,sysname); //Busca un ligar en mtx[0] y lo ubica o suma
        mtx[0][place] = syscall;		 // la cantidad de PIDS del contador en mtx[1]

        if (flag != -1) fprintf(stderr, "Syscall(%d) = ", syscall); //  imprima datos del syscall

        if (wait_for_syscall(child) != 0) break;

        retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*RAX); // imprima valor de retorno del syscall

        if (flag != -1) fprintf(stderr, "%d\n", retval);

        //any_key();
		/*
        gboolean button_state;
        
        button_state = gtk_toggle_button_get_active(siguiente);
        while(button_state){
			
		}
		
		gtk_widget_set_sensitive (siguiente, TRUE);
		*/
		son = child;
		returnval=retval;
		llamada=syscall;
		if (flag==1){
			break;
		}
    }

    return 0;
}

int otroMain(char **argv,int argc) {

    for (int i=0; i < PIDS; i++){ // Inicializa mtx
    	mtx[0][i] = -1;
    	mtx[1][i] = 0;
    }

    pid_t child = fork(); // Obtengo el PID del proceso hijo
    if (child == 0) {
    	 create_child(argc-1, argv+1); // Se ignora ./rastreador

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
