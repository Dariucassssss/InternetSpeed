#include <stdio.h>
#include <cJSON/cJSON.h>
#include <getopt.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

struct upload_ctx {
    const char *data;
    size_t size;
    size_t pos;
};

void upload_speed(CURL *curl){
    CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
    result = curl_easy_perform(curl);
    if(result != CURLE_OK){
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
        curl_easy_strerror(result));
    }

    curl_off_t uploaded;
    curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &uploaded);
    printf("Uploaded: %.3f Mbits\n", (double)uploaded/(1024*1024)*8); // doubledouble is silly but my c is bad so whatever
    curl_off_t speed;
    curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed);
    printf("Upload speed: %.3f Mbits/sec\n", (double)speed/(1024*1024)*8);
    
    return;
}

void download_speed(CURL *curl){
    CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
    result = curl_easy_perform(curl);
    if(result != CURLE_OK){
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
        curl_easy_strerror(result));
    }

    curl_off_t download_speed;
    curl_off_t downloaded;
    curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD_T, &download_speed);
    curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &downloaded);
    printf("Download speed: %.3f Mbits/sec\n", (double)download_speed/(1024*1024)*8);
    printf("Downloaded: %.3f Mbits\n", (double)downloaded/(1024*1024)*8);
    // curl_easy_cleanup(curl);
    return;
}


size_t read_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    struct upload_ctx *ctx = (struct upload_ctx *)userdata;
    size_t max = size * nmemb;

    size_t remaining = ctx->size - ctx->pos;
    size_t to_copy = remaining < max ? remaining : max;

    if (to_copy > 0) {
        memcpy(ptr, ctx->data + ctx->pos, to_copy);
        ctx->pos += to_copy;
    }

    return to_copy; // return 0 when done
    return 0;
}

size_t discard_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    return size * nmemb; // just ignore everything
}

// int something(){
//     CURL *curl;

//     CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
//     if(result != CURLE_OK)
//         return (int)result;

//     FILE *fp = fopen("speedtest_server_list.json", "r");

//     if (fp == NULL) {
//         perror("Failed to open file");
//         return 1;
//     }

//     // Get file size
//     fseek(fp, 0, SEEK_END);
//     long length = ftell(fp);
//     fseek(fp, 0, SEEK_SET);

//     // Allocate memory for content
//     char *content = (char *)malloc(length + 1);
//     fread(content, 1, length, fp);
//     fclose(fp);
//     content[length] = '\0'; // Null-terminate

//     cJSON *json = cJSON_Parse(content);
//     free(content); // Free the allocated memory

//     if (json == NULL) {
//         printf("Error parsing JSON\n");
//         return 1;
//     }

//     int count = cJSON_GetArraySize(json);
//     int index = -1;
//     for(int i =0; i<count; i++){
//         cJSON *item = cJSON_GetArrayItem(json, i);
//         cJSON *country = cJSON_GetObjectItemCaseSensitive(item, "country");
//         cJSON *host = cJSON_GetObjectItemCaseSensitive(item, "host");
//         // printf("%s \n", country->valuestring);
//         if(strcmp(country->valuestring, "Lithuania")==0 && index==-1){
//             // printf("%d \n",i);
//             index = i;
//         }
//         // if (host && cJSON_IsString(host)) {
//         //     printf("%s\n", host->valuestring);
//         // }
//     }
//     cJSON *item = cJSON_GetArrayItem(json, index);
//     cJSON *host = cJSON_GetObjectItemCaseSensitive(item, "host");
    
//     curl = curl_easy_init();
//     if(curl) {

//         size_t data_size = 20 * 1067 * 1069; // more than 20 MB
//         char *data = malloc(data_size);

//         memset(data, 'a', data_size); // fill with dummy data
//         // struct curl_slist *headers = NULL;
//         curl_easy_setopt(curl, CURLOPT_POST, 1L); 
//         curl_easy_setopt(curl, CURLOPT_URL, "https://httpbin.org/post");
//         // curl_easy_setopt(curl, CURLOPT_URL, host->valuestring);

        
//         struct upload_ctx ctx = {
//             .data = data,
//             .size = data_size,
//             .pos = 0
//         };

//         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_callback);
//         curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
//         curl_easy_setopt(curl, CURLOPT_READDATA, &ctx);
//         curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)data_size);
//         // curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_size);
//         // curl_easy_setopt(curl, CURLOPT_POSTFIELDS, &ctx);

//         /* Perform the request, result gets the return code */
//         result = curl_easy_perform(curl);
//         /* Check for errors */
//         if(result != CURLE_OK){
//             fprintf(stderr, "curl_easy_perform() failed: %s\n",
//             curl_easy_strerror(result));
//         }
        
//         upload_speed(curl);

//         download_speed(curl);

//         // printf("======================\n");
//         // printf("%s \n", host->valuestring);
        
//         /* always cleanup */
//         curl_easy_cleanup(curl);
//     }
//     curl_global_cleanup();


//     cJSON_Delete(json);
//     return 1;
// }

// getopts: https://www.youtube.com/watch?v=SjyR74lbZOc
void print_usage(){
    printf("usage: ./main -d | -u | -a\n");
    printf(" ./main -h will give you a simple help menu\n");
    exit(2);
}

int main(char argc, char **argv){
    // getopts: https://www.youtube.com/watch?v=SjyR74lbZOc <-- this guy helped out w getopt
    if(argc < 2){
        print_usage();
    }
    int option;
    int aflag=0;
    int uflag=0;
    int dflag=0;

    CURL *curl;
    // CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    size_t data_size = 10 * 1069 * 1067; // more than 10 MB
    char *data = malloc(data_size);

    memset(data, 'a', data_size); 
    char *host = "https://httpbin.org/post";
    struct upload_ctx ctx = {
        .data = data,
        .size = data_size,
        .pos = 0
    };
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L); 
        curl_easy_setopt(curl, CURLOPT_URL, host);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_callback);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &ctx);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)data_size);   
    }
    while((option = getopt(argc, argv, "aduh")) != -1){
        switch (option){
            case 'a':
                if(aflag){
                    print_usage();
                } else {
                    aflag++;
                    uflag++;
                    dflag++;
                }
                printf("Checking download speed\n");
                download_speed(curl);
                printf("==================\n");
                
                ctx.pos=0;

                printf("Checking upload speed\n");
                upload_speed(curl);
                printf("==================\n");
                printf("host: %s\n", host);
                break;
            case 'd':
                if(dflag){
                    print_usage();
                } else {
                    aflag++;
                    uflag++;
                    dflag++;
                }
                printf("Checking download speed\n");
                download_speed(curl);
                printf("==================\n");
                printf("host: %s\n", host);
                break;
            case 'u':
                if(uflag){
                        print_usage();
                    } else {
                        aflag++;
                        uflag++;
                        dflag++;
                    }
                printf("Checking upload speed\n");
                upload_speed(curl);
                printf("==================\n");
                printf("host: %s\n", host);
                break;
            case 'h':
                printf("-a || checks upload and download speed.\n");
                printf("-d || checks download speed.\n");
                printf("-u || checks upload speed.\n");
                printf("-h || gives help options (wow).\n");
                break;
            default: printf("-h for help\n");
        }
    }

    return 0;
}


