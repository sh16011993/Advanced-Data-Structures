#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include<sys/time.h>
#define INPUTBUF 1000
#define OUTPUTBUF 1000
#define RUNSIZE 1000
#define SUPRUN 15
#define PH 750
#define SH 750
#define BUF 250

//Structure to store file pointers for each run
typedef struct rund{
	FILE *fp; // File Pointer for each run
	int count; // Number of elements left in the run
}rundetails;

typedef struct elemdetails{
	int ind; // File Pointer for each run
	int key; // Each key
}elem;

/* Referenced from GeeksforGeeks */
void heapify(int *arr, int n, int i){ 
    int smallest = i;
    int left=2*i+1;
    int right=2*i+2;
  
    if(left<n && arr[left]<arr[smallest]){
		smallest = left; 
	}
    if(right<n && arr[right]<arr[smallest]){
	    smallest = right; 
	}
    if(smallest != i){ 
    	int temp;
    	temp = arr[i];
    	arr[i] = arr[smallest];
    	arr[smallest] = temp;
        heapify(arr, n, smallest); 
    } 
} 
/* Referenced from GeeksforGeeks */
void buildHeap(int *arr, int n){ 
	int i;
    int start = (n/2)-1; 
    for (i=start; i>=0; i--){ 
        heapify(arr, n, i); 
    } 
} 
/* Referenced from CodeProject */
void bubbleDown(int *arr, int index, int length){
    int leftCI = (2*index)+1;
    int rightCI = (2*index)+2;
    if(leftCI >= length){
    	return;	
	}
    int minIndex = index;
    if(arr[index] > arr[leftCI]){
        minIndex = leftCI;
    }
    if((rightCI < length) && (arr[minIndex] > arr[rightCI])){
        minIndex = rightCI;
    }
    if(minIndex != index){
        int temp = arr[index];
        arr[index] = arr[minIndex];
        arr[minIndex] = temp;
        bubbleDown(arr, minIndex, length);
    }
}
/* Referenced from CodeProject */
void bubbleUp(int *arr, int index){
    if(index == 0){
        return;
    }
    int pIndex = (index-1)/2;
    if(arr[pIndex] > arr[index]){
        int temp = arr[pIndex];
        arr[pIndex] = arr[index];
        arr[index] = temp;
        bubbleUp(arr, pIndex);
    }
}

int comparator(const void *a, const void *b){
	return (*(const int *)a - *(const int *)b);
}
int comp(const void *a, const void *b){
	return (((const elem *)a)->key - ((const elem *)b)->key);
}

int main(int argc, char *argv[]){
	struct timeval tm_start, tm_end, tm_diff;
	//Measuring time
	//starting time now
	gettimeofday(&tm_start, NULL);
	if(strcmp(argv[1], "--basic") == 0){
		//First, open input bin file
		FILE *inpfp = fopen(argv[2], "rb");
		if(inpfp == NULL){
			perror("Error: ");
		}
		//Here, first get the number of runs (given that each run has 1000 keys)
		fseek(inpfp, 0, SEEK_END);
		int totalkeys = ftell(inpfp)/sizeof(int);		
		int totalruns = totalkeys/RUNSIZE;		
		int lastruncnt = totalkeys%RUNSIZE;
		if(lastruncnt == 0){
			lastruncnt = RUNSIZE;
		}
		else{
			totalruns+=1;
		}
		//Seek to start of the input file
		fseek(inpfp, 0, SEEK_SET);
		// Now create an array containing file current positions
		FILE *fps[totalruns]; 
		FILE *outfp;  char outfile[30]; 
		int *ibuf = (int *)malloc(sizeof(int) * INPUTBUF);
		int i, j, h, tmp; // tmp holds integers temporarily
		elem tmpelem; // tmpelem holds the structure (key, index) temporarily
		int counter = 0; // Count the number of runs
		
		//Special Case: When totalkeys < RUNSIZE
		if(totalkeys<RUNSIZE){
			fread(ibuf, sizeof(int), totalkeys, inpfp);	
			qsort(ibuf, totalkeys, sizeof(int), comparator);
			FILE *sfp = fopen(argv[3], "wb");
			fwrite(ibuf, sizeof(int), totalkeys, sfp);
			goto x;
		}
		
		while(counter<totalruns){
			if(counter == (totalruns-1)){
				fread(ibuf, sizeof(int), lastruncnt, inpfp);		
				qsort(ibuf, lastruncnt, sizeof(int), comparator);
			}
			else{
				fread(ibuf, sizeof(int), RUNSIZE, inpfp);		
				qsort(ibuf, RUNSIZE, sizeof(int), comparator);
			}
			sprintf(outfile, "%s.%03d", argv[2], counter); 
			outfp = fopen(outfile, "wb");
			if(outfp == NULL){
				perror("Error: ");
			}
			if(counter == (totalkeys-1)){
				fwrite(ibuf, sizeof(int), lastruncnt, outfp);
			}
			else{
				fwrite(ibuf, sizeof(int), INPUTBUF, outfp);
			}
			fclose(outfp);
			counter++;
		}
		//Run generation is completed. Real Deal !!!
		elem *inpbuf = (elem *)malloc(sizeof(elem) * INPUTBUF); //This is the input buffer for structure type (key, ind)
		int bufelems = INPUTBUF/totalruns;
		counter = 0; // Important: This counter keeps track of the total number of elements present in the input buffer at any time
		for(i=0; i<totalruns; i++){
			sprintf(outfile, "%s.%03d", argv[2], i); 
    		outfp=fopen(outfile, "rb");
    		for(j=0; j<bufelems; j++){
    			fread(&tmp, sizeof(int), 1, outfp);
    			tmpelem.key = tmp;
    			tmpelem.ind = i;
    			inpbuf[counter++] = tmpelem;
			}
			fps[i]=outfp;
		}
		// Initial load of elements in the input buffer is complete
		qsort(inpbuf, counter, sizeof(elem), comp);
		//Opening sort.bin file for writing output
		FILE *ofp = fopen(argv[3], "wb");
		if(ofp == NULL){
			perror("Error: ");
		}
		//Creating output buffer
		int *outbuf = (int *)malloc(sizeof(int) * OUTPUTBUF);
		int index; 
		int totalcnt = 0; // This keeps track of the total number of elements that have entered the output buffer
		int outbufcnt = 0; // This keeps track of the number of elements currently present in the output buffer
		FILE *reqfp;
		
		while(totalcnt<totalkeys){
			outbuf[outbufcnt++] = inpbuf[0].key;
			totalcnt++;
			if(outbufcnt == OUTPUTBUF){
				fwrite(outbuf, sizeof(int), outbufcnt, ofp);
				outbufcnt = 0;
			}
			if(totalcnt == totalkeys){
				break;
			}
			index = inpbuf[0].ind; // Index tracks which run the current element belongs to
			reqfp = fps[index];
			//Reached end of the current run
			if(fread(&tmp, sizeof(int), 1, reqfp)){
				// First, store the file pointer (new position) in the file pointers array
				fps[index] = reqfp;
				tmpelem.key = tmp;
				tmpelem.ind = index;
				
				// Put the element at its correct position
				h=1;
				while(h<counter && tmpelem.key>inpbuf[h].key){
					inpbuf[h-1] = inpbuf[h];
					h++;
				}
				inpbuf[h-1] = tmpelem;
			}
			else{
				for(i=1; i<counter; i++){
					inpbuf[i-1] = inpbuf[i];
				}
				counter--;
				// Just close the file pointer pointed by this file since it has already been read
//				fclose(reqfp);
			}
		}
		fwrite(outbuf, sizeof(int), outbufcnt, ofp);
		fclose(ofp);
	}
	else if(strcmp(argv[1], "--multistep") == 0){
		FILE *inpfp = fopen(argv[2], "rb");
		if(inpfp == NULL){
			printf("Cannot Open File\n");
			return 0;
		}
		//Here, first get the number of runs (given that each run has 1000 keys)
		fseek(inpfp, 0, SEEK_END);
		int totalkeys = ftell(inpfp)/sizeof(int);
		int totalruns = totalkeys/RUNSIZE;
		int lastruncnt = totalkeys%RUNSIZE;
		if(lastruncnt == 0){
			lastruncnt = RUNSIZE;
		}
		else{
			totalruns+=1;
		}
		//Seek to start of the input file
		fseek(inpfp, 0, SEEK_SET);
		FILE *outfp;  char outfile[30]; 
		int *ibuf = (int *)malloc(sizeof(int) * INPUTBUF);
		int i, j, h; // tmp holds integers temporarily
		int tmp; elem tmpelem; // tmpelem holds the structure (key, index) temporarily
		int counter = 0; // Count the number of runs
		
		//Special Case: When totalkeys < RUNSIZE
		if(totalkeys<RUNSIZE){
			fread(ibuf, sizeof(int), totalkeys, inpfp);	
			qsort(ibuf, totalkeys, sizeof(int), comparator);
			FILE *sfp = fopen(argv[3], "wb");
			fwrite(ibuf, sizeof(int), totalkeys, sfp);
			goto x;
		}
		
		while(counter<totalruns){
			if(counter == (totalruns-1)){
				fread(ibuf, sizeof(int), lastruncnt, inpfp);		
				qsort(ibuf, lastruncnt, sizeof(int), comparator);
			}
			else{
				fread(ibuf, sizeof(int), RUNSIZE, inpfp);		
				qsort(ibuf, RUNSIZE, sizeof(int), comparator);
			}
			sprintf(outfile, "%s.%03d", argv[2], counter); 
			outfp = fopen(outfile, "wb");
			if(outfp == NULL){
				perror("Error: ");
			}
			if(counter == (totalkeys-1)){
				fwrite(ibuf, sizeof(int), lastruncnt, outfp);
			}
			else{
				fwrite(ibuf, sizeof(int), INPUTBUF, outfp);
			}
			fclose(outfp);
			counter++;
		}
		// Run generation is completed. Real Deal !!!
		// Now, create all the super runs
		elem *inpbuf = (elem *)malloc(sizeof(elem) * INPUTBUF);
		int *outbuf = (int *)malloc(sizeof(int) * RUNSIZE);
		int supruncnt = (totalruns/SUPRUN)+((totalruns%SUPRUN)!=0), lastsuprun = totalruns%SUPRUN;
		if(lastsuprun == 0){
			lastsuprun = SUPRUN;
		}
		int supbufelemperrun = INPUTBUF/SUPRUN, lastsupbufelemperrun = INPUTBUF/lastsuprun;
		int cond, newcond, index; // condition variable for special cases
		// Now create an array containing file current positions
		FILE *fps[totalruns];
		for(i=0; i<supruncnt; i++){
			counter = 0; // Important: This counter keeps track of the total number of elements present in the input buffer at any time
			if(i == supruncnt-1){
				cond = (i*SUPRUN) + lastsuprun;
				newcond = lastsuprun;
			}
			else{
				cond = (i+1)*SUPRUN;
				newcond = SUPRUN;
			}		
			for(j=i*SUPRUN; j<cond; j++){
				sprintf(outfile, "%s.%03d", argv[2], j); 
	    		outfp=fopen(outfile, "rb");
	    		if(outfp == NULL){
	    			perror("Error: ");
				}
				// If it is the last superrun running
				if(i == supruncnt-1){
					supbufelemperrun = INPUTBUF/lastsuprun;
				}
	    		for(h=0; h<supbufelemperrun; h++){
	    			fread(&tmp, sizeof(int), 1, outfp);
	    			tmpelem.key = tmp;
	    			tmpelem.ind = j;
	    			inpbuf[counter++] = tmpelem;	
				}
				fps[j] = outfp;
			}
			// After the initial loading of the elements, sort the inputbuffer
			qsort(inpbuf, counter, sizeof(elem), comp);
			//Now start copying to the output buffer
			int outbufcnt = 0, totalcnt = 0;
			// Open superrun file on disk to write superruns
			sprintf(outfile, "%s.super.%03d", argv[2], i); 
			FILE *supoutfile = fopen(outfile, "wb");
			if(supoutfile == NULL){
    			perror("Error: ");
			}
			if(i == supruncnt-1){
				newcond = lastsuprun;
			} 
			else{
				newcond = SUPRUN;
			}
			while(totalcnt != (RUNSIZE*newcond)){ // ie. until all the elements for this particular superrun are processed
				outbuf[outbufcnt++] = inpbuf[0].key;
				totalcnt++;
				// Check if the output buffer is full. If it is full, write it to disk
				if(outbufcnt == OUTPUTBUF){
					fwrite(outbuf, sizeof(int), outbufcnt, supoutfile);
					outbufcnt = 0;
				}
				if(totalcnt == (RUNSIZE*newcond)){
					break;
				}				
				// Getting next element
				int index = inpbuf[0].ind;
				FILE *reqfp = fps[index];
				if(fread(&tmp, sizeof(int), 1, reqfp)){
					fps[index] = reqfp;
					tmpelem.key = tmp;
					tmpelem.ind = index;
					// Now, put the 0th element in its correct position
					h=1;
					while(h<counter && tmpelem.key>inpbuf[h].key){
						inpbuf[h-1] = inpbuf[h];
						h++;
					}
					inpbuf[h-1] = tmpelem;
				}
				else{
					for(h=1; h<counter; h++){
						inpbuf[h-1] = inpbuf[h];
					}
					counter--;
					// Just close the file pointer pointed by this file since it has already been read
//					fclose(reqfp);
				}
			}
			// Write the remaining elements present in the output buffer
			fwrite(outbuf, sizeof(int), outbufcnt, supoutfile);	
			fclose(supoutfile);
		}
		//Now, superrun files are ready. It is time to perform the final merge
		
		//First, create a file pointer array for the superruns
		FILE *sfps[supruncnt];
		int elemread = INPUTBUF/supruncnt;
		counter = 0; // Counter here will be responsible for keeping a track of the number of elements present in the input buffer 
		for(i=0; i<supruncnt; i++){
			sprintf(outfile, "%s.super.%03d", argv[2], i); 
			FILE *supfile = fopen(outfile, "rb");
    		if(supfile == NULL){
    			perror("Error: ");
			}
			for(j=0; j<elemread; j++){
				// Read the initial set of elements into the input buffer
				fread(&tmp, sizeof(int), 1, supfile);
    			tmpelem.key = tmp;
    			tmpelem.ind = i;
    			inpbuf[counter++] = tmpelem;	
			}
			sfps[i] = supfile;
		}
		qsort(inpbuf, counter, sizeof(elem), comp);
		//Now, keep pushing elements to the output buffer
		int totalcnt = 0, outbufcnt = 0;
		outfp = fopen(argv[3], "wb");
		if(outfp == NULL){
			perror("Error: ");
		}
		while(totalcnt != totalkeys){ // ie. until all the elements for this particular superrun are processed
			outbuf[outbufcnt++] = inpbuf[0].key;
			totalcnt++;
			if(outbufcnt == RUNSIZE){
				fwrite(outbuf, sizeof(int), outbufcnt, outfp);	
				outbufcnt = 0;
			}
			if(totalcnt == totalkeys){
				break;
			}				
			// Getting next element
			int index = inpbuf[0].ind;
			FILE *reqfp = sfps[index];
			if(fread(&tmp, sizeof(int), 1, reqfp)){
				sfps[index] = reqfp;
				tmpelem.key = tmp;
				tmpelem.ind = index;
				// Now, put the 0th element in its correct position
				h=1;
//				printf("%d %d\n", outbufcnt, totalkeys);
				while(h<counter && tmpelem.key>inpbuf[h].key){
					inpbuf[h-1] = inpbuf[h];
					h++;
				}
				inpbuf[h-1] = tmpelem;
			}
			else{
				for(h=1; h<counter; h++){
					inpbuf[h-1] = inpbuf[h];
				}
				counter--;
//				fclose(reqfp);
			}
		}
		fclose(outfp);
	}
	else if(strcmp(argv[1], "--replacement") == 0){
		int i,j,h;
		//Allocating space for all structures
		int *pheap = (int *)malloc(sizeof(int) * PH);
		int *buf = (int *)malloc(sizeof(int) * BUF);
		int *sheap = (int *)malloc(sizeof(int) * SH);
		int *outbuf = (int *)malloc(sizeof(int) * OUTPUTBUF);
		//Opening input file
		FILE *fp = fopen(argv[2], "rb");
		if(fp == NULL){
			perror("Error: ");
		}
		fseek(fp, 0, SEEK_END);
		int totalkeys = ftell(fp)/sizeof(int);
		fseek(fp, 0, SEEK_SET);
		
		//Special Case: When totalkeys < RUNSIZE
		if(totalkeys<RUNSIZE){
			int *ibuf = (int *)malloc(sizeof(int) * totalkeys);
			fread(ibuf, sizeof(int), totalkeys, fp);	
			qsort(ibuf, totalkeys, sizeof(int), comparator);
			FILE *sfp = fopen(argv[3], "wb");
			fwrite(ibuf, sizeof(int), totalkeys, sfp);
			goto x;
		}
		
		fread(pheap, sizeof(int), PH, fp);
		fread(buf, sizeof(int), BUF, fp);
		buildHeap(pheap, PH);

		//Heapifying Completed
		int outbufcnt = 0; // Keeps track of the number of elements in the output buffer at any time
		int phcnt = PH, bufcnt = BUF, shcnt = 0; //Primary Heap Counter // Buffer Counter // Secondary heap Counter
		int bufhead = 0; // Points to the topmost element in the buffer at all times
		int elemsread; // Elements read successfully by fread
		int counter = PH+BUF; // Keeps track of the number of elements read from the disk till now
		char runfile[30]; int runcnt = 0;
		int flag = 0; // This is set to true if the input buffer becomes empty
		FILE *outfile;
		
		while(counter != totalkeys){
			//Open the output file for writing the runs
			sprintf(runfile, "%s.%03d", argv[2], runcnt++); 
			outfile = fopen(runfile, "wb");
			if(outfile == NULL){
				perror("Error: ");
			}
			//Till Primary Heap is empty, just do the below
			while(phcnt != 0){
				if(bufhead == bufcnt){
//					printf("Inner: %d\n", counter);
					// Buffer is actually empty. Either bring the next set of elements from the disk or if the file on disk is completely read, break after setting the flag
					if(elemsread = fread(buf, sizeof(int), BUF, fp)){
						counter+=elemsread;
						bufcnt = elemsread;
						bufhead = 0;	
					}
					else{
						flag = 1;
						break;
					}
				}
				// Compare primary heap top to the input buffer top element
				if(pheap[0] <= buf[bufhead]){
					// Copy heap top to output buffer
					outbuf[outbufcnt++] = pheap[0];
					//If the output buffer is complete, just write to the run file which is open
					if(outbufcnt == OUTPUTBUF){
						fwrite(outbuf, sizeof(int), outbufcnt, outfile);
						outbufcnt = 0;
					}
					// Replace primary heap top with buffer head 
					pheap[0] = buf[bufhead++];
				}
				else{
					// Copy heap top to output buffer
					outbuf[outbufcnt++] = pheap[0];
					//If the output buffer is complete, just write to the run file which is open
					if(outbufcnt == OUTPUTBUF){
						fwrite(outbuf, sizeof(int), outbufcnt, outfile);
						outbufcnt = 0;
					}
					// Replace primary heap top element with the last element of the primary heap
					pheap[0] = pheap[phcnt-1];
					// Primary heap size reduced by 1. Decrement counter
					phcnt--;
					// Also, move the buffer top element to the secondary heap
					sheap[shcnt++] = buf[bufhead++];
					// Reheap the secondary heap
					bubbleUp(sheap, shcnt-1);
				}
				if(phcnt != 0){
					// Reheap the primary heap
					bubbleDown(pheap, 0, phcnt);
				}
			}

			// If primary heap becomes empty, run is complete. Copy the remaining elements from the output buffer to the run file that is open and then close this run file
			fwrite(outbuf, sizeof(int), outbufcnt, outfile);
			outbufcnt = 0;
			fclose(outfile);
			//Check the flag first. If true, it means that file on disk is completely read and the input buffer is empty
			if(flag == 1){
				break;
			}
			//Now, copy the elements from the secondary heap to the primary heap
			for(i=0; i<shcnt; i++){
				pheap[i] = sheap[i];
			}
			phcnt = shcnt;
			shcnt = 0;
//			printf("%d %d\n", counter, totalkeys);
		}
		
		// Final Stage !!!
//		printf("%d %d %d %d\n", bufhead, bufcnt, phcnt, shcnt);
//		Now, create the last few run files using the remaining primary heap elements and buffer elements. At this point everything has been read from the disk
//		Open the output file for writing the runs
		if(flag){ // flag is true implies that input buffer is empty. So, complete the current run and then create the final run from the remaining elements in the secondary heap
			// In case, output buffer is ful, then write the keys to disk and empty it
			sprintf(runfile, "%s.%03d", argv[2], runcnt-1); 
			outfile = fopen(runfile, "a+b");
			while(phcnt != 0){
				// No comparisons needed. Just pull, heapify and repeat. Cheers !!!!
				// Copy heap top to output buffer
				outbuf[outbufcnt++] = pheap[0];
				// Replace primary heap top element with the last element of the primary heap
				pheap[0] = pheap[phcnt-1];
				phcnt--;
				if(phcnt != 0){
					// Reheap the primary heap
					bubbleDown(pheap, 0, phcnt);
				}
			}
			//Write the outputbuffer to file on disk
			fwrite(outbuf, sizeof(int), outbufcnt, outfile);
			outbufcnt = 0;
			//Now since this run is complete, just close the file
			fclose(outfile);

			//Now copy elements from the secondary heap
			for(i=0; i<shcnt; i++){
				pheap[i] = sheap[i];
			}
			phcnt = shcnt;
			shcnt = 0;
			//Now simply open a new run file on disk and push everything to it
			sprintf(runfile, "%s.%03d", argv[2], runcnt++); 
			outfile = fopen(runfile, "wb");
			if(outfile == NULL){
				perror("Error: ");
			}
			while(phcnt != 0){
				// No comparisons needed. Just pull, heapify and repeat. Cheers !!!!
				// Copy heap top to output buffer
				outbuf[outbufcnt++] = pheap[0];
				// Replace primary heap top element with the last element of the primary heap
				pheap[0] = pheap[phcnt-1];
				phcnt--;
				if(phcnt != 0){
					// Reheap the primary heap
					bubbleDown(pheap, 0, phcnt);
				}
			}
			fwrite(outbuf, sizeof(int), outbufcnt, outfile);
			fclose(outfile);
		}
		else{// flag is false implies that there are no elements on the file in the disk. There are elements in the input buffer and in the primary heap
			// Note: Look for the case when only a few elements from the secondary buffer where copied to the primary buffer ie. the case where primary heap empties
			// before the input buffer
			// If we have entered this else block, it means that the outbufcnt = 0 already (ie. the variable is set to 0)
			//Open the output file for writing the run (it is a new run)
			sprintf(runfile, "%s.%03d", argv[2], runcnt++); 
			outfile = fopen(runfile, "wb");
			if(outfile == NULL){
				perror("Error: ");
			}
			while(bufhead != bufcnt){
				// Compare primary heap top to the input buffer top element
				if(pheap[0] <= buf[bufhead]){
					// Copy heap top to output buffer
					outbuf[outbufcnt++] = pheap[0];
					//If the output buffer is complete, just write to the run file which is open
					if(outbufcnt == OUTPUTBUF){
						fwrite(outbuf, sizeof(int), outbufcnt, outfile);
						outbufcnt = 0;
					}
					// Replace primary heap top with buffer head 
					pheap[0] = buf[bufhead++];
				}
				else{
					// Copy heap top to output buffer
					outbuf[outbufcnt++] = pheap[0];
					//If the output buffer is complete, just write to the run file which is open
					if(outbufcnt == OUTPUTBUF){
						fwrite(outbuf, sizeof(int), outbufcnt, outfile);
						outbufcnt = 0;
					}
					// Replace primary heap top element with the last element of the primary heap
					pheap[0] = pheap[phcnt-1];
					// Primary heap size reduced by 1. Decrement counter
					phcnt--;
					// Also, move the buffer top element to the secondary heap
					sheap[shcnt++] = buf[bufhead++];
					// Reheap the secondary heap
					bubbleUp(sheap, shcnt-1);
				}
				if(phcnt != 0){
					// Reheap the primary heap
					bubbleDown(pheap, 0, phcnt);
				}
			}
			//Now, we have elements left only in primary heap and in the secondary heap
			//Complete the current run 
			// In case, output buffer is full, then write the keys to disk and empty it
			while(phcnt != 0){
				// No comparisons needed. Just pull, heapify and repeat. Cheers !!!!
				// Copy heap top to output buffer
				outbuf[outbufcnt++] = pheap[0];
				// Replace primary heap top element with the last element of the primary heap
				pheap[0] = pheap[phcnt-1];
				phcnt--;
				if(phcnt != 0){
					// Reheap the primary heap
					bubbleDown(pheap, 0, phcnt);
				}
			}
			//Write the outputbuffer to file on disk
			fwrite(outbuf, sizeof(int), outbufcnt, outfile);
			outbufcnt = 0;
			//Now since this run is complete, just close the file
			fclose(outfile);
			//Now copy secondary heap into primary heap
			for(i=0; i<shcnt; i++){
				pheap[i] = sheap[i];
			}
			phcnt = shcnt;
			shcnt = 0;
			//Now simply open a new run file on disk and push everything to it
			sprintf(runfile, "%s.%03d", argv[2], runcnt++); 
			outfile = fopen(runfile, "wb");
			if(outfile == NULL){
				perror("Error: ");
			}
			while(phcnt != 0){
				// No comparisons needed. Just pull, heapify and repeat. Cheers !!!!
				// Copy heap top to output buffer
				outbuf[outbufcnt++] = pheap[0];
				// Replace primary heap top element with the last element of the primary heap
				pheap[0] = pheap[phcnt-1];
				phcnt--;
				if(phcnt != 0){
					// Reheap the primary heap
					bubbleDown(pheap, 0, phcnt);
				}
			}
			fwrite(outbuf, sizeof(int), outbufcnt, outfile);
			fclose(outfile);
		}
		
		
		// Ok, so now all the runs have been generated
//		printf("%d\n", runcnt);
		FILE *fps[runcnt];
		elem *inpbuf = (elem *)malloc(sizeof(elem) * INPUTBUF); //This is the input buffer for structure type (key, ind)
		int bufelems = INPUTBUF/runcnt;
		int tmp; elem tmpelem;
		char of[30];
		counter = 0; // Important: This counter keeps track of the total number of elements present in the input buffer at any time
		for(i=0; i<runcnt; i++){
    		sprintf(of, "%s.%03d", argv[2], i); 
			FILE *outfp=fopen(of, "rb");
    		for(j=0; j<bufelems; j++){
    			fread(&tmp, sizeof(int), 1, outfp);
    			tmpelem.key = tmp;
    			tmpelem.ind = i;
    			inpbuf[counter++] = tmpelem;
			}
			fps[i]=outfp;
		}
		// Initial load of elements in the input buffer is complete
		qsort(inpbuf, counter, sizeof(elem), comp);
		//Opening sort.bin file for writing output
		FILE *ofp = fopen(argv[3], "wb");
		if(ofp == NULL){
			perror("Error: ");
		}
		//Creating output buffer
		int index; 
		int totalcnt = 0; // This keeps track of the total number of elements that have entered the output buffer
		outbufcnt = 0; // This keeps track of the number of elements currently present in the output buffer
		FILE *reqfp;
		while(totalcnt<totalkeys){
			outbuf[outbufcnt++] = inpbuf[0].key;
			totalcnt++;
			if(outbufcnt == OUTPUTBUF){
				fwrite(outbuf, sizeof(int), outbufcnt, ofp);
				outbufcnt = 0;
			}
			if(totalcnt == totalkeys){
				break;
			}
			index = inpbuf[0].ind; // Index tracks which run the current element belongs to
			reqfp = fps[index];
			//Reached end of the current run
			if(fread(&tmp, sizeof(int), 1, reqfp)){
				// First, store the file pointer (new position) in the file pointers array
				fps[index] = reqfp;
				tmpelem.key = tmp;
				tmpelem.ind = index;
				
				// Put the element at its correct position
				h=1;
				while(h<counter && tmpelem.key>inpbuf[h].key){
					inpbuf[h-1] = inpbuf[h];
					h++;
				}
				inpbuf[h-1] = tmpelem;
			}
			else{
				for(i=1; i<counter; i++){
					inpbuf[i-1] = inpbuf[i];
				}
				counter--;
				// Just close the file pointer pointed by this file since it has already been read
//				fclose(reqfp);
			}
		}
		fwrite(outbuf, sizeof(int), outbufcnt, ofp);
		fclose(ofp);
	}
	x:
	// ending time now
	gettimeofday(&tm_end, NULL);
	//Printing time difference now
	tm_diff.tv_sec=tm_end.tv_sec-tm_start.tv_sec;
	tm_diff.tv_usec=tm_end.tv_usec-tm_start.tv_usec;
	if(tm_diff.tv_usec<0){
		tm_diff.tv_sec--;
		tm_diff.tv_usec = 1000000 - tm_diff.tv_usec;
	}
	printf("Time: %ld.%06ld\n", tm_diff.tv_sec, tm_diff.tv_usec);
	
	return 0;
}
