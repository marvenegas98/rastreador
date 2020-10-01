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
static GtkImage* imagen;
static GtkBuilder *builder;
GtkWidget *fixed1;
GtkGrid *grid1;
GtkGrid *grid2; 
GtkGrid *grid3; 
GtkWidget *label[1000];
GtkWidget *button[1000];
GtkWidget *view1;
GtkButton *siguiente;
GtkButton *pausado;
GtkButton *ejecutar;
GtkButton *reiniciar;
GtkEntry *comando;
static GtkLabel *bitacora;
static GtkLabel *acumulada;

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
int frec[1000];	
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
	int k=system("gnuplot pastel.gnuplot");
	const gchar * filename = "piechart.png";
	gtk_image_set_from_file(GTK_IMAGE(imagen), filename);
	gtk_widget_show_all(window);
	actualizar();

}
char** split_description(char* cadena, char* sep) {

	char pivote[50]; //Variable para contar la cantidad de elementos.

	strcpy(pivote, cadena);
	char * token = strtok(pivote, sep);
	int elementos = 0;
	// Esto es solo para contar y sacar el largo de la lista final.
	while( token != NULL ) {
		token = strtok(NULL, sep);
		elementos++;
	}
  
	int i = 0;
    char *elemento;
	elemento=strtok (cadena, sep);
    char **lista;
    lista = malloc(elementos * sizeof(char*));

    while (elemento != NULL)
    {
        lista[i++] = elemento;
        elemento = strtok (NULL, sep);
    }
   return lista;
}

char *get_syscall_desc(int syscall_id) {
	
	
	FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

   fp = fopen("Syscall_desc", "r");
   
   if (fp == NULL){
	   printf("SE fue a la verga abriendo el archivo");
	   exit(EXIT_FAILURE);
	  }

	int x = 0;
	while (((read = getline(&line, &len, fp)) != -1) && (x < syscall_id)){
		//printf("Retrieved line of length %zu :\n", read);
        //printf("%s", line);
        x++;
    }
    
	char **recibido = split_description(line, ",");
    
    return recibido[1];
    
}
//flag1 = 0  -> ordenar lista de enteros
//flag1 = 1  -> ordenar lista de strings

//flag2 = 0  -> orden ascendente (default)
//flag2 = 1  -> orden descendente
void ordenar(int *lista_ids, char **lista, int tam, int flag1, int flag2) {
	
	char * aux2;
	int i, j, aux;
	
	switch (flag1){
		
		case 0:
		
			for (j=1;j<=tam-1;j++){
				for (i=0;i<=tam-2;i++){
					if (lista_ids[i]>lista_ids[i+1]){

						aux=lista_ids[i];
						aux2=lista[i];
						lista_ids[i] = lista_ids[i+1];
						lista[i] = lista[i+1];
						lista_ids[i+1] = aux;
						lista[i+1] = aux2;

					}
				}
			}
			
			if (flag2 == 1){
				char *copia[tam];
				int copia2[tam];
				int y = tam - 1;
				
				for (int x = 0; x<=tam-1; x++){
					copia[y] = lista[x];
					copia2[y] = lista_ids[x];
					y--;
				}
				
				for (i=0;i<=tam-1;i++ ){
					lista[i] = copia[i];
				}
				
				for (i=0;i<=tam-1;i++ ){
					lista_ids[i] = copia2[i];
				}
				
			}
			break;
			
			
		case 1:

				for(i=0;i<tam;i++){
					for(j=i+1;j<tam;j++){
						if(strcmp(lista[i],lista[j])>0){
							
							aux=lista_ids[i];
							aux2=lista[i];
							lista_ids[i] = lista_ids[j];
							lista[i] = lista[j];
							lista_ids[j] = aux;
							lista[j] = aux2;
							
						}
					}
				}
				
				if (flag2 == 1){
				char *copia[tam];
				int copia2[tam];
				int y = tam - 1;
				
				for (int x = 0; x<=tam-1; x++){
					copia[y] = lista[x];
					copia2[y] = lista_ids[x];
					y--;
				}
				
				for (i=0;i<=tam-1;i++ ){
					lista[i] = copia[i];
				}
				
				for (i=0;i<=tam-1;i++ ){
					lista_ids[i] = copia2[i];
				}
				
			}
      
			break;
	}

	for (int i=0;i<tam;i++ ){
		gtk_grid_remove_row(grid2,0);
	}

	for (int i=0;i<tam;i++ ){
		static GtkWidget* fila[2];
		for( int j = 0; j <2; j++)
			{
				fila[j] = gtk_entry_new();
				
			}
		gtk_grid_insert_row(GTK_GRID(grid2), i);

		//insertar entry con nombre
		gtk_entry_set_max_length(GTK_ENTRY(fila[0]),15);
		gtk_entry_set_alignment(GTK_ENTRY(fila[0]),0.5);
		gtk_widget_set_size_request(fila[0],1,1);
		gtk_entry_set_text(GTK_ENTRY(fila[0]),lista[i]);
		gtk_grid_attach(GTK_GRID(grid2),fila[0],1,i,1,1);

		//insertar entry con frec
		gtk_entry_set_max_length(GTK_ENTRY(fila[1]),4);
		gtk_entry_set_alignment(GTK_ENTRY(fila[1]),0.5);
		gtk_widget_set_size_request(fila[1],1,1);
		char frequ[4];
		sprintf(frequ,"%d",lista_ids[i]);
		gtk_entry_set_text(GTK_ENTRY(fila[1]),frequ);
		gtk_grid_attach (GTK_GRID(grid2),fila[1],2,i,1,1);
		
	}
	gtk_widget_show_all(window);
	actualizar();
}

// Va a ingresar una cadena a partir, dividida por espacios.
char** split_command(const char* cadena) {

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


void reiniciar_ejecucion(GtkButton *reiniciar){
	int k= system("rm piechart.png");
	k=system("rm tabla.csv");
	for(int i=0;i<row;i++){
		gtk_grid_remove_row(grid1,0);
	}

	for(int i=0;i<table_lenght;i++){
		gtk_grid_remove_row(grid2,0);
	}
	sysnames = (char **)calloc(1000,sizeof(char **));
	for(int i = 0; i <1000; i++){
        sysnames[i] = (char *)malloc(20);
		sysnames[i] = "";
    }
	for (int i=0; i < PIDS; i++){ // Inicializa mtx
    	mtx[0][i] = -1;
    	mtx[1][i] = 0;
    }

	for(int i=0;i<1000;i++){
		frec[i]=0;
	}
	gtk_label_set_text(bitacora,"");
	gtk_label_set_text(acumulada,"");
	flag = -1; // 0 -v , 1 -V
	global_idx=0;
	elementos=0;
	row=0;	
	lst_idx = 0;
	table_lenght=0;
	gtk_widget_hide(imagen);
	gtk_image_clear(imagen);
	gtk_entry_set_text(comando, "");
	gtk_widget_set_sensitive (siguiente, FALSE);
	gtk_widget_set_sensitive (pausado, TRUE);
	gtk_widget_set_sensitive (ejecutar, TRUE);
	gtk_widget_set_sensitive (reiniciar, FALSE);
	actualizar();
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
	gtk_entry_set_alignment(GTK_ENTRY(fila[0]),0.5);
	gtk_widget_set_size_request(fila[0],1,1);
	gtk_entry_set_text(GTK_ENTRY(fila[0]),sysname);
	gtk_grid_attach(GTK_GRID(grid2),fila[0],1,idx,1,1);

	//insertar entry con frec
	gtk_entry_set_max_length(GTK_ENTRY(fila[1]),4);
	gtk_entry_set_alignment(GTK_ENTRY(fila[1]),0.5);
	gtk_widget_set_size_request(fila[1],1,1);
	char frequ[4];
	sprintf(frequ,"%d",freq);
	gtk_entry_set_text(GTK_ENTRY(fila[1]),frequ);
	gtk_grid_attach (GTK_GRID(grid2),fila[1],2,idx,1,1);

	gtk_widget_show_all(window);
	actualizar();
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
	gtk_entry_set_alignment(GTK_ENTRY(fila[0]),0.5);
	gtk_widget_set_size_request(fila[0],1,1);
	char global_index[4];
	sprintf(global_index,"%d",row);
	gtk_entry_set_text(GTK_ENTRY(fila[0]),global_index);
	gtk_grid_attach(GTK_GRID(grid1),fila[0],1,row,1,1);

	//insertar entry con id
	gtk_entry_set_max_length(GTK_ENTRY(fila[1]),4);
	gtk_entry_set_alignment(GTK_ENTRY(fila[1]),0.5);
	gtk_widget_set_size_request(fila[1],1,1);
	char sysid[4];
	sprintf(sysid,"%d", syscall);
	gtk_entry_set_text(GTK_ENTRY(fila[1]),sysid);
	gtk_grid_attach (GTK_GRID(grid1),fila[1],2,row,1,1);

	//insertar entry con nombre
	gtk_entry_set_max_length(GTK_ENTRY(fila[2]),10);
	gtk_entry_set_alignment(GTK_ENTRY(fila[2]),0.5);
	gtk_widget_set_size_request(fila[2],1,5);
	gtk_entry_set_text(GTK_ENTRY(fila[2]),sysname);
	gtk_grid_attach (GTK_GRID(grid1),fila[2],3,row,1,1);

	//insertar entry con descripcion
	char *recibido = get_syscall_desc(syscall);
	recibido[strlen(recibido)-1]=' ';
	gtk_entry_set_max_length(GTK_ENTRY(fila[3]),35);
	gtk_entry_set_alignment(GTK_ENTRY(fila[3]),0.5);
	gtk_widget_set_size_request(fila[3],1,5);
	gtk_entry_set_text(GTK_ENTRY(fila[3]),recibido);
	gtk_grid_attach (GTK_GRID(grid1),fila[3],4,row,1,1);

	gtk_widget_show_all(window);
	actualizar();
	sleep(0.8);
	row ++;
}

// Busca un lugar en mtx para alojar un nuevo syscall o incrementar sus usos
int find_place(int pid, char* sysname)
{

	int i = 0;

	for(i=0; i < PIDS; i++){

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
		name[strlen(name)-1]=' ';
		
    }
    
    pclose(fp);
    return name;
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
void ktrace(pid_t hijo, int syscall, int retval){
	if (wait_for_syscall(hijo) != 0){
		char total[25]="Total de Syscalls: ";
		char t[4];
		sprintf(t, "%i",row);
		strcat(total, t);
		char totala[27]="Syscalls Distintos: ";
		char ta[4];
		sprintf(ta, "%i",table_lenght);
		strcat(totala, ta);
		gtk_label_set_text(bitacora,total);
		gtk_label_set_text(acumulada,totala);
		gtk_widget_show_all(window);
		actualizar();
		crear_csv();
		mostrar_imagen();
		gtk_widget_set_sensitive (siguiente, FALSE);
		gtk_widget_set_sensitive (pausado, FALSE);
		gtk_widget_set_sensitive (ejecutar, FALSE);
		gtk_widget_set_sensitive (reiniciar, TRUE);
	}

	syscall = ptrace(PTRACE_PEEKUSER, hijo, sizeof(long)*ORIG_RAX);
	char* sysname = get_syscall_name(syscall);
	insertar_bitacora(syscall,sysname);
	int place = find_place(syscall,sysname); //Busca un ligar en mtx[0] y lo ubica o suma
	mtx[0][place] = syscall;		 // la cantidad de PIDS del contador en mtx[1]

	if (wait_for_syscall(hijo) != 0){
		char total[25]="Total de Syscalls: ";
		char t[4];
		sprintf(t, "%i",row);
		strcat(total, t);
		char totala[27]="Syscalls Distintos: ";
		char ta[4];
		sprintf(ta, "%i",table_lenght);
		strcat(totala, ta);
		gtk_label_set_text(bitacora,total);
		gtk_label_set_text(acumulada,totala);
		gtk_widget_show_all(window);
		actualizar();
		crear_csv();
		mostrar_imagen();
		gtk_widget_set_sensitive (siguiente, FALSE);
		gtk_widget_set_sensitive (pausado, FALSE);
		gtk_widget_set_sensitive (ejecutar, FALSE);
		gtk_widget_set_sensitive (reiniciar, TRUE);
	}

	retval = ptrace(PTRACE_PEEKUSER, hijo, sizeof(long)*RAX); // imprima valor de retorno del syscall

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

        if (wait_for_syscall(child) != 0) break;

        retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*RAX); // imprima valor de retorno del syscall

		son = child;
		returnval=retval;
		llamada=syscall;
		if (flag==1){
			break;
		}
    }

    return 0;
}
void ejecutar_siguiente(GtkButton *ejecutar, gpointer data){
	ktrace(son,llamada,returnval);
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
void ejecutar_comando(GtkButton *ejecutar, gpointer data){
	flag=0;
	gtk_widget_set_sensitive (siguiente, FALSE);
	gtk_widget_set_sensitive (pausado, FALSE);
	gtk_widget_set_sensitive (ejecutar, FALSE);
	const char *text;
    text = gtk_entry_get_text(data);
	char** args = split_command(text);
	sysnames = (char **)calloc(1000,sizeof(char **));
	for(int i = 0; i < 1000; i++){
        sysnames[i] = (char *)malloc(20);
		sysnames[i] = "";
    }
	otroMain(args,elementos);
	char total[25]="Total de Syscalls: ";
	char t[4];
	sprintf(t, "%i",row);
	strcat(total, t);
	char totala[27]="Syscalls Distintos: ";
	char ta[4];
	sprintf(ta, "%i",table_lenght);
	strcat(totala, ta);
	gtk_label_set_text(bitacora,total);
	gtk_label_set_text(acumulada,totala);
	gtk_widget_show_all(window);
	actualizar();
	crear_csv();
	mostrar_imagen();
	gtk_widget_set_sensitive (reiniciar, TRUE);
	
}	

void ejecutar_pausado(GtkButton *pausado, gpointer data){
	flag=1;
	gtk_widget_set_sensitive (ejecutar, FALSE);
	gtk_widget_set_sensitive (pausado, FALSE);
	gtk_widget_set_sensitive (siguiente, TRUE);
	const char *text;
    text = gtk_entry_get_text(data);
	char** args = split_command(text);
	sysnames = (char **)calloc(1000,sizeof(char **));
	for(int i = 0; i < 1000; i++){
        sysnames[i] = (char *)malloc(20);
		sysnames[i] = "";
    }
	otroMain(args,elementos);
	
}
void on_destroy() { 
		gtk_main_quit();
}
//flag1 = 0  -> ordenar lista de enteros
//flag1 = 1  -> ordenar lista de strings

//flag2 = 0  -> orden ascendente (default)
//flag2 = 1  -> orden descendente
void nomasc_clicked(){
	ordenar(frec,sysnames,table_lenght,1,0);
}
void nomdesc_clicked(){
	ordenar(frec,sysnames,table_lenght,1,1);
}
void frecasc_clicked(){
	ordenar(frec,sysnames, table_lenght, 0,0);
}
void frecdesc_clicked(){
	ordenar(frec,sysnames,table_lenght,0,1);
}
int interfaz()
{
	GtkButton* nomasc;
	GtkButton* nomdesc;

	GtkButton* frecasc;
	GtkButton* frecdesc;

	GtkButton* destroy;

	window   = 0;

    
	builder = gtk_builder_new();
	gtk_builder_add_from_file (builder, "glade/interfaz.glade", NULL);
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);
	comando = GTK_ENTRY(gtk_builder_get_object(builder, "comando"));
	ejecutar = GTK_BUTTON(gtk_builder_get_object(builder, "ejecutar"));
	siguiente = GTK_BUTTON(gtk_builder_get_object(builder, "siguiente"));
	pausado = GTK_BUTTON(gtk_builder_get_object(builder, "pausado"));
	imagen = GTK_IMAGE(gtk_builder_get_object(builder,"imagen"));
	fixed1 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed1"));
	view1 = GTK_WIDGET(gtk_builder_get_object(builder, "view1"));
	grid1 = GTK_GRID(gtk_builder_get_object(builder, "grid1"));
	grid2 = GTK_GRID(gtk_builder_get_object(builder, "grid2"));
	grid3 = GTK_GRID(gtk_builder_get_object(builder, "grid3"));
	reiniciar = GTK_BUTTON(gtk_builder_get_object(builder, "reiniciar"));
	bitacora = GTK_LABEL(gtk_builder_get_object(builder, "bitacora"));
	acumulada = GTK_LABEL(gtk_builder_get_object(builder, "acumulada"));
	
	nomasc = GTK_BUTTON(gtk_builder_get_object(builder, "nomasc"));
	nomdesc = GTK_BUTTON(gtk_builder_get_object(builder, "nomdesc"));
	frecasc = GTK_BUTTON(gtk_builder_get_object(builder, "frecasc"));
	frecdesc = GTK_BUTTON(gtk_builder_get_object(builder, "frecdesc"));

	destroy = GTK_BUTTON(gtk_builder_get_object(builder, "terminar_ejecucion"));

	gtk_widget_set_sensitive (reiniciar, FALSE);
	gtk_widget_set_sensitive (siguiente, FALSE);
    gtk_builder_connect_signals(builder, NULL);
  
	g_object_unref(builder);

	gtk_widget_show(window);


	gtk_main();
	return EXIT_SUCCESS;
}

int main (int argc, char *argv[]) {
	gtk_init(&argc, &argv);
	interfaz();
	return 0;

}