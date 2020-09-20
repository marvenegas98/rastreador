#include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

static GtkWidget* window;
static GtkWidget* imagen;
static GtkBuilder *builder;
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
	printf("lst idx %i\n",lst_idx);

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
	mostrar_imagen();
}	

void interfaz()
{
	window   = 0;
	
	GtkButton *ejecutar;
    GtkEntry *comando;
    
	builder = gtk_builder_new();
	gtk_builder_add_from_file (builder, "glade/interfaz.glade", NULL);
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	comando = GTK_ENTRY(gtk_builder_get_object(builder, "comando"));
	ejecutar = GTK_BUTTON(gtk_builder_get_object(builder, "ejecutar"));
	imagen=GTK_WIDGET(gtk_builder_get_object(builder,"imagen"));
    gtk_builder_connect_signals(builder, NULL);
  
	g_object_unref(builder);

	gtk_widget_show(window);


	gtk_main();
	
}
int main (int argc, char *argv[]) {
	gtk_init(&argc, &argv);
	interfaz();
	return 0;

}
