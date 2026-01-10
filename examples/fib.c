void main(void)
{
	uint64_t prev1 = 1;
	uint64_t prev2 = 0;
	uint64_t curr = 0;
	uint64_t count = 0;
	
	while (count <= 10)
	{
		if (count > 2)
		{
			curr = prev1 + prev2;
			prev2 = prev1;
			prev1 = curr;
		}
		count = count + 1;
	}
}
