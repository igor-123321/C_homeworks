/*
<?php
$window = new GtkWindow();
$window->set_size_request(400, 280);
$window->connect_simple('destroy', array('Gtk','main_quit'));

$window->add($vbox = new GtkVBox());

// display title
$title = new GtkLabel("Iterate Through a TreeStore");
$title->modify_font(new PangoFontDescription("Times New Roman Italic 10"));
$title->modify_fg(Gtk::STATE_NORMAL, GdkColor::parse("#0000ff"));
$title->set_size_request(-1, 40);
$title->set_justify(Gtk::JUSTIFY_CENTER);
$alignment = new GtkAlignment(0.5, 0, 0, 0);
$alignment->add($title);
$vbox->pack_start($alignment, 0, 0);
$vbox->pack_start(new GtkLabel(), 0, 0);

// setup button
$button = new GtkButton('Iterate Through TreeStore');
$button->connect('clicked', 'on_click');
$alignment = new GtkAlignment(0.5, 0, 0, 0);
$alignment->add($button);
$vbox->pack_start($alignment, 0);
$vbox->pack_start(new GtkLabel(), 0, 0);

$list = array(
    array ('id' => '101', 'parent' => '0', 'data' => 'data0'),
    array ('id' => '102', 'parent' => '101', 'data' => 'data1'),
    array ('id' => '103', 'parent' => '0', 'data' => 'data2'),
    array ('id' => '104', 'parent' => '0', 'data' => 'data3'),
    array ('id' => '105', 'parent' => '104', 'data' => 'data4'),
    array ('id' => '106', 'parent' => '105', 'data' => 'data5')
);

// setup store
if (defined("GObject::TYPE_STRING")) {
    $model = new GtkTreeStore(GObject::TYPE_LONG, GObject::TYPE_STRING,
        GObject::TYPE_STRING);
} else {
    $model = new GtkTreeStore(Gtk::TYPE_LONG, Gtk::TYPE_STRING,
        Gtk::TYPE_STRING);
}

$nodes = array();
$nodes[0] = null; // root
foreach($list as $item) {
    $id = $item['id'];
    $parent = $item['parent'];
    $data = $item['data'];
    $nodes[$id] = $model->append($nodes[$parent],
        array($id, "this is id $id", $data));
}

//create the view with the tree store set as model
$view = new GtkTreeView($model);

//set up the columns
$cell_renderer = new GtkCellRendererText();
$view->append_column(new GtkTreeViewColumn('id', $cell_renderer, 'text', 0));
$view->append_column(new GtkTreeViewColumn('title', $cell_renderer, 'text', 1));
$view->append_column(new GtkTreeViewColumn('data', $cell_renderer, 'text', 2));

//display it
$view->expand_all();
$vbox->pack_start($view, 0);
$window->show_all();
Gtk::main();

function on_click($button) {
    global $model;
    $model->foreach('process'); // note 1
}

function process($model, $path, $iter) {
    $id = $model->get_value($iter, 0);
    $title = $model->get_value($iter, 1);
    $data = $model->get_value($iter, 2);
    $path2 = implode('-', $path);
    echo "Processing $path2. $id: $title ($data)\n"; // note 2
}

?>

*/

#include <gtk/gtk.h>

enum
{
   TITLE_COLUMN,
   AUTHOR_COLUMN,
   CHECKED_COLUMN,
   N_COLUMNS
};

void main (void)
{
   GtkTreeStore *store;
   GtkWidget *tree;
   GtkTreeViewColumn *column;
   GtkCellRenderer *renderer;

   /* Create a model.  We are using the store model for now, though we
    * could use any other GtkTreeModel */
   store = gtk_tree_store_new (N_COLUMNS,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_BOOLEAN);

   /* custom function to fill the model with data */
   populate_tree_model (store);

   /* Create a view */
   tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

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
   renderer = gtk_cell_renderer_toggle_new ();
   column = gtk_tree_view_column_new_with_attributes ("Checked out",
                                                      renderer,
                                                      "active", CHECKED_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

   /* Now we can manipulate the view just like any other GTK widget */
}
