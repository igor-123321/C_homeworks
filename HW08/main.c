#include <bsd/string.h>
#include <dirent.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define PATH_MAX_LENGTH 2048
#define APP_WIDTH 400
#define APP_HEIGTH 400

void listdir(const char *name, GtkTreeStore *store, GtkTreeIter *iterParent,
             int depth) {
  DIR *dir;
  struct dirent *entry;
  GtkTreeIter iterChild;

  if (!(dir = opendir(name)))
    return;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == 4) {
      char path[PATH_MAX_LENGTH + 1];
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;
      snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);

      if (depth) {
        gtk_tree_store_append(store, &iterChild, iterParent);
        gtk_tree_store_set(store, &iterChild, 0, entry->d_name, 1, FALSE, -1);
        listdir(path, store, &iterChild, ++depth);
      } else {
        gtk_tree_store_append(store, iterParent, NULL);
        gtk_tree_store_set(store, iterParent, 0, entry->d_name, 1, FALSE, -1);
        listdir(path, store, iterParent, ++depth);
      }
    } else {
      if (depth) {
        gtk_tree_store_append(store, &iterChild, iterParent);
        gtk_tree_store_set(store, &iterChild, 0, entry->d_name, 1, TRUE, -1);
      } else {
        gtk_tree_store_append(store, iterParent, NULL);
        gtk_tree_store_set(store, iterParent, 0, entry->d_name, 1, TRUE, -1);
      }
    }
  }
  closedir(dir);
}

enum { DIR_COLUMN, IS_FILE, N_COLUMNS };

void populate_tree_model(GtkTreeStore *store) {
  GtkTreeIter iterParent; /* Parent iter */
  listdir(".", store, &iterParent, 0);
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *box;
  GtkTreeStore *store;
  GtkWidget *tree;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), APP_WIDTH, APP_HEIGTH);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_window_set_child(GTK_WINDOW(window), box);

  store = gtk_tree_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_BOOLEAN);
  // set data to store
  populate_tree_model(store);

  tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

  gtk_box_append(GTK_BOX(box), tree);
  g_object_unref(G_OBJECT(store));
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("PATH", renderer, "text",
                                                    DIR_COLUMN, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
  gtk_tree_view_column_set_min_width(column, 300);

  renderer = gtk_cell_renderer_toggle_new();
  column = gtk_tree_view_column_new_with_attributes("IS-FILE", renderer,
                                                    "active", IS_FILE, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
  gtk_tree_view_column_set_max_width(column, 100);

  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("org.gtk.app", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
