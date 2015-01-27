#include "gui.h"
#include "gclient.h"
#include "util.h"

//Main Chat Program
GtkBuilder *builder;
GObject *chatlog;
GObject *window;
GObject *chatbox;
GObject *channels;
GtkTextBuffer *buffer;
gchar display_name[DISPLAY_NAME_SIZE];

//Popup Dialog
GtkWidget *grid;
GtkWidget *name_dialog;
GtkWidget *name_entry;
GtkWidget *label;
GtkWidget *content_area;

int client_id;
int client_sock;
GIOChannel *client_gchannel;
int current_channel_id;

// Needs to be freed
gchar * construct_message(gchar *display_name, gchar *message) {
    gchar timestamp[TIMESTAMP_SIZE]; //placeholders
    time_t current_time;
    struct tm *timeinfo;

    //get current time
    current_time = time(NULL);
    timeinfo = localtime(&current_time);
    strftime(timestamp, TIMESTAMP_SIZE, "%H:%M", timeinfo); 

    return g_strdup_printf("[%s] %s: %s\n", timestamp, display_name, message);
}

void append_to_chat_log(GtkTextBuffer *chat_buffer, gchar *message) {
    GtkTextIter chat_end;
    gtk_text_buffer_get_end_iter(chat_buffer, &chat_end);
    
    //insert message to chatlog
    //g_print("Append: %s\n",message);
    gtk_text_buffer_insert(chat_buffer, &chat_end, message, -1);
}

void change_display_name(const gchar *name){
    // TODO: Button to do this?
    g_strlcpy(display_name, name, DISPLAY_NAME_SIZE);
}

void on_create_channel_clicked(GtkWidget *widget, gpointer data){
    // TODO: Allow client to specify name of the channel
    gchar *channel_name = g_strdup_printf("> Channel by %d <", client_id);
    send_create_channel_to_server(client_sock, client_id, channel_name);
    g_free(channel_name);
}

void on_channel_selection_changed(GtkWidget *widget, gpointer data){
    g_print("Selected available channel.\n");
}

void on_user_channel_selection_changed(GtkWidget *widget, gpointer data){
    g_print("Changed user's channel!\n");
}

void add_item_to_list(GtkListStore *list, gchar *item_name){
    GValue name = G_VALUE_INIT;
    g_value_init(&name, G_TYPE_STRING);
    g_value_set_static_string(&name, item_name);

    GtkTreeIter iter;

    g_print("Appending...\n");
    gtk_list_store_append(list, &iter);
    g_print("Appended\n");
    gtk_list_store_set_value(list, &iter, 0, &name); 
    g_print("Added value to list.\n");
}

gboolean key_event(GtkWidget *widget, GdkEventKey *event){
    //g_printerr("%s\n", gdk_keyval_name (event->keyval));
    GtkEntry *chatbox = (GtkEntry*)widget;
    gchar *input = (gchar *)gtk_entry_get_text(chatbox);
    if (strcmp(gdk_keyval_name(event->keyval), "Return") == 0 && strcmp(input, "") != 0){
        GtkTextBuffer *chat_buffer = gtk_text_view_get_buffer((GtkTextView *) chatlog);
        gchar *message = construct_message(display_name, input);
        append_to_chat_log(chat_buffer, message);
        send_message_to_server(client_sock, client_id, current_channel_id, message);
        g_free(message);
        gtk_entry_set_text(chatbox, ""); //Resets the entry
    }
    return TRUE;
}

gboolean on_delete_event (GtkWidget *widget, GdkEvent *event, gpointer data) {
  /* If you return FALSE in the "delete_event" signal handler,
   * GTK will emit the "destroy" signal. Returning TRUE means
   * you don't want the window to be destroyed.
   *
   * This is useful for popping up 'are you sure you want to quit?'
   * type dialogs.
   */

    GtkWidget *dialog;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    dialog = gtk_message_dialog_new(GTK_WINDOW(widget), flags, GTK_MESSAGE_QUESTION,
                                    GTK_BUTTONS_YES_NO, "Are you sure you want to quit?");

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    switch (response){
        case GTK_RESPONSE_YES:
            gtk_widget_destroy(dialog);
            return FALSE;
        case GTK_RESPONSE_NO:
            g_print("Aborting exit...\n");
            gtk_widget_destroy(dialog);
            return TRUE;
    }
    g_print ("delete event occurred\n");
    return TRUE;
}

int main (int argc, char *argv[]) {

    gtk_init (&argc, &argv);
    
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder , "layout.ui" , NULL); 
    buffer = gtk_text_buffer_new(NULL);

    window = gtk_builder_get_object(builder, "window");
    chatlog = gtk_builder_get_object(builder, "chatlog");
    chatbox = gtk_builder_get_object(builder, "chatbox");
    channels = gtk_builder_get_object(builder, "channels");
     
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(chatlog), buffer); 

    //connect signal handlers to gui
    gtk_builder_connect_signals(builder, NULL);

    //set focus to chatbox 
    gtk_widget_grab_focus((GtkWidget*)chatbox);

    add_item_to_list((GtkListStore*)channels, (gchar*)"a channel");
   
    //Popup Window Code here
    label = gtk_label_new("Enter your desired display name.");

    name_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(name_entry) , TRUE);
    gtk_entry_set_max_length(GTK_ENTRY(name_entry) , 20);
    
    GtkDialogFlags flags = GTK_DIALOG_MODAL;
    
    name_dialog = gtk_dialog_new_with_buttons("Chitter-Chatter", GTK_WINDOW(window),
					 flags, "Submit", GTK_RESPONSE_ACCEPT, NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(name_dialog));
    gtk_box_pack_start(GTK_BOX(content_area), label, TRUE, TRUE, 5); 
    gtk_box_pack_start(GTK_BOX(content_area), name_entry, TRUE, TRUE, 5); 
    
    gtk_window_set_transient_for(GTK_WINDOW(name_dialog),GTK_WINDOW(window));

    gtk_widget_show_all(name_dialog);
    gint response = gtk_dialog_run(GTK_DIALOG(name_dialog));
    
    if(response == GTK_RESPONSE_ACCEPT){
        change_display_name(gtk_entry_get_text(GTK_ENTRY(name_entry)));
        gtk_widget_destroy(name_dialog);
    } else {
        change_display_name("Anonymous"); //maybe get client id here
    }

    client_id = connect_to_server(&client_sock);
    client_gchannel = g_io_channel_unix_new(client_sock);

    g_io_add_watch(client_gchannel, G_IO_IN, (GIOFunc) receive_data_from_server, NULL);

    gtk_main ();

    return 0;
}

gboolean receive_data_from_server(GIOChannel *source, GIOCondition condition, gpointer data) {
    int client_sock = g_io_channel_unix_get_fd(source);
    struct chat_packet package;
    int nbytes = recv(client_sock, &package, sizeof(struct chat_packet), 0);
    if (nbytes <= 0) {
        if (nbytes == 0) {
            print_error("Connection closed by the server :(");
        }
        else {
            print_error("Problem receiving data from the server");
        }
        cleanup_and_exit(1);
    }
    else {
        if (package.type == TYPE_MESSAGE) {
            char *recv_message = (char *) malloc((package.total+1) * MSG_SIZE * sizeof(char));
            strncpy(recv_message, package.message, MSG_SIZE);
            while (package.sequence < package.total) {
                recv(client_sock, &package, sizeof(struct chat_packet), 0);
                strncpy(recv_message + (package.sequence * MSG_SIZE), package.message, MSG_SIZE);
            }
            GtkTextBuffer *chat_buffer = gtk_text_view_get_buffer((GtkTextView *) chatlog);
            append_to_chat_log(chat_buffer, recv_message);
            g_print("\nReceived: %s\n", recv_message);
            free(recv_message);
        }
        else if (package.type == TYPE_JOIN_CHANNEL) {
            current_channel_id = package.channel_id;
            add_channel(package.channel_id);
            g_print("Joined channel %d\n", package.channel_id);
        }
        else if (package.type == TYPE_CREATE_CHANNEL) {
            //send_join_channel_to_server(client_sock, package.channel_id); // This line is not required because creator automatically joins channel
            current_channel_id = package.channel_id;
            add_channel(package.channel_id);
            g_print("Created and changed to channel %d\n", package.channel_id);
        }
    }
    return TRUE;
}

void cleanup_and_exit(int exit_code) {
    exit(exit_code);
}
