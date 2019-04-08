#include "telegram.h"

unsigned int counter;

unsigned int parse_id(char* event);
void tg_parse_data();
void tg_setup_chat();

void tg_initialize() {
    counter = 0;
    tg_parse_data();

    td_set_log_verbosity_level(2);
    td_set_log_file_path("./client.log");
    client = td_json_client_create();

    // Use "setTdlibParameters" method with our stuff 
    char login_req[500];
    sprintf(login_req,
            "{\"@type\": \"setTdlibParameters\", \"parameters\": {\"api_id\": \"%s\", \"api_hash\": \"%s\", \"database_directory\": \"./data\", \"use_message_database\": true, \"use_secret_chats\": false, \"system_language_code\": \"en-US\", \"device_model\": \"Horseshoe\", \"system_version\": \"0.0\", \"application_version\": \"0.0\"}, \"@extra\": \"connexion\"}",
            tg_data.api_id, tg_data.api_hash);
    
    td_json_client_send(client, login_req);

    // Normally we received an `updateAuthorizationState = authorizationStateWaitEncryptionKey` event
    // We can just reply with empty string
    char encryption_key_req[500] = "{\"@type\": \"checkDatabaseEncryptionKey\", \"parameters\": {\"encryption_key\": \"\"}, \"@extra\": \"encryptionkey\"}";
    td_json_client_send(client, encryption_key_req);

    // Check events to see what we need to do next.
    char* response;

    while(1) {
        response = td_json_client_receive(client, TIMEOUT);

        if(!response)
            break;

        if(strstr(response, "authorizationStateReady") != NULL) {
            // We're in!
            break;
        }

        if(strstr(response, "authorizationStateWaitPhoneNumber") != NULL) {
            printf("Phone number: ");
            char phone_number[20] = "+15817015326";
            scanf("%s", phone_number);

            char request_phone_number[500];
            sprintf(request_phone_number,
                "{\"@type\": \"setAuthenticationPhoneNumber\", \"phone_number\": \"%s\", \"allow_flash_call\": false, \"is_current_phone_number\": false, \"@extra\": \"phone_number\"}",
                phone_number);
            
            td_json_client_send(client, request_phone_number);
        }

        if(strstr(response, "authorizationStateWaitCode") != NULL) {
            printf("Verification code: ");
            char code[5];
            scanf("%s", code);

            char request_code_auth[500];
            sprintf(request_code_auth,
                "{\"@type\": \"checkAuthenticationCode\", \"code\": \"%s\", \"first_name\": \"\", \"last_name\": \"\", \"@extra\": \"auth_code\"}",
                code);
            
            td_json_client_send(client, request_code_auth);
        }

        if(strstr(response, "authorizationStateWaitPassword") != NULL) {
            printf("Sorry, telegram account passwords aren't supported.");
            td_json_client_destroy(client);
            exit(0);
        }
    }

    if(strcmp(tg_data.chat, "0") == 0 || strcmp(tg_data.supergroup, "0") == 0) {
        tg_setup_chat();
    }
}

BID tg_send_message(char* message) {
    char req[5500];
    sprintf(req,
        "{\"@type\": \"sendMessage\", \"chat_id\": \"%s\", \"input_message_content\": {\"@type\": \"inputMessageText\", \"text\": {\"@type\": \"formattedText\", \"text\": \"%s\", \"entities\": [{\"@type\": \"textEntity\", \"offset\": 0, \"length\": \"%d\", \"type\": {\"@type\": \"textEntityTypeCode\"}}]}}, \"@extra\": \"sendmessage\"}",
        tg_data.chat, message, strlen(message));
    
    td_json_client_send(client, req);

    int success = 0;

    char* event;
    while(1) {
        event = td_json_client_receive(client, TIMEOUT);
        if(!event)
            return 0;
        if(strstr(event, "updateMessageSendSucceeded") != NULL) {
            success = 1;
            break;
        }
        if(strstr(event, "error") != NULL) 
            printf("ERROR SENDING MESSAGE: %s\n", event);
    }

    if(!success)
        return 0;
        
    unsigned int id = parse_id(event) >> 20;

    return id;
}

int tg_read_message(BID id, char* message) {
    char extra[50];
    sprintf(extra, "get%dmessage", counter++);
    // Prepare request
    id = id << 20;
    char request[300];
    sprintf(request,
    "{\"@type\": \"getMessage\", \"chat_id\": \"%s\", \"message_id\": \"%d\", \"@extra\": \"%s\"}",
    tg_data.chat, id, extra);
    // Send request
    td_json_client_send(client, request);

    char* event;

    while(1) {
        event = td_json_client_receive(client, TIMEOUT);
        if(!event) 
            break;
        if(strstr(event, extra) != NULL) {
            break;
        }
    }

    if(!event)
        return -1;
    if(strstr(event, "error") != NULL) {
        printf("ERROR READING MESSAGE %d:\n%s\n", id >> 20, event);
        return -1;
    }

    // Parse JSON (this is the dumbest part)

    int parsing = 0;
    int msg_cur = 0;

    for(int i = 7; i < strlen(event); i++) {
        if(!parsing) {
            char c1,c2,c3,c4,c5,c6,c7,c8, c9;
            c1 = event[i-7];
            c2 = event[i-6];
            c3 = event[i-5];
            c4 = event[i-4];
            c5 = event[i-3];
            c6 = event[i-2];
            c7 = event[i-1];
            c8 = event[i-0];
            if(c1 == '\"' && c2 == 't' && c3 == 'e' && c4 == 'x' && c5 == 't' && c6 == '\"' && c7 == ':' && c8 == '\"')
                parsing = 1;
        } else {
            if(event[i] == '\"')
                break;
            message[msg_cur++] = event[i];
        }
    }

    return 0;
}

void tg_pin_message(BID id) {
    id = id << 20;

    char request[500];
    sprintf(request,
    "{\"@type\": \"pinSupergroupMessage\", \"supergroup_id\": \"%s\", \"message_id\": \"%d\", \"disable_notification\": true, \"@extra\": \"pinmessage\"}",
    tg_data.supergroup, id);

    td_json_client_send(client, request);

    char* event;

    while(1) {
        event = td_json_client_receive(client, TIMEOUT);
        if(!event) 
            break;
        if(strstr(event, "updateChatLastMessage") != NULL && strstr(event, "messagePinMessage") != NULL) {
            unsigned int announceId = parse_id(event);
            tg_delete_message(announceId >> 20);
        }
        if(strstr(event, "deletepinannouncement") != NULL) {
            break;
        }
    }
}

int tg_edit_message(BID id, char* message) {
    char req[5500];
    sprintf(req,
        "{\"@type\": \"editMessageText\", \"chat_id\": \"%s\", \"message_id\" : %d, \"input_message_content\": {\"@type\": \"inputMessageText\", \"text\": {\"@type\": \"formattedText\", \"text\": \"%s\", \"entities\": [{\"@type\": \"textEntity\", \"offset\": 0, \"length\": \"%d\", \"type\": {\"@type\": \"textEntityTypeCode\"}}]}}, \"@extra\": \"messageedited\"}",
        tg_data.chat, id << 20, message, strlen(message));
    
    td_json_client_send(client, req);

    char* event;
    int ok = -1;
    while(1) {
        event = td_json_client_receive(client, TIMEOUT);
        if(!event) 
            break;
        if(strstr(event, "messageedited") != NULL) {
            ok = 0;
            break;
        }
    }

    return 0;
}

BID tg_get_pinned_message() {
    char request[500];
    sprintf(request,
        "{\"@type\": \"getChatPinnedMessage\", \"chat_id\": \"%s\", \"@extra\": \"getpinned\"}",
        tg_data.chat);

    td_json_client_send(client, request);

    char* event;
    int ok = 0;
    while(1) {
        event = td_json_client_receive(client, TIMEOUT);
        if(!event) 
            break;
        if(strstr(event, "getpinned") != NULL) {
            ok = 1;
            break;
        }
    }

    if(!ok)
        return 0;

    return parse_id(event) >> 20;
}

void tg_delete_message(BID id) {
    char deleteReq[500];
                sprintf(deleteReq,
                "{\"@type\": \"deleteMessages\", \"chat_id\": \"%s\", \"message_ids\": [\"%d\"], \"@extra\": \"deletemessage\"}",
                tg_data.chat, id << 20);
                
    td_json_client_send(client, deleteReq);

    while(1) {
        char* event = td_json_client_receive(client, TIMEOUT);
        if(!event || strstr(event, "deletemessage") != NULL) {
            return;
        }
    }
}

void tg_parse_data() {
    char path[1000];
    FILE* ini_file = fopen("./config.ini", "r");

    char key[10];
    char* value = key;
    int i = 0;

    char c;
    while((c = fgetc(ini_file)) != EOF) {
        if(c == ' ' || c == '\r')
            continue;
        if(c == '=') {
            value[i] = '\0';
            i = 0;
            if(strcmp(key, "api_id") == 0)
                value = tg_data.api_id;
            if(strcmp(key, "api_hash") == 0)
                value = tg_data.api_hash;
            if(strcmp(key, "chat") == 0)
                value = tg_data.chat;
            if(strcmp(key, "supergroup") == 0)
                value = tg_data.supergroup;
            continue;
        }
        if(c == '\n') {
            value[i] = '\0';
            i = 0;
            value = key;
            continue;
        }

        value[i++] = c;
    }
    fclose(ini_file);
}

void tg_setup_chat() {
    printf("Please send a message containing \"DOMFS\" (in all caps) in the supergroup that will be used to store data.\n");

    char* event;
    while(1) {
        event = td_json_client_receive(client, TIMEOUT);
        if(!event)
            continue;
        if(strstr(event, "updateNewMessage") != NULL && strstr(event, "DOMFS") != NULL) {
            break;
        }
    }
    int msgid = parse_id(event) >> 20;

    printf("Message received!\n");
    // Parse chat_id

    {
        int j = 0;
        int parsing = 0;
        for(int i=8; i<strlen(event); i++) {
            if(!parsing) {
                char c1 = event[i-8];
                char c2 = event[i-7];
                char c3 = event[i-6];
                char c4 = event[i-5];
                char c5 = event[i-4];
                char c6 = event[i-3];
                char c7 = event[i-2];
                char c8 = event[i-1];
                char c9 = event[i];
                if(c1 == 'c' && c2 == 'h' && c3 == 'a' && c4 == 't' && c5 == '_' && c6 == 'i' && c7 == 'd' && c8 == '\"' && c9 == ':')
                    parsing = 1;
            } else {
                char c = event[i];
                if(c == ',')
                    break;
                tg_data.chat[j++] =c;
            }
        }
    }

    printf("Chat id:      %s\n", tg_data.chat);

    tg_delete_message(msgid);
    
    // Request supergroup id
    char request[500];
    sprintf(request,
        "{\"@type\": \"getChat\",\"chat_id\" : \"%s\",\"@extra\": \"setuprequest\"}",
        tg_data.chat);

    td_json_client_send(client, request);

    while(1) {
        event = td_json_client_receive(client, TIMEOUT);
        if(!event)
            break;
        if(strstr(event, "setuprequest") != NULL) {
            //printf("Req: %s\n", event);
            break;
        }
    }

    if(!event) {
        printf("Something went wrong.\n");
        td_json_client_destroy(client);
        exit(1);
    }

    // Parse supergroup id
    {
        int j = 0;
        int parsing = 0;
        for(int i=8; i<strlen(event); i++) {
            if(!parsing) {
                char c1 = event[i-8];
                char c2 = event[i-7];
                char c3 = event[i-6];
                char c4 = event[i-5];
                char c5 = event[i-4];
                char c6 = event[i-3];
                char c7 = event[i-2];
                char c8 = event[i-1];
                char c9 = event[i];
                if(c1 == 'r' && c2 == 'o' && c3 == 'u' && c4 == 'p' && c5 == '_' && c6 == 'i' && c7 == 'd' && c8 == '\"' && c9 == ':')
                    parsing = 1;
            } else {
                char c = event[i];
                if(c == ',')
                    break;
                tg_data.supergroup[j++] =c;
            }
        }
    }

    printf("Supergroup id: %s\n", tg_data.supergroup);

    // Save data to file

    FILE* ini_file = fopen("./config.ini", "w");
    fprintf(ini_file,
        "api_id = %s\napi_hash = %s\nchat = %s\nsupergroup = %s",
        tg_data.api_id, tg_data.api_hash, tg_data.chat, tg_data.supergroup);
    fclose(ini_file);

    printf("Setup complete!\n");
}

unsigned int parse_id(char* event) {
    int id = 0;
    int parsing_id = 0;
    for(int i=4; i<strlen(event); i++) {
        if(!parsing_id) {
            char c1 = event[i-4];
            char c2 = event[i-3];
            char c3 = event[i-2];
            char c4 = event[i-1];
            char c5 = event[i-0];

            if(c1 == '\"' && c2 == 'i' && c3 == 'd' && c4 == '\"' && c5 == ':') {
                parsing_id = 1;
            }
        } else {
            char n = event[i];
            if(n == ',')
                break;
            id *= 10;
            id += (n - 48);
        }
    }
    return id;
}

void tg_close() {
    td_json_client_destroy(client);
}