void main(void)
{
	uint64_t local = 5;
	uint64_t *ptr = &local + 3;

	local = *(ptr - 3);
}
