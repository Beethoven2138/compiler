uint64_t arr1[10];
uint64_t arr2[5];

uint64_t memcpy(uint64_t *dest, uint64_t *src, uint64_t length)
{
	for (uint64_t i = 0; i < length; i = i + 1)
	{
		*(dest + i) = *(src + i);
	}
	return 0;
}

void main(void)
{
	for (uint64_t i = 0; i < 5; i = i + 1)
	{
		*(arr2 + i) = i * 3;
	}
	uint64_t ret = memcpy(arr1, arr2, 5);
}
