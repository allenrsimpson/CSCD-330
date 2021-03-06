#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
//#include <zconf.h>
#include <strings.h>

bool threadFinished = 0;

void *readThread(int *sockfd) {
    int ret, x, length;
    while (1) {
        char reads[262173];
        char command[1];
        char option[20];
        char size[8];
        char content[262144];
        char fileName[20];
        char fileType[4];

        bzero(&reads, sizeof(reads));

        //bzero(&command, sizeof(command));
        bzero(&option, sizeof(option));
        bzero(&size, sizeof(size));
        bzero(&content, sizeof(content));
        bzero(&fileName, sizeof(fileName));


        ret = read(*sockfd, reads, sizeof(reads));
        if (reads[0] == 'd') {
            printf("SERVER DISCONNECTED...\nProgram Exiting...\n");
            threadFinished = 1;
            return NULL;
        } else if (ret > 0) {
            reads[ret] = '\0';

            memcpy(command, &reads[0], sizeof(command));
            memcpy(option, &reads[1], sizeof(option));
            memcpy(size, &reads[21], sizeof(size));

            if (command[0] != 'i')
                memcpy(content, &reads[29], sizeof(content));
            else{
                memcpy(fileType, &reads[29], sizeof(fileType));
                memcpy(content, &reads[33], sizeof(content));
                printf("\nfiletype: %s\n",fileType);
            }

            //memcpy(&packet[0], &command[0], sizeof(command));

            length = atoi(size);

            if (command[0] == 'g' || command[0] == 'f') {

                if (command[0] == 'f')
                    printf("Private ");

                printf("File from: %s\n", option);


                FILE *f2write = NULL;


                for (x = 0; x < 100; x++) {


                    sprintf(fileName, "%s%d", "download_", x);
                    strcat(fileName, ".jpg");
                    f2write = fopen(fileName, "r");


                    if (f2write == NULL) {

                        f2write = fopen(fileName, "w");
                        fwrite(&content, 1, length, f2write);
                        printf("File Downloaded: %s\n", fileName);
                        x = 100;
                    }

                }
                if (f2write != NULL)
                    fclose(f2write);


            } else if (command[0] == 'i') {

                printf("File from: %s\n", option);
                char prefix[3];
memcpy(prefix, &fileType[1], sizeof(prefix));

                FILE *f2write = NULL;


                for (x = 0; x < 100; x++) {


                    sprintf(fileName, "%s%s%d", prefix, "_download_", x);
                    strcat(fileName, fileType);
                    f2write = fopen(fileName, "r");


                    if (f2write == NULL) {

                        f2write = fopen(fileName, "w");
                        fwrite(&content, 1, length, f2write);
                        printf("File Downloaded: %s\n", fileName);
                        x = 100;
                    }

                }
                if (f2write != NULL)
                    fclose(f2write);


            } else {

                printf("%s: %s\n", option, content);
            }
        }


    }//end while
}//end readThread

int main(int argc, char **argv) {

    int port = atoi(argv[1]);
    int sockfd, len, ret;
    struct sockaddr_in saddr;

    // creating tcp socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port); //from command line
    saddr.sin_addr.s_addr = inet_addr(argv[2]);//loop back to self (OS loops packet back)
    len = sizeof(saddr);


    ret = connect(sockfd, (struct sockaddr *) &saddr, len);
    if (ret == -1) {
        perror("connect ");
        exit(1);
    }
    printf("::CLIENT READY::\n");
    printf("--Connected to Server--\n");

    pthread_t reader;
    pthread_create(&reader, NULL, (void *) *readThread, (void *) &sockfd);

    //CLIENT-SIDE EXAMPLE:
//This is one example (a very explicit one)
//of how to make a packet using the protocol.
//write functions/methods instead of doing it this way.

    //INITIALIZE FIELDS
    char command[1];
    char option[20];
    char size[8];
    char content[262144];
    char packet[262173]; //the packet size will be the command + option + size + content (which will be the MAX size of a file).
    char filePath[100];
    char fileType[4];


    char room1[2] = "X";

    while (!threadFinished) {
        bzero(&command, sizeof(command));
        bzero(&option, sizeof(option));
        bzero(&size, sizeof(size));
        bzero(&content, sizeof(content));
        bzero(&packet, sizeof(packet));
        bzero(&filePath, sizeof(filePath));
        bzero(&fileType, sizeof(fileType));
        FILE *f2;
        int x;

        char writes[262173];
        bzero(&writes, sizeof(writes));
        fgets(writes, 262173, stdin);

        if (writes[0] == '/') {
            int u;
            //command[0] = writes[1];
            switch (writes[1]) {
                case 'b'    ://////////////////////////////////////////////////////done
                    command[0] = 'b';//broadcast server wide

                    memcpy(content, &writes[3], sizeof(content));

//                    for (u = 0; u < sizeof(writes) - 3; u++) {
//                        content[u] = writes[u + 3];
//                    }
                    break;

                case 'c'    ://////////////////////////////////////////////////////done
                    command[0] = 'c';//command list//commands.commandList(i);break;
                    break;

                case 'd'    ://////////////////////////////////////////////////////done
                    command[0] = 'd';//commands.disconnect(i);
                    break;

                case 'e'    ://////////////////////////////////////////////////////done
                    command[0] = 'e';//commands.exitRoom(i);
                    break;


                case 'f'    :
                    command[0] = 'f';//commands.pFile(clientList,packet);private file


                    for (x = 0; x < 24 && writes[x + 3] != ' '; x++)
                        option[x] = writes[x + 3];

                    int w = x += 4;

                    for (; x < sizeof(filePath) + w && writes[x] != '\n'; x++)
                        filePath[(x - w)] = writes[x];

                    //printf("filename: %s", filePath);


                    FILE *f1 = fopen(filePath, "r");

                    if (f1 != NULL) {
                        fseek(f1, 0, SEEK_END);
                        long length2 = ftell(f1);
                        fseek(f1, 0, SEEK_SET);

                        sprintf(size, "%d", length2);

                        char picture[length2];

                        int lengthX = fread(content, 1, length2, f1);
                        fclose(f1);


                    } else {
                        printf("\n-----------------File does not exist--------------\n");
                        command[0] = 'z';
                    }

                    break;

                case 'g'   :
                    command[0] = 'g';//group file

                    for (x = 0; x < sizeof(filePath); x++)
                        filePath[x] = writes[x + 3];

                    for (x = 0; x < sizeof(filePath); x++)
                        if (filePath[x] == '\n')
                            filePath[x] = '\0';

                    f2 = fopen(filePath, "r");


                    ///////////////////////////////////check for null file////////////////////
                    if (f2 != NULL) {
                        fseek(f2, 0, SEEK_END);
                        long length2 = ftell(f2);
                        fseek(f2, 0, SEEK_SET);

                        sprintf(size, "%d", length2);

                        fread(content, 1, length2, f2);
                        fclose(f2);


                    } else {
                        printf("\n-----------------File does not exist--------------\n");
                        command[0] = 'z';
                    }


                    break;


                case 'h'    ://////////////////////////////////////////////////////done
                    command[0] = 'h';//who's in my room?
                    break;

                case 'i'    ://////////////////////////////////////////////////////done
                    command[0] = 'i';
                    for (x = 0; x < sizeof(filePath); x++)
                        filePath[x] = writes[x + 3];


                    for (x = 0; x < sizeof(filePath); x++)
                        if (filePath[x] == '\n')
                            filePath[x] = '\0';

                    f2 = fopen(filePath, "r");

                    printf("Please enter file type: ");
                    fgets(fileType, 10, stdin);

                    x = strlen(fileType)-1;
                    if( fileType[x] == '\n')
                        fileType[x] = '\0';

                    ///////////////////////////////////check for null file////////////////////
                    if (f2 != NULL) {
                        fseek(f2, 0, SEEK_END);
                        long length2 = ftell(f2);
                        fseek(f2, 0, SEEK_SET);

                        sprintf(size, "%d", length2);

                        bzero(&writes, sizeof(writes));
                        fread(writes, 1, length2, f2);
                        fclose(f2);


                        memcpy(content, &fileType[0], sizeof(fileType));
                        memcpy(&content[4], &writes, length2);

                    } else {
                        printf("\n-----------------File does not exist--------------\n");
                        command[0] = 'z';
                    }
                    break;

                case 'l'    ://////////////////////////////////////////////////////done
                    command[0] = 'l';//list all users in all rooms
                    break;

                case 'n'    ://////////////////////////////////////////////////////done
                    command[0] = 'n';//name registration


                    for (x = 0; x < 21 /*&& (writes[x + 3] != ' ' || writes[x + 3] != '\n')*/; x++)
                        option[x] = writes[x + 3];
                    break;

                case 'r'    ://////////////////////////////////////////////////////done
                    command[0] = 'r'; //explicitly send message to room
                    for (x = 0; x < sizeof(writes) - 2; x++)
                        content[x] = writes[x + 3];
                    break;

                case 's'    ://////////////////////////////////////////////////////done
                    command[0] = 's';//switch rooms
                    for (x = 0; x < 21; x++)
                        option[x] = writes[x + 3];
                    break;

                case 'w'    ://////////////////////////////////////////////////////done
                    command[0] = 'w';//Whisper

                    for (x = 0; x < 24 && writes[x + 3] != ' '; x++)
                        option[x] = writes[x + 3];

                    // leaves at space after name
                    int y = x += 4;

                    for (; x < sizeof(content) + y; x++)/////// puts message into content///
                    {
                        content[(x - y)] = writes[x];
                    }


                    break;


/////////////////////////////////////////////Invalid Command - should be on server
                default        :
                    printf("%c%c Is not a valid command. Type /c for list of current commands\n", writes[0], writes[1]);
                    command[0] = writes[1];
                    break;
            }

        }

            /////////////////////////////////////No Command Given//////////////////////
        else {
            int u;
            command[0] = 'r';

            for (u = 0; u < sizeof(writes); u++) {
                content[u] = writes[u];
            }

        }

/////////////////////////////////////////BUILDING PACKET///////////////////////////////


        if (command[0] != 'z') {
            //content 262144
            packet[0] = command[0];

            for (x = 1; x < 21; x++) {
                if (option[x - 1] == '\n')
                    packet[x] = '\0';

                else
                    packet[x] = option[x - 1];
            }
            //SIZE -> packet[21 - 29]
            for (x = 21; x < 29; x++) {
                packet[x] = size[x - 21];
            }
            //CONTENT -> packet[29 -
            for (x = 29; x < 262144; x++) {
                packet[x] = content[x - 29];
            }

            write(sockfd, packet, sizeof(packet));
        }//end If
//////////////////////////////////////PACKET BUILT//////////////////////////////////////



    }

}//end main
