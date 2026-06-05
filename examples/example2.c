uint64_t efunc(uint64_t a)
{
        if (a > 2)
	{
		return 2 * a;
	}
	return a + 3;
}

void main(void)
{
	uint64_t u = 45;
	uint64_t u1 = u * 2;

	if (u == u1 && u == 5)
	{
		uint64_t u4 = u1 - 34;
	}
	else
	{
		u = 4556;
	}
	u1 = efunc(u1);

	while (u != 45)
	{
		u = 45;
	}
}
