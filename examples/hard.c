uint64_t *ptr;
uint64_t data_var = 5;

uint64_t test_func(uint64_t *arg)
{
	return *(arg-2);
}

void main(void)
{
	uint64_t local = 3;
	ptr = &local;
	data_var = test_func(ptr + 2);
}
