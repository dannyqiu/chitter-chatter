#include <gtk/gtk.h>
#include "constants.h"

/* This is a callback function. The data arguments are ignored
 * in this example. More on callbacks below.
 */
static void
print_hello (GtkWidget *widget, gpointer data) {
    g_print ("Hello World\n");
}

static gboolean
key_event(GtkWidget *widget,
          GdkEventKey *event)
{
  g_printerr("%s\n",
	     gdk_keyval_name (event->keyval));
  return FALSE;
}

static gboolean on_delete_event (GtkWidget *widget, GdkEvent *event, gpointer data) {
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
    GObject *window;
    GObject *chatlog;
    GObject *chatbox;
    GtkTextBuffer *buffer;

    /* This is called in all GTK applications. Arguments are parsed
    * from the command line and are returned to the application.
    */
    gtk_init (&argc, &argv);
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder , "layout.ui" , NULL); 
    buffer = gtk_text_buffer_new(NULL);

    chatlog = gtk_builder_get_object(builder, "chatlog");
    chatbox = gtk_builder_get_object(builder, "chatbox");
    window = gtk_builder_get_object(builder, "window");

    gtk_text_buffer_set_text(buffer, "[21:00] alvin: haha you know that isnt true\n[21:01] bob: ofc it is what r u saying",-1);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(chatlog), buffer); 

    gtk_widget_grab_focus((GtkWidget*)chatbox);

    /* When the window emits the "delete-event" signal (which is emitted
    * by GTK+ in response to an event coming from the window manager,
    * usually as a result of clicking the "close" window control), we
    * ask it to call the on_delete_event() function as defined above.
    *
    * The data passed to the callback function is NULL and is ignored
    * in the callback function.
    */
    g_signal_connect (window, "delete-event", G_CALLBACK (on_delete_event), NULL);

    //Adding keypress catching for enter later on
    g_signal_connect(window, "key-release-event", G_CALLBACK(key_event), NULL);

    /* Here we connect the "destroy" event to the gtk_main_quit() function.
    *
    * This signal is emitted when we call gtk_widget_destroy() on the window,
    * or if we return FALSE in the "delete_event" callback.
    */
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    /* When the button receives the "clicked" signal, it will call the
    * function print_hello() passing it NULL as its argument.
    *
    * The print_hello() function is defined above.
    */
    //g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);

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
