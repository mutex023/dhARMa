/* Common utility routines
*
* @author: muteX023
*/

#include "utils.h"
#include "types.h"

void memcpy(void *dest, void *src, u32 size)
{
	u8 *end = NULL;
	u8 *dest8 = dest;
	u8 *src8 = src;

	end = dest8;
	end += size;
	
	while (dest8 < end) {
		*dest8 = *src8;
		++dest8;
		++src8;
	}
	
}

void memset(void *dest, u8 val, u32 size)
{
	u8 *end = NULL;
	u8 *dest8 = dest;

	end = dest8;
	end += size;
	
	while (dest8 < end) {
		*dest8 = val;
		++dest8;
	}
}

