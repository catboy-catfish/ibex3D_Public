#pragma once

// ----------------------------------------------------------------------------------------------------

template <typename t>
static bool getNthBit(t number, unsigned char n)
{
	return ((number >> n) & 1);
}