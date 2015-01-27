#include <gtk/gtk.h>
#include <glib-object.h>
#include <time.h>
#include <string.h>

gchar * construct_message(gchar *, gchar *);
void append_to_chat_log(GtkTextBuffer *chat_buffer, gchar *message);
void change_display_name(const gchar *name);

void on_create_channel_clicked(GtkWidget *widget, gpointer data);
void on_channel_selection_changed(GtkWidget *widget, gpointer data);
void on_user_selection_changed(GtkWidget *widget, gpointer data);

void add_item_to_list(GtkListStore *list, gchar *item_name);
gboolean key_event(GtkWidget *widget, GdkEventKey *event);
gboolean on_delete_event (GtkWidget *widget, GdkEvent *event, gpointer data);
 
gboolean receive_data_from_server(GIOChannel *, GIOCondition, gpointer);

void cleanup_and_exit(int);
