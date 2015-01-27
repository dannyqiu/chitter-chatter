#include "gui.h"
#include "client.h"
#include "util.h"

int client_id;
int client_sock;

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

    send_message_to_server(client_sock, message, strlen(message));

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

void chat_room(){
    GtkBuilder *builder;
    GObject *chatlog;
    GObject *chatbox;
    GtkTextBuffer *buffer;
       
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder , "chat_room.ui" , NULL); 
    buffer = gtk_text_buffer_new(NULL);

    chatlog = gtk_builder_get_object(builder, "chatlog");
    chatbox = gtk_builder_get_object(builder, "chatbox");
        
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(chatlog), buffer); 

    //connect signal handlers to gui
    gtk_builder_connect_signals(builder, NULL);

    //set focus to chatbox 
    gtk_widget_grab_focus((GtkWidget*)chatbox);

}

int main (int argc, char *argv[]) {

    gtk_init (&argc, &argv);

    //Main Chat Program
    GtkBuilder *builder;
    GObject *chatlog;
    GObject *chatbox;
    GObject *users;
    GObject *channels;
    GtkTextBuffer *buffer;
       
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder , "layout.ui" , NULL); 
    buffer = gtk_text_buffer_new(NULL);

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

    //Name Grabber
    GtkWidget *dialog;
    GtkDialogFlags flags = GTK_DIALOG_MODAL;
    //dialog = gtk_dialog_with_new_buttons("Chitter-Chatter",

    client_id = connect_to_server(&client_sock);

    /* Initialize shared memory to hold client current channel */
    if (init_shared_memory() < 0) {
        print_error("Problem creating shared memory");
    }

    gtk_main ();

    return 0;
}
