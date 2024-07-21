#include <gtk/gtk.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <bsd/string.h>

void listdir(const char *name, int indent)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == 4) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            printf("%*s[%s]\n", indent, "", entry->d_name);
            listdir(path, indent + 2);
        } else {
            printf("%*s- %s\n", indent, "", entry->d_name);
        }
    }
    closedir(dir);
}

enum {
   TITLE_COLUMN,
   AUTHOR_COLUMN,
   CHECKED_COLUMN,
   N_COLUMNS
};

void populate_tree_model(GtkTreeStore *store){

 //DIR *dir;
 //struct dirent *entry;
 //if (!(dir = opendir(name)))
 // return;

      
 GtkTreeIter iter1;  /* Parent iter1 */
 GtkTreeIter iter3;  /* Parent iter3 */
 GtkTreeIter iter2;  /* Child iter  */
 
 gtk_tree_store_append (store, &iter1, NULL);  
 gtk_tree_store_set (store, &iter1,
                    0, "The Art of Computer Programming",
                    1, "Knuth",
                    2, TRUE,
                    3, "BBBB",
                    -1);

 gtk_tree_store_append (store, &iter3, NULL); 
 gtk_tree_store_set (store, &iter3,
                    0, "Fundamental Algorithms",
                    1, "Donald",
                    2, FALSE,
                    3, "bbbb",
                    -1);
 
 
 /*
 gtk_tree_store_append (store, &iter2, &iter1);  // Acquire a child iterator
 gtk_tree_store_set (store, &iter2,
                    TITLE_COLUMN, "Volume 1: Fundamental Algorithms",
                    -1);

 gtk_tree_store_append (store, &iter2, &iter1);
 gtk_tree_store_set (store, &iter2,
                    TITLE_COLUMN, "Volume 2: Seminumerical Algorithms",
                    -1);

 gtk_tree_store_append (store, &iter2, &iter1);
 gtk_tree_store_set (store, &iter2,
                    TITLE_COLUMN, "Volume 3: Sorting and Searching",
                    -1);
*/
}


static void activate (GtkApplication* app, gpointer user_data)
{
  GtkWidget *window;
  //GtkWidget *button;
  GtkTreeStore *store;
  GtkWidget *tree;

  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 400, 400);

  store = gtk_tree_store_new (4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING);
  

  populate_tree_model(store);

  tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL (store));
  
  gtk_window_set_child(GTK_WINDOW (window), tree);
  
   /* The view now holds a reference.  We can get rid of our own
    * reference */

   g_object_unref(G_OBJECT (store));

   /* Create a cell render and arbitrarily make it red for demonstration
    * purposes */
   renderer = gtk_cell_renderer_text_new();
   //g_object_set (G_OBJECT (renderer),  "foreground", "red", NULL);
   /* Create a column, associating the "text" attribute of the
    * cell_renderer to the first column of the model */
   column = gtk_tree_view_column_new_with_attributes ("Author", renderer, "text", 1, NULL);
   /* Add the column to the view. */
   gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

   /* Second column.. title of the book. */
   renderer = gtk_cell_renderer_text_new();
   column = gtk_tree_view_column_new_with_attributes ("Title", renderer, "text", 0, NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

   /* Last column.. whether a book is checked out. */
   renderer = gtk_cell_renderer_toggle_new();
   column = gtk_tree_view_column_new_with_attributes ("Status", renderer, "active", 2, NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
   //column = gtk_tree_view_column_new_with_attributes ("Title", renderer, "text", 3, NULL);
   //gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  //GtkTreeIter iter4;  
  //gtk_tree_store_append(store, &iter4, NULL);
  //gtk_tree_store_append (store, &iter4, NULL); 
  //gtk_tree_store_append (store, &iter4, NULL); 
  //gtk_tree_store_set (store, &iter4, 0, "NAME_TITLE", 1, "AUTHOR!", 2, TRUE, 3, "lalala", -1);    

  //renderer = gtk_cell_renderer_text_new();
  //column = gtk_tree_view_column_new_with_attributes ("Status2", renderer, "text", 3, NULL);
  //gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
  //gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), 3, "lamer" , renderer, "text", 4, NULL);
  //gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), 4, "lamer2", renderer, "text", 6, NULL);
  //gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), 4, "lamer2", renderer, "text", NULL);
  //gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL (store));  

  //gtk_tree_store_clear(store);
  //gtk_tree_store_swap(store);
  //gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (tree), 5, "lamer2", renderer, "active", AUTHOR_COLUMN);
  //gtk_tree_store_clear(store);
  
  GType types[] = {G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING,G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING};
  
  gtk_tree_store_set_column_types(store, 7, types); //G_N_ELEMENTS(types)
  GtkTreeIter iter1;
  GtkTreeIter iter3;
  
  gtk_tree_store_set (store, &iter1,
                    0, "The Art of Computer Programming",
                    1, "Knuth",
                    2, TRUE,
                    3, "BBBB",
                    4, "CCCC",
                    5, "DDDD",
                    6, "EEEE",
                    -1);

 gtk_tree_store_set (store, &iter3,
                    0, "Fundamental Algorithms",
                    1, "Donald",
                    2, FALSE,
                    3, "bbbb",
                    4, "cccc",
                    5, "dddd",
                    6, "eeee",
                    -1);
  
  gtk_window_present(GTK_WINDOW (window));
}

int main (int argc, char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.lamer", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
