#include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

static GtkWidget* window;
static GtkWidget* imagen;
static GtkBuilder *builder;
GtkWidget *fixed1;
GtkWidget *grid1; 
GtkWidget *label[1000];
GtkWidget *button[1000];
GtkWidget *view1;

void on_destroy(); 
void on_row(GtkButton *);

char tmp[1024]; // general use
int	row;

char** s;
int frec[250];	
int lst_idx = 0;
int bandera1 = 0;
int indice = 0;
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
	char frecuencia[10000];
	char stringCount[10000];

	row = 0;
	int count = 0;
	while (ind<lst_idx-1) {
		gtk_grid_insert_row (GTK_GRID(grid1), row);
		
		snprintf(frecuencia, 10000, "%d", frec[ind]);
		snprintf(stringCount, 10000, "%d", count);
		char* string0 = malloc(strlen(s[ind])+strlen(frecuencia)+1);
		char* string1 = malloc(strlen(s[ind])+strlen(frecuencia)+1);
		char* string2 = malloc(strlen(s[ind])+strlen(frecuencia)+1);
		char* string3 = malloc(strlen(s[ind])+strlen(frecuencia)+1);
		char* string4 = malloc(strlen(s[ind])+strlen(frecuencia)+1);
		char* string5 = malloc(strlen(s[ind])+strlen(frecuencia)+1);
		string0 = strcat(stringCount, "                   ");
		string1 = strcat(string0, frecuencia);
		string2 = strcat(string1,"                   ");
		string3 = strcat(string2,s[ind]);
		string4 = strcat(string3,"                   ");
		string5 = strcat(string4,"Descripcion");

		button[row] = gtk_button_new_with_label (string5);
		gtk_button_set_alignment (GTK_BUTTON(button[row]), 0.0, 0.5); // hor left, ver center
		gtk_grid_attach (GTK_GRID(grid1), button[row], 1, row, 1, 1);
		g_signal_connect(button[row], "clicked", G_CALLBACK(on_row), NULL);
		row ++;
		ind ++;
		count ++;
	}
	mostrar_imagen();
}	

void ejecutar_pausado(GtkButton *ejecutarPausado, gpointer data){
    const char *text = gtk_entry_get_text(data);
	char strace[50];
	strcpy(strace, "strace -o syscalls.txt "); //Guardar en txt sus syscalls del comando
	strcat(strace,text);
	system(strace);
	crear_tabla();
	//gtk_widget_set_sensitive (ejecutarPausado, FALSE);
}

void ejecutar_siguiente(GtkButton *siguiente){
    FILE *fp;
	fp=fopen("tabla.csv","a");
	if (bandera1 == 0){
		fprintf(fp,"Nombre, Frecuencia");
		bandera1 = 1;
	}
	if(indice<lst_idx-1){
		fprintf(fp,"\n%s , %i",s[indice], frec[indice]);
		indice += 1;
	}
		
	else
		gtk_widget_set_sensitive (siguiente, FALSE);	
	
	fclose(fp);
	mostrar_imagen();
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

int main (int argc, char *argv[]) {
	gtk_init(&argc, &argv);
	interfaz();
	return 0;

}
