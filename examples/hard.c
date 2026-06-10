uint64_t *ptr;
uint64_t bss_var;

uint64_t test_func(uint64_t *arg)
{
	return *arg;
}

void main(void)
{
	uint64_t local = 3;
	ptr = &local;
	bss_var = test_func(ptr);
}
