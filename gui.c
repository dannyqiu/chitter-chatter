#include <gtk/gtk.h>
#include <string.h>
#include "constants.h"

/* This is a callback function. The data arguments are ignored
 * in this example. More on callbacks below.
 */
void print_hello (GtkWidget *widget, gpointer data) {
    g_print ("Hello World\n");
}

void append_to_buffer(GtkEntry *chatbox){
    const gchar *message;
    GtkTextIter *chat_start;
    GtkTextIter *chat_end;

    //get textview + buffer
    //GtkWidget *window = gtk_widget_get_toplevel((GtkWidget*)chatbox);
    GtkGrid *grid = (GtkGrid *)gtk_widget_get_parent((GtkWidget*)chatbox);
    g_print("Got grid...\n");  
    GtkTextView *chatlog = (GtkTextView*)gtk_bin_get_child((GtkBin*)gtk_grid_get_child_at(grid,1,0));
    g_print("Got Textview\n");
    GtkTextBuffer * chat_buffer = gtk_text_view_get_buffer(chatlog); 
    g_print("Got buffer\n");
    if(chat_buffer == NULL){
        g_print("NULLLLL\n");
    }

    gtk_text_buffer_get_iter_at_offset(chat_buffer, chat_start,0);
    
    g_print("Get start iter\n");
    //gtk_text_buffer_get_end_iter(chat_buffer, chat_end);
    //g_print("Got iters\n");
     
    message = gtk_entry_get_text(chatbox);
    g_print("Append: %s\n",message);
}

void on_clientlist_selection_changed(GtkWidget *widget, gpointer data){
    g_print("Changed selection!\n");
}

gboolean key_event(GtkWidget *widget,
          GdkEventKey *event)
{
  g_printerr("%s\n",
	     gdk_keyval_name (event->keyval));
  if(strcmp(gdk_keyval_name(event->keyval),"Return")==0){
      append_to_buffer((GtkEntry*)widget);
  }
  return FALSE;
}

gboolean on_delete_event (GtkWidget *widget, GdkEvent *event, gpointer data) {
  /* If you return FALSE in the "delete_event" signal handler,
   * GTK will emit the "destroy" signal. Returning TRUE means
   * you don't want the window to be destroyed.
   *
   * This is useful for popping up 'are you sure you want to quit?'
   * type dialogs.
   */

  g_print ("delete event occurred\n");

  return FALSE;
}

int main (int argc, char *argv[]) {

    GtkBuilder *builder;
    GObject *chatlog;
    GObject *chatbox;
    GtkTextBuffer *buffer;

    gtk_init (&argc, &argv);
    
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder , "layout.ui" , NULL); 
    buffer = gtk_text_buffer_new(NULL);

    chatlog = gtk_builder_get_object(builder, "chatlog");
    chatbox = gtk_builder_get_object(builder, "chatbox");

    gtk_text_buffer_set_text(buffer, "[21:00] alvin: haha you know that isnt true\n[21:01] bob: ofc it is what r u saying\n",-1);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(chatlog), buffer); 

    //connect signal handlers to gui
    gtk_builder_connect_signals(builder, NULL);

    //set focus to chatbox 
    gtk_widget_grab_focus((GtkWidget*)chatbox);

    /* The g_signal_connect_swapped() function will connect the "clicked" signal
    * of the button to the gtk_widget_destroy() function; instead of calling it
    * using the button as its argument, it will swap it with the user data
    * argument. This will cause the window to be destroyed by calling
    * gtk_widget_destroy() on the window.
    */
    //g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);

    gtk_main ();

    return 0;
}
