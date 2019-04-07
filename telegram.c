#include "telegram.h"


void tg_initialize() {
    tg_parse_data();

    td_set_log_verbosity_level(5);
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
            exit(0);
        }
    }
}

unsigned int tg_send_message(char* message) {
    char req[5500];
    sprintf(req,
        "{\"@type\": \"sendMessage\", \"chat_id\": \"%s\", \"input_message_content\": {\"@type\": \"inputMessageText\", \"text\": {\"@type\": \"formattedText\", \"text\": \"%s\", \"entities\": [{\"@type\": \"textEntity\", \"offset\": 0, \"length\": \"%d\", \"type\": {\"@type\": \"textEntityTypeCode\"}}]}}, \"@extra\": \"message sent\"}",
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
            printf("%s\n", event);
    }

    if(!success)
        return 0;

    long id = 0;

    int parsing_id = 0;

    for(int i=4; i<strlen(event); i++) {
        if(!parsing_id) {
            char c1 = event[i-4];
            char c2 = event[i-3];
            char c3 = event[i-2];
            char c4 = event[i-1];
            char c5 = event[i];

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

    return id >> 20;
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