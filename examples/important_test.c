uint64_t arr[5];
uint64_t data = 1;

void main(void);
{
	uint64_t local = 4;
	uint64_t *ptr = &local + 7;
        *(arr+1) = *(ptr-7);
}
