#include "telegram.h"


// Sends a request and returns the result of that request according to the "extra" param
char* tg_request(const char *request, const char* extra) {
    td_json_client_send(client, request);
    char* response;

    do {
        response = td_json_client_receive(client, TIMEOUT);
    } while(response && strstr(response, extra) == NULL);

    return response;
}

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
    // If nothing matches then we're good and connected.
    char* response;

    while(1) {
        response = td_json_client_receive(client, TIMEOUT);

        if(!response)
            break;

        if(strstr(response, "authorizationStateWaitPhoneNumber") != NULL) {
            // Need to give phone number
            printf("Phone number: ");
            char phone_number[20] = "+15817015326";
            scanf("%s", phone_number);

            char request_phone_number[500];
            sprintf(request_phone_number, "{\"@type\": \"setAuthenticationPhoneNumber\", \"phone_number\": \"%s\", \"allow_flash_call\": false, \"is_current_phone_number\": false, \"@extra\": \"phone_number\"}", phone_number);
            printf("%s\n", request_phone_number);

            td_json_client_send(client, request_phone_number);
        }

        if(strstr(response, "authorizationStateWaitCode") != NULL) {
            // Need to give the verification code
            printf("Verification code: ");

            char code[5];
            scanf("%s", code);

            char request_code_auth[500];
            sprintf(request_code_auth, "{\"@type\": \"checkAuthenticationCode\", \"code\": \"%s\", \"first_name\": \"\", \"last_name\": \"\", \"@extra\": \"auth_code\"}", code);

            td_json_client_send(client, request_code_auth);
        }

        if(strstr(response, "authorizationStateWaitPassword") != NULL) {
            // Need to give a password
            printf("Sorry, telegram account passwords aren't supported.");
            exit(0);
        }
    }
}

void tg_parse_data() {
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