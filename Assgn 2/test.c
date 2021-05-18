#include "my_alloc.c"

int main(int argc, char *argv[]){
	my_init();
	// printf("heap address: %lu\n", (unsigned long)heap);
	// printf("free list head start addr: %lu\n", (unsigned long)freelist_head->start_addr);
	// char* test1 = my_alloc(8);
	// char* test2 = my_alloc(16);
	// if (test2 == NULL){
	// 	printf("null pointer\n");
	// }else{
	// 	printf("allocated: %lu\n", (unsigned long)test2);
	// }
	// *test2 = 'a';
	// printf("allocated\n");
	// *(test2+1) = 'b';
	// printf("allocated\n");
	// *(test2+2) = 'c';
	// printf("allocated\n");
	// *(test2+3) = 'd';
	// printf("allocated\n");
	// printf("blocks allocated: %d\n", *blocks_allocated);
	// printf("smallest chunk: %d\n", *smallest_chunk_size);
	// printf("largest chunk: %d\n", *largest_chunk_size);

	// my_free(test1);
	// my_free(test2);
	// printf("blocks allocated: %d\n", *blocks_allocated);
	// printf("smallest chunk idx: %d\n", *smallest_chunk_idx);
	// printf("smallest chunk: %d\n", *smallest_chunk_size);
	// printf("largest chunk idx: %d\n", *largest_chunk_idx);
	// printf("largest chunk: %d\n", *largest_chunk_size);
	// // printf("%d\n", sizeof(void *) == sizeof(unsigned long));  // true

	// printf("-------------------------------------------------\n");

	// test 1.
	// int i=0;
	// for(;i<14;i+=2){
	// 	void *ptr1 = my_alloc(512);
	// 	my_heapinfo();
	// 	void *ptr2 = my_alloc(512);
	// 	my_heapinfo();
	// 	my_free(ptr1);
	// 	my_heapinfo();
	// }

	// test 2.
	my_heapinfo();
	char *ptr1 = my_alloc(16);
	my_heapinfo();
	char *ptr2 = my_alloc(32);
	my_heapinfo();
	char *ptr3 = my_alloc(8);
	my_heapinfo();
	char *ptr4 = my_alloc(160);
	my_heapinfo();

	my_free(ptr1);
	my_heapinfo();
	my_free(ptr3);
	my_heapinfo();
	my_free(ptr2); // coalescing 3 consecutive blocks
	my_heapinfo();
	my_free(ptr4);
	my_heapinfo();

	// test 3.
	//testing removal of node to allocate space
	// my_heapinfo();
	// char *ptr1 = my_alloc(4000);
	// my_heapinfo();
	// char *ptr2 = my_alloc(24);
	// printf("%d\n", ptr2 == NULL);
	// my_heapinfo();

	//test 4.
	// fill the heap completely
	// my_heapinfo();
	// char *ptr1 = my_alloc(1000);
	// my_heapinfo();
	// char *ptr2 = my_alloc(1000);
	// my_heapinfo();
	// char *ptr3 = my_alloc(2000);
	// my_heapinfo();
	// char *ptr5 = my_alloc(24);
	// my_heapinfo();
	// my_free(ptr1);
	// my_heapinfo();
	// my_free(ptr3);
	// my_heapinfo();
	// my_free(ptr2);
	// my_heapinfo();
	// char *ptr4 = my_alloc(4016);
	// my_heapinfo();

	//test 5.
	// my_heapinfo();
	// char *ptr1 = my_alloc(1000);
	// my_heapinfo();
	// char *ptr2 = my_alloc(1000);
	// my_heapinfo();
	// char *ptr3 = my_alloc(1976);
	// my_heapinfo();
	// char *ptr4 = my_alloc(8);
	// my_heapinfo();

	//test 6. top down coalescing
	// my_heapinfo();
	// char *ptr1 = my_alloc(1000);
	// my_heapinfo();
	// char *ptr2 = my_alloc(1000);
	// my_heapinfo();
	// char *ptr3 = my_alloc(1000);
	// my_heapinfo();
	// char *ptr4 = my_alloc(1000);
	// my_heapinfo();
	// my_free(ptr1);
	// my_heapinfo();
	// my_free(ptr2);
	// my_heapinfo();
	// my_free(ptr3);
	// my_heapinfo();
	// my_free(ptr4);

	//test 7. bottom up coalescing
	// my_heapinfo();
	// char *ptr1 = my_alloc(1000);
	// my_heapinfo();
	// char *ptr2 = my_alloc(1000);
	// my_heapinfo();
	// char *ptr3 = my_alloc(1000);
	// my_heapinfo();
	// my_free(ptr3);
	// my_heapinfo();
	// my_free(ptr2);
	// my_heapinfo();
	// my_free(ptr1);
	// my_heapinfo();

	//test 8. random free
	// my_heapinfo();
	// char *ptr1 = my_alloc(1000);
	// my_heapinfo();
	// char *ptr2 = my_alloc(1000);
	// my_heapinfo();
	// char *ptr3 = my_alloc(1000);
	// *(ptr3 + 999) = 'h';
	// my_heapinfo();
	// my_free(ptr2);
	// my_heapinfo();
	// my_free(ptr1);
	// my_heapinfo();
	// my_free(ptr3);
	// my_heapinfo();

	//test 9. free space, but not enough memory for Node and Header
	// my_heapinfo();
	// char *ptr1 = my_alloc(3840);
	// my_heapinfo();
	// char *ptr2 = my_alloc(88);
	// my_heapinfo();
	// char *ptr3 = my_alloc(40);
	// my_heapinfo();
	// my_free(ptr1);
	// my_heapinfo();

	//test 10. my_alloc inside my_free
	// my_heapinfo();
	// char *ptr1 = my_alloc(3200);
	// my_heapinfo();
	// char *ptr2 = my_alloc(24);
	// my_heapinfo();
	// char *ptr3 = my_alloc(400);
	// my_heapinfo();
	// char *ptr4 = my_alloc(160);
	// my_heapinfo();
	// my_free(ptr2);
	// my_heapinfo();
	// my_free(ptr4);
	// // my_heapinfo();
	// // my_alloc(24);
	// my_heapinfo();
	// my_free(ptr3);
	// my_heapinfo();

	//test 11. -> case 1
	// my_heapinfo();
	// char* m1 = my_alloc(8);
	// my_heapinfo();
	// char* m2 = my_alloc(16);
	// my_heapinfo();
	// char* m3 = my_alloc(8);
	// my_heapinfo();
	// char* m4 = my_alloc(8);
	// my_heapinfo();
	// char* m5 = my_alloc(8);
	// my_heapinfo();
	// my_free(m3);
	// my_heapinfo();
	// my_free(m4);
	// my_heapinfo();
	// char* m6 = my_alloc(8);
	// my_heapinfo();
	// my_free(m1);
	// my_heapinfo();
	// // my_free(m1);
	// char* mBIG = my_alloc(3864);
	// // printf("%d\n", mBIG == NULL);
	// my_heapinfo();
	// char* mTEST = my_alloc(32);
	// my_heapinfo();
	// // my_free(m1);
	// // my_heapinfo();
	// my_free(m2);
	// my_heapinfo();
	// // my_free(m3);
	// // my_heapinfo();
	// // my_free(m4);
	// printf("DEBUG???? %lu %lu\n", (uintptr_t) m4,(uintptr_t)mTEST);
	// my_heapinfo();
	// my_free(m5);
	// my_heapinfo();
	// printf("before free m6\n");
	// my_free(m6);
	// my_heapinfo();
	// printf("addr of mTEST: %lu\n", (uintptr_t)mTEST);
	// printf("addr of mBIG: %lu\n", (uintptr_t)mBIG);
	// my_free(mBIG);
	// my_heapinfo();
	// my_free(mTEST);
	// my_heapinfo();

	//test 12. -> case 2
	// char *temp = (char *)my_alloc(sizeof(char)*24);
	// char *ttemp = (char *)my_alloc(sizeof(char)*160);
	// char *tttemp = (char *)my_alloc(sizeof(char)*160);
	// // printf("%p %d \n", temp,sizeof(char)*8);
	// my_heapinfo();// All three allocated
	// printf("Ttemp is freed\n");
	// my_free(ttemp);
	// my_heapinfo();// One is freed
	// ttemp = (char *) my_alloc(sizeof(char)*160);// Reallocated
	// printf("TTemp is Reallocated\n");
	// my_heapinfo();
	// my_free(ttemp);
	// my_heapinfo();
	// my_free(temp);
	// my_heapinfo();
	// my_free(tttemp);
	// my_heapinfo();
	// my_clean();
	// printf("%s\n", temp);// Should cause segfault.

	//test 13. random alloc and free
	// my_heapinfo();
	// char *ptr1 = my_alloc(8);
	// my_heapinfo();
	// char *ptr2 = my_alloc(16);
	// my_heapinfo();
	// char *ptr3 = my_alloc(24);
	// my_heapinfo();
	// char *ptr4 = my_alloc(40);
	// my_heapinfo();
	// char *ptr5 = my_alloc(160);
	// my_heapinfo();
	// my_free(ptr3);
	// my_heapinfo();
	// char *ptr6 = my_alloc(2000);
	// my_heapinfo();
	// my_free(ptr1);
	// my_heapinfo();
	// my_free(ptr2);
	// my_heapinfo();
	// my_free(ptr4);
	// my_heapinfo();
	// my_free(ptr5);
	// my_heapinfo();
	// my_free(ptr6);
	// my_heapinfo();  // I should get back all my space

	//test 14. complete allocation
	// my_heapinfo();
	// for(int i=0; i<20; i++){
	// 	char *ptr1 = my_alloc(192);
	// 	my_heapinfo();
	// 	my_alloc(192);
	// 	my_heapinfo();
	// 	my_free(ptr1);
	// 	my_heapinfo();
	// }

	// my_heapinfo();
	// for(int i=0; i<400; i++){
	// 	my_alloc(8);
	// 	my_heapinfo();
	// }

	//test 15.
	// my_heapinfo();
	// my_alloc(4008);
	// my_heapinfo();




	my_clean();
	// uintptr_t
	return 0;
}