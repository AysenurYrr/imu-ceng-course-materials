#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#define INPUT_SIZE 1024

char **tokinize(char *str, char *del) {
    char **args = malloc(sizeof(char*) * 100);  
    if (args == NULL) {
        perror("malloc failed");
        exit(1);
    }

    int i = 0;
    char *token = strtok(str, del);
    while (token != NULL) {
        args[i] = token;
        token = strtok(NULL, del);
        i++;
    }
    args[i] = NULL;

    return args;
}


int main(){
    char usrinput[INPUT_SIZE];
    int size = 0;
    int r;
    int myvariable;
    //surekli calismasi icin
    while(1){
        write(1,"$ ",2);
        size=0;
        memset(usrinput, 0, sizeof(usrinput)); //usrinput sifirliyoruz

        // her satiri okumak icin while
        while((r = read(0,&usrinput[size],1)) > 0){
            size++;
            if (size > INPUT_SIZE){
                perror("buffer size limit");
                exit(1);
            }
            if ((usrinput[size-1]) == '\n'){
                usrinput[size-1] = '\0';
                break;
            }
        }
        if (r<=0){
            perror("read error");
            exit(1);
        }

        // if (strstr(usrinput, ">") != NULL) 
        //     myvariable = 1;

        char **args = tokinize(usrinput," ");
        
        //cat
        if ((args[0]!=NULL) && (strcmp(args[0],"cat") == 0)){
            //sadece cat ile acarsam duzgun calisiyor. q ve EOF ile cikis 
            if (args[1] == NULL){   
                while(1){
                    // memset ile usrinput icinde kalan eski datalari temizliyoruz.
                    memset(usrinput, 0, sizeof(usrinput));
                    size=0;
                    while((r = read(0,&usrinput[size],1)) > 0){
                        size++;
                        if (size > INPUT_SIZE){
                            perror("buffer size limit");
                            exit(1);
                        }
                        if ((usrinput[size-1]) == '\n'){
                            usrinput[size-1] = '\0';
                            break;
                        }
                    }
                    if (r == 0 || (usrinput[0]) == 'q'){
                        break;
                    }
                    write(1,usrinput,size);
                    write(1,"\n",1);
                }
            }
            // cat > example.txt
            else if (*args[1]=='>') {
                char *filename;
                filename = malloc(strlen(args[2]) + 1);  // dosya ismi için bellek ayır
                if (filename == NULL) {
                    perror("malloc failed");
                    exit(1);
                }
                strcpy(filename, args[2]);
                // write(1, filename, strlen(filename));
                int input_f = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0666);
                if (input_f == -1) {
                    perror("open failed");
                    return -1;
                }
                while(1){
                    memset(usrinput, 0, sizeof(usrinput));
                    size=0;
                    while((r = read(0,&usrinput[size],1)) > 0){
                        size++;
                        if (size > INPUT_SIZE){
                            perror("buffer size limit");
                            exit(1);
                        }
                        if ((usrinput[size-1]) == '\n'){
                            // usrinput[size-1] = '\0';
                            break;
                        }
                    }
                    if (r == 0 || (usrinput[0]) == 'q'){
                        break;
                    }
                    write(input_f,usrinput,size);
                    write(input_f,"\n",1);
                }
                close(input_f);
            }

            else if(args[2]==NULL){
                // burasi "cat den.txt" gibi olacak. den.txt göstersin? ya da göstermiesin
                char *filename;
                if (args[1] != NULL) {  // args[1]'in NULL olup olmadığını kontrol et
                    filename = malloc(strlen(args[1]) + 1);  // dosya ismi için bellek ayır
                    if (filename == NULL) {
                        perror("malloc failed");
                        exit(1);
                    }
                    strcpy(filename, args[1]);
                    // write(1, filename, strlen(filename));
                }else{
                    fprintf(stderr, "error: missing filename\n");
                    exit(1);
                }

                int input_f = open(filename, O_RDONLY);
                if (input_f == -1) {
                    perror("open failed");
                    return -1;
                }

                char buffer[512];  
                ssize_t bytesRead;  
                while ((bytesRead = read(input_f, buffer, 512)) > 0) {
                    write(1, buffer, bytesRead);
                }

                if (bytesRead == -1) {
                    perror("Dosya okuma hatası");
                }
                close(input_f);

            }
        
        }
        
        // pid_t pid = fork(); // fork() ile yeni süreç oluştur
        
        // if (pid == -1) { // fork başarısızsa hata
        //     perror("fork failed");
        //     exit(1);
        // }

        // if (pid == 0) { // Çocuk süreç
        //     // execv çağırarak yeni program çalıştır
        //     if (execv(args[0], args) == -1) {
        //         perror("execv failed");
        //         exit(1);
        //     }
        // } else { // Ebeveyn süreç
        //     // Çocuk sürecin bitmesini bekle
        //     wait(NULL); // wait() çocuk sürecin bitmesini bekler
        // }

        free(args);

}
    return 0;
}