void main(void)
{
	uint64_t prev1 = 1;
	uint64_t prev2 = 0;
	uint64_t curr = 0;


	for (uint64_t i = 0; i <= 10; i = i + 1)
	{
		if (i > 2)
		{
			curr = prev1 + prev2;
			prev2 = prev1;
			prev1 = curr;
		}
	}
}
