#include <gtk/gtk.h>

static void print_hello () //GtkWidget *widget, gpointer data
{
  g_print ("Hello World\n");
}


static void activate (GtkApplication* app, gpointer user_data)
{
  GtkWidget *window;
  GtkWidget *button;
  GtkWidget *box;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 300, 200);
  
  //box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  button = gtk_button_new_with_label ("Hello World");
  gtk_window_set_child(GTK_WINDOW (window), button);
  g_signal_connect         (button, "clicked", G_CALLBACK(print_hello), NULL);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_window_set_default_size), window);
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
