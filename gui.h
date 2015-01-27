#include <gtk/gtk.h>
#include <glib-object.h>
#include <time.h>
#include <string.h>

void append_to_chat_log(GtkTextBuffer *chat_buffer, gchar *message);
GtkTextBuffer* get_chat_log(GtkEntry *chatbox);
void on_channel_selection_changed(GtkWidget *widget, gpointer data);
void on_user_selection_changed(GtkWidget *widget, gpointer data);
void add_item_to_list(GtkListStore *list, gchar *item_name);
gboolean key_event(GtkWidget *widget, GdkEventKey *event);
gboolean on_delete_event (GtkWidget *widget, GdkEvent *event, gpointer data);
 
void chat_room();
