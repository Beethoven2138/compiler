void main(void)
{
	uint64_t local = 5;
        uint64_t *ptr;
	uint64_t *ptr1 = &local;
	local = *(ptr + *ptr1);
}
