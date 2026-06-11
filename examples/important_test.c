uint64_t *arr[5];

void main(void);
{
	uint64_t local = 4;
	uint64_t *ptr = &local + 7;
        *(arr + 2) = *(ptr - 7);
}
