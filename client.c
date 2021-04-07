#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#define ALL_SIZE 8192

#define IP_PROTOCOL 0
#define IP_ADDRESS "127.0.0.1" // localhost
#define PORT_NO 15050
#define NET_BUF_SIZE 1024
#define cipherKey 'S'
#define sendrecvflag 0

struct frame_t{
   	long int id;
    	long int length;
    	char data[ALL_SIZE+1];
};

/*Deklarasi*/
void clearBuf(char *b);
int recvFile(char buff[ALL_SIZE+1]);
int parity_check(char data[ALL_SIZE+1]);
unsigned char byte_to_ascii_char(char input[8]);

int main(int argc, char* argv[]){
	/*Deklarasi Variabel*/
    	int sockfd, nBytes;
    	struct sockaddr_in addr_con;
    	memset(&addr_con, 0, sizeof(addr_con));
    	addr_con.sin_family = AF_INET;
    	addr_con.sin_port = htons(PORT_NO);
    	addr_con.sin_addr.s_addr = inet_addr(argv[1]);
    	if (argc < 3){
        	printf("[!] input command : ./client [ip address] [file_name]\n");
        	return 1;
    	}
    	int addrlen = sizeof(addr_con);
    	char net_buf[NET_BUF_SIZE];
    	struct frame_t frame;
    	struct timeval t_out = {0, 0};
    	/*Pointer File*/
	FILE *fp;
    	int file_existed = 1;
    	strcpy(net_buf, argv[2]);
    	long int total_frame;
    	// socket()
    	sockfd = socket(AF_INET, SOCK_DGRAM,IP_PROTOCOL);
    	if (sockfd < 0)
        	printf("\nfile descriptor not received!!\n");
    	else
        	printf("\nfile descriptor %d received\n", sockfd);
    	/*Looping*/
	while (1){
		/*Send Nama File Entry*/
        sendto(sockfd, net_buf, NET_BUF_SIZE, sendrecvflag, (struct sockaddr *)&addr_con, addrlen);
        	/*Receive file entry yang diterima server*/
		recvfrom(sockfd, &file_existed, sizeof(int), sendrecvflag, (struct sockaddr *)&addr_con, &addrlen);
        	/*Kondisi File Tidak Tersedia*/
		if (file_existed == 0){
            		printf("File doesn't exist!\n");
            		break;
        	}
        	sleep(5);
        	t_out.tv_sec = 2;
        	/*menerima total frame dari server*/
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval));
		recvfrom(sockfd, &total_frame, sizeof(long int), sendrecvflag, (struct sockaddr *)&addr_con, &addrlen);
        	t_out.tv_sec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval));
		/*Kondisi Jika Total Frame Sudah Berhasil Diterima*/
		if (total_frame > 0){
		
		
			int flag_frame = 0;
			/*kirim total frame ke server*/
            	sendto(sockfd, &(total_frame), sizeof(total_frame), 0, (struct sockaddr *)&addr_con, addrlen);
            		printf("\n---------Data Received---------\n");
            	       /**/
		       FILE *file_target = fopen(net_buf, "w");
            	       for (long int i = 1; i <= total_frame; i++){
            	       //printf("==================frame================================================================\n");
                		memset(&frame, 0, sizeof(frame));
                		/*proses menerima data dari server*/
                		recvfrom(sockfd, &frame, sizeof(frame), sendrecvflag, (struct sockaddr *)&addr_con, &addrlen);
                		printf("Frame ID %ld \n", frame.id);
                		/*cek apakah frame id yang dikirim telah sesuai dengan loop dari total frame-nya*/ 
                		if ((frame.id < i) || (frame.id > i) ){
                			printf("\n receive ==> %ld , there is an error bit\n", frame.id);
                    			i--;
                    		}
                    		else if((frame.id==4) && (flag_frame==0)){
                				printf("\nerror receiving ==> %ld , resend \n", frame.id);
                    			i--;
                				flag_frame = 1;
                			}
                			else if((frame.id==7) && (flag_frame==0)){
                				printf("\nerror receiving ==> %ld , resend \n", frame.id);
                    			i--;
                				flag_frame = 1;
                			}
                			else{	
                				flag_frame=0;
                				sendto(sockfd, &(frame.id), sizeof(frame.id), 0, (struct sockaddr *)&addr_con, addrlen);
								/*cek parity bit dari data yang diterima dari server*/
                    			if (parity_check(frame.data) == 0){
                        			unsigned char the_stream[ALL_SIZE];
	 	                        	unsigned char dump_8[8];
                        			int x = 0;
								/*loop untuk membaca data per 8 char dari frame data yang dikirim tadi (2048) untuk dubah 							ke fungsi byte2ASCII */
                        			for(int i = 0; i < ALL_SIZE; i++){
                       				// mengisi elemen dari tiap array dari frame data sebanyak 8 char
                       				dump_8[x]=frame.data[i];
                       				if((i+1)%8==0){
                        					//dump_8[8]='\0';
                        					unsigned char the_char = byte_to_ascii_char(dump_8);
                            					//printf("%c", the_char); //print text yang telah diubah
                            					the_stream[i] = the_char;
                            					/*menulis the_char ke file target*/
                            					fputc(the_stream[i], file_target);
                            					//fwrite(the_stream, 8, sizeof(unsigned char), file_target);
                        					x=0;
                        				}
                        				else{
                        					x++;
                        				}
                        			 }
                        			//fwrite(the_stream, 1024 , sizeof(unsigned char), file_target);
                        			/**/	
                    			    }
                                       /*Kondisi Terdapat bit error pada frame data*/
                    			else{
                    			
                        			recvFile(frame.data);
                        			if ((frame.id < i) || (frame.id > i)){
                    					i--;
                				}
                        			printf(" receive ==> %ld , there is an error bit\n", frame.id);
                        			
                    			}
                		}
            		}
            		printf("\n-------------------------------\n");
            		//printf("Frame frame = %s\n", frame.data); //print frame data yang diterima
            		//printf("total frame = %ld\n", total_frame);
            		//printf(" send ACK NUMBER==> %ld\n", ack_num);
                	printf("\n send FRAME ID==> %ld\n", frame.id);
            		fclose(file_target);
            		break;
        	}
	}
    	return 0;
}

// function to clear buffer
void clearBuf(char *b){
    	int i;
    	for (i = 0; i < NET_BUF_SIZE; i++)
        	b[i] = '\0';
}

// function to receive file dan print 
int recvFile(char buff[ALL_SIZE+1]){
        for (int y = 0; y < ALL_SIZE+1; y++){
            printf("%c", buff[y]);
        }
        printf(" - ");
    	return 0;
}
/*check parity dari frame data yg dikirim*/
int parity_check(char data[ALL_SIZE+1]){
    	int count_one = 0;
    	int any_error = 0;
   	for (int j = 0; j < ALL_SIZE+1; j++){
            	if (data[j] == '1')
                	count_one++;
        	}
        	if (count_one % 2 != 0){
            		any_error = 1;
        	}
    		if (any_error == 1){
        		return 1;
    		}
   		 return 0;
	}

/**/
unsigned char byte_to_ascii_char(char input[8]){
    
    	unsigned char dump[8];
    	int num_count = 0;
    	for(int i = 0; i < 8; i++){
        	dump[i] = input[i];
    	}
    	
    	//printf("isinya dump brad === %s\n",dump);
    	for(int i = 0; i < 8; i ++){
        	num_count += (((int) dump[i] - 48) * pow(2, 7 - i));
    	}
    	//printf("char nya brad === %c\n",(char)num_count);
    	return (char)num_count;
}
