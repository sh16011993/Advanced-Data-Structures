#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define MAX_RECORD_SIZE 4096

// Primary key Index Structure
typedef struct{ 
	int key; /* Record's key */ 
	long off; /* Record's offset in file */ 
}index_S;

// Availibility List Structure
typedef struct{ 
	int siz; /* Hole's size */ 
	long off; /* Hole's offset in file */ 
}avail_S;

// Create index and availability list arrays
index_S idx[10000]; avail_S avail[10000]; 
// Counters for index and availability lists
int idxcount = 0, availcount = 0;

//Function to check if File Exists
int fileExists(char *filename){
	FILE *filep = NULL;
	if(filep = fopen(filename, "r")){
		fclose(filep);
		return 1;
	}
	return 0;
}

int indexBSearch(int sid){
	int l=0, h=idxcount-1;
	while(l<=h){
		int mid=(l+h)/2;
		if(idx[mid].key == sid)
			return mid;
		else if(sid < idx[mid].key)
			h=mid-1;
		else if(sid > idx[mid].key)
			l=mid+1;
	}
	return -1;
}

int holeBSearch(int reqholesize){
	int l=0, h=availcount-1;
	while(l<=h){
		int mid=(l+h)/2;
		if(avail[mid].siz<=reqholesize){
			l=mid+1;
		}
		else if(avail[mid].siz>reqholesize){
			h=mid-1;
		}
	}
	//IF suitable hole size doesn't exist, return -1
	if(l == availcount)
		return -1;
	//Otherwise return the best-fit hole index
	return l;
}

void removeHole(int holeidx){
	int i;
	for(i=holeidx+1; i<availcount; i++){
		avail[i-1] = avail[i];
	}
	availcount--;
}

int incr(const void* p, const void* q){
	avail_S a = *(avail_S *)p;
	avail_S b = *(avail_S *)q;
	if(a.siz == b.siz){
		return a.off-b.off;
	}
	return a.siz-b.siz;
}

int decr(const void* p, const void* q){
	avail_S a = *(avail_S *)p;
	avail_S b = *(avail_S *)q;
	if(a.siz == b.siz){
		return a.off-b.off;
	}
	return b.siz-a.siz;
}

void addHole(int leftholesize, long leftholeoffset, char* holetype){
	avail_S newhole = {leftholesize, leftholeoffset};
	avail[availcount] = newhole;
	availcount++;
	//Now, based on the availability order specified, sort the availability list
	//For first-fit, nothing needs to be done
	if(!strcmp(holetype, "--best-fit")){
		//sort ascending
		qsort(avail, availcount, sizeof(avail_S), incr);
	}
	else if(!strcmp(holetype, "--worst-fit")){
		//sort descending
		qsort(avail, availcount, sizeof(avail_S), decr);
	}
}

int comp(const void* p, const void* q){
	index_S a = *(index_S *)p;
	index_S b = *(index_S *)q;
	return a.key-b.key;
}

void addIndex(index_S tmpidx){
	idx[idxcount++] = tmpidx;
	//Now put this entry correctly at its place (sort).
	qsort(idx, idxcount, sizeof(index_S), comp);
}

void removeIndex(int reply){
	int i;
	for(i=reply+1; i<idxcount; i++){
		idx[i-1] = idx[i];
	}
	idxcount--;
}

int main(int argc, char* argv[]){
	if(argc != 3){
		printf("Enter the correct number of arguments\n");
		return 0;
	}
	if(strcmp(argv[1], "--first-fit") != 0 && strcmp(argv[1], "--best-fit") != 0 && strcmp(argv[1], "--worst-fit") != 0){
		printf("Enter correct arguments\n");
		return 0;
	}
	// If all arguments are correct, proceed
	char *filename = argv[2]; char *idxfile = "indfile.bin"; char *availfile = "availfile.bin"; char *holetype = argv[1];
	FILE *fp = NULL, *fpindex = NULL, *fpavail = NULL; 
	int ffi = 0, ffa = 0;
	int totalholesize = 0;
	int i, j;
	/* If output file doesn't exist, create it for writing or else append to the end of the existing file. */ 
	if(!fileExists(filename)){
		// Open for reading and writing from scratch
		fp = fopen(filename, "w+b");
	}
	else{
		// Open for reading and updating the file
		fp = fopen(filename, "r+b");
	}
	/* Check if index and availability list files exist or not. If they don't exist, create them. If they exist, copy their data to the respective arrays. */
	// If Index File exists
	if(fileExists(idxfile)){
		//Currenty open for reading the file, later reopen it for writing
		fpindex = fopen(idxfile, "rb");
		// Copy data to index array
		// Reading Index File Data
		index_S tmpindex;
		while(fread(&tmpindex, sizeof(index_S), 1, fpindex)){
			idx[idxcount++] = tmpindex;
		}
		ffi = 1;
	}
	// If Availability File exists
	if(fileExists(availfile)){
		//Currenty open for reading the file, later reopen it for writing
		fpavail = fopen(availfile, "rb");
		// Reading Availability List File Data
		avail_S tmpavail;
		while(fread(&tmpavail, sizeof(avail_S), 1, fpavail)){
			avail[availcount++] = tmpavail;
		} 
		ffa = 1;
	}
	// Assumption: Maximum length of each command is 4096
    size_t maxbufsize = MAX_RECORD_SIZE;
	char *tmpbufptr = (char *)malloc(sizeof(char) * maxbufsize);
	getline(&tmpbufptr, &maxbufsize, stdin);
	char *tmp = strtok(tmpbufptr, "\n");
	while(strcmp(tmp, "end")){
		char *cmd = strtok(tmp, " ");
		while(cmd != NULL){
			if(!strcmp(cmd, "add")){
//				printf("%d\n", idxcount);
				cmd = strtok(NULL, " ");
				char *s = cmd;
				int sid = atoi(s);
				cmd = strtok(NULL, " ");
				char *record = cmd;
				cmd = strtok(NULL, " ");
				//Search to find if SID already exists or not
				int reply = indexBSearch(sid);
				if(reply != -1){
					//Record exists
					printf("Record with SID=%d exists\n", sid);
				}
				else{
					//Record doesn't exist
					//Calculate minimum size needed for hole
					int reclen = strlen(record);
					int reqholesize = sizeof(int)+reclen, holeidx=-1;
					//First, search for a hole (get required hole index)
					if(!strcmp(holetype, "--first-fit")){
						for(i=0; i<availcount; i++){
							if(avail[i].siz >= reqholesize){
								holeidx=i;
								break;
							}
						}
					}
					else if(!strcmp(holetype, "--best-fit")){
						// Since it is best-fit, look for the smallest size hole that is greater than reqholesize-1
						if(availcount != 0)
							holeidx=holeBSearch(reqholesize-1);
					}
					else if(!strcmp(holetype, "--worst-fit")){
						if(avail[0].siz >= reqholesize){
							holeidx=0;
						}
						else{
							//holeidx of availcount indicates that the required size hole couldn't be found
							holeidx=-1;
						}
					}
					if(holeidx == -1){
						//Couldn't get a hole. Append to file end
						fseek(fp, 0, SEEK_END);
                		//Create record for the index file
                		index_S tmpidx = {sid, ftell(fp)};
						//Make entry for this record in the index
						addIndex(tmpidx);
						//Write record size and record to the file						
                		fwrite(&reclen, sizeof(int), 1, fp);
                		fwrite(record, sizeof(char), reclen, fp);
					}
					else{
						//If a hole meeting the size requirement is present, then
						//Create record for the index file
						index_S tmpidx = {sid, avail[holeidx].off};
						//Make entry for this record in the index
						addIndex(tmpidx);
						//Append record size and record to the offset as pointed by the hole.
						fseek(fp, avail[holeidx].off, SEEK_SET);
						fwrite(&reclen, sizeof(int), 1, fp);
                		fwrite(record, sizeof(char), reclen, fp);
						//If hole size still left, then remove and make fresh entry in the hole file for a new smaller hole
						int leftholesize = avail[holeidx].siz-(sizeof(int)+reclen);
						int leftholeoffset = ftell(fp);
						//Remove old hole in all cases
						removeHole(holeidx);
						//Add hole only if there is a residual hole size
						if(leftholesize){
							addHole(leftholesize, leftholeoffset, holetype);
						}
					}
				}
			}
			else if(!strcmp(cmd, "find")){
				cmd = strtok(NULL, " ");
				char *s = cmd;
				int sid = atoi(s);
				cmd = strtok(NULL, " ");
				int reply = indexBSearch(sid);
				if(reply == -1){
					//Record doesn't exist
					printf("No record with SID=%d exists\n", sid);
				}
				else{
					//Record exists
					//Find offset of the record
					long offset = idx[reply].off;
					//Read the record
					int recsize;
					fseek(fp, offset, SEEK_SET);
					fread(&recsize, sizeof(int), 1, fp);
					char* rec=(char*)malloc(sizeof(char)*(recsize+1));
					fread(rec, sizeof(char), recsize, fp);
					// rec is not a null terminated string. Terminating the string with a null character.
					rec[recsize] = '\0';
					printf("%s\n", rec);
				}
			}
			else if(!strcmp(cmd, "del")){
				cmd = strtok(NULL, " ");
				char *s = cmd;
				int sid = atoi(s);
				cmd = strtok(NULL, " ");
				int reply = indexBSearch(sid);
				if(reply == -1){
					//Record doesn't exist
					printf("No record with SID=%d exists\n", sid);
				}
				else{
					//Record exists
					//Find offset of the record
					int offset = idx[reply].off;
					char* rec;
					//Read the record
					int recsize;
					fseek(fp, offset, SEEK_SET);
					fread(&recsize, sizeof(int), 1, fp);
					//Adding new entry into the availability list
					addHole(sizeof(int)+recsize, offset, holetype);
					//Removing the record from the index
					removeIndex(reply);
				}
			}
		}
		// Get the next line from input
		getline(&tmpbufptr, &maxbufsize, stdin);
		tmp = strtok(tmpbufptr, "\n");
	}
	//When "end" command is encountered
		
	//Writing back to index and availability list files. First, reopen the files for writing
	//Close index and availability files if opened in reading mode
	if(ffi){
		fclose(fpindex);
	}
	if(ffa){
		fclose(fpavail);
	}
	// Reopen index and availability files in write mode
	fpindex = fopen(idxfile, "wb");
	fpavail = fopen(availfile, "wb");
	//Writing to index file
	fwrite(idx, sizeof(index_S), idxcount, fpindex);
	//Writing to availability list file
	fwrite(avail, sizeof(avail_S), availcount, fpavail);
	
	//Printing index entries on screen
	printf("Index:\n");
	for(i=0; i<idxcount; i++){
		printf( "key=%d: offset=%ld\n", idx[i].key, idx[i].off);
	}
	//Printing availability list entries on screen
	printf("Availability:\n");
	for(i=0; i<availcount; i++){
		printf( "size=%d: offset=%ld\n", avail[i].siz, avail[i].off);
		totalholesize+=avail[i].siz;
	}	
	//Number of holes in the availability list
	printf( "Number of holes: %d\n", availcount);
	//Total amount of space occupied by all the holes
	printf( "Hole space: %d\n", totalholesize);
	
	//Close student file
	fclose(fp);
	//Close index and availability list files
	fclose(fpindex); fclose(fpavail);
	return 0;
}
