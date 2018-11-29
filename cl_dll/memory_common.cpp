#include "memory.h"

namespace Memory
{
	// Converts HEX string containing pairs of symbols 0-9, A-F, a-f with possible space splitting into byte array
	size_t ConvertHexString(const char *srcHexString, unsigned char *outBuffer, size_t bufferSize)
	{
		unsigned char *in = (unsigned char *)srcHexString;
		unsigned char *out = outBuffer;
		unsigned char *end = outBuffer + bufferSize;
		bool low = false;
		uint8_t byte = 0;
		while (*in && out < end)
		{
			if (*in >= '0' && *in <= '9') { byte |= *in - '0'; }
			else if (*in >= 'A' && *in <= 'F') { byte |= *in - 'A' + 10; }
			else if (*in >= 'a' && *in <= 'f') { byte |= *in - 'a' + 10; }
			else if (*in == ' ') { in++; continue; }

			if (!low)
			{
				byte = byte << 4;
				in++;
				low = true;
				continue;
			}
			low = false;

			*out = byte;
			byte = 0;

			in++;
			out++;
		}
		return out - outBuffer;
	}
	size_t MemoryFindForward(size_t start, size_t end, const unsigned char *pattern, const unsigned char *mask, size_t pattern_len)
	{
		// Ensure start is lower than the end
		if (start > end)
		{
			size_t reverse = end;
			end = start;
			start = reverse;
		}

		unsigned char *cend = (unsigned char*)(end - pattern_len + 1);
		unsigned char *current = (unsigned char*)(start);

		// Just linear search for sequence of bytes from the start till the end minus pattern length
		size_t i;
		if (mask)
		{
			// honoring mask
			while (current < cend)
			{
				for (i = 0; i < pattern_len; i++)
				{
					if ((current[i] & mask[i]) != (pattern[i] & mask[i]))
						break;
				}

				if (i == pattern_len)
					return (size_t)(void*)current;

				current++;
			}
		}
		else
		{
			// without mask
			while (current < cend)
			{
				for (i = 0; i < pattern_len; i++)
				{
					if (current[i] != pattern[i])
						break;
				}

				if (i == pattern_len)
					return (size_t)(void*)current;

				current++;
			}
		}

		return NULL;
	}
	// Signed char versions assume pattern and mask are in HEX string format and perform conversions
	size_t MemoryFindForward(size_t start, size_t end, const char *pattern, const char *mask)
	{
		unsigned char p[MAX_PATTERN];
		unsigned char m[MAX_PATTERN];
		size_t pl = ConvertHexString(pattern, p, sizeof(p));
		size_t ml = mask != NULL ? ConvertHexString(mask, m, sizeof(m)) : 0;
		return MemoryFindForward(start, end, p, mask != NULL ? m : NULL, pl >= ml ? pl : ml);
	}
	size_t MemoryFindBackward(size_t start, size_t end, const unsigned char *pattern, const unsigned char *mask, size_t pattern_len)
	{
		// Ensure start is higher than the end
		if (start < end)
		{
			size_t reverse = end;
			end = start;
			start = reverse;
		}

		unsigned char *cend = (unsigned char*)(end);
		unsigned char *current = (unsigned char*)(start - pattern_len);

		// Just linear search backward for sequence of bytes from the start minus pattern length till the end
		size_t i;
		if (mask)
		{
			// honoring mask
			while (current >= cend)
			{
				for (i = 0; i < pattern_len; i++)
				{
					if ((current[i] & mask[i]) != (pattern[i] & mask[i]))
						break;
				}

				if (i == pattern_len)
					return (size_t)(void*)current;

				current--;
			}
		}
		else
		{
			// without mask
			while (current >= cend)
			{
				for (i = 0; i < pattern_len; i++)
				{
					if (current[i] != pattern[i])
						break;
				}

				if (i == pattern_len)
					return (size_t)(void*)current;

				current--;
			}
		}

		return NULL;
	}
	// Signed char versions assume pattern and mask are in HEX string format and perform conversions
	size_t MemoryFindBackward(size_t start, size_t end, const char *pattern, const char *mask)
	{
		unsigned char p[MAX_PATTERN];
		unsigned char m[MAX_PATTERN];
		size_t pl = ConvertHexString(pattern, p, sizeof(p));
		size_t ml = mask != NULL ? ConvertHexString(mask, m, sizeof(m)) : 0;
		return MemoryFindBackward(start, end, p, mask != NULL ? m : NULL, pl >= ml ? pl : ml);
	}
}