uint64_t data = 5;

uint64_t bss;

void main(void)
{
	data = 6;
	uint64_t *ptr = &data;
	bss = *ptr;
}
