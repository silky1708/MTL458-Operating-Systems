#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <inttypes.h>

struct Header{
	int size;   // size of the allocated block
	int magic_number; // = 97979
};
int* size_of_header = NULL;
int* smallest_chunk_idx = NULL;
int* smallest_chunk_size = NULL;
int* largest_chunk_idx = NULL;
int* largest_chunk_size = NULL;
int* blocks_allocated = NULL;
int* current_size_of_heap = NULL;
int* alloc_idx = NULL;
int* free_idx = NULL;
int* temp_idx = NULL;

// free list
struct Node{
	void* start_addr;
	int size;
	struct Node* next;
};

void *heap = NULL;
struct Node* freelist_head = NULL;

void iterate_for_smallest(){
	*temp_idx = 0;
	struct Node *temp_smallest = freelist_head;
	*smallest_chunk_size = 5000;
	*smallest_chunk_idx = -2;
	while(temp_smallest != NULL){
		if(temp_smallest->size < *smallest_chunk_size){
			*smallest_chunk_size = temp_smallest->size;
			*smallest_chunk_idx = *temp_idx;
		}
		temp_smallest = temp_smallest->next;
		*temp_idx += 1;
	}
	return;
}

void iterate_for_largest(){
	*temp_idx = 0;
	struct Node *temp_largest = freelist_head;
	*largest_chunk_size = 0;
	*largest_chunk_idx = -2;
	while(temp_largest != NULL){
		if(temp_largest->size > *largest_chunk_size){
			*largest_chunk_size = temp_largest->size;
			*largest_chunk_idx = *temp_idx;
		}
		temp_largest = temp_largest->next;
		*temp_idx += 1;
	}
	return;
}

void insert_new_node(struct Node *node){
	struct Node *tempo = freelist_head;
	struct Node *previous = NULL;
	while(tempo != NULL){
		if((uintptr_t)node->start_addr > (uintptr_t)((char *)tempo->start_addr + tempo->size)){
			node->next = tempo->next;
			tempo->next = node;
			return;
		}
		previous = tempo;
		tempo = tempo->next;
	}

	if(previous != NULL){
		if((uintptr_t)node->start_addr > (uintptr_t)((char *)previous->start_addr + previous->size)){
			node->next = NULL;
			previous->next = node;
			return;
		}
	}
	return;
}


int my_init(){
		heap = mmap(NULL, 4096, PROT_READ| PROT_WRITE, MAP_PRIVATE| MAP_ANONYMOUS, -1, 0);
		if(heap == MAP_FAILED){
			return errno; // return error code
		}

		size_of_header = (int*)heap;
		smallest_chunk_idx = (int*)heap +1;
		smallest_chunk_size = (int*)heap + 2;
		largest_chunk_idx = (int*)heap + 3;
		largest_chunk_size = (int*)heap + 4;
		blocks_allocated = (int*)heap + 5;
		current_size_of_heap = (int*)heap + 6;
		alloc_idx = (int*)heap + 7;
		free_idx = (int*)heap + 8;
		temp_idx = (int*)heap + 9;

		struct Header *first_header = (struct Header *)((int *)heap + 10);
		first_header->size = sizeof(struct Node);
		first_header->magic_number = 97979;
		*temp_idx = 10;  // no. of global pointers

		// initialize free list, a linked list
		freelist_head = (struct Node*) ((char *)heap+(*temp_idx)*sizeof(int)+sizeof(struct Header));
		// printf("heap addr: %lu\n", (unsigned long)heap);
		freelist_head->start_addr = (void *)((char *)heap+ ((*temp_idx)*sizeof(int))+ sizeof(struct Node)+ sizeof(struct Header));
		freelist_head->size = 4096 - (sizeof(struct Node) + (*temp_idx)*sizeof(int) + sizeof(struct Header));
		freelist_head->next = NULL;

		//initialize smallest and largest chunks' idx and size, blocks
		*size_of_header = sizeof(struct Header);
		*smallest_chunk_idx = 0;
		*smallest_chunk_size = freelist_head->size;
		*largest_chunk_idx = 0;
		*largest_chunk_size = freelist_head->size;
		*blocks_allocated = 0;
		*current_size_of_heap = sizeof(struct Node) + sizeof(struct Header);


		return 0;   // successful
}

void my_free(void *);

void* my_alloc(int count){
	// count must be a multiple of 8 bytes
	if(count % 8 != 0 || count <= 0){
		return NULL;
	}
	// int blocks = count/8;
	struct Node* iter = freelist_head;
	struct Node* prev = NULL;

	//search for (count + size_of_header) bytes. (first-fit algorithm)
	*alloc_idx = 0;
	while(iter != NULL){
		if(iter->size >= (count + *size_of_header)){
			*blocks_allocated += 1;
			*current_size_of_heap += (count + *size_of_header);

			//store size in header
			struct Header* memory_header = (struct Header*)(iter->start_addr);
			memory_header->size = count;
			memory_header->magic_number = 97979;

			//update smallest and largest chunks accordingly

			if(iter->size > (count+ *size_of_header)){
				iter->start_addr = (void *)((char *)iter->start_addr + (count + *size_of_header));  // pointer arithmetic w/ void *
				iter->size = iter->size - (count+ *size_of_header);

				// only when remaining size is not 0; need to update smallest_chunk_size
				if(iter->size < *smallest_chunk_size){
					*smallest_chunk_size = iter->size;
					*smallest_chunk_idx = *alloc_idx;
				}

				if(*alloc_idx == *largest_chunk_idx){
					iterate_for_largest();
				}
				return (iter->start_addr - count);
			}
			else{
				void * found_space = iter->start_addr;
				if(prev != NULL){
					prev->next = iter->next;
				}
				else{
					if(iter->next == NULL && found_space != NULL){
						//no free space left
						freelist_head->start_addr = (void *)((char *)heap+4096);
						freelist_head->size = 0;
						*smallest_chunk_size = 0;
						*largest_chunk_size = 0;
						*smallest_chunk_idx = -1;
						*largest_chunk_idx = -1;
						return ((char *)found_space + *size_of_header);
					}
					else freelist_head = iter->next;
				}

				*blocks_allocated += 1;    // TODO: what if free doesn't succeed? done

				if(*largest_chunk_idx > *alloc_idx){
					*largest_chunk_idx-= 1;
				}
				else if(*largest_chunk_idx == *alloc_idx){
					*largest_chunk_idx = -1;
				}
				if(*smallest_chunk_idx > *alloc_idx){
					*smallest_chunk_idx -= 1;
				}
				else if(*smallest_chunk_idx == *alloc_idx){
					*smallest_chunk_idx = -1;
				}
				my_free((void *)iter);

				if(*largest_chunk_idx == -1){
					iterate_for_largest();
				}
				if(*smallest_chunk_idx == -1){
					iterate_for_smallest();
				}

				return (found_space + *size_of_header);
			}
		}

		prev = iter;
		iter = iter->next;
		*alloc_idx += 1;
	}
	return NULL;
}



void my_free(void *ptr){
	// Replace free with your functionality
	if(ptr == NULL){
		return;
	}
	struct Header *start_of_memory = (struct Header *)((char *)ptr - *size_of_header);
	if(start_of_memory->magic_number != 97979){
		return;
	}

	*blocks_allocated -= 1;   // caution here
	// free (start_of_memory->size + *size_of_header)
	struct Node *tempf = freelist_head;
	struct Node *prev = NULL;
	struct Node *prev2 = NULL;
	*free_idx = 0;
	void * node_begin = (void *)start_of_memory;
	void * node_end = (void *)((char *)ptr + start_of_memory->size);
	*current_size_of_heap -= (*size_of_header + start_of_memory->size);

	if(tempf->next == NULL && tempf->size == 0){
		// freeing a block ,change of state from fully occupied heap
		tempf->start_addr = node_begin;
		tempf->size = *size_of_header + start_of_memory->size;
		*smallest_chunk_size = tempf->size;
		*largest_chunk_size = tempf->size;
		*smallest_chunk_idx = 0;
		*largest_chunk_idx = 0;
		return;

	}

	while(tempf != NULL){
		if(((uintptr_t)start_of_memory == (uintptr_t)tempf->start_addr) && ((*size_of_header + start_of_memory->size) == tempf->size)){
			// invalid address; already free
			*blocks_allocated += 1;
			*current_size_of_heap += (*size_of_header + start_of_memory->size);
			return;
		}
		if((uintptr_t)start_of_memory < (uintptr_t)tempf->start_addr){
			void * node2_begin = tempf->start_addr;

			if(prev != NULL){
				void * node1_end = (void *)((char *)(prev->start_addr) + prev->size);
				if((uintptr_t)node1_end == (uintptr_t)node_begin && (uintptr_t)node_end == (uintptr_t)node2_begin){
					prev->next = tempf->next;
					prev->size = prev->size + tempf->size + *size_of_header + start_of_memory->size;

					// update smallest and largest chunks
					if(*largest_chunk_idx == *free_idx){
						*largest_chunk_idx -= 1;
						*largest_chunk_size = prev->size;
					}
					else if(*largest_chunk_idx == *free_idx - 1){
						*largest_chunk_size = prev->size;
					}
					else{
						if(prev->size > *largest_chunk_size){
							*largest_chunk_size = prev->size;
							*largest_chunk_idx = *free_idx - 1;
						}
						else if(*largest_chunk_idx > *free_idx){
							*largest_chunk_idx -=1;
						}
					}

					if(*smallest_chunk_idx == *free_idx || *smallest_chunk_idx == *free_idx-1){
						iterate_for_smallest();
					}
					else{
						if(*smallest_chunk_idx > *free_idx){
							*smallest_chunk_idx -= 1;
						}
					}

					*blocks_allocated += 1;
					my_free((void *)tempf);

				}
				else if((uintptr_t)node1_end == (uintptr_t)node_begin){

					prev->size = prev->size + *size_of_header + start_of_memory->size;
					//update smallest and largest chunks
					if(*free_idx-1 == *largest_chunk_idx){
						*largest_chunk_size = prev->size;
					}
					else{
						if(*largest_chunk_size < prev->size){
							*largest_chunk_size = prev->size;
							*largest_chunk_idx = *free_idx-1;
						}
					}

					if(*free_idx-1 == *smallest_chunk_idx){
						iterate_for_smallest();
					}
					else{
						// do nothing
					}



					if( ( (uintptr_t)((char *)prev - *size_of_header) == (uintptr_t)((char *)prev->start_addr + prev->size) ) &&
						((uintptr_t)((char *)prev + sizeof(struct Node)) == (uintptr_t)(tempf->start_addr))){
						if(prev2 == NULL){
								freelist_head = tempf;
						}
						else{
								prev2->next = tempf;
						}
						*current_size_of_heap -= (*size_of_header + sizeof(struct Node));
						tempf->start_addr = prev->start_addr;
						tempf->size = prev->size + tempf->size + *size_of_header +sizeof(struct Node);

						// update smallest and largest chunks
						if(*free_idx == *largest_chunk_idx){
							*largest_chunk_size = tempf->size;
							*largest_chunk_idx -=1;
						}
						else if(*free_idx-1 == *largest_chunk_idx){
								*largest_chunk_size = tempf->size;
						}
						else{
							if(tempf->size > *largest_chunk_size){
								*largest_chunk_size = tempf->size;
								*largest_chunk_idx = *free_idx-1;
							}
							else{
								if(*largest_chunk_idx >= *free_idx){
									*largest_chunk_idx -=1;
								}
							}
						}

						if((*smallest_chunk_idx == *free_idx-1) || (*smallest_chunk_idx == *free_idx)){
								iterate_for_smallest();
						}
						else{
							if(*smallest_chunk_idx >= *free_idx){
								*smallest_chunk_idx -= 1;
							}
						}
					}
				}
				else if((uintptr_t)node2_begin == (uintptr_t)node_end){
					tempf->start_addr = (void *)start_of_memory;
					tempf->size = tempf->size + *size_of_header + start_of_memory->size;

					//update smallest and largest chunks
					if(*free_idx == *largest_chunk_idx){
						*largest_chunk_size = tempf->size;
					}
					else{
						if(*largest_chunk_size < tempf->size){
							*largest_chunk_size = tempf->size;
							*largest_chunk_idx = *free_idx;
						}
					}

					if(*free_idx == *smallest_chunk_idx){
						iterate_for_smallest();
					}
					else{
						// do nothing
					}


					if( ( (uintptr_t)((char *)prev - *size_of_header) == (uintptr_t)((char *)prev->start_addr + prev->size) ) &&
						((uintptr_t)((char *)prev + sizeof(struct Node)) == (uintptr_t)(tempf->start_addr))){

						if(prev2 == NULL){
								freelist_head = tempf;
						}
						else{
								prev2->next = tempf;
						}
						*current_size_of_heap -= (*size_of_header + sizeof(struct Node));
						tempf->start_addr = prev->start_addr;
						tempf->size = prev->size + tempf->size + *size_of_header +sizeof(struct Node);

						// update smallest and largest chunks
						if(*free_idx == *largest_chunk_idx){
							*largest_chunk_size = tempf->size;
							*largest_chunk_idx -=1;
						}
						else if(*free_idx-1 == *largest_chunk_idx){
								*largest_chunk_size = tempf->size;
						}
						else{
							if(tempf->size > *largest_chunk_size){
								*largest_chunk_size = tempf->size;
								*largest_chunk_idx = *free_idx-1;
							}
							else{
								if(*largest_chunk_idx >= *free_idx){
									*largest_chunk_idx -=1;
								}
							}
						}

						if((*smallest_chunk_idx == *free_idx-1) || (*smallest_chunk_idx == *free_idx)){
								iterate_for_smallest();
						}
						else{
							if(*smallest_chunk_idx >= *free_idx){
								*smallest_chunk_idx -= 1;
							}
						}


					}
				}
				else{
					//create a new freelist node..
					*blocks_allocated -= 1;
					void * space = my_alloc(sizeof(struct Node));
					if(space != NULL){
						if(freelist_head != NULL){
							if(freelist_head->next == NULL && freelist_head->size == 0){
								freelist_head->start_addr = (void *)start_of_memory;
								freelist_head->size = *size_of_header + start_of_memory->size;
								// freelist_head->next = NULL;
								*smallest_chunk_size = freelist_head->size;
								*largest_chunk_size = freelist_head->size;
								*smallest_chunk_idx = 0;
								*largest_chunk_idx = 0;
								return;
							}
						}
						else{
							freelist_head = (struct Node *)my_alloc(sizeof(struct Node));
							if(freelist_head != NULL){
								freelist_head->start_addr = (void *)start_of_memory;
								freelist_head->size = *size_of_header + start_of_memory->size;
								// freelist_head->next = NULL;
								*smallest_chunk_size = freelist_head->size;
								*largest_chunk_size = freelist_head->size;
								*smallest_chunk_idx = 0;
								*largest_chunk_idx = 0;
							}
							return;
						}

						if(((uintptr_t)((char *)tempf->start_addr - sizeof(struct Node)) == (uintptr_t)space) &&
							((uintptr_t)((char *)tempf - *size_of_header) == (uintptr_t)((char *)start_of_memory + *size_of_header + start_of_memory->size)) &&
							((uintptr_t)((char *)tempf + sizeof(struct Node)+ *size_of_header) == (uintptr_t)space)) {
							struct Node * new_node = (struct Node *)((char *)start_of_memory + *size_of_header);
							new_node->start_addr = (void *)((char *)start_of_memory + sizeof(struct Node) + *size_of_header);
							new_node->size = *size_of_header*2 + start_of_memory->size + tempf->size + sizeof(struct Node);
							new_node->next = tempf->next;
							prev->next = new_node;
							*current_size_of_heap -= (*size_of_header + sizeof(struct Node));
							iterate_for_smallest();
							iterate_for_largest();
							start_of_memory->size = sizeof(struct Node);
							return;
						}

						if(((uintptr_t)((char *)prev->start_addr - sizeof(struct Node)) == (uintptr_t)space) &&
							((uintptr_t)((char *)prev + sizeof(struct Node)) == (uintptr_t)(start_of_memory)) &&
							((uintptr_t)((char *)prev - *size_of_header) == (uintptr_t)((char *)prev->start_addr + prev->size))){

							struct Node * new_node = (struct Node *)space;
							new_node->start_addr = (void *)((char *)space + sizeof(struct Node));
							new_node->size = *size_of_header*2 + start_of_memory->size + prev->size + sizeof(struct Node);
							new_node->next = tempf;
							if(prev2 != NULL){
								prev2->next = new_node;
							}
							else{
								freelist_head = new_node;
							}
							iterate_for_smallest();
							iterate_for_largest();
							*current_size_of_heap -= (sizeof(struct Node) + *size_of_header);
							return;
						}

						struct Node * new_node = (struct Node *)space;
						new_node->start_addr = (void *)start_of_memory;
						new_node->size = *size_of_header + start_of_memory->size;
						// new_node->next = tempf;
						// prev->next = new_node;
						insert_new_node(new_node);
						iterate_for_smallest();
						iterate_for_largest();
					}
					else{
						// printf("my_free()[1]: Not enough space in the heap.\n");
						*blocks_allocated += 2; //TODO: check
					}
				}
			}
			else{
				// tempf = head of freelist node, prev = NULL
				if((uintptr_t)node2_begin == (uintptr_t)node_end){
					tempf->start_addr = (void *)start_of_memory;
					tempf->size = tempf->size + *size_of_header + start_of_memory->size;

					//update smallest and largest chunks
					if(*free_idx == *largest_chunk_idx){
						*largest_chunk_size = tempf->size;
					}
					else{
						if(tempf->size > *largest_chunk_size){
							*largest_chunk_size = tempf->size;
							*largest_chunk_idx = *free_idx;
						}

					}

					if(*free_idx == *smallest_chunk_idx){
						iterate_for_smallest();
					}

				}
				else if((uintptr_t)node_end < (uintptr_t)node2_begin){
					*blocks_allocated -= 1;
					void * space = my_alloc(sizeof(struct Node));

					if(space != NULL){
						if(freelist_head != NULL){
							if(freelist_head->next == NULL && freelist_head->size == 0){
								freelist_head->start_addr = (void *)start_of_memory;
								freelist_head->size = *size_of_header + start_of_memory->size;
								// freelist_head->next = NULL;
								*smallest_chunk_size = freelist_head->size;
								*largest_chunk_size = freelist_head->size;
								*smallest_chunk_idx = 0;
								*largest_chunk_idx = 0;
								return;
							}
						}
						else{
							freelist_head = (struct Node *)my_alloc(sizeof(struct Node));
							if(freelist_head != NULL){
								freelist_head->start_addr = (void *)start_of_memory;
								freelist_head->size = *size_of_header + start_of_memory->size;
								// freelist_head->next = NULL;
								*smallest_chunk_size = freelist_head->size;
								*largest_chunk_size = freelist_head->size;
								*smallest_chunk_idx = 0;
								*largest_chunk_idx = 0;
							}
							return;
						}

						if(((uintptr_t)((char *)tempf->start_addr - sizeof(struct Node)) == (uintptr_t)space) &&
							((uintptr_t)((char *)tempf - *size_of_header) == (uintptr_t)((char *)start_of_memory + *size_of_header + start_of_memory->size)) &&
							((uintptr_t)((char *)tempf + sizeof(struct Node) + *size_of_header) == (uintptr_t)space)) {
							struct Node * new_node = (struct Node *)((char *)start_of_memory + *size_of_header);
							new_node->start_addr = (void *)((char *)start_of_memory + sizeof(struct Node) + *size_of_header);
							new_node->size = *size_of_header*2 + start_of_memory->size + tempf->size + sizeof(struct Node);
							new_node->next = tempf->next;
							freelist_head = new_node;
							*current_size_of_heap -= (*size_of_header + sizeof(struct Node));
							iterate_for_smallest();
							iterate_for_largest();
							start_of_memory->size = sizeof(struct Node);
							return;
						}

						struct Node * new_node = (struct Node *)space;
						new_node->start_addr = (void *)start_of_memory;
						new_node->size = *size_of_header + start_of_memory->size;
						new_node->next = freelist_head;
						freelist_head = new_node;
						// freelist_head->start_addr = new_node->start_addr;
						// freelist_head->size = new_node->size;
						// freelist_head->next = tempf;

						//update smallest and largest chunks
						// if(new_node->size > *largest_chunk_size){
						// 	*largest_chunk_size = new_node->size;
						// 	*largest_chunk_idx = *free_idx; //0
						// }
						// else{
						// 	// if(*largest_chunk_idx >= *free_idx){
						// 	// 	*largest_chunk_idx += 1;
						// 	// }
						// 	*largest_chunk_idx += 1;
						// }

						// if(new_node->size < *smallest_chunk_size){
						// 	*smallest_chunk_size = new_node->size;
						// 	*smallest_chunk_idx = *free_idx;  //0
						// }
						// else{

						// 	*smallest_chunk_idx +=1;
						// }
						iterate_for_smallest();
						iterate_for_largest();
						return;
					}
					else{
						// printf("my_free()[2]: Not enough space in the heap.\n");
						*blocks_allocated += 2; //TODO: confirm this!
					}

				}
				else{
					// printf("my_free()[3]: error, overlapping memory spaces, recheck.\n");
				}
			}

			return;
		}
		prev2 = prev;
		prev = tempf;
		tempf = tempf->next;
		*free_idx += 1;
	}

	*free_idx -=1;

	void * prev_end = (void *)((char *)prev->start_addr + prev->size);
	if((uintptr_t)prev_end == (uintptr_t)node_begin){
		prev->size = prev->size + *size_of_header + start_of_memory->size;
		// prev->next = NULL; //redundant

		//update smallest and largest chunks
		if(*free_idx == *largest_chunk_idx){
			*largest_chunk_size = prev->size;

		}
		else{
			if(prev->size > *largest_chunk_size){
				*largest_chunk_size = prev->size;
				*largest_chunk_idx = *free_idx;
			}
		}

		if(*free_idx == *smallest_chunk_idx){
			iterate_for_smallest();
		}
	}
	else{

		*blocks_allocated -= 1;
		void * space = my_alloc((int) sizeof(struct Node));
		if(space != NULL){
			if(freelist_head != NULL){
				if(freelist_head->next == NULL && freelist_head->size == 0){
					freelist_head->start_addr = (void *)start_of_memory;
					freelist_head->size = *size_of_header + start_of_memory->size;
					// freelist_head->next = NULL;
					*smallest_chunk_size = freelist_head->size;
					*largest_chunk_size = freelist_head->size;
					*smallest_chunk_idx = 0;
					*largest_chunk_idx = 0;
					return;
				}
			}
			else{
				freelist_head = (struct Node *)my_alloc(sizeof(struct Node));
				if(freelist_head != NULL){
					freelist_head->start_addr = (void *)start_of_memory;
					freelist_head->size = *size_of_header + start_of_memory->size;
					// freelist_head->next = NULL;
					*smallest_chunk_size = freelist_head->size;
					*largest_chunk_size = freelist_head->size;
					*smallest_chunk_idx = 0;
					*largest_chunk_idx = 0;
				}
				return;
			}

			if(((uintptr_t)((char *)prev- *size_of_header ) == (uintptr_t)((char *)prev->start_addr + prev->size))
				&& ((uintptr_t)((char *)prev->start_addr - sizeof(struct Node)) == (uintptr_t)space)
				&& ((uintptr_t)((char *)prev + sizeof(struct Node)) == (uintptr_t)start_of_memory)) {
				struct Node *new_node = (struct Node *)space;
				new_node->start_addr = (void *)((char *)space + sizeof(struct Node));
				new_node->size = *size_of_header*2 + start_of_memory->size + prev->size + sizeof(struct Node);
				new_node->next = NULL;
				if(prev2 != NULL){
					prev2->next = new_node;
				}
				else{
					freelist_head = new_node;
				}

				iterate_for_smallest();
				iterate_for_largest();
				*current_size_of_heap -= (*size_of_header + sizeof(struct Node));
				return;
			}

			struct Node * new_node = (struct Node *)space;
			new_node->start_addr = (void *)start_of_memory;
			new_node->size = *size_of_header + start_of_memory->size;
			new_node->next = NULL;
			struct Node *last = freelist_head;
			while(last != NULL && last->next != NULL){
				last = last->next;
			}
			last->next = new_node;

			//update smallest and largest chunks
			if(new_node->size > *largest_chunk_size){
				*largest_chunk_size = new_node->size;
				*largest_chunk_idx = *free_idx+1;
			}

			if(new_node->size < *smallest_chunk_size){
				*smallest_chunk_size = new_node->size;
				*smallest_chunk_idx = *free_idx+1;
			}

		}
		else{
			// printf("my_free()[4]: Not enough space in the heap.\n");
			*blocks_allocated += 2; //TODO: check
		}
	}
	return;
}

void my_clean(){
	int ret = munmap(heap, 4096);
	// if(ret == 0){
	// 	//success
	// 	// printf("successfully cleaned!\n");
	// }
	// else if(ret==-1){
	// 	//error: errno is set accordingly
	// 	// printf("error in my_clean\n");
	// }
	return;
}

void my_heapinfo(){

	*temp_idx = 4096- 10*sizeof(int); //MAX size of heap

	// Do not edit below output format
	printf("=== Heap Info ================\n");
	printf("Max Size: %d\n", *temp_idx);
	printf("Current Size: %d\n", *current_size_of_heap);
	printf("Free Memory: %d\n", *temp_idx - *current_size_of_heap);
	printf("Blocks allocated: %d\n", *blocks_allocated);
	printf("Smallest available chunk: %d\n", *smallest_chunk_size);
	printf("Largest available chunk: %d\n", *largest_chunk_size);
	printf("==============================\n");
	// Do not edit above output format
	return;
}