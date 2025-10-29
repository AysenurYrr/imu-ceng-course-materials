#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#define INPUT_SIZE 1024

char **tokinize(char *str){ 
    char **args = malloc(sizeof(char*) * 100 + sizeof(char) * 100 * 50);
    int i=0;
    char *saveptr;
    char* token = strtok_r(str, " ", &saveptr);
    while (token != NULL) {
        args[i] = token;
        token = strtok_r(NULL, " ", &saveptr);
        i++;
    }
    args[i] = NULL;
    return args;
}


int main(){
    char usrinput[INPUT_SIZE];
    int size = 0;
    int r;

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

        char **args = tokinize(usrinput);
        if (args[0]!=NULL &&(strcmp(args[0],"cat") == 0)){
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
                        usrinput[size-1] = '\0';
                        break;
                    }
                }
                if ((usrinput[size-2]) == 'q'){
                    break;
                }
                write(1,usrinput,size);
                write(1,"\n",1);
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