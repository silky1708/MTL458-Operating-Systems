#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

struct PTE{
	int valid;   // 0: invalid(on disk), 1: valid(in physical memory)
	int dirty;   // 0: not changed; 1: changed
	int PFN;
};

struct cache_entry{
	int valid;  //0: empty, 1: occupied frame
	int use_bit;
	int time_of_access;
	int VPN;
};

int page_table_size = 1048576;  // 1024*1024

long VPN_mask = 0xfffff000;
unsigned long time = 0;


void OPT(int frames, FILE *file, bool verbose, struct PTE *page_table);

void FIFO(int frames, FILE *file, bool verbose, struct PTE *page_table);

void LRU(int frames, FILE *file, bool verbose, struct PTE *page_table);

void CLOCK(int frames, FILE *file, bool verbose, struct PTE *page_table);

void RANDOM(int frames, FILE *file, bool verbose, struct PTE *page_table);

void print_stats(int mem, int m, int w, int d){
	printf("Number of memory accesses: %d\n", mem);
	printf("Number of misses: %d\n", m);
	printf("Number of writes: %d\n", w);
	printf("Number of drops: %d\n", d);
}

void print_verbose_stats(int vbit, int brought_in, int replaced){
	char hex1[10];
	snprintf(hex1, 10, "%x", brought_in);
	char temp[6] = "00000";
	if(strlen(hex1) < 5){
		temp[5-strlen(hex1)] = '\0';
		strcat(temp, hex1);
		hex1[0] = '\0';
		strcat(hex1, temp);
	}


	char hex2[10];
	snprintf(hex2, 10, "%x", replaced);
	temp[0] = '0';
	temp[1] = '0';
	temp[2] = '0';
	temp[3] = '0';
	temp[4] = '0';
	if(strlen(hex2) < 5){
		temp[5-strlen(hex2)] = '\0';
		strcat(temp, hex2);
		hex2[0] = '\0';
		strcat(hex2, temp);
	}

	if(strlen(hex1)== 5 && strlen(hex2) == 5){
		if(vbit == 1){
			//write
			printf("Page 0x%s was read from disk, page 0x%s was written to the disk.\n", hex1, hex2);
		}
		else if(vbit == 2){
			//drop
			printf("Page 0x%s was read from disk, page 0x%s was dropped (it was not dirty).\n", hex1, hex2);
		}
	}

}


int main(int argc, char *argv[]){

	struct PTE *page_table = malloc(page_table_size*sizeof(struct PTE));

	struct PTE page_init;
	page_init.valid = 0;
	for(int pg=0;pg<page_table_size;pg++){
		page_table[pg]= page_init;
	}

	char* trace_file = argv[1];
	int frames = strtol(argv[2], NULL, 10);
	if(frames <= 0){
		// printf("Insufficient frames.. %d\n", frames);
		return 0;
	}

	char *strategy = argv[3];
	bool verbose = false;
	if(argc==5 && strcmp(argv[4], "-verbose") == 0){
		verbose = true;
	}

	FILE *fp = fopen(trace_file, "r");
	if(fp == NULL){
		// printf("Error in opening file..\n");
		return 0;
	}
	if(strcmp(strategy, "OPT") == 0){
		OPT(frames, fp, verbose, page_table);
	}
	else if(strcmp(strategy, "FIFO") == 0){
		FIFO(frames, fp, verbose, page_table);
	}
	else if(strcmp(strategy, "CLOCK") == 0){
		CLOCK(frames, fp, verbose, page_table);
	}
	else if(strcmp(strategy, "LRU") == 0){
		LRU(frames, fp, verbose, page_table);
	}
	else if(strcmp(strategy, "RANDOM") == 0){
		RANDOM(frames, fp, verbose, page_table);
	}
	fclose(fp);
	return 0;

}

/////////////////////// OPT ///////////////////////////////////////////////

//new--

void OPT(int frames, FILE *file, bool verbose, struct PTE *page_table){

	struct cache_entry *cache = malloc(frames*sizeof(struct cache_entry));
	struct cache_entry cache_init;
	cache_init.valid = 0;
	for(int fr=0;fr<frames;fr++){
		cache[fr]= cache_init;
	}

	int mem_accesses = 0;
	int misses = 0;
	int writes = 0;
	int drops = 0;

	char addr[50];  // virtual address
	char rw[5];      // read/write

	int MAX_ = 2*1000*10000 + 1;

	int* all_entries = malloc(MAX_*sizeof(int)); // parse the file and store mem accesses

	memset(all_entries, 0, sizeof(all_entries));

	int entry = 0;
	while(fscanf(file, "%s", addr) != EOF){
		fscanf(file, "%s", rw);
		mem_accesses++;
		long Virtualaddr = (long)strtol(addr, NULL, 0);
		long VPN = (Virtualaddr & VPN_mask)>>12;
		all_entries[entry] = (int)VPN;
		if(strcmp(rw, "W") == 0){
			all_entries[entry+1] = 1; //0: read, 1:write
		}
		entry += 2;
	}

	for(int i=0;i<2*mem_accesses; i+=2) {
		int page_no = all_entries[i];

		if(page_table[page_no].valid == 0){
			//invalid
			misses++;
			int j=0;
			for(;j<frames; j++){
				if(cache[j].valid == 0){
					struct cache_entry memory;
					memory.valid = 1;
					memory.use_bit = 0;
					memory.time_of_access = 0;
					memory.VPN = page_no;
					cache[j] = memory;

					page_table[page_no].valid = 1;
					page_table[page_no].dirty = 0;
					if(all_entries[i+1] == 1){
						// brought in memory and then write
						page_table[page_no].dirty = 1;
					}
					page_table[page_no].PFN = j;

					break;
				}
			}
			if(j == frames){
				//page replacement OPT
				int last_required_PFN = 0;
				if(frames > 1) {
					time = 0;
					for(int k=0;k<frames;k++){
						cache[k].time_of_access = 0;
					}

					for(entry=i+2; entry<2*mem_accesses; entry+=2){
						int vpn = all_entries[entry];

						if(page_table[vpn].valid == 1 && cache[page_table[vpn].PFN].time_of_access == 0){
							time++;
							cache[page_table[vpn].PFN].time_of_access = time;
							last_required_PFN++;
							if(last_required_PFN == frames){
								last_required_PFN = page_table[vpn].PFN;
								break;
							}
						}

					}

					if(time < frames) {
						for(int z=0;z<frames;z++){
							if(cache[z].time_of_access == 0){
								// breaking ties w/ minm frame number
								last_required_PFN = z;
								break;
							}
						}
					}

				}

				int vbit = 1;
				if(page_table[cache[last_required_PFN].VPN].dirty == 1){
					writes++;   // write to disk
				}
				else{
					drops++;
					vbit = 2;
				}
				if(verbose){
					int replaced = cache[last_required_PFN].VPN;
					print_verbose_stats(vbit, page_no, replaced);
				}

				page_table[cache[last_required_PFN].VPN].valid = 0;
				cache[last_required_PFN].valid = 1;
				cache[last_required_PFN].use_bit = 0;
				cache[last_required_PFN].time_of_access = 0;
				cache[last_required_PFN].VPN = page_no;

				page_table[page_no].valid = 1;
				page_table[page_no].dirty = 0;
				page_table[page_no].PFN = last_required_PFN;

				if(all_entries[i+1] == 1){
					page_table[page_no].dirty = 1;

				}

			}

		}
		else{
			// valid page
			if(all_entries[i+1] == 1){
				page_table[page_no].dirty = 1;
				// writes++;
			}
		}
	}

	print_stats(mem_accesses, misses, writes, drops);
}


/////////////////// FIFO ////////////////////////////////////////////////////
void FIFO(int frames, FILE *file, bool verbose, struct PTE *page_table){
	struct cache_entry *cache = malloc(frames*sizeof(struct cache_entry));
	struct cache_entry cache_init;
	cache_init.valid = 0;
	for(int fr=0;fr<frames;fr++){
		cache[fr] = cache_init;
	}

	int mem_accesses = 0;
	int misses = 0;
	int writes = 0;
	int drops = 0;

	time = 0;
	char addr[50];  // virtual address
	char rw[5];      // read/write
	while(fscanf(file, "%s", addr) != EOF){

		fscanf(file, "%s", rw);
		mem_accesses++;

		long Virtualaddr = (long)strtol(addr, NULL, 0);
		long VPN = (Virtualaddr & VPN_mask)>>12;

		if(page_table[VPN].valid == 0){
			//invalid

			misses++;
			int j=0;
			for(;j<frames; j++){
				if(cache[j].valid == 0){
					struct cache_entry memory;
					memory.valid = 1;
					memory.use_bit = 0;
					memory.time_of_access = time;
					time++;
					memory.VPN = VPN;
					cache[j] = memory;

					page_table[VPN].valid = 1;
					page_table[VPN].dirty = 0;
					if(strcmp(rw ,"W") == 0){
						// brought in memory and then write
						page_table[VPN].dirty = 1;
						// writes++;
					}
					page_table[VPN].PFN = j;

					break;
				}
			}
			if(j == frames){
				//page replacement FIFO

				int last_required_PFN = 0;
				int minm_time_stamp = cache[0].time_of_access;

				for(int z=0;z<frames;z++){
					if(cache[z].time_of_access < minm_time_stamp){
						last_required_PFN = z;
						minm_time_stamp = cache[z].time_of_access;

					}
				}

				int vbit = 1;
				if(page_table[cache[last_required_PFN].VPN].dirty == 1){
					writes++;   // write to disk
				}
				else{
					drops++;
					vbit = 2;
				}
				if(verbose){
					int replaced = cache[last_required_PFN].VPN;
					print_verbose_stats(vbit, VPN, replaced);
				}

				page_table[cache[last_required_PFN].VPN].valid = 0;
				cache[last_required_PFN].valid = 1;
				cache[last_required_PFN].use_bit = 0;
				cache[last_required_PFN].time_of_access = time;
				time++;
				cache[last_required_PFN].VPN = VPN;

				page_table[VPN].valid = 1;
				page_table[VPN].dirty = 0;
				page_table[VPN].PFN = last_required_PFN;

				if(strcmp(rw ,"W") == 0){
					page_table[VPN].dirty = 1;
				}

			}

		}
		else{
			//valid page
			if(strcmp(rw ,"W") == 0){
				page_table[VPN].dirty = 1;
			}
		}
	}

	print_stats(mem_accesses, misses, writes, drops);
}

////////////////////////////////////////// LRU ////////////////////////////////////
void LRU(int frames, FILE *file, bool verbose, struct PTE *page_table){
	struct cache_entry *cache = malloc(frames*sizeof(struct cache_entry));
	struct cache_entry cache_init;
	cache_init.valid = 0;
	for(int fr=0;fr<frames;fr++){
		cache[fr]= cache_init;
	}

	int mem_accesses = 0;
	int misses = 0;
	int writes = 0;
	int drops = 0;

	time = 0;
	char addr[50];  // virtual address
	char rw[5];      // read/write
	while(fscanf(file, "%s", addr) != EOF){
		fscanf(file, "%s", rw);
		mem_accesses++;
		// printf("{%s}\n", addr);
		// printf("{%s}\n", rw);
		long Virtualaddr = (long)strtol(addr, NULL, 0);
		long VPN = (Virtualaddr & VPN_mask)>>12;
		// printf("VPN: %lu\n", VPN);
		if(page_table[VPN].valid == 0){
			//invalid

			misses++;
			int j=0;
			for(j=0;j<frames; j++){
				if(cache[j].valid == 0){
					struct cache_entry memory;
					memory.valid = 1;
					memory.use_bit = 0;
					memory.time_of_access = time;
					time++;
					memory.VPN = VPN;
					cache[j] = memory;

					page_table[VPN].valid = 1;
					page_table[VPN].dirty = 0;
					if(strcmp(rw ,"W") == 0){
						// brought in memory and then write
						page_table[VPN].dirty = 1;
						// writes++;
					}
					page_table[VPN].PFN = j;

					break;
				}
			}
			if(j == frames){
				//page replacement LRU

				int last_required_PFN = 0;
				int minm_time_stamp = cache[0].time_of_access;

				for(int z=0;z<frames;z++){
					if(cache[z].time_of_access < minm_time_stamp){
						last_required_PFN = z;
						minm_time_stamp = cache[z].time_of_access;

					}
				}

				int vbit = 1;
				if(page_table[cache[last_required_PFN].VPN].dirty == 1){
					writes++;   // write to disk
				}
				else{
					drops++;
					vbit = 2;
				}

				if(verbose){
					int replaced = cache[last_required_PFN].VPN;
					print_verbose_stats(vbit, VPN, replaced);
				}

				page_table[cache[last_required_PFN].VPN].valid = 0;
				cache[last_required_PFN].valid = 1;
				cache[last_required_PFN].use_bit = 0;
				cache[last_required_PFN].time_of_access = time;
				time++;
				cache[last_required_PFN].VPN = VPN;

				page_table[VPN].valid = 1;
				page_table[VPN].dirty = 0;
				page_table[VPN].PFN = last_required_PFN;

				if(strcmp(rw ,"W") == 0){
					page_table[VPN].dirty = 1;
				}

			}

		}
		else{
			//valid page
			if(strcmp(rw ,"W") == 0){
				page_table[VPN].dirty = 1;
			}
			cache[page_table[VPN].PFN].time_of_access = time;
			time++;
		}
	}

	print_stats(mem_accesses, misses, writes, drops);

}

////////////////// CLOCK ///////////////////////////////////////////////////
void CLOCK(int frames, FILE *file, bool verbose, struct PTE *page_table){
	struct cache_entry *cache = malloc(frames*sizeof(struct cache_entry));
	struct cache_entry cache_init;
	cache_init.valid = 0;
	for(int fr=0;fr<frames;fr++){
		cache[fr] = cache_init;
	}

	int mem_accesses = 0;
	int misses = 0;
	int writes = 0;
	int drops = 0;

	time = 0;
	char addr[50];  // virtual address
	char rw[5];      // read/write
	while(fscanf(file, "%s", addr) != EOF){
		fscanf(file, "%s", rw);
		mem_accesses++;

		long Virtualaddr = (long)strtol(addr, NULL, 0);
		long VPN = (Virtualaddr & VPN_mask)>>12;

		if(page_table[VPN].valid == 0){
			//invalid
			misses++;
			int j=0;
			for(j=0;j<frames; j++){
				if(cache[j].valid == 0){
					struct cache_entry memory;
					memory.valid = 1;
					memory.use_bit = 1;
					memory.time_of_access = 0;
					// time++;
					memory.VPN = VPN;
					cache[j] = memory;

					page_table[VPN].valid = 1;
					page_table[VPN].dirty = 0;
					if(strcmp(rw ,"W") == 0){
						// brought in memory and then write
						page_table[VPN].dirty = 1;
						// writes++;
					}
					page_table[VPN].PFN = j;

					break;
				}
			}
			if(j == frames){
				//page replacement CLOCK

				int last_required_PFN = time;
				while(cache[last_required_PFN].use_bit == 1){
					cache[last_required_PFN].use_bit = 0;
					last_required_PFN = (last_required_PFN + 1)%frames;
					time = last_required_PFN;
				}
				time = (time + 1)%frames;

				int vbit = 1;
				if(page_table[cache[last_required_PFN].VPN].dirty == 1){
					writes++;   // write to disk
				}
				else{
					drops++;
					vbit = 2;
				}

				if(verbose){
					int replaced = cache[last_required_PFN].VPN;
					print_verbose_stats(vbit, VPN, replaced);
				}

				page_table[cache[last_required_PFN].VPN].valid = 0;
				cache[last_required_PFN].valid = 1;
				cache[last_required_PFN].use_bit = 1;
				cache[last_required_PFN].time_of_access = 0;
				cache[last_required_PFN].VPN = VPN;

				page_table[VPN].valid = 1;
				page_table[VPN].dirty = 0;
				page_table[VPN].PFN = last_required_PFN;

				if(strcmp(rw ,"W") == 0){
					page_table[VPN].dirty = 1;
				}

			}

		}
		else{
			//valid page
			if(strcmp(rw ,"W") == 0){
				page_table[VPN].dirty = 1;
			}
			cache[page_table[VPN].PFN].use_bit = 1;

		}
	}

	print_stats(mem_accesses, misses, writes, drops);

}

////////////////////// RANDOM /////////////////////////////////////////////
void RANDOM(int frames, FILE *file, bool verbose, struct PTE *page_table){
	struct cache_entry *cache = malloc(frames*sizeof(struct cache_entry));
	struct cache_entry cache_init;
	cache_init.valid = 0;
	for(int fr=0;fr<frames;fr++){
		cache[fr]= cache_init;
	}

	int mem_accesses = 0;
	int misses = 0;
	int writes = 0;
	int drops = 0;
	srand((unsigned) 5635);
	// int replacements = 0;

	char addr[50];  // virtual address
	char rw[5];      // read/write
	while(fscanf(file, "%s", addr) != EOF){
		fscanf(file, "%s", rw);
		mem_accesses++;

		long Virtualaddr = (long)strtol(addr, NULL, 0);
		long VPN = (Virtualaddr & VPN_mask)>>12;
		if(page_table[VPN].valid == 0){
			//invalid
			misses++;
			int j=0;
			for(j=0;j<frames; j++){
				if(cache[j].valid == 0){
					struct cache_entry memory;
					memory.valid = 1;
					memory.use_bit = 0;
					memory.time_of_access = 0;
					// time++;
					memory.VPN = VPN;
					cache[j] = memory;

					page_table[VPN].valid = 1;
					page_table[VPN].dirty = 0;
					if(strcmp(rw ,"W") == 0){
						// brought in memory and then write
						page_table[VPN].dirty = 1;
						// writes++;
					}
					page_table[VPN].PFN = j;

					break;
				}
			}
			if(j == frames){
				//page replacement RANDOM
				int last_required_PFN = rand()%frames;

				int vb_bit = 1;
				if(page_table[cache[last_required_PFN].VPN].dirty == 1){
					writes++;   // write to disk
				}
				else{
					drops++;
					vb_bit = 2;
				}
				int replaced = cache[last_required_PFN].VPN;
				page_table[cache[last_required_PFN].VPN].valid = 0;
				cache[last_required_PFN].valid = 1;
				cache[last_required_PFN].use_bit = 0;
				cache[last_required_PFN].time_of_access = 0;
				// time++;
				cache[last_required_PFN].VPN = VPN;

				page_table[VPN].valid = 1;
				page_table[VPN].dirty = 0;
				page_table[VPN].PFN = last_required_PFN;

				if(strcmp(rw ,"W") == 0){
					page_table[VPN].dirty = 1;
				}

				if(verbose){
					print_verbose_stats(vb_bit, VPN, replaced);
				}

			}

		}
		else{
			//valid page
			if(strcmp(rw ,"W") == 0){
				page_table[VPN].dirty = 1;
			}
		}
	}

	print_stats(mem_accesses, misses, writes, drops);
}