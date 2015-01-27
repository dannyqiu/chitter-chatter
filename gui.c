#include "gui.h"
#include "client.h"
#include "util.h"

//Main Chat Program
GtkBuilder *builder;
GObject *chatlog;
GObject *window;
GObject *chatbox;
GObject *users;
GObject *channels;
GtkTextBuffer *buffer;

int client_id;
int client_sock;
GIOChannel *client_gchannel;

void append_to_chat_log(GtkTextBuffer *chat_buffer, gchar *message){
    GtkTextIter chat_end;
    gchar * display_name = "bob";
    gchar timestamp[TIMESTAMP_SIZE]; //placeholders
    time_t current_time;
    struct tm *timeinfo;

    //get current time
    current_time = time(NULL);
    timeinfo = localtime(&current_time);
    strftime(timestamp, TIMESTAMP_SIZE, "%H:%M", timeinfo); 

    gtk_text_buffer_get_end_iter(chat_buffer, &chat_end);
    //g_print("Got iter\n");
    
    gchar *display_message = g_strdup_printf("[%s] %s: %s\n", timestamp, display_name, message);
    
    //insert message to chatlog
    //g_print("Append: %s\n",message);
    gtk_text_buffer_insert(chat_buffer,&chat_end,display_message,-1);

    g_free(display_message);
}

GtkTextBuffer* get_chat_log(GtkEntry *chatbox){
    GtkTextView *chatlog;
    GtkTextBuffer *chat_buffer;
    GtkWidget *grid;
    GtkBin *scroll;

    //get textview + buffer
    grid = gtk_widget_get_parent((GtkWidget*)chatbox);
    if(!GTK_IS_CONTAINER(grid)){
        g_print("Grid is not a container\n");
    }
    //g_print("Got grid...\n");  
   
    scroll = (GtkBin*)gtk_grid_get_child_at((GtkGrid*)grid,1,0);
    chatlog = (GtkTextView*)gtk_bin_get_child(scroll);
    //g_print("Got Textview\n");
    
    //chatlog = find_child(window,"chatlog");
    chat_buffer = gtk_text_view_get_buffer(chatlog); 
    //g_print("Got buffer\n");
   
    return chat_buffer;
}

void change_display_name(char *name){
    display_name = name;
}

void on_channel_selection_changed(GtkWidget *widget, gpointer data){
    g_print("Changed channel!\n");
}

void on_user_selection_changed(GtkWidget *widget, gpointer data){
    g_print("Changed user!\n");
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
    if(strcmp(gdk_keyval_name(event->keyval),"Return")==0 && strcmp(input, "") != 0){
        GtkTextBuffer * chat_buffer = get_chat_log(chatbox);
        append_to_chat_log(chat_buffer, input);
        send_message_to_server(client_sock, input, strlen(input));
        gtk_entry_set_text(chatbox,"");//Resets the entry
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
    users = gtk_builder_get_object(builder, "users");
     
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(chatlog), buffer); 

    //connect signal handlers to gui
    gtk_builder_connect_signals(builder, NULL);

    //set focus to chatbox 
    gtk_widget_grab_focus((GtkWidget*)chatbox);

    add_item_to_list((GtkListStore*)channels, (gchar*)"a channel");
    add_item_to_list((GtkListStore*)users, (gchar*)"admin");
    
    GtkWidget *grid;
    GtkWidget *name_dialog;
    GtkWidget *name_entry;
    GtkWidget *label;
    GtkWidget *content_area;

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
        change_display_name((char *)gtk_entry_get_text(GTK_ENTRY(name_entry)));
        gtk_widget_destroy(name_dialog);
    }

    client_id = connect_to_server(&client_sock);
    client_gchannel = g_io_channel_unix_new(client_sock);

    g_io_add_watch(client_gchannel, G_IO_IN, (GIOFunc) receive_data_from_server, NULL);
    g_io_add_watch(client_gchannel, G_IO_HUP, (GIOFunc) close_connection_from_server, NULL);

    /* Initialize shared memory to hold client current channel */
    if (init_shared_memory() < 0) {
        print_error("Problem creating shared memory");
    }

    gtk_main ();

    return 0;
}

gboolean receive_data_from_server(GIOChannel *source, GIOCondition condition, gpointer data) {
    int client_sock = g_io_channel_unix_get_fd(source);
    struct chat_packet package;
    recv(client_sock, &package, sizeof(struct chat_packet), 0);
    if (package.type == TYPE_MESSAGE) {
        char *recv_message = (char *) malloc((package.total+1) * MSG_SIZE * sizeof(char));
        strncpy(recv_message, package.message, MSG_SIZE);
        while (package.sequence < package.total) {
            recv(client_sock, &package, sizeof(struct chat_packet), 0);
            strncpy(recv_message + (package.sequence * MSG_SIZE), package.message, MSG_SIZE);
        }
        g_print("\nReceived: %s\n", recv_message);
        free(recv_message);
    }
    else if (package.type == TYPE_JOIN_CHANNEL) {
        change_current_channel(package.channel_id);
        add_channel(package.channel_id);
        g_print("Joined channel %d\n", package.channel_id);
    }
    else if (package.type == TYPE_CREATE_CHANNEL) {
        //send_join_channel_to_server(client_sock, package.channel_id); // This line is not required because creator automatically joins channel
        change_current_channel(package.channel_id);
        add_channel(package.channel_id);
        g_print("Created and changed to channel %d\n", package.channel_id);
    }
    return TRUE;
}

gboolean close_connection_from_server(GIOChannel *source, GIOCondition condition, gpointer data) {
    return FALSE;
}
