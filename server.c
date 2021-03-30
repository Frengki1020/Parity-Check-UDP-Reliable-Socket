#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define ALL_SIZE 8192


#define IP_PROTOCOL 0
#define PORT_NO 15050
#define NET_BUF_SIZE 2048+1
#define FILE_BUFF 8192
#define BUFF 1
#define cipherKey 'S'
#define sendrecvflag 0
#define nofile "File Not Found!"


struct frame_t{
	long int id;
    	long int length;
    	char data[ALL_SIZE+1];
};

/*Deklarasi Variabel*/
void clearBuf(char *b);
char count_parity(char input[], int size);
int sendFile(FILE *fp, char *buf, char out[ALL_SIZE+1], int s);
int printBuff(char buff[ALL_SIZE+1]);

int main(){
   	int sockfd, nBytes;
    	struct sockaddr_in addr_con;
    	memset(&addr_con, 0, sizeof(addr_con));

    	addr_con.sin_family = AF_INET;
    	addr_con.sin_port = htons(PORT_NO);
    	addr_con.sin_addr.s_addr = INADDR_ANY;

   	int addrlen = sizeof(addr_con);
    	char net_buf[NET_BUF_SIZE];
    	char sendbuff[BUFF];
    	/**/
    	struct frame_t frame;
    	struct stat st;
    	struct timeval t_out = {0, 0};
    	FILE *fp, *file_binary;
    	char bug;
    	long int frame_id_bug;

    	/*Membuat Socket*/
    	sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL);
    	if (sockfd < 0)
        	printf("\nfile descriptor not received!!\n");
    	else
        	printf("\nfile descriptor %d received\n", sockfd);
    	/*Membuat Binding*/
    	if (bind(sockfd, (struct sockaddr *)&addr_con, sizeof(addr_con)) == 0)
        	printf("\nSuccessfully binded!\n");
    	else
        	printf("\nBinding Failed!\n");
    	/*Loop*/
    	while (1){
        	printf("Waiting for file name...\n");
        	clearBuf(net_buf);
        	/*menerima file yang telah di-entry di client*/
        	nBytes = recvfrom(sockfd, net_buf,NET_BUF_SIZE, sendrecvflag, (struct sockaddr *)&addr_con, &addrlen);
        	printf("\nFile Name Received: %s\n", net_buf);
        	char find_file[100] = "";
        	/**/
        	strcat(find_file, net_buf);
        	printf("%s\n", find_file);
        	
        	fp = fopen(find_file, "r");
        	
        	long int file_size = 0, total_frame = 0;
        	int file_found = 0;
        	/*Kondisi File tidak ditemukan*/
        	if (fp == NULL){
            		printf("\nFile open failed!\n");
            		sendto(sockfd, &file_found, sizeof(int), sendrecvflag, (struct sockaddr *)&addr_con, addrlen);
        	}
        	/*Kondisi File ada*/
        	else{
            		file_found = 1;
            		//kirim status file found ke client
            		sendto(sockfd, &file_found, sizeof(int), sendrecvflag, (struct sockaddr *)&addr_con, addrlen);
            		printf("\nFile Successfully opened!\n");
            		char bin_file[100] = "";
            		/*Menambahkan String dari net_buf ke bin_file*/
            		strcat(bin_file, net_buf);
            		strcat(bin_file, ".txt");
            		file_binary = fopen(bin_file, "r");
            		/**/
            		if (file_binary == NULL){
                		printf("\nBinary file is not existed! Creating process ...\n");
                		char command[200] = "xxd -b -c1 ";
                		strcat(command, find_file);
                		strcat(command, " | cut -d\" \" -f2 | tr -d \"\\n\" > ");
                		strcat(command, bin_file);
                		system(command);
                		file_binary = fopen(bin_file, "r");
                		printf("Load file ... \n");
                		sleep(5);
            		} 
            		else{
                		printf("Load file ... \n");
                		sleep(5);
            		}
            		long int ack_num;
            		long int drop_frame = 0, resend_frame = 0, t_out_flag = 0;
            		/**/
            		stat(bin_file, &st);
            		file_size = st.st_size;
			
			
			printf("%ld\n",file_size);
			
			
			
            		t_out.tv_sec = 2;
            		t_out.tv_usec = 0;
            		/**/
            		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval));
            		/*partisi frame data terhadap file buff*/
            		if ((file_size % FILE_BUFF) != 0)
                		total_frame = (file_size / FILE_BUFF) + 1;
            		else
                		total_frame = (file_size / FILE_BUFF);
            			printf("Total Frame : %ld\n", total_frame);
            		/*kirim total frame ke client*/
            		sendto(sockfd, &total_frame, sizeof(long int), sendrecvflag, (struct sockaddr *)&addr_con, addrlen);
            		//menerima ack number dari client
            		recvfrom(sockfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *)&addr_con, &addrlen);
            		printf("ACK Number : %ld\n", ack_num);
            		/*kondisi jika ack number yg dikirim tidak sesuai dengan yg diterima*/
            		while (ack_num != total_frame){
            			/**/
                		sendto(sockfd, &total_frame, sizeof(long int), sendrecvflag, (struct sockaddr *)&addr_con, addrlen);
                		recvfrom(sockfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *)&addr_con, &addrlen);
                		resend_frame++;
                		/**/
                		if (resend_frame == 10){
                    			t_out_flag = 1;
                    			break;
                		}
            		}
            		/*proses kirim data*/
            		for (long int i = 1; i <= total_frame; i++){
                		memset(&frame, 0, sizeof(frame));
                		ack_num = 0;
                		frame.id = i;
                		/*kirim frame file binary ke client*/
                		sendFile(file_binary, net_buf, frame.data, FILE_BUFF);
                		//kirim file binary ke client
                		sendto(sockfd, &frame, sizeof(frame), sendrecvflag, (struct sockaddr *)&addr_con, addrlen);
                		clearBuf(net_buf);
                		//menerima ack num dari client
                		recvfrom(sockfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *)&addr_con, &addrlen);
                		/*kondisi jika terjadi loss connection*/
                		while (ack_num != frame.id){
                			/**/
                    			sendFile(fp, net_buf, frame.data, FILE_BUFF);
                    			sendto(sockfd, &frame, sizeof(frame), sendrecvflag, (struct sockaddr *)&addr_con, addrlen);
                    			clearBuf(net_buf);
                    			/**/
                    			recvfrom(sockfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *)&addr_con, &addrlen);
                    			printf("[-] Frame ===> %ld	dropped, %ld times\n", frame.id, ++drop_frame);
                    			resend_frame++;
                    			/**/
                    			printf("[-] Frame ===> %ld	dropped, %ld times\n", frame.id, drop_frame);
                    			if (resend_frame == 200){
                        			t_out_flag = 1;
                        			break;
                    			}
                		}
                		resend_frame = 0;
                		drop_frame = 0;
                		/**/
                		if (t_out_flag == 1){
                    			fclose(file_binary);
                    			printf("File not sent\n");
                    			break;
                		}
                		printf(" send ==> %ld\n", ack_num);
                		
                		printf(" send ==> %ld\n", frame.id);
                		
                		/*Kondisi saat ack_num sudah sama dengan total_frame maka file berhasil dikirim*/
                		if (ack_num == total_frame){
                    			printf("File sent successfully\n");
                    			fclose(file_binary);
                		}
            		}
            		fclose(fp);
        	}
		t_out.tv_sec = 0;
		t_out.tv_usec = 0;
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval));
    	}
    	return 0;
}
/**/
void clearBuf(char *b){
    	int i;
    	for (i = 0; i < NET_BUF_SIZE; i++)
        	b[i] = '\0';
}
/*menghitung parity*/
char count_parity(char input[], int size){
    	char count = 0;
    	/**/
    	for (int i = 0; i < size; i++)
    	{
        	count += input[i];
    	}
    	/**/
    	if (count % 2 == 0)
        	return '0';
    	else
        	return '1';
}
/*function sending file*/
int sendFile(FILE *fp, char *buf, char out[ALL_SIZE+1], int s){
    	char ch, ch2;
    	int j = 0, k = 0;
    	//char temp[8][8];
    	char temp[ALL_SIZE];
    	/**/
    	for (int i = 0; i < s; i++){
		ch = fgetc(fp);    
		ch2 = ch;
		temp[i] = ch2;
		if (ch == EOF)
	    		return 1;
    	}
    	/*menambahkann parity bit di akhir frame bergantung thd All_Size*/
    	for (int y = 0; y < ALL_SIZE+1; y++){
    		/**/
    		if (y < ALL_SIZE){
                	out[y] = temp[y];
            	}
            	/**/
            	else{
                	out[y] = count_parity(temp, ALL_SIZE);
               }
        }
        printBuff(out);
        return 0;
}
/*print send frame data ke client*/
int printBuff(char buff[ALL_SIZE+1]){
	/**/
	for (int y = 0; y < ALL_SIZE+1; y++){
		printf("%c", buff[y]);	
		/**/
		if((y+1)%8==0){
			printf("-");
		}
	}  
    	return 0;
}
