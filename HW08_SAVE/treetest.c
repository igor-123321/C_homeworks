#include <gtk/gtk.h>


enum {
   TITLE_COLUMN,
   AUTHOR_COLUMN,
   CHECKED_COLUMN,
   N_COLUMNS
};

static void activate (GtkApplication* app, gpointer user_data)
{
  GtkWidget *window;
  //GtkWidget *button;
  GtkWidget *box;
  GtkTreeStore *store;
  GtkWidget *tree;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 400, 400);
  
  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_window_set_child(GTK_WINDOW (window), box);

  store = gtk_tree_store_new (N_COLUMNS,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_BOOLEAN);

  

  
 GtkTreeIter iter1;  /* Parent iter */
 GtkTreeIter iter2;  /* Child iter  */

 gtk_tree_store_append (store, &iter1, NULL);  /* Acquire a top-level iterator */
 gtk_tree_store_set (store, &iter1,
                    TITLE_COLUMN, "The Art of Computer Programming",
                    AUTHOR_COLUMN, "Donald E. Knuth",
                    CHECKED_COLUMN, FALSE,
                    -1);

 gtk_tree_store_append (store, &iter2, &iter1);  /* Acquire a child iterator */
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




  tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
 
  gtk_box_append (GTK_BOX (box), tree);
   /* The view now holds a reference.  We can get rid of our own
    * reference */
   g_object_unref (G_OBJECT (store));

   /* Create a cell render and arbitrarily make it red for demonstration
    * purposes */
   renderer = gtk_cell_renderer_text_new ();
   g_object_set (G_OBJECT (renderer),
                 "foreground", "red",
                 NULL);

   /* Create a column, associating the "text" attribute of the
    * cell_renderer to the first column of the model */
   column = gtk_tree_view_column_new_with_attributes ("Author", renderer,
                                                      "text", AUTHOR_COLUMN,
                                                      NULL);

   /* Add the column to the view. */
   gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

   /* Second column.. title of the book. */
   renderer = gtk_cell_renderer_text_new ();
   column = gtk_tree_view_column_new_with_attributes ("Title",
                                                      renderer,
                                                      "text", TITLE_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

   /* Last column.. whether a book is checked out. */
   renderer = gtk_cell_renderer_toggle_new();

   column = gtk_tree_view_column_new_with_attributes ("Checked out",
                                                      renderer,
                                                      "active", CHECKED_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);


  //g_signal_connect         (button, "clicked", G_CALLBACK(print_hello), NULL);
  //g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_window_set_default_size), window);
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
