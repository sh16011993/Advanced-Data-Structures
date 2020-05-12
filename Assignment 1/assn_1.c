#include<stdio.h>
#include<sys/time.h>
#include <string.h>
#include <unistd.h>

struct timeval tm_start, tm_end, tm_diff;

int main(int argc, char* argv[]){
	// Check if the argument format is correct ie. length and value of each argument
	if(argc!=4){
		printf("Enter the arguments correctly\n");
		return 0;
	}
	if(strcmp(argv[1], "--mem-lin") && strcmp(argv[1], "--mem-bin") && strcmp(argv[1], "--disk-lin") && strcmp(argv[1], "--disk-bin")){
		printf("Argument 2 is wrongly entered.\n");
		return 0;
	}
	if(strcmp(argv[2], "key.db")){
		printf("Argument 3 is wrongly entered.\n");
		return 0;
	}
	if(strcmp(argv[3], "seek.db")){
		printf("Argument 4 is wrongly entered.\n");
		return 0;
	}
	//Everything ok from commandline. Proceed with main program execution
	// Opeing seek file and putting it in memory 
	FILE *fs = fopen("seek.db", "rb");
	// Finding seek.db file size
	fseek(fs, 0, SEEK_END);
	int fs_size = ftell(fs)/4;
	// Moving file pointer to the start of the file
	rewind(fs);
	// Creating integer array of fs_size
	int s[fs_size];
	fread(s, sizeof(int), fs_size, fs);
	// Creating hit array of size as s (seek.db)
	int hit[fs_size];
	//Intialize all the array values to zero
	memset(hit, 0, sizeof(hit));
	
	FILE *fk;
	int temp, i, j, left, right, mid, fk_size;
	// Calling for each of the 4 different cases
	if(!strcmp(argv[1], "--mem-lin")){
		//starting time now
		gettimeofday(&tm_start, NULL);
		
		// Open key.db file
		fk = fopen("key.db", "rb");
		// Finding key.db file size
		fseek(fk, 0, SEEK_END);
		// Finding key.db file size
		fk_size = ftell(fk)/4;
		// Moving file pointer to the start of the file
		rewind(fk);
		
		// Creating integer array of fk_size
		int k[fk_size];
		fread(k, sizeof(int), fk_size, fk);
		//Reading key.db file complete
		for(i=0; i<fs_size; i++){
			temp = s[i];
			for(j=0; j<fk_size; j++){
				if(temp == k[j]){
					hit[i] = 1;
					break;
				}
			}
		}
	}
	else if(!strcmp(argv[1], "--mem-bin")){
		//starting time now
		gettimeofday(&tm_start, NULL);
		
		// Open key.db file
		fk = fopen("key.db", "rb");
		// Finding key.db file size
		fseek(fk, 0, SEEK_END);
		// Finding key.db file size
		fk_size = ftell(fk)/4;
		// Moving file pointer to the start of the file
		rewind(fk);
		
		// Creating integer array of fk_size
		int k[fk_size];
		fread(k, sizeof(int), fk_size, fk);
		//Reading key.db file complete
		for(i=0; i<fs_size; i++){
			left = 0; right = fk_size-1;
			while(left<=right){
				int mid = (left+right)/2;
				if(k[mid] == s[i]){
					hit[i] = 1;
					break;
				}
				else if(k[mid] < s[i]){
					left = mid+1;
				}
				else{
					right = mid-1;
				}
			}
		}
	}
	else if(!strcmp(argv[1], "--disk-lin")){
		// Open key.db file
		fk = fopen("key.db", "rb");
		// Finding key.db file size
		fseek(fk, 0, SEEK_END);
		// Finding key.db file size
		fk_size = ftell(fk)/4;
		// Moving file pointer to the start of the file
		rewind(fk);
		
		//starting time now
		gettimeofday(&tm_start, NULL);
		
		for(i=0; i<fs_size; i++){
			for(j=0; j<fk_size; j++){
				// Read 1 integer at a time
				fread(&temp, sizeof(int), 1, fk);
				if(s[i] == temp){
					hit[i] = 1;
					break;
				}
			}
			rewind(fk);
		}		
	}
	else if(!strcmp(argv[1], "--disk-bin")){
		// Open key.db file
		fk = fopen("key.db", "rb");
		// Finding key.db file size
		fseek(fk, 0, SEEK_END);
		// Finding key.db file size
		fk_size = ftell(fk)/4;
		// Moving file pointer to the start of the file
		rewind(fk);
		
		//starting time now
		gettimeofday(&tm_start, NULL);
		
		for(i=0; i<fs_size; i++){
			left = 0; right = fk_size-1;
			while(left<=right){
				mid = (left+right)/2;
				// Read 1 integer at a time
				fseek(fk, mid*sizeof(int), SEEK_SET);
				fread(&temp, sizeof(int), 1, fk);
				if(s[i] == temp){
					hit[i] = 1;
					break;
				}
				else if(temp < s[i]){
					left = mid+1;
				}
				else{
					right = mid-1;
				}
			}
			rewind(fk);
		}
	}
	// ending time now
	gettimeofday(&tm_end, NULL);
	
	//Printing the hit array
	for(i=0; i<fs_size; i++){
		if(hit[i]){
			printf("%12d: Yes\n", s[i]);
		}
		else{
			printf("%12d: No\n", s[i]);
		}
	}
	//Printing time difference now
	tm_diff.tv_sec=tm_end.tv_sec-tm_start.tv_sec;
	tm_diff.tv_usec=tm_end.tv_usec-tm_start.tv_usec;
	if(tm_diff.tv_usec<0){
		tm_diff.tv_sec--;
		tm_diff.tv_usec = 1000000 - tm_diff.tv_usec;
	}
	printf("Time: %ld.%06ld\n", tm_diff.tv_sec, tm_diff.tv_usec);
	// Closing all the open file handlers
	fclose(fs);
	fclose(fk);
	return 0;
}
